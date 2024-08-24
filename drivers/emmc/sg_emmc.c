/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <timer.h>
#include <driver/emmc/emmc.h>
#include <lib/libc/errno.h>
#include <lib/mmio.h>
#include <driver/emmc/sg_emmc.h>
#include <memmap.h>
#include <framework/common.h>
#include <framework/module.h>
#include <string.h>

#define EMMC_INIT_FREQ				(400 * 1000)
#ifdef SUPPORT_SD_EMMC_CLOCK_ADJUSTMENT
#define EMMC_TRAN_FREQ				(emmc_tran_freq)
#else
#define EMMC_TRAN_FREQ				(12 * 1000 * 1000)
#endif

#define DEFAULT_DIV_EMMC_INIT_CLOCK 0x2

static void bm_emmc_hw_init(void);
static void bm_emmc_dev_reset(void);
static int bm_emmc_send_cmd(emmc_cmd_t *cmd);
static int bm_emmc_set_ios(int clk, int width);
static int bm_emmc_prepare(int lba, uintptr_t buf, size_t size);
static int bm_emmc_read(int lba, uintptr_t buf, size_t size);
static int bm_emmc_write(int lba, uintptr_t buf, size_t size);

static const emmc_ops_t bm_emmc_ops = {
	.init           = bm_emmc_hw_init,
	.reset          = bm_emmc_dev_reset,
	.send_cmd       = bm_emmc_send_cmd,
	.set_ios        = bm_emmc_set_ios,
	.prepare        = bm_emmc_prepare,
	.read           = bm_emmc_read,
	.write          = bm_emmc_write,
};

static bm_emmc_params_t bm_params = {
	.reg_base = EMMC_BASE,
	.clk_rate = 50 * 1000 * 1000,
	.bus_width = EMMC_BUS_WIDTH_1,
	.flags = 0
};

#ifdef SUPPORT_SD_EMMC_CLOCK_ADJUSTMENT
const uint32_t emmc_tran_freq = 12 * 1000 * 1000;
#endif

static void bm_emmc_hw_reset(void)
{
	uintptr_t base = EMMC_BASE;

	// set default clk div before reset IP to avoid CLK glitch
	mmio_write_16(base + SDHCI_CLK_CTRL,
		      (mmio_read_16(base + SDHCI_CLK_CTRL) & 0x3F) | DEFAULT_DIV_EMMC_INIT_CLOCK << 8);

	mdelay(1);

	// reset IP
	mmio_write_8(base + SDHCI_SOFTWARE_RESET, 0x7);
	while (mmio_read_8(base + SDHCI_SOFTWARE_RESET))
		;
}

static int bm_emmc_send_cmd_with_data(emmc_cmd_t *cmd)
{
	uintptr_t base;
	uint32_t mode = 0, stat, dma_addr, flags = 0;
	uint32_t emmc_timer = 0;

	base = bm_params.reg_base;

	//make sure cmd line is clear
	while (1) {
		if (!(mmio_read_32(base + SDHCI_PSTATE) & SDHCI_CMD_INHIBIT))
			break;
	}

	switch (cmd->cmd_idx) {
	case EMMC_CMD17:
	case EMMC_CMD18:
		mode = SDHCI_TRNS_DMA | SDHCI_TRNS_BLK_CNT_EN
			| SDHCI_TRNS_MULTI | SDHCI_TRNS_READ;
		break;
	case EMMC_CMD24:
	case EMMC_CMD25:
		mode = (SDHCI_TRNS_DMA | SDHCI_TRNS_BLK_CNT_EN
			| SDHCI_TRNS_MULTI) & ~SDHCI_TRNS_READ;
		break;
	default:
		pr_err("%s Invalid CMD: %d\n", __func__, cmd->cmd_idx);
		return -EINVAL;
	}

	mmio_write_16(base + SDHCI_TRANSFER_MODE, mode);
	mmio_write_32(base + SDHCI_ARGUMENT, cmd->cmd_arg);

	// set cmd flags
	if (cmd->cmd_idx == 0)
		flags |= SDHCI_CMD_RESP_NONE;
	else if (cmd->resp_type & EMMC_RESPONSE_R2)
		flags |= SDHCI_CMD_RESP_LONG;
	else
		flags |= SDHCI_CMD_RESP_SHORT;

	flags |= SDHCI_CMD_CRC;
	flags |= SDHCI_CMD_INDEX;
	flags |= SDHCI_CMD_DATA;

	// issue the cmd
	mmio_write_16(base + SDHCI_COMMAND,
		SDHCI_MAKE_CMD(cmd->cmd_idx, flags));

	// check cmd complete if necessary
	if ((mmio_read_16(base + SDHCI_TRANSFER_MODE)
		& SDHCI_TRNS_RESP_INT) == 0) {
		emmc_timer = get_timer(0);
		while (1) {
			stat = mmio_read_16(base + SDHCI_INT_STATUS);
			if (stat & SDHCI_INT_ERROR) {
				pr_err("%s CMD: %u INT_STAT: 0x%04x INT_ERR: 0x%04x\n", __func__, cmd->cmd_idx,
					mmio_read_16(base + SDHCI_INT_STATUS),
					mmio_read_16(base + SDHCI_ERR_INT_STATUS));
				return -EIO;
			}
			if (stat & SDHCI_INT_CMD_COMPLETE) {
				mmio_write_16(base + SDHCI_INT_STATUS,
					stat | SDHCI_INT_CMD_COMPLETE);
				break;
			}
			if (get_timer(emmc_timer) > EMMC_CMD_TIMEOUT) {
				pr_err("%s send CMD %u timeout!\n", __func__, cmd->cmd_idx);
				return -EIO;
			}
		}

		// get cmd respond
		if (!(flags & SDHCI_CMD_RESP_NONE))
			cmd->resp_data[0] = mmio_read_32(base + SDHCI_RESPONSE_01);
		if (flags & SDHCI_CMD_RESP_LONG) {
			cmd->resp_data[1] = mmio_read_32(base + SDHCI_RESPONSE_23);
			cmd->resp_data[2] = mmio_read_32(base + SDHCI_RESPONSE_45);
			cmd->resp_data[3] = mmio_read_32(base + SDHCI_RESPONSE_67);
		}
	}

	// check dma/transfer complete
	emmc_timer = get_timer(0);
	while (1) {
		stat = mmio_read_16(base + SDHCI_INT_STATUS);
		if (stat & SDHCI_INT_ERROR) {
			pr_err("%s CMD: %u INT_STAT: 0x%04x INT_ERR: 0x%04x\n", __func__, cmd->cmd_idx,
				mmio_read_16(base + SDHCI_INT_STATUS), mmio_read_16(base + SDHCI_ERR_INT_STATUS));
			return -EIO;
		}

		if (stat & SDHCI_INT_XFER_COMPLETE) {
			mmio_write_16(base + SDHCI_INT_STATUS, stat);
			break;
		}

		if (get_timer(emmc_timer) > EMMC_DATA_CMD_TIMEOUT) {
			pr_err("%s DATA_Transfer %u timeout!\n", __func__, cmd->cmd_idx);
			return -EIO;
		}

		if (stat & SDHCI_INT_DMA_END) {
			mmio_write_16(base + SDHCI_INT_STATUS, stat);
			if (mmio_read_16(base + SDHCI_HOST_CONTROL2) & SDHCI_HOST_VER4_ENABLE) {
				dma_addr = mmio_read_32(base + SDHCI_ADMA_SA_LOW);
				mmio_write_32(base + SDHCI_ADMA_SA_LOW, dma_addr);
				mmio_write_32(base + SDHCI_ADMA_SA_HIGH, 0);
			} else {
				dma_addr = mmio_read_32(base + SDHCI_DMA_ADDRESS);
				mmio_write_32(base + SDHCI_DMA_ADDRESS, dma_addr);
			}
		}
	}

	return 0;
}

static int bm_emmc_send_cmd_without_data(emmc_cmd_t *cmd)
{
	uintptr_t base;
	uint32_t stat, flags = 0x0;
	uint32_t emmc_timer = 0;

	base = bm_params.reg_base;

	//make sure cmd line is clear
	while (1) {
		if (!(mmio_read_32(base + SDHCI_PSTATE) & SDHCI_CMD_INHIBIT))
			break;
	}

	// set cmd flags
	if (cmd->cmd_idx == EMMC_CMD0)
		flags |= SDHCI_CMD_RESP_NONE;
	else if (cmd->cmd_idx == EMMC_CMD1)
		flags |= SDHCI_CMD_RESP_SHORT;
	else if (cmd->cmd_idx == EMMC_CMD6)
		flags |= SDHCI_CMD_RESP_SHORT_BUSY;
	else if (cmd->resp_type & EMMC_RESPONSE_R2) {
		flags |= SDHCI_CMD_RESP_LONG;
		flags |= SDHCI_CMD_CRC;
	} else {
		flags |= SDHCI_CMD_RESP_SHORT;
		flags |= SDHCI_CMD_CRC;
		flags |= SDHCI_CMD_INDEX;
	}

	// make sure dat line is clear if necessary
	if (flags & SDHCI_CMD_RESP_SHORT_BUSY) {
		while (1) {
			if (!(mmio_read_32(base + SDHCI_PSTATE) & SDHCI_CMD_INHIBIT_DAT))
				break;
		}
	}

	// issue the cmd
	mmio_write_32(base + SDHCI_ARGUMENT, cmd->cmd_arg);
	mmio_write_16(base + SDHCI_COMMAND,
		SDHCI_MAKE_CMD(cmd->cmd_idx, flags));

	// check cmd complete
	emmc_timer = get_timer(0);
	while (1) {
		stat = mmio_read_16(base + SDHCI_INT_STATUS);
		if (stat & SDHCI_INT_ERROR) {
			pr_err("%s CMD: %u INT_STAT: 0x%04x INT_ERR: 0x%04x\n", __func__, cmd->cmd_idx,
				mmio_read_16(base + SDHCI_INT_STATUS), mmio_read_16(base + SDHCI_ERR_INT_STATUS));
			return -EIO;
		}
		if (stat & SDHCI_INT_CMD_COMPLETE) {
			mmio_write_16(base + SDHCI_INT_STATUS,
				stat | SDHCI_INT_CMD_COMPLETE);
			break;
		}
		if (get_timer(emmc_timer) > EMMC_CMD_TIMEOUT) {
			pr_err("%s send CMD %u timeout!\n", __func__, cmd->cmd_idx);
			return -EIO;
		}
	}

	// get cmd respond
	if (!(flags & SDHCI_CMD_RESP_NONE))
		cmd->resp_data[0] = mmio_read_32(base + SDHCI_RESPONSE_01);
	if (flags & SDHCI_CMD_RESP_LONG) {
		cmd->resp_data[1] = mmio_read_32(base + SDHCI_RESPONSE_23);
		cmd->resp_data[2] = mmio_read_32(base + SDHCI_RESPONSE_45);
		cmd->resp_data[3] = mmio_read_32(base + SDHCI_RESPONSE_67);
	}

	return 0;
}

static int bm_emmc_send_cmd(emmc_cmd_t *cmd)
{
#if 0
	pr_info("send cmd, cmd_idx=%d, cmd_arg=0x%x, resp_type=0x%x\n",
		cmd->cmd_idx,
		cmd->cmd_arg,
		cmd->resp_type);
#endif

	switch (cmd->cmd_idx) {
	case EMMC_CMD17:
	case EMMC_CMD18:
	case EMMC_CMD24:
	case EMMC_CMD25:
		return bm_emmc_send_cmd_with_data(cmd);
	default:
		return bm_emmc_send_cmd_without_data(cmd);
	}
}

static int bm_emmc_set_ext_csd(unsigned int index, unsigned int value)
{
	emmc_cmd_t cmd;

	memset(&cmd, 0, sizeof(emmc_cmd_t));
	cmd.cmd_idx = EMMC_CMD6;
	cmd.cmd_arg = EXTCSD_WRITE_BYTES | EXTCSD_CMD(index) |
		EXTCSD_VALUE(value) | 1;

	return bm_emmc_send_cmd(&cmd);
}

int bm_emmc_set_clk(int clk)
{
	unsigned long base, div;

	if (clk <= 0) {
		pr_err("%s clk: %d is invalid\n", __func__, clk);
		return -EINVAL;
	}

	if (bm_params.clk_rate <= clk) {
		div = 0;
	} else {
		for (div = 0x1; div < 0x3FF; div++) {
			if (bm_params.clk_rate / (2 * div) <= clk)
				break;
		}
	}

	if (div > 0x3FF) {
		pr_err("%s div: 0x%x is invalid\n", __func__, div);
		return -EINVAL;
	}

	base = bm_params.reg_base;
	if (mmio_read_16(base + SDHCI_HOST_CONTROL2) & 1<<15) {
		pr_info("Use SDCLK Preset Value\n");
	} else {
		pr_info("Set SDCLK by driver. (Source, Target, div, Actual) = (%d, %d, %d, %d)\n",
			bm_params.clk_rate, clk, div, bm_params.clk_rate / (2 * div));
		mmio_write_16(base + SDHCI_CLK_CTRL,
			mmio_read_16(base + SDHCI_CLK_CTRL) & ~0x9); // disable INTERNAL_CLK_EN and PLL_ENABLE
		mmio_write_16(base + SDHCI_CLK_CTRL,
			(mmio_read_16(base + SDHCI_CLK_CTRL) & 0x1F) | div << 8); // set clk div
		mmio_write_16(base + SDHCI_CLK_CTRL,
			mmio_read_16(base + SDHCI_CLK_CTRL) | 0x1); // set INTERNAL_CLK_EN
		udelay(150);
		if ((mmio_read_16(base + SDHCI_CLK_CTRL) & 0x2) == 0x0) {
			pr_err("eMMC clock seting FAILED!\n");
			return -EIO;
		}
		mmio_write_16(base + SDHCI_CLK_CTRL,
			mmio_read_16(base + SDHCI_CLK_CTRL) | 0x8); // set PLL_ENABLE
		udelay(150);
		if ((mmio_read_16(base + SDHCI_CLK_CTRL) & 0x2) == 0x0) {
			pr_err("eMMC PLL seting FAILED!\n");
			return -EIO;
		}
	}
	return 0;
}

static void bm_emmc_hw_init(void)
{
	uintptr_t base, vendor_base;

	base = bm_params.reg_base;
	vendor_base = base + (mmio_read_16(base + P_VENDOR_SPECIFIC_AREA) & ((1 << 12) - 1));
	bm_params.vendor_base = vendor_base;

	// reset data & cmd
	mmio_write_8(base + SDHCI_SOFTWARE_RESET, 0x6);

	// init common parameters
	mmio_write_8(base + SDHCI_PWR_CONTROL, SDHCI_BUS_VOL_VDD1_1_8V);
	mmio_write_8(base + SDHCI_TOUT_CTRL, 0xe); // for TMCLK 50Khz
	mmio_setbits_16(base + SDHCI_HOST_CONTROL2, 1 << 11); // set cmd23 support
	mmio_setbits_16(vendor_base + VENDOR_EMMC_CTRL, 0x1);

	// set clk.Toshiba/Scandisk eMMC don't support 400khz.
	bm_emmc_set_clk(EMMC_INIT_FREQ);

	// set host version 4 parameters
	mmio_setbits_16(base + SDHCI_HOST_CONTROL2, 0x1 << 12); // set HOST_VER4_ENABLE
	if (mmio_read_32(base + SDHCI_CAPABILITIES1) & 0x1 << 27) {
		mmio_setbits_16(base + SDHCI_HOST_CONTROL2, 0x1 << 13); // set 64bit addressing
	}

	// set rx src sel to 1
	mmio_clrsetbits_32(base + SDHCI_RX_DELAY_LINE, REG_RX_SRC_SEL_CLR_MASK,
		REG_RX_SRC_SEL_CLK_TX_INV << REG_RX_SRC_SEL_SHIFT);

	// set tx src sel to 1
	mmio_clrsetbits_32(base + SDHCI_TX_DELAY_LINE, REG_TX_SRC_SEL_CLR_MASK,
		REG_TX_SRC_SEL_CLK_TX_INV << REG_TX_SRC_SEL_SHIFT);

	// set PHY_TX_BPS to 1
	mmio_clrsetbits_32(base + SDHCI_PHY_CONFIG, REG_TX_BPS_SEL_CLR_MASK,
		REG_TX_BPS_SEL_BYPASS << REG_TX_BPS_SEL_SHIFT);

	// eMMc card setup
	mmio_clrbits_16(base + SDHCI_HOST_CONTROL2, (0x1 << 8)); // clr UHS2_IF_ENABLE
	mmio_write_8(base + SDHCI_PWR_CONTROL,
		mmio_read_8(base + SDHCI_PWR_CONTROL) | 0x1); // set SD_BUS_PWR_VDD1
	mmio_clrbits_16(base + SDHCI_HOST_CONTROL2, 0x7); // clr UHS_MODE_SEL

	mmio_setbits_16(base + SDHCI_HOST_CONTROL2, (0x1 << 3)); // set 1.8v signal

	mmio_setbits_16(base + SDHCI_CLK_CTRL, 0x1 << 2); // set SD_CLK_EN
	udelay(400); // wait for voltage ramp up time at least 74 cycle,1/200k*80=400us

	//we enable all interrupt status here for testing
	mmio_setbits_16(base + SDHCI_INT_STATUS_EN, 0xFFFF);
	mmio_setbits_16(base + SDHCI_ERR_INT_STATUS_EN, 0xFFFF);

	pr_info("eMMC init done\n");
}

static void bm_emmc_dev_reset(void)
{
	uintptr_t base, vendor_base;

	base = bm_params.reg_base;
	vendor_base = base + (mmio_read_16(base + P_VENDOR_SPECIFIC_AREA) & ((1<<12)-1));

	pr_info("eMMC RST_n 0x%x\n", mmio_read_16(vendor_base + VENDOR_EMMC_CTRL));
	mmio_clrbits_16(vendor_base + VENDOR_EMMC_CTRL, SDHCI_RESET_N);
	pr_info("eMMC RST_n1 n 0x%x\n", mmio_read_16(vendor_base + VENDOR_EMMC_CTRL));
	mdelay(1);
	mmio_setbits_16(vendor_base + VENDOR_EMMC_CTRL, SDHCI_RESET_N);
	pr_info("eMMC RST_n2 n 0x%x\n", mmio_read_16(vendor_base + VENDOR_EMMC_CTRL));
	mdelay(1);
}

static int bm_emmc_set_ios(int clk, int width)
{
	int ret;

	switch (width) {
	case EMMC_BUS_WIDTH_1:
		ret = bm_emmc_set_ext_csd(183, 0);
		if (ret < 0)
			return ret;
		mmio_write_8(bm_params.reg_base + SDHCI_HOST_CONTROL,
			mmio_read_8(bm_params.reg_base + SDHCI_HOST_CONTROL)
			& ~SDHCI_DAT_XFER_WIDTH);
		break;
	case EMMC_BUS_WIDTH_4:
		ret = bm_emmc_set_ext_csd(183, 1);
		if (ret < 0)
			return ret;
		mmio_write_8(bm_params.reg_base + SDHCI_HOST_CONTROL,
			mmio_read_8(bm_params.reg_base + SDHCI_HOST_CONTROL)
			| SDHCI_DAT_XFER_WIDTH);
		break;
	case EMMC_BUS_WIDTH_8:
		ret = bm_emmc_set_ext_csd(183, 2);
		if (ret < 0)
			return ret;
		mmio_write_8(bm_params.reg_base + SDHCI_HOST_CONTROL,
			mmio_read_8(bm_params.reg_base + SDHCI_HOST_CONTROL)
			| SDHCI_EXT_DAT_XFER);
		break;
	default:
		pr_err("%s Invalid Bus width: %d\n", __func__, width);
		return -EINVAL;
	}

	return bm_emmc_set_clk(clk);
}

static int bm_emmc_prepare(int lba, uintptr_t buf, size_t size)
{
	uint64_t load_addr;
	uint64_t base, block_cnt;
	uint8_t tmp;

	//load_addr = phys_to_dma(buf);
	load_addr = buf;
	if (((buf & EMMC_BLOCK_MASK) != 0)
		|| ((size % EMMC_BLOCK_SIZE) != 0))
		return -EINVAL;

	base = bm_params.reg_base;
	block_cnt = size / EMMC_BLOCK_SIZE;

	if (mmio_read_16(base + SDHCI_HOST_CONTROL2) & SDHCI_HOST_VER4_ENABLE) {
		mmio_write_32(base + SDHCI_ADMA_SA_LOW, load_addr);
		mmio_write_32(base + SDHCI_ADMA_SA_HIGH, (load_addr >> 32));
		mmio_write_32(base + SDHCI_DMA_ADDRESS, block_cnt);
		mmio_write_16(base + SDHCI_BLOCK_COUNT, 0);
	} else {
		if ((load_addr >> 32) != 0)
			return -EINVAL;
		mmio_write_32(base + SDHCI_DMA_ADDRESS, load_addr);
		mmio_write_16(base + SDHCI_BLOCK_COUNT, block_cnt);
	}
	mmio_write_16(base + SDHCI_BLOCK_SIZE, SDHCI_MAKE_BLKSZ(7, 512));

	// select SDMA
	tmp = mmio_read_8(base + SDHCI_HOST_CONTROL);
	tmp &= ~SDHCI_CTRL_DMA_MASK;
	tmp |= SDHCI_CTRL_SDMA;
	mmio_write_8(base + SDHCI_HOST_CONTROL, tmp);

	return 0;
}

int bm_emmc_read_data(int offset, uintptr_t buf, size_t size)
{
	int lba = 0;

	if (((offset & EMMC_BLOCK_MASK) != 0) || ((buf & EMMC_BLOCK_MASK) != 0) || ((size & EMMC_BLOCK_MASK) != 0))
		return -EINVAL;

	lba = offset / EMMC_BLOCK_SIZE;

	pr_info("emmc offset %x,lba %d, size %d, dst buf 0x%lx\n", offset, lba, size, buf);

	if (size != emmc_partition_read_blocks(EMMC_PARTITION_BOOT1, lba, buf, size))
		return -EIO;

	return 0;
}

static int bm_emmc_read(int lba, uintptr_t buf, size_t size)
{
	return 0;
}

static int bm_emmc_write(int lba, uintptr_t buf, size_t size)
{
	return 0;
}

int emmc_get_clk(void)
{
	return EMMC_CLOCK;
}

int bm_emmc_init(void)
{
	bm_params.clk_rate = emmc_get_clk();

	bm_emmc_hw_reset();

	if (emmc_init(&bm_emmc_ops, EMMC_TRAN_FREQ, bm_params.bus_width, bm_params.flags) < 0)
		pr_err("eMMC initializing failed\n");

	return 0;
}
