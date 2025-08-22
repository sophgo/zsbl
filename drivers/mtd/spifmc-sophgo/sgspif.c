#include <assert.h>
#include <stddef.h>
#include <errno.h>
#include <lib/mmio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <timer.h>
#include <common/common.h>

#include <driver/mtd.h>
#include <driver/platform.h>
#include <common/module.h>

#define SPI_CMD_WREN		0x06
#define SPI_CMD_WRDI		0x04
#define SPI_CMD_RDID		0x9F
#define SPI_CMD_RDSR		0x05
#define SPI_CMD_WRSR		0x01
#define SPI_CMD_READ		0x03
#define SPI_CMD_FAST_READ	0x0B
#define SPI_CMD_PP		0x02
#define SPI_CMD_SE		0x20
#define SPI_CMD_BE		0xD8
#define SPI_CMD_CE		0xC7

#define SPI_STATUS_WIP		(0x01 << 0)
#define SPI_STATUS_WEL		(0x01 << 1)
#define SPI_STATUS_BP0		(0x01 << 2)
#define SPI_STATUS_BP1		(0x01 << 3)
#define SPI_STATUS_BP2		(0x01 << 4)
#define SPI_STATUS_SRWD		(0x01 << 7)

#define SPI_FLASH_BLOCK_SIZE		256
#define SPI_TRAN_CSR_ADDR_BYTES_SHIFT	8
#define SPI_MAX_FIFO_DEPTH		8

/* register definitions */
#define REG_SPI_CTRL		0x000
#define REG_SPI_CE_CTRL		0x004
#define REG_SPI_DLY_CTRL	0x008
#define REG_SPI_DMMR		0x00C
#define REG_SPI_TRAN_CSR	0x010
#define REG_SPI_TRAN_NUM	0x014
#define REG_SPI_FIFO_PORT	0x018
#define REG_SPI_FIFO_PT		0x020
#define REG_SPI_INT_STS		0x028
#define REG_SPI_INT_EN		0x02C

/* bit definition */
#define BIT_SPI_CTRL_CPHA		(0x01 << 12)
#define BIT_SPI_CTRL_CPOL		(0x01 << 13)
#define BIT_SPI_CTRL_HOLD_OL		(0x01 << 14)
#define BIT_SPI_CTRL_WP_OL		(0x01 << 15)
#define BIT_SPI_CTRL_LSBF		(0x01 << 20)
#define BIT_SPI_CTRL_SRST		(0x01 << 21)
#define BIT_SPI_CTRL_SCK_DIV_SHIFT	0
#define BIT_SPI_CTRL_FRAME_LEN_SHIFT	16

#define BIT_SPI_CE_CTRL_CEMANUAL	(0x01 << 0)
#define BIT_SPI_CE_CTRL_CEMANUAL_EN	(0x01 << 1)

#define BIT_SPI_CTRL_FM_INTVL_SHIFT	0
#define BIT_SPI_CTRL_CET_SHIFT		8

#define BIT_SPI_DMMR_EN			(0x01 << 0)

#define BIT_SPI_TRAN_CSR_TRAN_MODE_RX		(0x01 << 0)
#define BIT_SPI_TRAN_CSR_TRAN_MODE_TX		(0x01 << 1)
#define BIT_SPI_TRAN_CSR_CNTNS_READ		(0x01 << 2)
#define BIT_SPI_TRAN_CSR_FAST_MODE		(0x01 << 3)
#define BIT_SPI_TRAN_CSR_BUS_WIDTH_1_BIT	(0x0 << 4)
#define BIT_SPI_TRAN_CSR_BUS_WIDTH_2_BIT	(0x01 << 4)
#define BIT_SPI_TRAN_CSR_BUS_WIDTH_4_BIT	(0x02 << 4)
#define BIT_SPI_TRAN_CSR_DMA_EN			(0x01 << 6)
#define BIT_SPI_TRAN_CSR_MISO_LEVEL		(0x01 << 7)
#define BIT_SPI_TRAN_CSR_ADDR_BYTES_NO_ADDR	(0x0 << 8)
#define BIT_SPI_TRAN_CSR_WITH_CMD		(0x01 << 11)
#define BIT_SPI_TRAN_CSR_FIFO_TRG_LVL_1_BYTE	(0x0 << 12)
#define BIT_SPI_TRAN_CSR_FIFO_TRG_LVL_2_BYTE	(0x01 << 12)
#define BIT_SPI_TRAN_CSR_FIFO_TRG_LVL_4_BYTE	(0x02 << 12)
#define BIT_SPI_TRAN_CSR_FIFO_TRG_LVL_8_BYTE	(0x03 << 12)
#define BIT_SPI_TRAN_CSR_GO_BUSY		(0x01 << 15)

#define BIT_SPI_TRAN_CSR_TRAN_MODE_MASK		0x0003
#define BIT_SPI_TRAN_CSR_ADDR_BYTES_MASK	0x0700
#define BIT_SPI_TRAN_CSR_FIFO_TRG_LVL_MASK	0x3000

#define BIT_SPI_INT_TRAN_DONE		(0x01 << 0)
#define BIT_SPI_INT_RD_FIFO		(0x01 << 2)
#define BIT_SPI_INT_WR_FIFO		(0x01 << 3)
#define BIT_SPI_INT_RX_FRAME		(0x01 << 4)
#define BIT_SPI_INT_TX_FRAME		(0x01 << 5)

#define BIT_SPI_INT_TRAN_DONE_EN	(0x01 << 0)
#define BIT_SPI_INT_RD_FIFO_EN		(0x01 << 2)
#define BIT_SPI_INT_WR_FIFO_EN		(0x01 << 3)
#define BIT_SPI_INT_RX_FRAME_EN		(0x01 << 4)
#define BIT_SPI_INT_TX_FRAME_EN		(0x01 << 5)

#define SPI_ID_M25P128		0x00182020
#define SPI_ID_N25Q128		0x0018ba20
#define SPI_ID_GD25LQ128	0x001860c8

static size_t spi_flash_read_blocks(unsigned long spi_base, int lba, uintptr_t buf, size_t size);
static size_t spi_flash_write_blocks(int lba, const uintptr_t buf, size_t size);
static void bm_spi_init(unsigned long base);
static int bm_spi_flash_program(unsigned long spi_base, uint8_t *src_buf, uint32_t base, uint32_t size);
static int bm_spi_flash_read(unsigned long spi_base, uint8_t *dst_buf, uint32_t addr, uint32_t size);
static int spi_data_read(unsigned long spi_base, uint8_t *dst_buf, int addr, int size);
static uint32_t bm_spi_read_id(unsigned long spi_base);
static void bm_spi_flash_read_sector(unsigned long spi_base, uint32_t addr, uint8_t *buf);
static int bm_spi_flash_erase_sector(unsigned long spi_base, uint32_t addr);
static int bm_spi_flash_program_sector(unsigned long spi_base, uint32_t addr);

struct sgspif {
	unsigned long reg;
	struct platform_device *pdev;
	struct mtd *mdev;
};

static int spi_data_in_tran(unsigned long spi_base, uint8_t *dst_buf, uint8_t *cmd_buf,
			    int with_cmd, int addr_bytes, int data_bytes)
{
	uint32_t *p_data = (uint32_t *)cmd_buf;
	uint32_t tran_csr = 0;
	int cmd_bytes = addr_bytes + ((with_cmd) ? 1 : 0);
	int i, xfer_size, off;
	uint64_t start;

	/* 64Kb = 8KB */
	if (data_bytes > 65536) {
		pr_debug("SPI data in overflow, should be less than 65536 bytes(%d)\n", data_bytes);
		return -1;
	}

	/* init tran_csr */
	tran_csr = mmio_read_32(spi_base + REG_SPI_TRAN_CSR);
	tran_csr &= ~(BIT_SPI_TRAN_CSR_TRAN_MODE_MASK |
			BIT_SPI_TRAN_CSR_ADDR_BYTES_MASK |
			BIT_SPI_TRAN_CSR_FIFO_TRG_LVL_MASK |
			BIT_SPI_TRAN_CSR_WITH_CMD);
	tran_csr |= (addr_bytes << SPI_TRAN_CSR_ADDR_BYTES_SHIFT);
	tran_csr |= (with_cmd) ? BIT_SPI_TRAN_CSR_WITH_CMD : 0;
	tran_csr |= BIT_SPI_TRAN_CSR_FIFO_TRG_LVL_8_BYTE;
	tran_csr |= BIT_SPI_TRAN_CSR_TRAN_MODE_RX;

	mmio_write_32(spi_base + REG_SPI_FIFO_PT, 0); // flush FIFO before filling fifo
	if (with_cmd) {
		for (i = 0; i < ((cmd_bytes - 1) / 4 + 1); i++)
			mmio_write_32(spi_base + REG_SPI_FIFO_PORT, p_data[i]);
	}

	/* issue tran */
	mmio_write_32(spi_base + REG_SPI_INT_STS, 0); // clear all int
	mmio_write_32(spi_base + REG_SPI_TRAN_NUM, data_bytes);
	tran_csr |= BIT_SPI_TRAN_CSR_GO_BUSY;
	mmio_write_32(spi_base + REG_SPI_TRAN_CSR, tran_csr);

	start = timer_get_tick();
	/* check rd int to make sure data out done and in data started */
	while ((mmio_read_32(spi_base + REG_SPI_INT_STS) & BIT_SPI_INT_RD_FIFO) == 0) {
		if (timer_tick2ms(timer_get_tick() - start) > 100)
			return -1;
	}

	/* get data */
	p_data = (uint32_t *)dst_buf;
	off = 0;
	while (off < data_bytes) {
		if (data_bytes - off >= SPI_MAX_FIFO_DEPTH)
			xfer_size = SPI_MAX_FIFO_DEPTH;
		else
			xfer_size = data_bytes - off;

		while ((mmio_read_32(spi_base + REG_SPI_FIFO_PT) & 0xF) != xfer_size)
			;
		for (i = 0; i < ((xfer_size - 1) / 4 + 1); i++)
			p_data[off / 4 + i] = mmio_read_32(spi_base + REG_SPI_FIFO_PORT);
		off += xfer_size;
	}

	/* wait tran done */
	while ((mmio_read_32(spi_base + REG_SPI_INT_STS) & BIT_SPI_INT_TRAN_DONE) == 0)
		;
	mmio_write_32(spi_base + REG_SPI_FIFO_PT, 0);    //should flush FIFO after tran
	return 0;
}

static int spi_data_read(unsigned long spi_base, uint8_t *dst_buf, int addr, int size)
{
	uint8_t cmd_buf[5];
	int err = 0;

	if (addr & ((uint32_t)0xff << 24)) {
		cmd_buf[0] = 0x13;
		cmd_buf[1] = ((addr) >> 24) & 0xff;
		cmd_buf[2] = ((addr) >> 16) & 0xFF;
		cmd_buf[3] = ((addr) >> 8) & 0xFF;
		cmd_buf[4] = (addr) & 0xFF;
		err = spi_data_in_tran(spi_base, dst_buf, cmd_buf, 1, 4, size);
	} else {
		cmd_buf[0] = SPI_CMD_READ;
		cmd_buf[1] = ((addr) >> 16) & 0xFF;
		cmd_buf[2] = ((addr) >> 8) & 0xFF;
		cmd_buf[3] = (addr) & 0xFF;
		err = spi_data_in_tran(spi_base, dst_buf, cmd_buf, 1, 3, size);
	}

	return err;
}

static void __attribute__((unused)) bm_spi_flash_read_sector(unsigned long spi_base, uint32_t addr, uint8_t *buf)
{
	spi_data_read(spi_base, buf, (int)addr, 256);
}

static size_t __attribute__((unused)) spi_flash_read_blocks(unsigned long spi_base,
		int lba, uintptr_t buf, size_t size)
{
	spi_data_read(spi_base, (uint8_t *)buf, lba * SPI_FLASH_BLOCK_SIZE, size);
	return size;
}

static size_t __attribute__((unused)) spi_flash_write_blocks(int lba, const uintptr_t buf, size_t size)
{
	pr_debug("SPI flash writing is not supported\n");
	return -ENODEV;
}

static void bm_spi_init(unsigned long spi_base)
{
	uint32_t tran_csr = 0;
	uint32_t tmp;

	/* disable DMMR (direct memory mapping read) */
	pr_debug("spif init disable DMMR\n");
	mmio_write_32(spi_base + REG_SPI_DMMR, 0);
	/* soft reset and set SCK frequency = HCLK frequency/(2(sckdiv+1))*/
	tmp = mmio_read_32(spi_base + REG_SPI_CTRL);
	tmp |= BIT_SPI_CTRL_SRST;
	tmp &= ~((1 << 11) - 1);
	tmp |= 3;
	mmio_write_32(spi_base + REG_SPI_CTRL, tmp);
	/* sent 4 byte each time */
	tran_csr |= (0x03 << SPI_TRAN_CSR_ADDR_BYTES_SHIFT);
	/* itr and DMA req when >= 4byte */
	tran_csr |= BIT_SPI_TRAN_CSR_FIFO_TRG_LVL_4_BYTE;
	/* with cmd */
	tran_csr |= BIT_SPI_TRAN_CSR_WITH_CMD;
	mmio_write_32(spi_base + REG_SPI_TRAN_CSR, tran_csr);
}

/* here are APIs for SPI flash programming */

static uint8_t spi_non_data_tran(unsigned long spi_base, uint8_t *cmd_buf,
				 uint32_t with_cmd, uint32_t addr_bytes)
{
	uint32_t *p_data = (uint32_t *)cmd_buf;
	uint32_t tran_csr = 0;

	if (addr_bytes > 3) {
		pr_debug("non-data: addr bytes should be less than 3 (%d)\n", addr_bytes);
		return -1;
	}

	/* init tran_csr */
	tran_csr = mmio_read_32(spi_base + REG_SPI_TRAN_CSR);
	tran_csr &= ~(BIT_SPI_TRAN_CSR_TRAN_MODE_MASK |
			BIT_SPI_TRAN_CSR_ADDR_BYTES_MASK |
			BIT_SPI_TRAN_CSR_FIFO_TRG_LVL_MASK |
			BIT_SPI_TRAN_CSR_WITH_CMD);
	tran_csr |= (addr_bytes << SPI_TRAN_CSR_ADDR_BYTES_SHIFT);
	tran_csr |= BIT_SPI_TRAN_CSR_FIFO_TRG_LVL_1_BYTE;
	tran_csr |= (with_cmd ? BIT_SPI_TRAN_CSR_WITH_CMD : 0);

	mmio_write_32(spi_base + REG_SPI_FIFO_PT, 0); //do flush FIFO before filling fifo

	mmio_write_32(spi_base + REG_SPI_FIFO_PORT, p_data[0]);

	/* issue tran */
	mmio_write_32(spi_base + REG_SPI_INT_STS, 0); //clear all int
	tran_csr |= BIT_SPI_TRAN_CSR_GO_BUSY;
	mmio_write_32(spi_base + REG_SPI_TRAN_CSR, tran_csr);

	/* wait tran done */
	while ((mmio_read_32(spi_base + REG_SPI_INT_STS) & BIT_SPI_INT_TRAN_DONE) == 0)
		;
	mmio_write_32(spi_base + REG_SPI_FIFO_PT, 0); //should flush FIFO after tran
	return 0;
}

static uint8_t spi_data_out_tran(unsigned long spi_base, uint8_t *src_buf, uint8_t *cmd_buf,
				 uint32_t with_cmd, uint32_t addr_bytes, uint32_t data_bytes)
{
	uint32_t *p_data = (uint32_t *)cmd_buf;
	uint32_t tran_csr = 0;
	uint32_t cmd_bytes = addr_bytes + (with_cmd ? 1 : 0);
	uint32_t xfer_size, off;
	uint32_t wait = 0;
	int i;

	if (data_bytes > 65535) {
		pr_debug("data out overflow, should be less than 65535 bytes(%d)\n", data_bytes);
		return -1;
	}

	/* init tran_csr */
	tran_csr = mmio_read_32(spi_base + REG_SPI_TRAN_CSR);
	tran_csr &= ~(BIT_SPI_TRAN_CSR_TRAN_MODE_MASK |
			BIT_SPI_TRAN_CSR_ADDR_BYTES_MASK |
			BIT_SPI_TRAN_CSR_FIFO_TRG_LVL_MASK |
			BIT_SPI_TRAN_CSR_WITH_CMD);
	tran_csr |= (addr_bytes << SPI_TRAN_CSR_ADDR_BYTES_SHIFT);
	tran_csr |= (with_cmd ? BIT_SPI_TRAN_CSR_WITH_CMD : 0);
	tran_csr |= BIT_SPI_TRAN_CSR_FIFO_TRG_LVL_8_BYTE;
	tran_csr |= BIT_SPI_TRAN_CSR_TRAN_MODE_TX;

	mmio_write_32(spi_base + REG_SPI_FIFO_PT, 0); //do flush FIFO before filling fifo
	if (with_cmd)
		for (i = 0; i < ((cmd_bytes - 1) / 4 + 1); i++)
			mmio_write_32(spi_base + REG_SPI_FIFO_PORT, p_data[i]);

	/* issue tran */
	mmio_write_32(spi_base + REG_SPI_INT_STS, 0); //clear all int
	mmio_write_32(spi_base + REG_SPI_TRAN_NUM, data_bytes);
	tran_csr |= BIT_SPI_TRAN_CSR_GO_BUSY;
	mmio_write_32(spi_base + REG_SPI_TRAN_CSR, tran_csr);
	while ((mmio_read_32(spi_base + REG_SPI_FIFO_PT) & 0xF) != 0)
		; //wait for cmd issued

	/* fill data */
	p_data = (uint32_t *)src_buf;
	off = 0;
	while (off < data_bytes) {
		if (data_bytes - off >= SPI_MAX_FIFO_DEPTH)
			xfer_size = SPI_MAX_FIFO_DEPTH;
		else
			xfer_size = data_bytes - off;

		wait = 0;
		while ((mmio_read_32(spi_base + REG_SPI_FIFO_PT) & 0xF) != 0) {
			wait++;
			udelay(10);
			if (wait > 30000) { // 300ms
				pr_debug("wait to write FIFO timeout\n");
				return -1;
			}
		}

		for (i = 0; i < ((xfer_size - 1) / 4 + 1); i++)
			mmio_write_32(spi_base + REG_SPI_FIFO_PORT, p_data[off / 4 + i]);

		off += xfer_size;
	}

	/* wait tran done */
	while ((mmio_read_32(spi_base + REG_SPI_INT_STS) & BIT_SPI_INT_TRAN_DONE) == 0)
		;
	mmio_write_32(spi_base + REG_SPI_FIFO_PT, 0); //should flush FIFO after tran
	return 0;
}

/*
 * spi_in_out_tran is a workaround function for current 32-bit access to spic fifo:
 * AHB bus could only do 32-bit access to spic fifo, so cmd without 3-bytes addr will leave 3-byte
 * data in fifo, so set tx to mark that these 3-bytes data would be sent out.
 * So send_bytes should be 3 (write 1 dw into fifo) or 7(write 2 dw), get_bytes sould be the same value.
 * software would mask out useless data in get_bytes.
 */
static uint8_t spi_in_out_tran(unsigned long spi_base, uint8_t *dst_buf, uint8_t *src_buf,
			       uint32_t with_cmd, uint32_t addr_bytes, uint32_t send_bytes, uint32_t get_bytes)
{
	uint32_t *p_data = (uint32_t *)src_buf;
	uint32_t total_out_bytes;
	uint32_t tran_csr = 0;
	int i;

	if (send_bytes != get_bytes) {
		pr_debug("data in&out: get_bytes should be the same as send_bytes\n");
		return -1;
	}

	if ((send_bytes > SPI_MAX_FIFO_DEPTH) || (get_bytes > SPI_MAX_FIFO_DEPTH)) {
		pr_debug("data in&out: FIFO will overflow\n");
		return -1;
	}

	/* init tran_csr,include FastMode/AddrBN/Cmd/FFTrgLvl */
	tran_csr = mmio_read_32(spi_base + REG_SPI_TRAN_CSR);
	tran_csr &= ~(BIT_SPI_TRAN_CSR_TRAN_MODE_MASK |
			BIT_SPI_TRAN_CSR_ADDR_BYTES_MASK |
			BIT_SPI_TRAN_CSR_FIFO_TRG_LVL_MASK |
			BIT_SPI_TRAN_CSR_WITH_CMD);
	tran_csr |= (addr_bytes << SPI_TRAN_CSR_ADDR_BYTES_SHIFT);
	tran_csr |= BIT_SPI_TRAN_CSR_FIFO_TRG_LVL_1_BYTE;
	tran_csr |= BIT_SPI_TRAN_CSR_WITH_CMD;
	tran_csr |= BIT_SPI_TRAN_CSR_TRAN_MODE_TX;
	tran_csr |= BIT_SPI_TRAN_CSR_TRAN_MODE_RX;

	mmio_write_32(spi_base + REG_SPI_FIFO_PT, 0); //do flush FIFO before filling fifo
	total_out_bytes = addr_bytes + send_bytes + (with_cmd ? 1 : 0);
	for (i = 0; i < ((total_out_bytes - 1) / 4 + 1); i++)
		mmio_write_32(spi_base + REG_SPI_FIFO_PORT, p_data[i]);

	/* issue tran */
	mmio_write_32(spi_base + REG_SPI_INT_STS, 0); //clear all int
	mmio_write_32(spi_base + REG_SPI_TRAN_NUM, get_bytes);

	tran_csr |= BIT_SPI_TRAN_CSR_GO_BUSY;
	mmio_write_32(spi_base + REG_SPI_TRAN_CSR, tran_csr);

	/* wait tran done and get data */
	while ((mmio_read_32(spi_base + REG_SPI_INT_STS) & BIT_SPI_INT_TRAN_DONE) == 0)
		;
	p_data = (uint32_t *)dst_buf;
	for (i = 0; i < ((get_bytes - 1) / 4 + 1); i++)
		p_data[i] = mmio_read_32(spi_base + REG_SPI_FIFO_PORT);

	mmio_write_32(spi_base + REG_SPI_FIFO_PT, 0); //should flush FIFO after tran
	return 0;
}

static uint8_t spi_write_en(unsigned long spi_base)
{
	uint8_t cmd_buf[4];

	memset(cmd_buf, 0, sizeof(cmd_buf));
	cmd_buf[0] = SPI_CMD_WREN;
	spi_non_data_tran(spi_base, cmd_buf, 1, 0);
	return 0;
}

static uint8_t spi_read_status(unsigned long spi_base)
{
	uint8_t cmd_buf[4];
	uint8_t data_buf[4];

	memset(cmd_buf, 0, sizeof(cmd_buf));
	memset(data_buf, 0, sizeof(data_buf));
	cmd_buf[0] = SPI_CMD_RDSR;
	spi_in_out_tran(spi_base, data_buf, cmd_buf, 1, 0, 3, 3);

	pr_debug("got status %x %x %x %x\n", data_buf[3], data_buf[2], data_buf[1], data_buf[0]);
	return data_buf[0];
}

static uint32_t bm_spi_read_id(unsigned long spi_base)
{
	uint8_t cmd_buf[4];
	uint8_t data_buf[4];
	uint32_t read_id = 0;

	memset(cmd_buf, 0, sizeof(cmd_buf));
	memset(data_buf, 0, sizeof(data_buf));
	cmd_buf[0] = SPI_CMD_RDID;
	spi_in_out_tran(spi_base, data_buf, cmd_buf, 1, 0, 3, 3);
	read_id = (data_buf[2] << 16) | (data_buf[1] << 8) | (data_buf[0]);

	return read_id;
}

static uint8_t spi_page_program(unsigned long spi_base, uint8_t *src_buf, uint32_t addr, uint32_t size)
{
	uint8_t cmd_buf[4];

	memset(cmd_buf, 0, sizeof(cmd_buf));
	cmd_buf[0] = SPI_CMD_PP;
	cmd_buf[1] = (addr >> 16) & 0xFF;
	cmd_buf[2] = (addr >> 8) & 0xFF;
	cmd_buf[3] = addr & 0xFF;

	spi_data_out_tran(spi_base, src_buf, cmd_buf, 1, 3, size);
	return 0;
}

static void spi_sector_erase(unsigned long spi_base, uint32_t addr)
{
	uint8_t cmd_buf[4];

	memset(cmd_buf, 0, sizeof(cmd_buf));
	cmd_buf[0] = SPI_CMD_SE;
	cmd_buf[1] = (addr >> 16) & 0xFF;
	cmd_buf[2] = (addr >> 8) & 0xFF;
	cmd_buf[3] = addr & 0xFF;

	spi_non_data_tran(spi_base, cmd_buf, 1, 3);
}

static int bm_spi_flash_erase_sector(unsigned long spi_base, uint32_t addr)
{
	uint32_t spi_status, wait = 0;

	pr_debug("disable DMMR\n");
	mmio_write_32(spi_base + REG_SPI_DMMR, 0);

	spi_write_en(spi_base);
	spi_status = spi_read_status(spi_base);
	if ((spi_status & SPI_STATUS_WEL) == 0) {
		pr_debug("write en failed, get status: 0x%x\n", spi_status);
		return -1;
	}

	spi_sector_erase(spi_base, addr);

	while (1) {
		mdelay(100);
		spi_status = spi_read_status(spi_base);
		if (((spi_status & SPI_STATUS_WIP) == 0) || (wait > 30)) { // 3s, spec 0.15~1s
			pr_debug("sector erase done, get status: 0x%x, wait: %d\n", spi_status, wait);
			break;
		}
		wait++;
		pr_debug("device busy, get status: 0x%x\n", spi_status);
	}
	pr_debug("enable DMMR\n");
	mmio_write_32(spi_base + REG_SPI_DMMR, 1);
	return 0;
}

static int __attribute__((unused)) bm_spi_flash_program_sector(unsigned long spi_base, uint32_t addr)
{
	uint8_t cmd_buf[256], spi_status;
	uint32_t wait = 0;

	pr_debug("disable DMMR\n");
	mmio_write_32(spi_base + REG_SPI_DMMR, 0);

	memset(cmd_buf, 0x5d, sizeof(cmd_buf));
	pr_debug("write 0x%x\n", cmd_buf[0]);
	spi_write_en(spi_base);

	spi_status = spi_read_status(spi_base);
	if (spi_status != 0x02) {
		pr_debug("spi status check failed, get status: 0x%x\n", spi_status);
		return -1;
	}

	spi_page_program(spi_base, cmd_buf, addr, sizeof(cmd_buf));

	while (1) {
		udelay(100);
		spi_status = spi_read_status(spi_base);
		if (((spi_status & SPI_STATUS_WIP) == 0) || (wait > 600)) { // 60ms, spec 120~2800us
			pr_debug("sector prog done, get status: 0x%x\n", spi_status);
			break;
		}
		wait++;
		pr_debug("device busy, get status: 0x%x\n", spi_status);
	}

	pr_debug("enable DMMR\n");
	mmio_write_32(spi_base + REG_SPI_DMMR, 1);
	return 0;
}

static int do_page_program(unsigned long spi_base, uint8_t *src_buf, uint32_t addr, uint32_t size)
{
	uint8_t spi_status;
	uint32_t wait = 0;

	pr_debug("do page prog @ 0x%x, size: %d\n", addr, size);

	if (size > SPI_FLASH_BLOCK_SIZE) {
		pr_debug("size larger than a page\n");
		return -1;
	}
	if ((addr % SPI_FLASH_BLOCK_SIZE) != 0) {
		pr_debug("addr not alignned to page\n");
		return -1;
	}

	spi_write_en(spi_base);

	spi_status = spi_read_status(spi_base);
	if (spi_status != 0x02) {
		pr_debug("spi status check failed, get status: 0x%x\n", spi_status);
		return -1;
	}

	spi_page_program(spi_base, src_buf, addr, size);

	while (1) {
		udelay(100);
		spi_status = spi_read_status(spi_base);
		if (((spi_status & SPI_STATUS_WIP) == 0) || (wait > 600)) { // 60ms, spec 120~2800us
			pr_debug("page prog done, get status: 0x%x\n", spi_status);
			break;
		}
		wait++;
		pr_debug("device busy, get status: 0x%x\n", spi_status);
	}
	return 0;
}

static int __attribute__((unused)) bm_spi_flash_program(unsigned long spi_base,
		uint8_t *src_buf, uint32_t base, uint32_t size)
{
	uint32_t xfer_size, off, cmp_ret;
	uint8_t cmp_buf[SPI_FLASH_BLOCK_SIZE], erased_sectors, i;
	uint32_t id, sector_size;

	id = bm_spi_read_id(spi_base);
	if (id == SPI_ID_M25P128) {
		sector_size = 256 * 1024;
	} else if (id == SPI_ID_N25Q128 || id == SPI_ID_GD25LQ128) {
		sector_size = 64 * 1024;
	} else {
		pr_debug("unrecognized flash ID 0x%x\n", id);
		return -EINVAL;
	}
	pr_debug("SPI flash ID 0x%x\n", id);

	if ((base % sector_size) != 0) {
		pr_debug("<flash offset addr> is not aligned erase sector size (0x%x)!\n", sector_size);
		return -EINVAL;
	}

	erased_sectors = (size + sector_size) / sector_size;
	pr_debug("Start erasing %d sectors, each %d bytes...\n", erased_sectors, sector_size);

	for (i = 0; i < erased_sectors; i++)
		bm_spi_flash_erase_sector(spi_base, base + i * sector_size);

	off = 0;
	i = 0;
	pr_debug("progress:    ");
	while (off < size) {
		if ((size - off) >= SPI_FLASH_BLOCK_SIZE)
			xfer_size = SPI_FLASH_BLOCK_SIZE;
		else
			xfer_size = size - off;

		if (do_page_program(spi_base, src_buf + off, base + off, xfer_size) != 0) {
			pr_debug("page prog failed @ 0x%x\n", base + off);
			return -1;
		}

		spi_data_read(spi_base, cmp_buf, base + off, xfer_size);
		cmp_ret = memcmp(src_buf + off, cmp_buf, xfer_size);
		if (cmp_ret != 0) {
			pr_debug("memcmp failed\n");
			return cmp_ret;
		}
		pr_debug("page read compare ok @ %d\n", off);
		off += xfer_size;

		pr_debug("\b\b\b%02d%%", off * 100 / size);
	}

	pr_debug("\n--program boot fw success\n");
	return 0;
}

#define TRAN_NUM	65536
static int bm_spi_flash_read(unsigned long spi_base, uint8_t *dst_buf, uint32_t addr, uint32_t size)
{
	int nr_frame, bytes;
	int offset;
	int i, ret;

	nr_frame = size / TRAN_NUM;
	bytes = size % TRAN_NUM;

	for (i = 0; i < nr_frame; i++) {
		offset = i * TRAN_NUM;
		ret = spi_data_read(spi_base, dst_buf + offset,
				    addr + offset, TRAN_NUM);
		if (ret)
			return ret;
	}

	if (bytes) {
		offset = nr_frame * TRAN_NUM;
		ret = spi_data_read(spi_base, dst_buf + offset,
				    addr + offset, bytes);
		if (ret)
			return ret;
	}

	return 0;
}

/* -------------------- mtd framework -------------------- */

static long read(struct mtd *mdev, unsigned long offset, unsigned long size, void *buf)
{
	int err;
	struct sgspif *spif;

	spif = mdev->data;

	err = bm_spi_flash_read((unsigned long)spif->reg, buf, offset, size);

	return err ? -EIO : size;
}

static struct mtdops mtdops = {
	.read = read,
};

static int probe(struct platform_device *pdev)
{
	/* const struct of_device_id *match_id; */
	struct mtd *mdev;
	struct sgspif *spif;

	/* match_id = platform_get_match_id(pdev); */

	bm_spi_init(pdev->reg_base);

	mdev = mtd_alloc();
	if (!mdev)
		return -ENOMEM;

	/* FIXME: block size and total size */
	mdev->block_size = 4096;
	mdev->total_size = 64UL * 1024 * 1024;
	mdev->ops = &mtdops;
	mdev->hwdev = &pdev->device;

	spif = malloc(sizeof(struct sgspif));
	if (!spif)
		return -ENOMEM;

	spif->reg = pdev->reg_base;
	spif->pdev = pdev;
	spif->mdev = mdev;

	mdev->data = spif;

	device_set_child_name(&mdev->device, &pdev->device, "mtd");

	return mtd_register(mdev);
}

static struct of_device_id match_table[] = {
	{
		.compatible = "sophgo,spifmc",
		.data = (void *)NULL,
	},
	{},
};

static struct platform_driver driver = {
	.driver.name = "spifmc",
	.probe = probe,
	.of_match_table = match_table,
};

static int spifmc_init(void)
{
	platform_driver_register(&driver);
	return 0;
}

module_init(spifmc_init);

