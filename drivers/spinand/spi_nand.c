#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <driver/spinand/spinand.h>
#include <lib/mmio.h>
#include <timer.h>
#include <platform.h>
#include <framework/common.h>
#include <framework/module.h>
#include <riscv_cache.h>
#include <memmap.h>
#include <string.h>

static unsigned long spi_nand_ctrl_base;

#define NR_PG_PER_NATIVE_BLK    (spinand_info.pages_per_block)
#define NAND_OOB_SIZE			 (spinand_info.spare_size)
#define SPI_NAND_PLANE_BIT_OFFSET	(1U << 12)

#define SPI_NAND_INTERRUPT_UDELAY_TIME  10
#define SPI_NAND_INTERRUPT_UDELAY_COUNT (100000000 / SPI_NAND_INTERRUPT_UDELAY_TIME) // wait up to 10 second

#define BASE_SYSDMAC		0x5090020000
#define TOP_DMA_CH_REMAP0	(TOP_BASE+0x154)
#define DMA_SPI_NAND		0x3e
#define DMA_REMAP_CH0_OFFSET	0x0
#define DMA_REMAP_UPDATE_OFFSET	31

static struct spi_nand_info_t spinand_info;
static uint8_t ecc_en_status = 0xff;

/*
 * Minimal and safe setting for booting
 * We might overwrite setting from spi nand vec in order for better performance
 */

#define ROUND_UP_DIV(a, b) (((a) + (b)-1) / (b))

static uint8_t spi_nand_get_feature(uint32_t fe);
static int spi_nand_set_feature(uint8_t fe, uint8_t val);
static uint8_t spi_nand_device_reset(void);
//static void spi_nand_enable_ecc(void);
static void spi_nand_polling_oip(void);
static uint32_t spi_nand_parsing_ecc_info(uint32_t);
static int spi_nand_read_page_by_row_addr(uint32_t row_addr, void *buf,
					  uint32_t len);
static int spi_nand_read_from_cache(uint32_t blk_id, uint32_t page_nr,
				    uint32_t col_addr, uint32_t len, void *buf);

static void spi_nand_polling_oip(void)
{
	uint32_t status = 0xff;
	uint32_t retry = 0;

	while ((status & SPI_NAND_STATUS0_OIP) && (retry++ < (SPI_NAND_INTERRUPT_UDELAY_COUNT * 10))) {
		status = spi_nand_get_feature(spinand_info.ecc_status_offset);
		udelay(1);
	}

	if (retry > (SPI_NAND_INTERRUPT_UDELAY_COUNT * 10)) {
		pr_err("SPINAND is still busy over 10 sec after reseting spinand %d %s\n", retry, __func__);
		//assert(0);
	}
}

uint8_t spi_nand_query_ecc_en_status(void)
{
	uint8_t status;

	status = spi_nand_get_feature(SPI_NAND_FEATURE_FEATURE0);

	if (status & (1 << 4)) {
		ecc_en_status = 1;
		return 1;
	} else {
		ecc_en_status = 0;
		return 0;
	}
}

static void spi_nand_ctrl_ecc(uint8_t enable)
{
	uint8_t status;

	if ((ecc_en_status == 1) && (enable == 1)) {
		pr_info("NAND_DEBUG: ecc is already enabled\n");
		return;
	} else if ((ecc_en_status == 0) && (enable == 0)) {
		pr_info("NAND_DEBUG: ecc is already disabled\n");
		return;
	}

	status = spi_nand_get_feature(spinand_info.ecc_en_feature_offset);

	if (enable) {
		spi_nand_set_feature(spinand_info.ecc_en_feature_offset,
			status | spinand_info.ecc_en_mask); // set ECC_EN to enable
		ecc_en_status = 1;
	} else {
		spi_nand_set_feature(spinand_info.ecc_en_feature_offset,
			status & (~spinand_info.ecc_en_mask)); // set ECC_EN to disable
		ecc_en_status = 0;
	}
}

uint32_t spi_nand_read_oob(uint32_t row_addr, void *buf)
{
	uint32_t col_addr = spinand_info.page_size;
	uint32_t blk_id = row_addr >> spinand_info.pages_per_block_shift;

	spi_nand_ctrl_ecc(0);
	spi_nand_read_page_by_row_addr(row_addr, buf, spinand_info.spare_size);


	if ((spinand_info.flags & FLAGS_SET_PLANE_BIT) && (blk_id % 2 == 1))
		col_addr |= SPI_NAND_PLANE_BIT_OFFSET;

	spi_nand_read_from_cache(0, 0, col_addr, spinand_info.spare_size, buf);

	spi_nand_ctrl_ecc(1);

	return 0;
}


static void spi_nand_setup_intr(void)
{
	mmio_setbits_32(spi_nand_ctrl_base + REG_SPI_NAND_INT_EN, 3);
	mmio_setbits_32(spi_nand_ctrl_base + REG_SPI_NAND_INT_CLR, BITS_SPI_NAND_INT_CLR_ALL);
	mmio_setbits_32(spi_nand_ctrl_base + REG_SPI_NAND_INT_MASK, 0x00000f00);
}

static int spi_nand_set_feature(uint8_t fe, uint8_t val)
{
	uint32_t fe_set = fe | (val << 8);
	uint32_t retry = 0;

	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL2, 2 << TRX_CMD_CONT_SIZE_SHIFT);
	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL3, BIT_REG_TRX_RW);
	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CMD0,
		(fe_set << TRX_CMD_CONT0_SHIFT) | SPI_NAND_CMD_SET_FEATURE);

	mmio_setbits_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL0, BIT_REG_TRX_START);

	while(((mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_INT) & BIT_REG_TRX_DONE_INT) == 0)
		&& (retry++ < SPI_NAND_INTERRUPT_UDELAY_COUNT))
		udelay(1);

	if (retry > SPI_NAND_INTERRUPT_UDELAY_COUNT) {
		pr_err("SPINAND command error, no interrupt, %d %s\n", retry, __func__);
		//assert(0);
	}

	// clr trx done intr
	mmio_setbits_32(spi_nand_ctrl_base + REG_SPI_NAND_INT_CLR, BIT_REG_TRX_DONE_INT);

	return 0;
}

static uint8_t spi_nand_get_feature(uint32_t fe)
{
	uint32_t val = 0;
	uint32_t retry = 0;

	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL2,
		1 << TRX_DATA_SIZE_SHIFT | 1 << TRX_CMD_CONT_SIZE_SHIFT);

	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL3, BIT_REG_RSP_CHK_EN);

	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CMD0, fe << 8 | SPI_NAND_CMD_GET_FEATURE);

	spi_nand_setup_intr();

	// Trigger communication start
	mmio_setbits_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL0, BIT_REG_TRX_START);

	while(((mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_INT) & BIT_REG_TRX_DONE_INT) == 0)
		&& (retry++ < SPI_NAND_INTERRUPT_UDELAY_COUNT))
		udelay(1);

	if (retry > SPI_NAND_INTERRUPT_UDELAY_COUNT) {
		pr_err("SPINAND command error, no interrupt, %d %s\n", retry, __func__);
		//assert(0);
	}

	val = mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_RX_DATA);

	uint32_t intr = mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_INT);
	mmio_setbits_32(spi_nand_ctrl_base + REG_SPI_NAND_INT_CLR, intr);

	return (val & 0xff);
}

static uint8_t spi_nand_device_reset(void)
{
	uint32_t retry = 0;

	pr_info("NAND_RS\n");

	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL2, 0); // 3 bytes for 24-bit row address
	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL3, 0x0);
	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CMD0, SPI_NAND_CMD_RESET);

	spi_nand_setup_intr();

	mmio_setbits_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL0, BIT_REG_TRX_START);

	while(((mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_INT) & BIT_REG_TRX_DONE_INT) == 0)
		&& (retry++ < SPI_NAND_INTERRUPT_UDELAY_COUNT))
		udelay(1);

	if (retry > SPI_NAND_INTERRUPT_UDELAY_COUNT) {
		pr_err("SPINAND command error, no interrupt, %d %s\n", retry, __func__);
		//assert(0);
	}

	uint32_t intr = mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_INT);

	mmio_setbits_32(spi_nand_ctrl_base + REG_SPI_NAND_INT_CLR, intr); // clr intr

	mdelay(1); //The OIP status can be read from 300ns after the reset command is sent

	return 0;
}

/*
 * spi_nand user guide v0.3.docx
 * 5.4 : Write Command with DMA Data
 *
 * DW_axi_dmac_databook（sysdma）.pdf
 *
 *
 * rw : 0 for read, 1 for write
*/
void spi_nand_rw_dma_setup(void *buf, uint32_t len, uint32_t rw)
{
	uint32_t ch = 0;
	pr_info("%s: buf %p, len %d\n", __func__, buf, len);

	mmio_write_32(BASE_SYSDMAC + 0x010, 0x00000003);
	mmio_write_32(BASE_SYSDMAC + 0x018, 0x00000f00);

	mmio_write_32(BASE_SYSDMAC + 0x110 + ch * (0x100), (len / 4) - 1);
	mmio_write_32(BASE_SYSDMAC + 0x11C + ch * (0x100), 0x000f0792);

	//buf = (void *)phys_to_dma((uintptr_t) buf);

	if (rw) {
		// for dma write
		mmio_write_64(BASE_SYSDMAC + 0x100 + ch * (0x100), (uint64_t) buf);
		mmio_write_64(BASE_SYSDMAC + 0x108 + ch * (0x100), spi_nand_ctrl_base + 0x800);
		mmio_write_32(BASE_SYSDMAC + 0x118 + ch * (0x100), 0x00045441);

		// [0:2] = 1 : MEM_TO_PER_DMAC, PER dst = 0
		mmio_write_32(BASE_SYSDMAC + 0x124 + ch * (0x100), 0x00000001);
		mmio_write_32(TOP_DMA_CH_REMAP0, DMA_SPI_NAND); //PER dst = 0 => remap ch0
		mmio_setbits_32(TOP_DMA_CH_REMAP0, (1U << 31)); // set sdma remap update bit

		pr_info("0x100: 0x%lx\n", mmio_read_64(BASE_SYSDMAC + 0x100 + ch * (0x100)));
		pr_info("0x108: 0x%lx\n", mmio_read_64(BASE_SYSDMAC + 0x108 + ch * (0x100)));
	} else {
		mmio_write_64(BASE_SYSDMAC + 0x100 + ch * (0x100), spi_nand_ctrl_base + 0xC00);
		mmio_write_64(BASE_SYSDMAC + 0x108 + ch * (0x100), (uint64_t)buf);
		mmio_write_32(BASE_SYSDMAC + 0x118 + ch * (0x100), 0x00046214);

		// [0:2] = 2 : PER_TO_MEM_DMAC, PER src = 0
		mmio_write_32(BASE_SYSDMAC + 0x124 + ch * (0x100), 0x00000002);
		mmio_clrsetbits_32(TOP_DMA_CH_REMAP0, 0x3f << DMA_REMAP_CH0_OFFSET,
				   DMA_SPI_NAND << DMA_REMAP_CH0_OFFSET);
		mmio_clrsetbits_32(TOP_DMA_CH_REMAP0, 0x1 << DMA_REMAP_UPDATE_OFFSET, 1 << DMA_REMAP_UPDATE_OFFSET);

		pr_info("0x100: 0x%lx\n", mmio_read_64(BASE_SYSDMAC + 0x100 + ch * (0x100)));
		pr_info("0x108: 0x%lx\n", mmio_read_64(BASE_SYSDMAC + 0x108 + ch * (0x100)));
	}

	mmio_write_32(BASE_SYSDMAC + 0x120 + ch * (0x100), 0x0);
	mmio_write_32(BASE_SYSDMAC + 0x018, 0x00000f0f);
}

void  spi_nand_set_qe(uint32_t enable)
{
	if (enable) {
		uint32_t val = 0;
		val = spi_nand_get_feature(spinand_info.ecc_en_feature_offset);
		val |= SPI_NAND_FEATURE0_QE;
		spi_nand_set_feature(spinand_info.ecc_en_feature_offset, val);
	} else {
		uint32_t val = 0;
		val = spi_nand_get_feature(spinand_info.ecc_en_feature_offset);

		val &= ~SPI_NAND_FEATURE0_QE;
		spi_nand_set_feature(spinand_info.ecc_en_feature_offset, val);
	}
}

static void spi_nand_set_read_from_cache_mode(uint32_t col_addr)
{

	if (spinand_info.flags & FLAGS_ENABLE_X4_BIT) {
		pr_info("NAND_DEBUG: Read by 4 bit mode\n");
		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL3,
			BIT_REG_TRX_DMA_EN | SPI_NAND_CTRL3_IO_TYPE_X4_MODE | BIT_REG_TRX_DUMMY_HIZ);
		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CMD0,
			col_addr << TRX_CMD_CONT0_SHIFT | SPI_NAND_CMD_READ_FROM_CACHEX4);
	} else if (spinand_info.flags & FLAGS_ENABLE_X2_BIT) {
		pr_info("NAND_DEBUG: Read by 2 bit mode\n");
		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL3,
			BIT_REG_TRX_DMA_EN | SPI_NAND_CTRL3_IO_TYPE_X2_MODE | BIT_REG_TRX_DUMMY_HIZ);
		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CMD0,
			col_addr << TRX_CMD_CONT0_SHIFT | SPI_NAND_CMD_READ_FROM_CACHEX2);
	} else {
		pr_info("NAND_DEBUG: Read by 1 bit mode\n");
		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL3,
			BIT_REG_TRX_DMA_EN | BIT_REG_TRX_DUMMY_HIZ);
		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CMD0,
			col_addr << TRX_CMD_CONT0_SHIFT | SPI_NAND_CMD_READ_FROM_CACHE);
	}


	return;
}

static int spi_nand_read_page_by_row_addr(uint32_t row_addr, void *buf, uint32_t len)
{
	uint32_t retry = 0;

	uint32_t r_row_addr = ((row_addr & 0xff0000) >> 16) | (row_addr & 0xff00) | ((row_addr & 0xff) << 16);
	pr_info("%s r_row_addr 0x%x\n", __func__, r_row_addr);

	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL2,
		0 << TRX_DATA_SIZE_SHIFT | 3 << TRX_CMD_CONT_SIZE_SHIFT); // 0x8

	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL3, 0);
	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CMD0,
		r_row_addr << TRX_CMD_CONT0_SHIFT | SPI_NAND_CMD_PAGE_READ_TO_CACHE);

	// Trigger communication start
	mmio_setbits_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL0, BIT_REG_TRX_START);

	while(((mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_INT) & BIT_REG_TRX_DONE_INT) == 0)
		&& (retry++ < SPI_NAND_INTERRUPT_UDELAY_COUNT))
		udelay(1);

	if (retry > SPI_NAND_INTERRUPT_UDELAY_COUNT) {
		pr_err("SPINAND command error, no interrupt, %d %s\n", retry, __func__);
		//assert(0);
	}

	mmio_setbits_32(spi_nand_ctrl_base + REG_SPI_NAND_INT_CLR, BIT_REG_TRX_DONE_INT);

	spi_nand_polling_oip();

	return 0;
}

static int spi_nand_read_from_cache(uint32_t blk_id, uint32_t page_nr, uint32_t col_addr,
				    uint32_t len, void *buf)
{
	uint32_t r_col_addr = ((col_addr & 0xff00) >> 8) | ((col_addr & 0xff) << 8);

	pr_info("%s col_addr 0x%x, r_col_addr 0x%x\n", __func__, col_addr, r_col_addr);

	spi_nand_rw_dma_setup(buf, len, 0);

	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL2,
		len << TRX_DATA_SIZE_SHIFT | 3 << TRX_CMD_CONT_SIZE_SHIFT);

	spi_nand_set_read_from_cache_mode(r_col_addr);

	mmio_setbits_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL0, BIT_REG_TRX_START);

	while ((mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_INT) & BITS_REG_TRX_DMA_DONE_INT) == 0)
		; // should we wait?

	uint32_t intr = mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_INT);
	mmio_setbits_32(spi_nand_ctrl_base + REG_SPI_NAND_INT_CLR, intr);

	return 0;
}

uint16_t spi_nand_read_id(void)
{
	uint32_t read_id = 0;
	uint32_t retry = 0;

	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL2, 0x00020001);
	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL3, 0);
	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CMD0, SPI_NAND_CMD_READ_ID);

	spi_nand_setup_intr();

	// Trigger communication start
	mmio_setbits_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL0, BIT_REG_TRX_START);

	while(((mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_INT) & BIT_REG_TRX_DONE_INT) == 0)
		&& (retry++ < SPI_NAND_INTERRUPT_UDELAY_COUNT))
		udelay(1);

	if (retry > SPI_NAND_INTERRUPT_UDELAY_COUNT) {
		pr_err("SPINAND command error, no interrupt, %d %s\n", retry, __func__);
		//assert(0);
	}

	read_id = mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_RX_DATA);

	pr_info("MID = 0x%x, DID = 0x%x\n", read_id & 0xff, (read_id >> 8) & 0xff);

	mmio_setbits_32(spi_nand_ctrl_base + REG_SPI_NAND_INT_CLR, BIT_REG_TRX_DONE_INT);

	return (read_id & 0xffff);
}

void spi_nand_dump_reg(void)
{
	for (uint32_t i = 0; i < 0x80; i += 4) {
		uint32_t reg = mmio_read_32(spi_nand_ctrl_base + i);
		pr_info("0x%x: 0x%x\n", i, reg);
	}
}

void spi_nand_dump_dma_reg(void)
{
	for (uint32_t i = 0; i < 0x34; i += 4) {
		uint32_t reg = mmio_read_32(0x04330000 + i);

		pr_info("0x%x: 0x%x\n", i, reg);
	}

	for (uint32_t i = 0x100; i < 0x134; i += 4) {
		uint32_t reg = mmio_read_32(0x04330000 + i);

		pr_info("0x%x: 0x%x\n", i, reg);
	}

	return;
}

/*
 * Register Addr. 7        6        5      4      3        2        1        0
 * Status C0H     Reserved Reserved ECCS1  ECCS0  P_FAIL   E_FAIL   WEL      OIP
 * Status F0H     Reserved Reserved ECCSE1 ECCSE0 Reserved Reserved Reserved Reserved

 * ECCS1 ECCS0 ECCSE1 ECCSE0 Description
 *  0     0     x      x     No bit errors were detected during the previous read algorithm.
 *  0     1     0      0     Bit errors(<4) were detected and corrected.
 *  0     1     0      1     Bit errors(=5) were detected and corrected.
 *  0     1     1      0     Bit errors(=6) were detected and corrected.
 *  0     1     1      1     Bit errors(=7) were detected and corrected.
 *  1     0     x      x     Bit errors greater than ECC capability(8 bits) and not corrected
 *  1     1     x      x     Bit errors reach ECC capability( 8 bits) and corrected
 */

#define ECC_NOERR  0
#define ECC_CORR   1
#define ECC_UNCORR 2

uint32_t spi_nand_parsing_ecc_info(uint32_t page)
{
	uint32_t statusc0 = spi_nand_get_feature(spinand_info.ecc_status_offset);
	uint8_t ecc_sts = (statusc0 & spinand_info.ecc_status_mask) >> spinand_info.ecc_status_shift;

	if (ecc_sts) {
		pr_info("statusc0 0x%x\n", statusc0);
		pr_info("block %d, page %d, ecc sts : %d!!\n",
		       page >> spinand_info.pages_per_block_shift, page % spinand_info.pages_per_block, ecc_sts);
	}

	if (ecc_sts == spinand_info.ecc_status_uncorr_val) {
		pr_info("ECC UNCORR\n");
		return ECC_UNCORR;
	}

	return ECC_NOERR;
}

uint32_t spi_nand_read_page(uint32_t row_addr, void *buf, uint32_t len)
{
	uint32_t col_addr = 0;
	uint32_t blk_id = row_addr >> spinand_info.pages_per_block_shift;

	pr_info("%s, row_addr 0x%x, buf %p, len %d, spinand_info.flags 0x%x\n", __func__, row_addr, buf, len,
	     spinand_info.flags);

	spi_nand_ctrl_ecc(1);

	spi_nand_read_page_by_row_addr(row_addr, buf, len);

	if ((spinand_info.flags & FLAGS_SET_PLANE_BIT) && (blk_id & 1))
		col_addr |= SPI_NAND_PLANE_BIT_OFFSET;

	spi_nand_read_from_cache(0, 0, col_addr, len, buf);

	return spi_nand_parsing_ecc_info(row_addr);
}


static void dump_nand_info(void)
{
	pr_info("NAND_DEBUG: NAND version=0x%x, id=0x%x\n", spinand_info.version, spinand_info.id);
	pr_info("NAND_DEBUG: page size=%d, block size=%d, page per block=%d, badblock_pos=%d\n",
		spinand_info.page_size, spinand_info.block_size,
		spinand_info.pages_per_block, spinand_info.badblock_pos);
	pr_info("NAND_DEBUG: fip block count=%d, page per block shift=%d, flags=0x%x\n", spinand_info.fip_block_cnt,
		spinand_info.pages_per_block_shift, spinand_info.flags);
	pr_info("NAND_DEBUG: ECC feature offset=0x%x, en mask=0x%x, status offset=0x%x, status mask=0x%x\n",
		spinand_info.ecc_en_feature_offset, spinand_info.ecc_en_mask,
		spinand_info.ecc_status_offset, spinand_info.ecc_status_mask);
	pr_info("NAND_DEBUG: ECC status shift=%d, uncorr_val=0x%x, erase count=%d\n", spinand_info.ecc_status_shift,
		spinand_info.ecc_status_uncorr_val, spinand_info.erase_count);
	pr_info("NAND_DEBUG: sck_l=%d, sck_h=0x%x, max_freq=%d\n", spinand_info.sck_l,
		spinand_info.sck_h, spinand_info.max_freq);
	pr_info("NAND_DEBUG: xtal_switch=%d\n",	spinand_info.xtal_switch);
}

void spi_nand_set_default_info(void)
{
	spinand_info.id = 0x0;
	spinand_info.page_size = 2048;
	spinand_info.spare_size = 64;
	spinand_info.block_size = 0x20000;
	spinand_info.pages_per_block = 64;
	spinand_info.fip_block_cnt = 1024;
	spinand_info.pages_per_block_shift = 6;
	spinand_info.flags = 0;
	spinand_info.badblock_pos = BBP_FIRST_PAGE;
	spinand_info.ecc_en_feature_offset = SPI_NAND_FEATURE_FEATURE0;
	spinand_info.ecc_en_mask = SPI_NAND_FEATURE0_ECC_EN;
	spinand_info.ecc_status_offset = SPI_NAND_FEATURE_STATUS0;
	spinand_info.ecc_status_mask = 0x30;
	spinand_info.ecc_status_shift = 4;
	spinand_info.ecc_status_uncorr_val = 0x2;
	spinand_info.erase_count = 1;
	spinand_info.sck_l = 1;
	spinand_info.sck_h = 1;
	spinand_info.max_freq = 0;
	spinand_info.sample_param = 0x100;
	spinand_info.xtal_switch = 0;
}

void spi_nand_set_device(struct spi_nand_info_t *info)
{
	memcpy((void *)&spinand_info, (void *)info, sizeof(struct spi_nand_info_t));

	dump_nand_info();
}

struct spi_nand_info_t *spi_nand_get_nand_info(void)
{
	return &spinand_info;
}

void spi_nand_downgrade_read_mode(void)
{
	spinand_info.flags &= ~(FLAGS_ENABLE_X4_BIT | FLAGS_ENABLE_X2_BIT | FLAGS_SET_QE_BIT);
}

void spi_nand_downgrade_freq(void)
{
	spinand_info.max_freq = 0; /* use XTAL, downgrade to 6.25Mhz */
}


#define SPI_NAND_XTAL_SWITCH_REG 0x281001A4
#define SPI_NAND_CLK_FROM_PLL	0xFFFFBFFF
#define SPI_NAND_CLK_FROM_XTAL	(1U << 14)
#define SPI_NAND_FREQ_XTAL	0 /* 6.25 Mhz */
#define SPI_NAND_FREQ_23MHz	1 /* 23.4375 Mhz */
#define SPI_NAND_FREQ_26MHz	2 /* 26.7857 Mhz */
#define SPI_NAND_FREQ_31MHz	3 /* 31.25 Mhz */
#define SPI_NAND_FREQ_37MHz	4 /* 37.5 Mhz */
#define SPI_NAND_FREQ_46MHz	5 /* 46.875 Mhz */
#define SPI_NAND_FREQ_62MHz	6 /* 62.5 Mhz */
#define SPI_NAND_FREQ_93MHz	7 /* 93.75 Mhz */

void spi_nand_adjust_freq(uint8_t sck_l, uint8_t sck_h, uint16_t max_freq, uint8_t xtal_switch)
{
	uint32_t val;

	switch (xtal_switch) {
	case 1: /* use fpll 187.5 Mhz*/
		val = mmio_read_32(SPI_NAND_XTAL_SWITCH_REG);
		mmio_write_32(SPI_NAND_XTAL_SWITCH_REG, val & SPI_NAND_CLK_FROM_PLL);
		break;
	default:
	case 0:
		val = mmio_read_32(SPI_NAND_XTAL_SWITCH_REG);
		mmio_write_32(SPI_NAND_XTAL_SWITCH_REG, val | SPI_NAND_CLK_FROM_XTAL);
		break;
	}

	switch (max_freq) {
	case SPI_NAND_FREQ_XTAL: /* use XTAL, 25/4 = 6.25Mhz */
		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_BOOT_CTRL,
			(mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_BOOT_CTRL) | BIT_REG_BOOT_PRD));

		val = mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1) & ~SPI_NAND_SCK_MASK;

		val |= SPI_NAND_SET_SCK_L(1) | SPI_NAND_SET_SCK_H(0);

		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1, val);
		break;
	case SPI_NAND_FREQ_23MHz: /* use FPLL, 187.5/8 = 23.4375Mhz */
		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_BOOT_CTRL,
			(mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_BOOT_CTRL) & ~BIT_REG_BOOT_PRD));

		val = mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1) & ~SPI_NAND_SCK_MASK;

		if ((sck_l + sck_h) != 6)
			val |= SPI_NAND_SET_SCK_L(3) | SPI_NAND_SET_SCK_H(3);
		else
			val |= SPI_NAND_SET_SCK_L(sck_l) | SPI_NAND_SET_SCK_H(sck_h);

		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1, val);
		break;
	case SPI_NAND_FREQ_26MHz: /* use FPLL, 187.5/7 = 26.7857Mhz */
		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_BOOT_CTRL,
			(mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_BOOT_CTRL) & ~BIT_REG_BOOT_PRD));

		val = mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1) & ~SPI_NAND_SCK_MASK;

		if ((sck_l + sck_h) != 5)
			val |= SPI_NAND_SET_SCK_L(3) | SPI_NAND_SET_SCK_H(2);
		else
			val |= SPI_NAND_SET_SCK_L(sck_l) | SPI_NAND_SET_SCK_H(sck_h);

		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1, val);
		break;
	case SPI_NAND_FREQ_31MHz: /* use FPLL, 187.5/6 = 31.25Mhz */
		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_BOOT_CTRL,
			(mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_BOOT_CTRL) & ~BIT_REG_BOOT_PRD));

		val = mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1) & ~SPI_NAND_SCK_MASK;

		if ((sck_l + sck_h) != 4)
			val |= SPI_NAND_SET_SCK_L(2) | SPI_NAND_SET_SCK_H(2);
		else
			val |= SPI_NAND_SET_SCK_L(sck_l) | SPI_NAND_SET_SCK_H(sck_h);

		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1, val);
		break;
	case SPI_NAND_FREQ_37MHz: /* use FPLL, 187.5/5 = 37.5Mhz */
		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_BOOT_CTRL,
			(mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_BOOT_CTRL) & ~BIT_REG_BOOT_PRD));

		val = mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1) & ~SPI_NAND_SCK_MASK;

		if ((sck_l + sck_h) != 3)
			val |= SPI_NAND_SET_SCK_L(2) | SPI_NAND_SET_SCK_H(1);
		else
			val |= SPI_NAND_SET_SCK_L(sck_l) | SPI_NAND_SET_SCK_H(sck_h);

		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1, val);
		break;
	case SPI_NAND_FREQ_46MHz: /* use FPLL, 187.5/4 = 46.875Mhz */
		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_BOOT_CTRL,
			(mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_BOOT_CTRL) & ~BIT_REG_BOOT_PRD));

		val = mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1) & ~SPI_NAND_SCK_MASK;

		if ((sck_l + sck_h) != 2)
			val |= SPI_NAND_SET_SCK_L(1) | SPI_NAND_SET_SCK_H(1);
		else
			val |= SPI_NAND_SET_SCK_L(sck_l) | SPI_NAND_SET_SCK_H(sck_h);

		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1, val);
		break;
	case SPI_NAND_FREQ_62MHz: /* use FPLL, 187.5/3 = 62.5Mhz */
		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_BOOT_CTRL,
			(mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_BOOT_CTRL) & ~BIT_REG_BOOT_PRD));

		val = mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1) & ~SPI_NAND_SCK_MASK;

		if ((sck_l + sck_h) != 1)
			val |= SPI_NAND_SET_SCK_L(1) | SPI_NAND_SET_SCK_H(0);
		else
			val |= SPI_NAND_SET_SCK_L(sck_l) | SPI_NAND_SET_SCK_H(sck_h);

		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1, val);
		break;
	case SPI_NAND_FREQ_93MHz: /* use FPLL, 187.5/2 = 93.75Mhz */
		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_BOOT_CTRL,
			(mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_BOOT_CTRL) & ~BIT_REG_BOOT_PRD));

		val = mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1) & ~SPI_NAND_SCK_MASK;

		val |= SPI_NAND_SET_SCK_L(0) | SPI_NAND_SET_SCK_H(0);

		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1, val);
		break;
	default:
		break;
	}

	if (spinand_info.sample_param != 0x0) {
		uint32_t val;

		val = mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_BOOT_CTRL) & ~SPI_NAND_BOOT_SAMPLE_MASK;
		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_BOOT_CTRL, (val | spinand_info.sample_param));
	}
}

void spi_nand_set_freq(struct spi_nand_info_t nand_info)
{
	spi_nand_adjust_freq(nand_info.sck_l, nand_info.sck_h, nand_info.max_freq, nand_info.xtal_switch);
}

void spi_nand_current_freq(void)
{
	pr_info("NAND_DEBUG: current boot ctrl(0x24)=0x%x, trx ctrl=0x%x\n",
		mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_BOOT_CTRL),
		mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1));
}

uint32_t spi_nand_reset(void)
{
	spi_nand_device_reset();
	spi_nand_polling_oip();

	return 0;
}

void dump_hex(char *desc, void *addr, int len)
{
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char *)addr;

	/* Output description if given. */
	if (desc != NULL)
		printf("%s:\n", desc);

	/* Process every byte in the data. */
	for (i = 0; i < len; i++) {
		/* Multiple of 16 means new line (with line offset). */
		if ((i % 16) == 0) {
			/* Just don't print ASCII for the zeroth line. */
			if (i != 0)
				printf("  %s\n", buff);

			/* Output the offset. */
			printf("  %04x ", i);
		}

		/* Now the hex code for the specific character. */
		printf(" %02x", pc[i]);

		/* And store a printable ASCII character for later. */
		if ((pc[i] < 0x20) || (pc[i] > 0x7e))
			buff[i % 16] = '.';
		else
			buff[i % 16] = pc[i];
		buff[(i % 16) + 1] = '\0';
	}

	/* Pad out last line if not exactly 16 characters. */
	while ((i % 16) != 0) {
		printf("   ");
		i++;
	}

	/* And print the final ASCII bit. */
	printf("  %s\n", buff);
}

int load_from_spinand(uint32_t offset, uint8_t* buf, uint32_t len)
{
	int page_idx, pg_idx, blk_id;
	uint32_t row_addr;
	uint8_t src[2048] = {0};
	uint8_t* dst = buf;
	uint32_t resize = len;
	uint32_t read_len = 0;

	int pg_size =  spinand_info.page_size;
	int pg_per_bk = spinand_info.pages_per_block;

	page_idx = offset / (pg_size << 1);

	while(resize) {
		blk_id = page_idx / pg_per_bk;
		pg_idx = page_idx % pg_per_bk;

		pr_info("read from blk:%d, page:%d\n", blk_id, pg_idx);

		if (resize > pg_size)
			read_len = pg_size;
		else
			read_len = resize;

		row_addr = blk_id << spinand_info.pages_per_block_shift | pg_idx;

		int ret = spi_nand_read_page(row_addr, (void *)src, pg_size);

		pr_info("read error:%d\n", ret);
		pr_info("dst 0x%p\n", dst);

		memcpy(dst, src, read_len);

		dst += read_len;
		pr_info("read_len %d\n", read_len);
		pr_info("resize %d\n", resize);
		resize -= read_len;
		page_idx++;
	}

	return 0;
}

static void spi_nand_write_dis(void)
{
	uint32_t retry = 0;
	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL2, 0x0);
	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL3, 0x0);
	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CMD0, SPI_NAND_CMD_WRDI);

	mmio_setbits_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL0, BIT_REG_TRX_START);
	while(((mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_INT) & BIT_REG_TRX_DONE_INT) == 0)
		&& (retry++ < SPI_NAND_INTERRUPT_UDELAY_COUNT))
		udelay(1);
}

static void spi_nand_write_en(void)
{
	uint32_t retry = 0;
	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL2, 0x0);
	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL3, 0x0);
	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CMD0, SPI_NAND_CMD_WREN);

	mmio_setbits_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL0, BIT_REG_TRX_START);
	while(((mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_INT) & BIT_REG_TRX_DONE_INT) == 0)
		&& (retry++ < SPI_NAND_INTERRUPT_UDELAY_COUNT))
		udelay(1);
}

static uint8_t spi_nand_block_erase(uint32_t row_addr)
{
	uint32_t retry = 0;
	uint32_t r_row_addr = ((row_addr & 0xff0000) >> 16) | (row_addr & 0xff00) | ((row_addr & 0xff) << 16);

	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL2,
		      3 << TRX_CMD_CONT_SIZE_SHIFT); // 3 bytes for 24-bit row address
	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL3, 0x0);
	mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CMD0,
		      (r_row_addr << TRX_CMD_CONT0_SHIFT) | SPI_NAND_CMD_BLOCK_ERASE);

	mmio_setbits_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL0, BIT_REG_TRX_START);

	while(((mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_INT) & BIT_REG_TRX_DONE_INT) == 0)
		&& (retry++ < SPI_NAND_INTERRUPT_UDELAY_COUNT))
		udelay(1);

	return 0;
}

uint32_t bm_spi_nand_erase_block(uint32_t blk_id)
{
	uint32_t row_addr = blk_id * NR_PG_PER_NATIVE_BLK;

	pr_info("%s erase blk id %d, row_addr 0x%08x, spinand_info->flags %d\n", __func__, blk_id, row_addr,
	       spinand_info.flags);

	spi_nand_set_feature(SPI_NAND_FEATURE_PROTECTION, FEATURE_PROTECTION_NONE);

	spi_nand_write_en();

	spi_nand_block_erase(row_addr);

	spi_nand_polling_oip();

	spi_nand_write_dis();

	return 0;
}

int spi_nand_init(void)
{
	uint32_t val = 0;
	spi_nand_ctrl_base = SPINAND_BASE;

	spi_nand_set_default_info();

	spi_nand_setup_intr();

	spi_nand_reset();

	mmio_clrbits_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1, BIT_REG_IO_CPHA);
	mmio_clrbits_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1, BIT_REG_IO_CPOL);
	if ((spinand_info.sck_h >= 0 && spinand_info.sck_h < 0x10) &&
	    (spinand_info.sck_l >= 0 && spinand_info.sck_l < 0x10)) {
		val = mmio_read_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1);
		val &= (~0xff0000);
		val |= (spinand_info.sck_h << 16);
		val |= (spinand_info.sck_l << 20);
		mmio_write_32(spi_nand_ctrl_base + REG_SPI_NAND_TRX_CTRL1, val);
	}

	spi_nand_read_id();

	return 0;
}
