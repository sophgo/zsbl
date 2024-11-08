#include <assert.h>
#include <stdio.h>
#include <framework/common.h>
#include <lib/libc/errno.h>
#include <driver/sd/sd.h>
#include <memmap.h>
#include <lib/mmio.h>
#include <timer.h>
#include <stdint.h>

#define SDCARD_INIT_FREQ	(200 * 1000)
#define SDCARD_TRAN_FREQ	(25 * 1000 * 1000)

void flush_dcache_range(unsigned long start, unsigned long stop);
static void bm_sd_hw_init(void);
static int bm_sd_send_cmd(struct mmc_cmd *cmd);
static int bm_sd_set_ios(unsigned int clk, unsigned int width);
static int bm_sd_prepare(int lba, uintptr_t buf, size_t size);
static int bm_sd_read(int lba, uintptr_t buf, size_t size);
static int bm_sd_write(int lba, uintptr_t buf, size_t size);

static const struct mmc_ops bm_sd_ops = {
	.init		= bm_sd_hw_init,
	.send_cmd	= bm_sd_send_cmd,
	.set_ios	= bm_sd_set_ios,
	.prepare	= bm_sd_prepare,
	.read		= bm_sd_read,
	.write		= bm_sd_write,
};

static struct mmc_device_info sd_info = {
	.mmc_dev_type = MMC_IS_SD_HC,
	.ocr_voltage = 0x00300000, // OCR 3.2~3.3 3.3~3.4
};

static bm_sd_params_t bm_params = {
	.reg_base	= SDIO_BASE,
	.clk_rate	= 50 * 1000 * 1000,
	.bus_width	= MMC_BUS_WIDTH_4,
	.flags		= 0,
	.card_in	= SDCARD_STATUS_UNKNOWN,
};

int bm_get_sd_clock(void)
{
	return 400*1000*1000;
}

static int bm_sd_send_cmd_with_data(struct mmc_cmd *cmd)
{
	uintptr_t base;
	uint32_t mode = 0, stat, dma_addr, flags = 0;
	uint32_t timeout;

	base = bm_params.reg_base;

	//make sure cmd line is clear
	while (1) {
		if (!(mmio_read_32(base + SDHCI_PSTATE) & SDHCI_CMD_INHIBIT))
			break;
	}

	switch (cmd->cmd_idx) {
	case MMC_CMD(17):
	case MMC_CMD(18):
	case MMC_ACMD(51):
		mode = SDHCI_TRNS_BLK_CNT_EN
			   | SDHCI_TRNS_MULTI | SDHCI_TRNS_READ;
		if (!(bm_params.flags & SD_USE_PIO))
			mode |= SDHCI_TRNS_DMA;
		break;
	case MMC_CMD(24):
	case MMC_CMD(25):
		mode = (SDHCI_TRNS_BLK_CNT_EN
			   | SDHCI_TRNS_MULTI) & ~SDHCI_TRNS_READ;
		if (!(bm_params.flags & SD_USE_PIO))
			mode |= SDHCI_TRNS_DMA;
		break;
	default:
		assert(0);
	}

	mmio_write_16(base + SDHCI_TRANSFER_MODE, mode);
	mmio_write_32(base + SDHCI_ARGUMENT, cmd->cmd_arg);

	// set cmd flags
	if (cmd->cmd_idx == MMC_CMD(0))
		flags |= SDHCI_CMD_RESP_NONE;
	else {
		if (cmd->resp_type & MMC_RSP_136)
			flags |= SDHCI_CMD_RESP_LONG;
		else
			flags |= SDHCI_CMD_RESP_SHORT;
		if (cmd->resp_type & MMC_RSP_CRC)
			flags |= SDHCI_CMD_CRC;
		if (cmd->resp_type & MMC_RSP_CMD_IDX)
			flags |= SDHCI_CMD_INDEX;
	}
	flags |= SDHCI_CMD_DATA;
	//verbose("SDHCI flags 0x%x\n", flags);

	// issue the cmd
	mmio_write_16(base + SDHCI_COMMAND, SDHCI_MAKE_CMD(cmd->cmd_idx, flags));

	// check cmd complete if necessary
	if ((mmio_read_16(base + SDHCI_TRANSFER_MODE) & SDHCI_TRNS_RESP_INT) == 0) {
		timeout = 100000;
		while (1) {
			stat = mmio_read_16(base + SDHCI_INT_STATUS);
			if (stat & SDHCI_INT_ERROR) {
				// printf("%s interrupt error: 0x%x 0x%x\n", __func__,
				//       mmio_read_16(base + SDHCI_INT_STATUS),
				//       mmio_read_16(base + SDHCI_ERR_INT_STATUS));
				return -EIO;
			}
			if (stat & SDHCI_INT_CMD_COMPLETE) {
				mmio_write_16(base + SDHCI_INT_STATUS,
					      stat | SDHCI_INT_CMD_COMPLETE);
				break;
			}

			udelay(1);
			if (!timeout--) {
				printf("%s timeout!\n", __func__);
				return -EIO;
			}
		}

		// get cmd respond
		if (flags != SDHCI_CMD_RESP_NONE)
			cmd->resp_data[0] = mmio_read_32(base + SDHCI_RESPONSE_01);
		if (flags & SDHCI_CMD_RESP_LONG) {
			cmd->resp_data[1] = mmio_read_32(base + SDHCI_RESPONSE_23);
			cmd->resp_data[2] = mmio_read_32(base + SDHCI_RESPONSE_45);
			cmd->resp_data[3] = mmio_read_32(base + SDHCI_RESPONSE_67);
		}
	}

	// check dma/transfer complete
	if (!(bm_params.flags & SD_USE_PIO)) {
		while (1) {
			stat = mmio_read_16(base + SDHCI_INT_STATUS);
			if (stat & SDHCI_INT_ERROR) {
				printf("%s interrupt error: 0x%x 0x%x\n", __func__,
				      mmio_read_16(base + SDHCI_INT_STATUS),
				      mmio_read_16(base + SDHCI_ERR_INT_STATUS));
				return -EIO;
			}

			if (stat & SDHCI_INT_XFER_COMPLETE) {
				mmio_write_16(base + SDHCI_INT_STATUS, stat);
				break;
			}

			if (stat & SDHCI_INT_DMA_END) {
				mmio_write_16(base + SDHCI_INT_STATUS, stat);
				if (mmio_read_16(base + SDHCI_HOST_CONTROL2) &
									SDHCI_HOST_VER4_ENABLE) {
					dma_addr = mmio_read_32(base + SDHCI_ADMA_SA_LOW);
					mmio_write_32(base + SDHCI_ADMA_SA_LOW, dma_addr);
					mmio_write_32(base + SDHCI_ADMA_SA_HIGH, 0);
				} else {
					dma_addr = mmio_read_32(base + SDHCI_DMA_ADDRESS);
					mmio_write_32(base + SDHCI_DMA_ADDRESS, dma_addr);
				}
			}

		}
	}

	return 0;
}

static int bm_sd_send_cmd_without_data(struct mmc_cmd *cmd)
{
	uintptr_t base;
	uint32_t stat, flags = 0x0;
	uint32_t timeout = 10000;

	base = bm_params.reg_base;

	// make sure cmd line is clear
	while (1) {
		if (!(mmio_read_32(base + SDHCI_PSTATE) & SDHCI_CMD_INHIBIT))
			break;
	}

	// set cmd flags
	if (cmd->cmd_idx == MMC_CMD(0))
		flags |= SDHCI_CMD_RESP_NONE;
	else if (cmd->cmd_idx == MMC_CMD(1))
		flags |= SDHCI_CMD_RESP_SHORT;
	else if (cmd->cmd_idx == MMC_ACMD(41))
		flags |= SDHCI_CMD_RESP_SHORT;
	else {
		if (cmd->resp_type & MMC_RSP_136)
			flags |= SDHCI_CMD_RESP_LONG;
		else
			flags |= SDHCI_CMD_RESP_SHORT;
		if (cmd->resp_type & MMC_RSP_CRC)
			flags |= SDHCI_CMD_CRC;
		if (cmd->resp_type & MMC_RSP_CMD_IDX)
			flags |= SDHCI_CMD_INDEX;
	}
	//verbose("SDHCI flags 0x%x\n", flags);

	// make sure dat line is clear if necessary
	if (flags != SDHCI_CMD_RESP_NONE) {
		while (1) {
			if (!(mmio_read_32(base + SDHCI_PSTATE) & SDHCI_CMD_INHIBIT_DAT))
				break;
		}
	}

	// issue the cmd
	mmio_write_32(base + SDHCI_ARGUMENT, cmd->cmd_arg);
	mmio_write_16(base + SDHCI_COMMAND, SDHCI_MAKE_CMD(cmd->cmd_idx, flags));

	// check cmd complete
	timeout = 100000;
	while (1) {
		stat = mmio_read_16(base + SDHCI_INT_STATUS);
		if (stat & SDHCI_INT_ERROR) {
			printf("%s interrupt error: 0x%x 0x%x\n", __func__,
			      mmio_read_16(base + SDHCI_INT_STATUS),
			      mmio_read_16(base + SDHCI_ERR_INT_STATUS));
			return -EIO;
		}
		if (stat & SDHCI_INT_CMD_COMPLETE) {
			mmio_write_16(base + SDHCI_INT_STATUS,
				      stat | SDHCI_INT_CMD_COMPLETE);
			break;
		}

		udelay(1);
		if (!timeout--) {
			printf("%s timeout!\n", __func__);
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

static int bm_sd_send_cmd(struct mmc_cmd *cmd)
{
	pr_debug("SDHCI cmd, idx=%d, arg=0x%x, resp_type=0x%x\n",
		cmd->cmd_idx, cmd->cmd_arg, cmd->resp_type);

	switch (cmd->cmd_idx) {
	case MMC_CMD(17):
	case MMC_CMD(18):
	case MMC_CMD(24):
	case MMC_CMD(25):
	case MMC_ACMD(51):
		return bm_sd_send_cmd_with_data(cmd);
	default:
		return bm_sd_send_cmd_without_data(cmd);
	}
}

void bm_sd_set_clk(int clk)
{
	int i, div;
	uint64_t base;

	assert(clk > 0);

	if (bm_params.clk_rate <= clk) {
		div = 0;
	} else {
		for (div = 0x1; div < 0x3FF; div++) {
			if (bm_params.clk_rate / (2 * div) <= clk)
				break;
		}
	}
	assert(div <= 0x3FF);

	base = bm_params.reg_base;
	if (mmio_read_16(base + SDHCI_HOST_CONTROL2) & (1 << 15)) {
		//verbose("Use SDCLK Preset Value\n");
	} else {
		//verbose("Set SDCLK by driver. div=0x%x(%d)\n", div, div);
		mmio_write_16(base + SDHCI_CLK_CTRL,
			      mmio_read_16(base + SDHCI_CLK_CTRL) & ~0x9); // disable INTERNAL_CLK_EN and PLL_ENABLE
		mmio_write_16(base + SDHCI_CLK_CTRL,
				(mmio_read_16(base + SDHCI_CLK_CTRL) & 0x3F) | ((div & 0xFF) << 8) |
				(((div >> 8) & 0x3) << 6)); // set 10bits clk div
		mmio_write_16(base + SDHCI_CLK_CTRL,
			      mmio_read_16(base + SDHCI_CLK_CTRL) | 0x1); // set INTERNAL_CLK_EN

		for (i = 0; i <= 150000; i += 100) {
			if (mmio_read_16(base + SDHCI_CLK_CTRL) & 0x2)
				break;
			udelay(100);
		}

		if (i > 150000) {
			printf("SD INTERNAL_CLK_EN setting FAILED!\n");
			assert(0);
		}

		mmio_write_16(base + SDHCI_CLK_CTRL,
			      mmio_read_16(base + SDHCI_CLK_CTRL) | 0x8); // set PLL_ENABLE

		for (i = 0; i <= 150000; i += 100) {
			if (mmio_read_16(base + SDHCI_CLK_CTRL) & 0x2)
				return;
			udelay(100);
		}
	}

	printf("SD PLL setting FAILED!\n");
}

void bm_sd_change_clk(int clk)
{
	int i, div;
	uint64_t base;

	assert(clk > 0);

	if (bm_params.clk_rate <= clk) {
		div = 0;
	} else {
		for (div = 0x1; div < 0x3FF; div++) {
			if (bm_params.clk_rate / (2 * div) <= clk)
				break;
		}
	}
	assert(div <= 0x3FF);

	base = bm_params.reg_base;

	mmio_write_16(base + SDHCI_CLK_CTRL,
		      mmio_read_16(base + SDHCI_CLK_CTRL) & ~(0x1 << 2)); // stop SD clock

	mmio_write_16(base + SDHCI_CLK_CTRL,
		      mmio_read_16(base + SDHCI_CLK_CTRL) & ~0x8); // disable  PLL_ENABLE

	if (mmio_read_16(base + SDHCI_HOST_CONTROL2) & (1 << 15)) {
		//verbose("Use SDCLK Preset Value\n");
		mmio_write_16(base + SDHCI_HOST_CONTROL2,
			      mmio_read_16(base + SDHCI_HOST_CONTROL2) & ~0x7); // clr UHS_MODE_SEL
	} else {
		//verbose("Set SDCLK by driver. div=0x%x(%d)\n", div, div);
		mmio_write_16(base + SDHCI_CLK_CTRL,
				(mmio_read_16(base + SDHCI_CLK_CTRL) & 0x3F) | ((div & 0xFF) << 8) |
				(((div >> 8) & 0x3) << 6)); // set 10bits clk div
		mmio_write_16(base + SDHCI_CLK_CTRL,
			      mmio_read_16(base + SDHCI_CLK_CTRL) & ~(0x1 << 5)); // CLK_GEN_SELECT
	}
	mmio_write_16(base + SDHCI_CLK_CTRL,
		      mmio_read_16(base + SDHCI_CLK_CTRL) | 0xc); // enable  PLL_ENABLE

	for (i = 0; i <= 150000; i += 100) {
		if (mmio_read_16(base + SDHCI_CLK_CTRL) & 0x2)
			return;
		udelay(100);
	}

	printf("SD PLL setting FAILED!\n");
}

int bm_sd_card_detect(void)
{
	uintptr_t base, reg;

	base = bm_params.reg_base;

	if (bm_params.card_in != SDCARD_STATUS_UNKNOWN)
		return bm_params.card_in;

	mmio_write_16(base + SDHCI_INT_STATUS_EN,
		      mmio_read_16(base + SDHCI_INT_STATUS_EN) | SDHCI_INT_CARD_INSERTION_EN);

	reg = mmio_read_32(base + SDHCI_PSTATE);

	if (reg & SDHCI_CARD_INSERTED)
		bm_params.card_in = SDCARD_STATUS_INSERTED;
	else
		bm_params.card_in = SDCARD_STATUS_NOT_INSERTED;

	return bm_params.card_in;
}

static void bm_sd_hw_init(void)
{
	uintptr_t base, vendor_base;

	base = bm_params.reg_base;
	vendor_base = base + (mmio_read_16(base + P_VENDOR_SPECIFIC_AREA) & ((1 << 12) - 1));
	bm_params.vendor_base = vendor_base;

	// deasset reset of phy
	mmio_write_32(base + SDHCI_P_PHY_CNFG, mmio_read_32(base + SDHCI_P_PHY_CNFG) | (1 << PHY_CNFG_PHY_RSTN));

	// reset data & cmd
	mmio_write_8(base + SDHCI_SOFTWARE_RESET, 0x6);

	// init common parameters
	mmio_write_8(base + SDHCI_PWR_CONTROL, (0x7 << 1));
	mmio_write_8(base + SDHCI_TOUT_CTRL, 0xe);  // for TMCLK 50Khz
	mmio_write_16(base + SDHCI_HOST_CONTROL2,
		      mmio_read_16(base + SDHCI_HOST_CONTROL2) | 1 << 11);  // set cmd23 support
	mmio_write_16(base + SDHCI_CLK_CTRL, mmio_read_16(base + SDHCI_CLK_CTRL) & ~(0x1 << 5));  // divided clock mode

	// set host version 4 parameters
	mmio_write_16(base + SDHCI_HOST_CONTROL2,
		      mmio_read_16(base + SDHCI_HOST_CONTROL2) | (1 << 12)); // set HOST_VER4_ENABLE
	if (mmio_read_32(base + SDHCI_CAPABILITIES1) & (0x1 << 27)) {
		mmio_write_16(base + SDHCI_HOST_CONTROL2,
			      mmio_read_16(base + SDHCI_HOST_CONTROL2) | 0x1 << 13); // set 64bit addressing
	}

	// if support asynchronous int
	if (mmio_read_32(base + SDHCI_CAPABILITIES1) & (0x1 << 29))
		mmio_write_16(base + SDHCI_HOST_CONTROL2,
			      mmio_read_16(base + SDHCI_HOST_CONTROL2) | (0x1 << 14)); // enable async int

	// give some time to power down card
	mdelay(20);

	mmio_write_16(base + SDHCI_HOST_CONTROL2,
		      mmio_read_16(base + SDHCI_HOST_CONTROL2) & ~(0x1 << 8)); // clr UHS2_IF_ENABLE
	mmio_write_8(base + SDHCI_PWR_CONTROL,
		     mmio_read_8(base + SDHCI_PWR_CONTROL) | 0x1); // set SD_BUS_PWR_VDD1
	mmio_write_16(base + SDHCI_HOST_CONTROL2,
		      mmio_read_16(base + SDHCI_HOST_CONTROL2) & ~0x7); // clr UHS_MODE_SEL
	bm_sd_set_clk(SDCARD_INIT_FREQ);
	mdelay(50);

	mmio_write_16(base + SDHCI_CLK_CTRL,
		      mmio_read_16(base + SDHCI_CLK_CTRL) | (0x1 << 2)); // supply SD clock
	udelay(400); // wait for voltage ramp up time at least 74 cycle, 400us is 80 cycles for 200Khz

	mmio_write_16(base + SDHCI_INT_STATUS, mmio_read_16(base + SDHCI_INT_STATUS) | (0x1 << 6));

	//we enable all interrupt status here for testing
	mmio_write_16(base + SDHCI_INT_STATUS_EN, mmio_read_16(base + SDHCI_INT_STATUS_EN) | 0xFFFF);
	mmio_write_16(base + SDHCI_ERR_INT_STATUS_EN, mmio_read_16(base + SDHCI_ERR_INT_STATUS_EN) | 0xFFFF);

	//verbose("SD init done\n");
}

static int bm_sd_set_ios(unsigned int clk, unsigned int width)
{
	//verbose("SDHCI set ios %d %d\n", clk, width);

	switch (width) {
	case MMC_BUS_WIDTH_1:
		mmio_write_8(bm_params.reg_base + SDHCI_HOST_CONTROL,
			     mmio_read_8(bm_params.reg_base + SDHCI_HOST_CONTROL) &
			     ~SDHCI_DAT_XFER_WIDTH);
		break;
	case MMC_BUS_WIDTH_4:
		mmio_write_8(bm_params.reg_base + SDHCI_HOST_CONTROL,
			     mmio_read_8(bm_params.reg_base + SDHCI_HOST_CONTROL) |
			     SDHCI_DAT_XFER_WIDTH);
		break;
	default:
		assert(0);
	}

	bm_sd_change_clk(clk);

	return 0;
}

static int bm_sd_prepare(int lba, uintptr_t buf, size_t size)
{
	uint64_t load_addr = buf;
	uint64_t base;
	uint32_t block_cnt, block_size;
	uint8_t tmp;

	//verbose("SDHCI prepare %d 0x%lx 0x%lx\n", lba, buf, size);
	flush_dcache_range(buf, buf+size);

	if (size >= MMC_BLOCK_SIZE) {
		// CMD17, 18, 24, 25
#if 0
		assert(((load_addr & MMC_BLOCK_MASK) == 0) && ((size % MMC_BLOCK_SIZE) == 0));
#else
		assert((size % MMC_BLOCK_SIZE) == 0);
#endif
		block_size = MMC_BLOCK_SIZE;
		block_cnt = size / MMC_BLOCK_SIZE;
	} else {
		// ACMD51
#if 0
		assert(((load_addr & 8) == 0) && ((size % 8) == 0));
#else
		assert((size % 8) == 0);
#endif
		block_size = 8;
		block_cnt = size / 8;
	}

	base = bm_params.reg_base;

	if (!(bm_params.flags & SD_USE_PIO)) {
		if (mmio_read_16(base + SDHCI_HOST_CONTROL2) & SDHCI_HOST_VER4_ENABLE) {
			mmio_write_32(base + SDHCI_ADMA_SA_LOW, load_addr);
			mmio_write_32(base + SDHCI_ADMA_SA_HIGH, (load_addr >> 32));
			mmio_write_32(base + SDHCI_DMA_ADDRESS, block_cnt);
			mmio_write_16(base + SDHCI_BLOCK_COUNT, 0);
		} else {
			assert((load_addr >> 32) == 0);
			mmio_write_32(base + SDHCI_DMA_ADDRESS, load_addr);
			mmio_write_16(base + SDHCI_BLOCK_COUNT, block_cnt);
		}

		// 512K bytes SDMA buffer boundary
		mmio_write_16(base + SDHCI_BLOCK_SIZE, SDHCI_MAKE_BLKSZ(7, block_size));

		// select SDMA
		tmp = mmio_read_8(base + SDHCI_HOST_CONTROL);
		tmp &= ~SDHCI_CTRL_DMA_MASK;
		tmp |= SDHCI_CTRL_SDMA;
		mmio_write_8(base + SDHCI_HOST_CONTROL, tmp);
	} else {
		mmio_write_16(base + SDHCI_BLOCK_SIZE, block_size);
		mmio_write_16(base + SDHCI_BLOCK_COUNT, block_cnt);
	}

	return 0;
}

static int bm_sd_read(int lba, uintptr_t buf, size_t size)
{
	uint32_t timeout = 0;
	uint64_t base = bm_params.reg_base;
	uint32_t *data = (uint32_t *)buf;
	uint32_t block_size = 0;
	uint32_t block_cnt = 0;
	uint32_t status = 0;

	if (bm_params.flags & SD_USE_PIO) {
		block_size = mmio_read_16(base + SDHCI_BLOCK_SIZE);
		block_cnt = size / block_size;
		block_size /= 4;

		for (int i = 0; i < block_cnt; ) {
			status = mmio_read_16(base + SDHCI_INT_STATUS);
			if ((status & SDHCI_INT_BUF_RD_READY) &&
			    (mmio_read_32(base + SDHCI_PSTATE) & SDHCI_BUF_RD_ENABLE)) {
				mmio_write_16(base + SDHCI_INT_STATUS,
					      SDHCI_INT_BUF_RD_READY);
				for (int j = 0; j < block_size; j++) {
					*(data++) = mmio_read_32(base + SDHCI_BUF_DATA_R);
				}

				timeout = 0;
				i++;
			} else {
				udelay(1);
				timeout++;
			}

			if (timeout >= 10000) {
				printf("sdhci read data timeout\n");
				goto timeout;
			}
		}

		timeout = 0;
		while (1) {
			status = mmio_read_16(base + SDHCI_INT_STATUS);
			if (status & SDHCI_INT_XFER_COMPLETE) {
				mmio_write_16(base + SDHCI_INT_STATUS,
					      status | SDHCI_INT_XFER_COMPLETE);

				return 0;

			} else {
				udelay(1);
				timeout++;
			}

			if (timeout >= 10000) {
				printf("wait xfer complete timeout\n");
				goto timeout;
			}
		}
	} else
		return 0;

timeout:
		return -1;

}

static int bm_sd_write(int lba, uintptr_t buf, size_t size)
{
	uint32_t timeout = 0;
	uint64_t base = bm_params.reg_base;
	uint32_t *data = (uint32_t *)buf;
	uint32_t block_size = 0;
	uint32_t block_cnt = 0;
	uint32_t status = 0;

	if (bm_params.flags & SD_USE_PIO) {
		block_size = mmio_read_16(base + SDHCI_BLOCK_SIZE);
		block_cnt = size / block_size;
		block_size /= 4;

		for (int j = 0; j < block_size; j++) {
			mmio_write_32(base + SDHCI_BUF_DATA_R, *(data++));
		}

		for (int i = 0; i < block_cnt-1; ) {
			status = mmio_read_16(base + SDHCI_INT_STATUS);
			if ((status & SDHCI_INT_BUF_WR_READY) &&
			    (mmio_read_32(base + SDHCI_PSTATE) &
			     SDHCI_BUF_WR_ENABLE)) {
				mmio_write_16(base + SDHCI_INT_STATUS,
					      SDHCI_INT_BUF_WR_READY);
				for (int j = 0; j < block_size; j++) {
					mmio_write_32(base + SDHCI_BUF_DATA_R, *(data++));
				}

				timeout = 0;
				i++;
			} else {
				udelay(1);
				timeout++;
			}

			if (timeout >= 10000000) {
				printf("sdhci write data timeout\n");
				goto timeout;
			}
		}

		timeout = 0;
		while (1) {
			status = mmio_read_16(base + SDHCI_INT_STATUS);
			if (status & SDHCI_INT_XFER_COMPLETE) {
				mmio_write_16(base + SDHCI_INT_STATUS,
					      status | SDHCI_INT_XFER_COMPLETE);

				return 0;

			} else {
				udelay(1);
				timeout++;
			}

			if (timeout >= 10000) {
				printf("wait xfer complete timeout\n");
				goto timeout;
			}
		}
	} else
		return 0;

timeout:
		return -1;
}

static void bm_sd_phy_init(void)
{
	uintptr_t base = SDIO_BASE;
	int loop = 100;

	// reset hardware
	mmio_write_8(base + SDHCI_SOFTWARE_RESET, 0x7);
	while (mmio_read_8(base + SDHCI_SOFTWARE_RESET)) {
		if (loop-- > 0)
			mdelay(10);
		else
			break;
	}

	// Wait for the PHY power on ready
	loop = 100;
	while (!(mmio_read_32(base + SDHCI_P_PHY_CNFG) & (1 << PHY_CNFG_PHY_PWRGOOD))) {
		if (loop-- > 0)
			mdelay(10);
		else
			break;
	}

	// Asset reset of phy
	mmio_clrbits_32(base + SDHCI_P_PHY_CNFG, (1 << PHY_CNFG_PHY_RSTN));

	// Set PAD_SN PAD_SP
	mmio_write_32(base + SDHCI_P_PHY_CNFG,
		      (1 << PHY_CNFG_PHY_PWRGOOD) | (0x9 << PHY_CNFG_PAD_SP) | (0x8 << PHY_CNFG_PAD_SN));

	// Set CMDPAD
	mmio_write_16(base + SDHCI_P_CMDPAD_CNFG,
		      (0x2 << PAD_CNFG_RXSEL) | (1 << PAD_CNFG_WEAKPULL_EN) |
		      (0x3 << PAD_CNFG_TXSLEW_CTRL_P) | (0x2 << PAD_CNFG_TXSLEW_CTRL_N));

	// Set DATAPAD
	mmio_write_16(base + SDHCI_P_DATPAD_CNFG,
		      (0x2 << PAD_CNFG_RXSEL) | (1 << PAD_CNFG_WEAKPULL_EN) |
		      (0x3 << PAD_CNFG_TXSLEW_CTRL_P) | (0x2 << PAD_CNFG_TXSLEW_CTRL_N));

	// Set CLKPAD
	mmio_write_16(base + SDHCI_P_CLKPAD_CNFG,
		      (0x2 << PAD_CNFG_RXSEL) | (0x3 << PAD_CNFG_TXSLEW_CTRL_P) | (0x2 << PAD_CNFG_TXSLEW_CTRL_N));

	// Set STB_PAD
	mmio_write_16(base + SDHCI_P_STBPAD_CNFG,
		      (0x2 << PAD_CNFG_RXSEL) | (0x2 << PAD_CNFG_WEAKPULL_EN) |
		      (0x3 << PAD_CNFG_TXSLEW_CTRL_P) | (0x2 << PAD_CNFG_TXSLEW_CTRL_N));

	// Set RSTPAD
	mmio_write_16(base + SDHCI_P_RSTNPAD_CNFG,
		      (0x2 << PAD_CNFG_RXSEL) | (1 << PAD_CNFG_WEAKPULL_EN) |
		      (0x3 << PAD_CNFG_TXSLEW_CTRL_P) | (0x2 << PAD_CNFG_TXSLEW_CTRL_N));

	// Set SDCLKDL_CNFG, EXTDLY_EN = 1, fix delay
	mmio_write_8(base + SDHCI_P_SDCLKDL_CNFG, (1 << SDCLKDL_CNFG_EXTDLY_EN));

	// Set SMPLDL_CNFG, Bypass
	mmio_write_8(base + SDHCI_P_SMPLDL_CNFG, (1 << SMPLDL_CNFG_BYPASS_EN));

	// Set ATDL_CNFG, tuning clk not use for init
	mmio_write_8(base + SDHCI_P_ATDL_CNFG, (2 << ATDL_CNFG_INPSEL_CNFG));
}

int bm_sd_init(uint32_t flags)
{
	int ret;

	bm_params.clk_rate = bm_get_sd_clock();
	printf("SD initializing %dHz (transfer frequency at %dHz)\n", bm_params.clk_rate, SDCARD_TRAN_FREQ);

	bm_params.flags = flags;

	bm_sd_phy_init();

	ret = mmc_init(&bm_sd_ops, SDCARD_TRAN_FREQ, bm_params.bus_width,
		       bm_params.flags, &sd_info);

	if (ret != 0)
		printf("SD initialization failed %d\n", ret);
	return ret;
}

/* register to device model */
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <driver/platform.h>
#include <driver/blkdev.h>
#include <framework/common.h>
#include <framework/module.h>

static int open(struct blkdev *blkdev)
{
	pr_info("%s open\n", blkdev->device.name);
	return 0;
}

static long read(struct blkdev *blkdev, unsigned long offset, unsigned long size, void *buf)
{
	pr_info("%s read, offset %lu, size %lu\n", blkdev->device.name, offset, size);

	return mmc_read_blocks(offset / 512, (uintptr_t)buf, size);
}

static long write(struct blkdev *blkdev, unsigned long offset, unsigned long size, void *buf)
{
	pr_info("%s write, offset %lu, size %lu\n", blkdev->device.name, offset, size);

	return mmc_write_blocks(offset / 512, (uintptr_t)buf, size);
}

static int close(struct blkdev *blkdev)
{
	pr_info("%s close\n", blkdev->device.name);
	return 0;
}

static struct blkops blkops = {
	.open = open,
	.read = read,
	.write = write,
	.close = close,
};

static int probe(struct platform_device *pdev)
{
	const struct of_device_id *match_id;
	struct blkdev *blkdev;

	match_id = platform_get_match_id(pdev);

	pr_info("compatible: %s\n", match_id->compatible);
	pr_info("data: %lu\n", (unsigned long)match_id->data);
	pr_info("register base: 0x%lx\n", pdev->reg_base);
	pr_info("register size: 0x%lx\n", pdev->reg_size);

	bm_params.reg_base = pdev->reg_base;

	if (bm_sd_card_detect()) {
                pr_info("sd card insert\n");
        } else {
                pr_info("sd card not insert, please insert sd to continue sd test\n");
		return -ENODEV;
        }

        if (bm_sd_init(SD_USE_PIO))
		return -EIO;

	pr_info("sd card init ok\n");

	blkdev = blkdev_alloc();
	if (!blkdev)
		return -ENOMEM;

	/* FIXME: block size and total size */
	blkdev->block_size = 512;
	blkdev->total_size = 1UL * 1024 * 1024 * 1024;
	blkdev->ops = &blkops;

	strcpy(blkdev->suffix, "sd");

	return blkdev_register(blkdev);
}

static struct of_device_id match_table[] = {
	{
		.compatible = "sophgo,sg2044-dwcmshc",
		.data = (void *)NULL,
	},
	{},
};

static struct platform_driver driver = {
	.driver.name = "mmc",
	.probe = probe,
	.of_match_table = match_table,
};

static int sd_init(void)
{
	platform_driver_register(&driver);
	return 0;
}

module_init(sd_init);

