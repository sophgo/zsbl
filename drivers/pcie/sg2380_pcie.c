// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2021-2025, Sophgo Limited and Contributors. All rights reserved.
 *
 */

#include <assert.h>
#include <string.h>
#include <timer.h>
#include <framework/common.h>
#include <lib/mmio.h>

#include "driver/pcie/sg2380_pcie.h"

#define PCIE_PLD_TEST
static void pcie_config_button_reset(uint32_t sys_id, uint32_t ctrl_id)
{
	uint32_t val = 0;
	uint64_t reg_base = 0;

	reg_base = PCIE_CFG_BASE(sys_id) + PCIE_CTRL_INTF_REG_BASE(ctrl_id);

	//cfg button rstn
	val = mmio_read_32(reg_base + PCIE_CTRL_RST_CTRL_0_REG);
	val |= (1 << PCIE_CTRL_RST_CTRL_0_BUTTON_RSTN_BIT);
	mmio_write_32((reg_base + PCIE_CTRL_RST_CTRL_0_REG), val);
}

static void pcie_config_power_reset(uint32_t sys_id, uint32_t ctrl_id)
{
	uint32_t val = 0;
	uint64_t reg_base = 0;

	reg_base = PCIE_CFG_BASE(sys_id) + PCIE_CTRL_INTF_REG_BASE(ctrl_id);

	//cfg pwr rstn
	val = mmio_read_32(reg_base + PCIE_CTRL_RST_CTRL_0_REG);
	val |= (1 << PCIE_CTRL_RST_CTRL_0_PWR_UP_RSTN_BIT);
	mmio_write_32((reg_base + PCIE_CTRL_RST_CTRL_0_REG), val);
}

void pcie_config_phy_pwr_stable(uint32_t sys_id, uint32_t phy_id)
{
	uint32_t val = 0;
	uint64_t base_addr = 0;
	uint32_t reg_addr = 0;

	if (sys_id == PCIE_SUBSYS) {
		base_addr = PCIE_PHY_INTF_REG_BASE;
		//default  pciex8 need cfg
		//cfg  phy0_upcs_pwr_stable
		val = mmio_read_32(base_addr + PHY_INTF_REG_RX400);
		val |= (0x1 << PHY_INTF_REG_RX400_UPCS_PWR_STABLE_BIT);
		mmio_write_32((base_addr + PHY_INTF_REG_RX400), val);

		for (phy_id = 0; phy_id < PCIE_PHY_ID_BUTT; phy_id++) {
			reg_addr = PHY_INTF_REG_RX000 + (phy_id * 0x100);
			//cfg  phy_x_pcs_pwr_stable
			//cfg  phy_x_pma_pwr_stable
			val = mmio_read_32(base_addr + reg_addr);
			val |= (0x1 << PHY_INTF_REG_RX000_PHY_X_PCS_PWR_STABLE_BIT);
			val |= (0x1 << PHY_INTF_REG_RX000_PHY_X_PMA_PWR_STABLE_BIT);
			mmio_write_32((base_addr + reg_addr), val);
		}
	} else {
		base_addr = PCIE_CFG_BASE(sys_id) + SSPERI_PHY_INTF_REG_BASE(phy_id);
		//default  pciex8 need cfg
		//cfg  phy0_upcs_pwr_stable
		val = mmio_read_32(base_addr + SSPERI_PHY_INTF_REG_RX2b8);
		val |= (0x1 << SSPERI_PHY_INTF_REG_RX2b8_UPCS_PWR_STABLE_BIT);
		mmio_write_32((base_addr + SSPERI_PHY_INTF_REG_RX2b8), val);

		val = mmio_read_32(base_addr + SSPERI_PHY_INTF_REG_RX004);
		val |= (0x1 << SSPERI_PHY_INTF_REG_RX004_PHY0_PCS_PWR_STABLE_BIT);
		val |= (0x1 << SSPERI_PHY_INTF_REG_RX004_PHY0_PMA_PWR_STABLE_BIT);
		mmio_write_32((base_addr + SSPERI_PHY_INTF_REG_RX004), val);

	}

}

void pcie_config_ss_mode(uint32_t sys_id, uint32_t phy_id, uint32_t ss_mode)
{
	uint32_t val = 0;
	uint64_t base_addr = 0;

	if (sys_id == PCIE_SUBSYS) {
		base_addr = PCIE_PHY_INTF_REG_BASE;

		val = mmio_read_32(base_addr + PHY_INTF_REG_RX410);
		val &= (~PHY_INTF_REG_RX410_SS_MODE_MASK);
		val |= ss_mode & PHY_INTF_REG_RX410_SS_MODE_MASK;
		mmio_write_32((base_addr + PHY_INTF_REG_RX410), val);
		val = mmio_read_32(base_addr + PHY_INTF_REG_RX410);
		pr_info("%s, val = %d\n", __func__, val);
	} else {
		base_addr = PCIE_CFG_BASE(sys_id) + SSPERI_PHY_INTF_REG_BASE(phy_id);
		val = mmio_read_32(base_addr + SSPERI_PHY_INTF_REG_RX000);
		val &= (~SSPERI_PHY_INTF_REG_RX000_SS_MODE_MASK);
		val |= ss_mode & SSPERI_PHY_INTF_REG_RX000_SS_MODE_MASK;
		mmio_write_32((base_addr + SSPERI_PHY_INTF_REG_RX000), val);

	}

}

void pcie_config_phy_bl_bypass(uint32_t sys_id, uint32_t phy_id,
			uint32_t sram_bypass_mode)
{
	uint32_t val = 0;
	uint64_t reg_base = 0;
	uint32_t reg_addr = 0;

	if (sys_id == PCIE_SUBSYS) {
		reg_base = PCIE_PHY_INTF_REG_BASE;
		reg_addr = PHY_INTF_REG_RX000 + (phy_id * 0x100);

		//cfg phy0 phy0_sram_bypass bypass
		val = mmio_read_32(reg_base + reg_addr);
		val &= (~PHY_INTF_REG_RX000_PHY_X_SRAM_BYPASS_MASK);
		val |= (sram_bypass_mode << PHY_INTF_REG_RX000_PHY_X_SRAM_BL_BYPASS_BIT);
		mmio_write_32((reg_base + reg_addr), val);

	} else {
		reg_base = PCIE_CFG_BASE(sys_id) + SSPERI_PHY_INTF_REG_BASE(phy_id);
		reg_addr = SSPERI_PHY_INTF_REG_RX004;

		//cfg phy0 phy0_sram_bypass bypass
		val = mmio_read_32(reg_base + reg_addr);
		val &= (~SSPERI_PHY_INTF_REG_RX004_PHY0_SRAM_BYPASS_MASK);
		val |= (sram_bypass_mode << SSPERI_PHY_INTF_REG_RX004_PHY0_SRAM_BL_BYPASS_BIT);
		mmio_write_32((reg_base + reg_addr), val);
	}
}

void pcie_wait_sram_init_done(uint32_t sys_id, uint32_t phy_id)
{
	uint32_t val = 0;
	uint32_t reg_addr = 0;
	uint64_t reg_base = 0;

	if (sys_id == PCIE_SUBSYS)
		reg_base = PCIE_PHY_INTF_REG_BASE;
	else
		reg_base = PCIE_CFG_BASE(sys_id) + SSPERI_PHY_INTF_REG_BASE(phy_id);

	reg_addr = PHY_INTF_REG_RX000 + (phy_id * 0x100);
	do {
		val = mmio_read_32(reg_base + reg_addr);
		val = (val >> PHY_INTF_REG_RX000_PHY_X_SRAM_EXT_LD_DONE_BIT) & 0x1;
		udelay(1);
	} while (val != 1);
}

void pcie_config_exload_phy(uint32_t sys_id, uint32_t phy_id)
{
	uint32_t val = 0;
	uint32_t reg_addr = 0;
	uint64_t reg_base = 0;

	if (sys_id == PCIE_SUBSYS)
		reg_base = PCIE_PHY_INTF_REG_BASE;
	else
		reg_base = PCIE_CFG_BASE(sys_id) + SSPERI_PHY_INTF_REG_BASE(phy_id);
	reg_addr = PHY_INTF_REG_RX000 + (phy_id * 0x100);

	//cfg  phy0_sram_ext_ld_done
	val = mmio_read_32(reg_base + reg_addr);
	val |= (0x1 << PHY_INTF_REG_RX000_PHY_X_SRAM_EXT_LD_DONE_BIT);
	mmio_write_32((reg_base + reg_addr), val);
}

void pcie_wait_core_clk(uint32_t sys_id, uint32_t ctrl_id)
{
	uint32_t val = 0;
	uint64_t pcie_sii_base = 0;

	pcie_sii_base = PCIE_CFG_BASE(sys_id) + PCIE_SII_INTF_REG_BASE(ctrl_id);

	do {
		udelay(10);
		val = mmio_read_32(pcie_sii_base + 0x5c); //GEN_CTRL_4
		val = (val >> 8) & 0x1; //bit8, pcie_rst_n
		pr_info("%s, val = %d\n", __func__, val);
	} while (val != 1);
}

void pcie_config_link(uint32_t sys_id, uint32_t ctrl_id, uint32_t pcie_ln_cnt)
{
	uint32_t val = 0;
	uint64_t base_addr = 0;

	if ((sys_id == PCIE_SUBSYS) && (ctrl_id == PCIE_CTRL_X16_0))
		base_addr = PCIE_CFG_BASE(sys_id) + 0x01000000;
	else
		base_addr = PCIE_CFG_BASE(sys_id) + PCIE_CTRL_DBI_REG_BASE(ctrl_id);

	val = mmio_read_32(base_addr + 0x710);
	val &= 0xffc0ffff;
	switch (pcie_ln_cnt) {
	case 1:
		val |= (0x1 << 16);
		break;
	case 2:
		val |= (0x3 << 16);
		break;
	case 4:
		val |= (0x7 << 16);
		break;
	case 8:
		val |= (0xf << 16);
		break;
	case 16:
		val |= (0x1f << 16);
		break;
	default:
		val |= (0x1f << 16);
		break;
	}
	mmio_write_32((base_addr + 0x710), val);

	val = mmio_read_32(base_addr + 0x80c);
	val = (val & 0xffffe0ff) | (pcie_ln_cnt << 8);
	mmio_write_32((base_addr + 0x80c), val);

	val = mmio_read_32(base_addr + 0x7c);
	val = (val & 0xfffffc0f) | (pcie_ln_cnt << 4);
	mmio_write_32((base_addr + 0x7c), val);


	//config lane_count
	val = mmio_read_32(base_addr + 0x8c0);
	val = (val & 0xffffffc0) | pcie_ln_cnt;
	mmio_write_32((base_addr + 0x8c0), val);

	if (sys_id == SSPERI_SUBSYS) {
		//enable ACS contrl
		val = mmio_read_32(base_addr + 0x1b8);
		val &= 0xffff;
		val |= 0x3f << 16;
		mmio_write_32((base_addr + 0x1b8), val);
	} else {
		if (ctrl_id < 3) {
			//enable ACS contrl
			val = mmio_read_32(base_addr + 0x1bc);
			val &= 0xffff;
			val |= 0x3f << 16;
			mmio_write_32((base_addr + 0x1bc), val);
		} else if (ctrl_id < 5) {
			//enable ACS contrl
			val = mmio_read_32(base_addr + 0x1d4);
			val &= 0xffff;
			val |= 0x3f << 16;
			mmio_write_32((base_addr + 0x1d4), val);
		} else {
			//enable ACS contrl
			val = mmio_read_32(base_addr + 0x298);
			val &= 0xffff;
			val |= 0x3f << 16;
			mmio_write_32((base_addr + 0x298), val);
		}
	}

}

struct pcie_eq_preset ctrl_x2_rc_g3_preset[1] = {
	//cfg SPCIE_CAP_OFF_0CH_REG - GEN3 TX/RX preset hint lan 0 and 1
	{0x164, 0x31650810}
};

struct pcie_eq_preset ctrl_x2_ep_g3_preset[1] = {
	//cfg SPCIE_CAP_OFF_0CH_REG - GEN3 TX/RX preset hint lan 0 and 1
	{0x164, 0x5735904}
};

struct pcie_eq_preset ctrl_x2_rc_g4_preset[1] = {
	//cfg PL16G_CAP_OFF_20H_REG - GEN4 TX/RX preset hint lan 0, 1, 2, 3
	{0x188, 0x228a7683}
};

struct pcie_eq_preset ctrl_x2_ep_g4_preset[1] = {
	//cfg PL16G_CAP_OFF_20H_REG - GEN4 TX/RX preset hint lan 0, 1, 2, 3
	{0x188, 0x434863a3}
};

struct pcie_eq_preset ctrl_x4_rc_g3_preset[2] = {
	//cfg SPCIE_CAP_OFF_0CH_REG - GEN3 TX/RX preset hint lan 0 and 1
	{0x164, 0x31650810},
	//cfg SPCIE_CAP_OFF_10H_REG - GEN3 TX/RX preset hint lan 2 and 3
	{0x168, 0x11793453}
};

struct pcie_eq_preset ctrl_x4_ep_g3_preset[2] = {
	//cfg SPCIE_CAP_OFF_0CH_REG - GEN3 TX/RX preset hint lan 0 and 1
	{0x164, 0x5735904},
	//cfg SPCIE_CAP_OFF_10H_REG - GEN3 TX/RX preset hint lan 2 and 3
	{0x168, 0x65634207}
};

struct pcie_eq_preset ctrl_x4_rc_g4_preset[1] = {
	//cfg PL16G_CAP_OFF_20H_REG - GEN4 TX/RX preset hint lan 0, 1, 2, 3
	{0x198, 0x228a7683}
};

struct pcie_eq_preset ctrl_x4_ep_g4_preset[1] = {
	//cfg PL16G_CAP_OFF_20H_REG - GEN4 TX/RX preset hint lan 0, 1, 2, 3
	{0x198, 0x434863a3}
};

struct pcie_eq_preset ctrl_x16_rc_g3_preset[8] = {
	//cfg SPCIE_CAP_OFF_0CH_REG - GEN3 TX/RX preset hint lan 0 and 1
	{0x19c, 0x31650810},
	//cfg SPCIE_CAP_OFF_10H_REG - GEN3 TX/RX preset hint lan 2 and 3
	{0x1a0, 0x11793453},
	//cfg SPCIE_CAP_OFF_14H_REG - GEN3 TX/RX preset hint lan 4 and 5
	{0x1a4, 0x45626a57},
	//cfg SPCIE_CAP_OFF_18H_REG - GEN3 TX/RX preset hint lan 6 and 7
	{0x1a8, 0x74074a71},
	//cfg SPCIE_CAP_OFF_1CH_REG - GEN3 TX/RX preset hint lan 8 and 9
	{0x1ac, 0x5a243415},
	//cfg SPCIE_CAP_OFF_20H_REG - GEN3 TX/RX preset hint lan 10 and 11
	{0x1b0, 0x17725906},
	//cfg SPCIE_CAP_OFF_24H_REG - GEN3 TX/RX preset hint lan 12 and 13
	{0x1b4, 0x37657862},
	//cfg SPCIE_CAP_OFF_28H_REG - GEN3 TX/RX preset hint lan 14 and 15
	{0x1b8, 0x32427664}
};

struct pcie_eq_preset ctrl_x16_ep_g3_preset[8] = {
	//cfg SPCIE_CAP_OFF_0CH_REG - GEN3 TX/RX preset hint lan 0 and 1
	{0x19c, 0x5735904},
	//cfg SPCIE_CAP_OFF_10H_REG - GEN3 TX/RX preset hint lan 2 and 3
	{0x1a0, 0x65634207},
	//cfg SPCIE_CAP_OFF_14H_REG - GEN3 TX/RX preset hint lan 4 and 5
	{0x1a4, 0x28252957},
	//cfg SPCIE_CAP_OFF_18H_REG - GEN3 TX/RX preset hint lan 6 and 7
	{0x1a8, 0x46340115},
	//cfg SPCIE_CAP_OFF_1CH_REG - GEN3 TX/RX preset hint lan 8 and 9
	{0x1ac, 0x62492a55},
	//cfg SPCIE_CAP_OFF_20H_REG - GEN3 TX/RX preset hint lan 10 and 11
	{0x1b0, 0x72043450},
	//cfg SPCIE_CAP_OFF_24H_REG - GEN3 TX/RX preset hint lan 12 and 13
	{0x1b4, 0x33183115},
	//cfg SPCIE_CAP_OFF_28H_REG - GEN3 TX/RX preset hint lan 14 and 15
	{0x1b8, 0x75681a39}
};

struct pcie_eq_preset ctrl_x16_rc_g4_preset[4] = {
	//cfg PL16G_CAP_OFF_20H_REG - GEN4 TX/RX preset hint lan 0, 1, 2, 3
	{0x1e0, 0x228a7683},
	//cfg PL16G_CAP_OFF_24H_REG - GEN4 TX/RX preset hint lan 4, 5, 6, 7
	{0x1e4, 0x4a267265},
	//cfg PL16G_CAP_OFF_28H_REG - GEN4 TX/RX preset hint lan 8, 9, 10, 11
	{0x1e8, 0x37657862},
	//cfg PL16G_CAP_OFF_2CH_REG - GEN4 TX/RX preset hint lan 12, 13, 14, 15
	{0x1ec, 0x027a053a}
};

struct pcie_eq_preset ctrl_x16_ep_g4_preset[4] = {
	//cfg PL16G_CAP_OFF_20H_REG - GEN4 TX/RX preset hint lan 0, 1, 2, 3
	{0x1e0, 0x434863a3},
	//cfg PL16G_CAP_OFF_24H_REG - GEN4 TX/RX preset hint lan 4, 5, 6, 7
	{0x1e4, 0xa61656a0},
	//cfg PL16G_CAP_OFF_28H_REG - GEN4 TX/RX preset hint lan 8, 9, 10, 11
	{0x1e8, 0x67a29003},
	//cfg PL16G_CAP_OFF_2CH_REG - GEN4 TX/RX preset hint lan 12, 13, 14, 15
	{0x1ec, 0x93676a90}
};

void pcie_config_eq(uint32_t sys_id, uint32_t ctrl_id,
			uint32_t pcie_gen_slt, uint32_t pcie_ln_cnt, uint32_t pcie_dev_type)
{
	uint32_t val = 0;
	uint64_t dbi_reg_base = 0;
	struct pcie_eq_preset *pcie_eq_preset = NULL;

	if ((sys_id == PCIE_SUBSYS) && (ctrl_id == PCIE_CTRL_X16_0))
		dbi_reg_base = PCIE_CFG_BASE(sys_id) + 0x01000000;
	else
		dbi_reg_base = PCIE_CFG_BASE(sys_id) + PCIE_CTRL_DBI_REG_BASE(ctrl_id);

	//cfg DBI_RO_WR_EN enable
	val = mmio_read_32(dbi_reg_base + 0x8bc);
	val |= 0x1;
	mmio_write_32((dbi_reg_base + 0x8bc), val);

	if (pcie_dev_type == PCIE_DEV_TYPE_RC) {
		//cfg Gen2_ctrl_off - low_Swing(default full Swing)
		val = mmio_read_32(dbi_reg_base + 0x80c);
		pr_info("%s: 0x80c = 0x%x\n", __func__, val);
		val &= 0xfff80000;
		val |= 0x710c8;
		mmio_write_32((dbi_reg_base + 0x80c), val);
		val = mmio_read_32(dbi_reg_base + 0x80c);
		pr_info("%s: 0x80c = 0x%x\n", __func__, val);

		//cfg Gen3_related_off - EQ_REDO
		mmio_write_32((dbi_reg_base + 0x890), 0x2000);
		mmio_write_32((dbi_reg_base + 0x890), 0x2800);
		if (sys_id == SSPERI_SUBSYS)
			pcie_eq_preset = ctrl_x2_rc_g3_preset;
		else {
			if (ctrl_id < 3)
				pcie_eq_preset = ctrl_x2_rc_g3_preset;
			else if (ctrl_id < 5)
				pcie_eq_preset = ctrl_x4_rc_g3_preset;
			else
				pcie_eq_preset = ctrl_x16_rc_g3_preset;
		}
		//cfg SPCIE_CAP_OFF_0CH_REG - GEN3 TX/RX preset hint lan 0 and 1
		mmio_write_32((dbi_reg_base + pcie_eq_preset->addr), pcie_eq_preset->value);
		if (pcie_ln_cnt > 2) {
			//cfg SPCIE_CAP_OFF_10H_REG - GEN3 TX/RX preset hint lan 2 and 3
			mmio_write_32((dbi_reg_base + (pcie_eq_preset + 1)->addr),
				      (pcie_eq_preset + 1)->value);
		}
		if (pcie_ln_cnt > 4) {
			//cfg SPCIE_CAP_OFF_14H_REG - GEN3 TX/RX preset hint lan 4 and 5
			mmio_write_32((dbi_reg_base + (pcie_eq_preset + 2)->addr),
				      (pcie_eq_preset + 2)->value);
			//cfg SPCIE_CAP_OFF_18H_REG - GEN3 TX/RX preset hint lan 6 and 7
			mmio_write_32((dbi_reg_base + (pcie_eq_preset + 3)->addr),
				      (pcie_eq_preset + 3)->value);
		}
		if (pcie_ln_cnt > 8) {
			//cfg SPCIE_CAP_OFF_1CH_REG - GEN3 TX/RX preset hint lan 8 and 9
			mmio_write_32((dbi_reg_base + (pcie_eq_preset + 4)->addr),
				      (pcie_eq_preset + 4)->value);
			//cfg SPCIE_CAP_OFF_20H_REG - GEN3 TX/RX preset hint lan 10 and 11
			mmio_write_32((dbi_reg_base + (pcie_eq_preset + 5)->addr),
				      (pcie_eq_preset + 5)->value);
			//cfg SPCIE_CAP_OFF_24H_REG - GEN3 TX/RX preset hint lan 12 and 13
			mmio_write_32((dbi_reg_base + (pcie_eq_preset + 6)->addr),
				      (pcie_eq_preset + 6)->value);
			//cfg SPCIE_CAP_OFF_28H_REG - GEN3 TX/RX preset hint lan 14 and 15
			mmio_write_32((dbi_reg_base + (pcie_eq_preset + 7)->addr),
				      (pcie_eq_preset + 7)->value);
		}

		//cfg GEN3_EQ_FB_MODE_DIR_CHANGE_OFF - Min GEN3_EQ_FMDC_N_EVALS
		mmio_write_32((dbi_reg_base + 0x8ac), 0x0);
		//cfg Gen3 EQ Control Register - Preset 4 is requested and evaluated
		//in EQ Master Phase and Don't include FOM
		mmio_write_32((dbi_reg_base + 0x8a8), 0xc001071);

		if (pcie_gen_slt == PCIE_GEN_4) {
			// cfg Gen3_related_off - Gen4 Data Rate is selected for shadow register.
			mmio_write_32((dbi_reg_base + 0x890), 0x1002800);
			// cfg GEN3_RELATED_OFF -USP does not send 8GT or 16GT EQ TS2
			mmio_write_32((dbi_reg_base + 0x890), 0x1402800);
			if (sys_id == SSPERI_SUBSYS)
				pcie_eq_preset = ctrl_x2_rc_g4_preset;
			else {
				if (ctrl_id < 3)
					pcie_eq_preset = ctrl_x2_rc_g4_preset;
				else if (ctrl_id < 5)
					pcie_eq_preset = ctrl_x4_rc_g4_preset;
				else
					pcie_eq_preset = ctrl_x16_rc_g4_preset;
			}

			//cfg PL16G_CAP_OFF_20H_REG - GEN4 TX/RX preset hint lan 0, 1, 2, 3
			mmio_write_32((dbi_reg_base + (pcie_eq_preset + 0)->addr),
				      (pcie_eq_preset + 0)->value);
			if (pcie_ln_cnt > 4) {
				//cfg PL16G_CAP_OFF_24H_REG - GEN4 TX/RX preset hint lan
				//4, 5, 6, 7
				mmio_write_32((dbi_reg_base + (pcie_eq_preset + 1)->addr),
					      (pcie_eq_preset + 1)->value);
			}
			if (pcie_ln_cnt > 8) {
				//cfg PL16G_CAP_OFF_28H_REG - GEN4 TX/RX preset hint lan
				//8, 9, 10, 11
				mmio_write_32((dbi_reg_base + (pcie_eq_preset + 2)->addr),
					      (pcie_eq_preset + 2)->value);
				//cfg PL16G_CAP_OFF_2CH_REG - GEN4 TX/RX preset hint lan
				//12, 13, 14, 15
				mmio_write_32((dbi_reg_base + (pcie_eq_preset + 3)->addr),
					      (pcie_eq_preset + 3)->value);
			}
			//cfg GEN3_EQ_FB_MODE_DIR_CHANGE_OFF - Min GEN3_EQ_FMDC_N_EVALS
			mmio_write_32((dbi_reg_base + 0x8ac), 0x0);
			// cfg Gen3 EQ Control Register - Preset 9 is requested and evaluated in
			// EQ Master Phase and Request controller to send back-to-back EIEOS in
			// Recovery.RcvrLock state until presets to coefficients mapping is complete
			mmio_write_32((dbi_reg_base + 0x8a8), 0x4020031);
		}

	} else {
		//cfg Gen2_ctrl_off - low_Swing(default full Swing)
		val = mmio_read_32(dbi_reg_base + 0x80c);
		val &= 0xfffe0000;
		val |= 0x110c8;
		mmio_write_32((dbi_reg_base + 0x80c), val);

		//cfg Gen3_related_off - EQ_REDO
		mmio_write_32((dbi_reg_base + 0x890), 0x2000);
		mmio_write_32((dbi_reg_base + 0x890), 0x2800);

		if (sys_id == SSPERI_SUBSYS)
			pcie_eq_preset = ctrl_x2_ep_g3_preset;
		else {
			if (ctrl_id < 3)
				pcie_eq_preset = ctrl_x2_ep_g3_preset;
			else if (ctrl_id < 5)
				pcie_eq_preset = ctrl_x4_ep_g3_preset;
			else
				pcie_eq_preset = ctrl_x16_ep_g3_preset;
		}
		//cfg SPCIE_CAP_OFF_0CH_REG - GEN3 TX/RX preset hint lan 0 and 1
		mmio_write_32((dbi_reg_base + pcie_eq_preset->addr), pcie_eq_preset->value);
		if (pcie_ln_cnt > 2) {
			//cfg SPCIE_CAP_OFF_10H_REG - GEN3 TX/RX preset hint lan 2 and 3
			mmio_write_32((dbi_reg_base + (pcie_eq_preset + 1)->addr),
				      (pcie_eq_preset + 1)->value);
		}
		if (pcie_ln_cnt > 4) {
			//cfg SPCIE_CAP_OFF_14H_REG - GEN3 TX/RX preset hint lan 4 and 5
			mmio_write_32((dbi_reg_base + (pcie_eq_preset + 2)->addr),
				      (pcie_eq_preset + 2)->value);
			//cfg SPCIE_CAP_OFF_18H_REG - GEN3 TX/RX preset hint lan 6 and 7
			mmio_write_32((dbi_reg_base + (pcie_eq_preset + 3)->addr),
				      (pcie_eq_preset + 3)->value);
		}
		if (pcie_ln_cnt > 8) {
			//cfg SPCIE_CAP_OFF_1CH_REG - GEN3 TX/RX preset hint lan 8 and 9
			mmio_write_32((dbi_reg_base + (pcie_eq_preset + 4)->addr),
				      (pcie_eq_preset + 4)->value);
			//cfg SPCIE_CAP_OFF_20H_REG - GEN3 TX/RX preset hint lan 10 and 11
			mmio_write_32((dbi_reg_base + (pcie_eq_preset + 5)->addr),
				      (pcie_eq_preset + 5)->value);
			//cfg SPCIE_CAP_OFF_24H_REG - GEN3 TX/RX preset hint lan 12 and 13
			mmio_write_32((dbi_reg_base + (pcie_eq_preset + 6)->addr),
				      (pcie_eq_preset + 6)->value);
			//cfg SPCIE_CAP_OFF_28H_REG - GEN3 TX/RX preset hint lan 14 and 15
			mmio_write_32((dbi_reg_base + (pcie_eq_preset + 7)->addr),
				      (pcie_eq_preset + 7)->value);
		}

		//cfg GEN3_EQ_FB_MODE_DIR_CHANGE_OFF - Min GEN3_EQ_FMDC_N_EVALS
		mmio_write_32((dbi_reg_base + 0x8ac), 0x0);
		//cfg Gen3 EQ Control Register - Preset 4 is requested and evaluated
		//in EQ Master Phase and Don't include FOM
		mmio_write_32((dbi_reg_base + 0x8a8), 0xc001071);

		if (pcie_gen_slt == PCIE_GEN_4) {
			// cfg Gen3_related_off - Gen4 Data Rate is selected for shadow register.
			mmio_write_32((dbi_reg_base + 0x890), 0x1002800);
			// cfg GEN3_RELATED_OFF -USP does not send 8GT or 16GT EQ TS2
			mmio_write_32((dbi_reg_base + 0x890), 0x1402800);

			if (sys_id == SSPERI_SUBSYS)
				pcie_eq_preset = ctrl_x2_ep_g4_preset;
			else {
				if (ctrl_id < 3)
					pcie_eq_preset = ctrl_x2_ep_g4_preset;
				else if (ctrl_id < 5)
					pcie_eq_preset = ctrl_x4_ep_g4_preset;
				else
					pcie_eq_preset = ctrl_x16_ep_g4_preset;
			}

			//cfg PL16G_CAP_OFF_20H_REG - GEN4 TX/RX preset hint lan 0, 1, 2, 3
			mmio_write_32((dbi_reg_base + pcie_eq_preset->addr), pcie_eq_preset->value);
			if (pcie_ln_cnt > 4) {
				//cfg PL16G_CAP_OFF_24H_REG - GEN4 TX/RX preset hint lan 4, 5, 6, 7
				mmio_write_32((dbi_reg_base + (pcie_eq_preset + 1)->addr),
					      (pcie_eq_preset + 1)->value);
			}
			if (pcie_ln_cnt > 8) {
				//cfg PL16G_CAP_OFF_28H_REG - GEN4 TX/RX preset hint lan
				//8, 9, 10, 11
				mmio_write_32((dbi_reg_base + (pcie_eq_preset + 2)->addr),
					      (pcie_eq_preset + 2)->value);
				//cfg PL16G_CAP_OFF_2CH_REG - GEN4 TX/RX preset hint lan
				//12, 13, 14, 15
				mmio_write_32((dbi_reg_base + (pcie_eq_preset + 3)->addr),
					      (pcie_eq_preset + 3)->value);
			}
			//cfg GEN3_EQ_FB_MODE_DIR_CHANGE_OFF - Min GEN3_EQ_FMDC_N_EVALS
			mmio_write_32((dbi_reg_base + 0x8ac), 0x0);
			// cfg Gen3 EQ Control Register - Preset 8 is requested and evaluated
			// in EQ Master Phase and Request controller to send back-to-back EIEOS
			// in Recovery.RcvrLock state until presets to coefficients mapping is
			// complete
			mmio_write_32((dbi_reg_base + 0x8a8), 0x4010031);
		}

	}

	//cfg DBI_RO_WR_EN disblae
	val = mmio_read_32(dbi_reg_base + 0x8bc);
	val &= 0xfffffffe;
	mmio_write_32((dbi_reg_base + 0x8bc), val);

}


void pcie_config_ctrl(uint32_t sys_id, uint32_t ctrl_id,
			uint32_t pcie_gen_slt, uint32_t pcie_dev_type)
{
	uint32_t val = 0;
	uint64_t ctrl_reg_base = 0;
	uint64_t dbi_reg_base = 0;

	ctrl_reg_base = PCIE_CFG_BASE(sys_id) + PCIE_CTRL_INTF_REG_BASE(ctrl_id);

	if ((sys_id == PCIE_SUBSYS) && (ctrl_id == PCIE_CTRL_X16_0))
		dbi_reg_base = PCIE_CFG_BASE(sys_id) + PCIE_X16_DBI_OFFSET;
	else
		dbi_reg_base = PCIE_CFG_BASE(sys_id) + PCIE_CTRL_DBI_REG_BASE(ctrl_id);

	//config device_type
	val = mmio_read_32(ctrl_reg_base + PCIE_CTRL_GENERAL_CORE_CTRL);
	val &= (~PCIE_CTRL_GENERAL_CORE_CTRL_DEVICE_TYPE_MASK);
	val |= (pcie_dev_type << 4);
	mmio_write_32((ctrl_reg_base + PCIE_CTRL_GENERAL_CORE_CTRL), val);

	//config Directed Speed Change	Writing '1' to this field instructs the LTSSM to initiate
	//a speed change to Gen2 or Gen3 after the link is initialized at Gen1 speed
	val = mmio_read_32(dbi_reg_base + 0x80c);
	val |= (0x1 << 17);
	mmio_write_32((dbi_reg_base + 0x80c), val);
	val = mmio_read_32(dbi_reg_base + 0x80c);
	pr_info("%s: 0x80c = 0x%x\n", __func__, val);

	//config generation_select-pcie_cap_target_link_speed
	val = mmio_read_32(dbi_reg_base + 0x7c);
	val = (val & 0xfffffff0) | pcie_gen_slt;
	mmio_write_32((dbi_reg_base + 0x7c), val);

	val = mmio_read_32(dbi_reg_base + 0xa0);
	val = (val & 0xfffffff0) | pcie_gen_slt;
	mmio_write_32((dbi_reg_base + 0xa0), val);

	// config ecrc generation enable
	val = mmio_read_32(dbi_reg_base + 0x118);
	val = 0x3e0;
	mmio_write_32((dbi_reg_base + 0x118), val);
}

void pcie_enable_ltssm(uint32_t sys_id, uint32_t ctrl_id)
{
	uint32_t val = 0;
	uint64_t pcie_sii_base = 0;

	pcie_sii_base = PCIE_CFG_BASE(sys_id) + PCIE_SII_INTF_REG_BASE(ctrl_id);

	//config app_ltssm_enable
	val = mmio_read_32(pcie_sii_base + PCIE_SII_GENERAL_CTRL3_REG);
	val |= 0x1;
	mmio_write_32((pcie_sii_base + PCIE_SII_GENERAL_CTRL3_REG), val);
}

void pcie_wait_link(uint32_t sys_id, uint32_t ctrl_id)
{
	uint32_t val = 0;
	uint64_t pcie_sii_base = 0;

	pcie_sii_base = PCIE_CFG_BASE(sys_id) + PCIE_SII_INTF_REG_BASE(ctrl_id);

	do {
		udelay(10);
		val = mmio_read_32(pcie_sii_base + 0xb4); //LNK_DBG_2
		val = (val >> 7) & 0x1; //bit7, RDLH_LINK_UP
	} while (val != 1);
}

void pcie_check_link_status(uint32_t sys_id, uint32_t ctrl_id,
				uint32_t pcie_gen_slt, uint32_t pcie_ln_cnt)
{
	uint32_t val = 0;
	uint32_t speed = 0;
	uint32_t width = 0;
	uint32_t ltssm_state = 0;
	uint64_t pcie_sii_base = 0;
	uint64_t pcie_dbi_base = 0;

	pcie_sii_base = PCIE_CFG_BASE(sys_id) + PCIE_SII_INTF_REG_BASE(ctrl_id);

	if ((sys_id == PCIE_SUBSYS) && (ctrl_id == PCIE_CTRL_X16_0))
		pcie_dbi_base = PCIE_CFG_BASE(sys_id) + 0x01000000;
	else
		pcie_dbi_base = PCIE_CFG_BASE(sys_id) + PCIE_CTRL_DBI_REG_BASE(ctrl_id);

	val = mmio_read_32(pcie_sii_base + 0xb4); //LNK_DBG_2
	ltssm_state = val & 0x3f; //bit[5,0]
	if (ltssm_state != 0x11)
		pr_err("PCIe link fail, ltssm_state = 0x%x\n", ltssm_state);

	speed = (val >> 8) & 0x7; //bit[10,8]
	if ((speed + 1) != pcie_gen_slt)
		pr_err("link speed, expect gen%d, current gen%d\n", pcie_gen_slt, (speed + 1));

	val = mmio_read_32(pcie_dbi_base + 0x80);
	width = (val >> 20) & 0x3f; //bit[25:20]
	if (width != pcie_ln_cnt)
		pr_err("link width, expect x%d, current x%d\n", pcie_ln_cnt, width);

	pr_info("PCIe Link status, ltssm[0x%x], gen%d, x%d.\n", ltssm_state, (speed + 1), width);
}

void pcie_init_phy(uint32_t sys_id, uint32_t ctrl_id, uint32_t phy_id, uint32_t ss_mode)
{
	pr_info("%s:1\n", __func__);
	pcie_config_power_reset(sys_id, ctrl_id);
	pr_info("%s:2\n", __func__);
	pcie_config_phy_pwr_stable(sys_id, phy_id);
	pr_info("%s:3\n", __func__);
	pcie_config_ss_mode(sys_id, phy_id, ss_mode);
	pr_info("%s:4\n", __func__);
	pcie_config_phy_bl_bypass(sys_id, phy_id, PCIE_PHY_NO_SRAM_BYPASS);
	pr_info("%s:5\n", __func__);
	pcie_config_button_reset(sys_id, ctrl_id);
	pr_info("%s:6\n", __func__);
#ifdef PCIE_PLD_TEST
	udelay(10);
#else
	pr_info("%s:7\n", __func__);
	pcie_wait_sram_init_done(sys_id, phy_id);
	pr_info("%s:8\n", __func__);
	pcie_config_exload_phy(sys_id, phy_id);
	pr_info("%s:9\n", __func__);
	if (ss_mode == 0) {
		pcie_wait_sram_init_done(sys_id, 1);
		pr_info("%s:10\n", __func__);
		pcie_config_exload_phy(sys_id, 1);
	}
	pr_info("%s:11\n", __func__);
#endif
}

void pcie_switch_to_cs2(uint32_t sys_id, uint32_t ctrl_id, uint32_t dbi_cs2)
{
	uint32_t val = 0;
	uint64_t ctrl_reg_base = 0;

	ctrl_reg_base = PCIE_CFG_BASE(sys_id) + PCIE_CTRL_INTF_REG_BASE(ctrl_id);

	val = mmio_read_32(ctrl_reg_base + PCIE_CTRL_DBI_CDM_ADDR_REG);
	val &= (~PCIE_CTRL_CS2_CDM_ADDR_HIGH_RD_X16_MASK);
	val &= (~PCIE_CTRL_CS2_CDM_ADDR_HIGH_WR_X16_MASK);
	if (dbi_cs2 == PCIE_CONFIG_DBI) {
		val |= 0x0;
		val |= (0x0 << 16);
	} else {
		val |= 0x100;
		val |= (0x100 << 16);
	}
	mmio_write_32((ctrl_reg_base + PCIE_CTRL_DBI_CDM_ADDR_REG), val);
	//clear PASID
	mmio_write_32((ctrl_reg_base + 0x104), 0x0);
	mmio_write_32((ctrl_reg_base + 0x108), 0x0);
}

void pcie_config_atu_access_addr(uint32_t sys_id, uint32_t ctrl_id, uint32_t atu_elbi)
{
	uint32_t val = 0;
	uint64_t ctrl_reg_base = 0;

	ctrl_reg_base = PCIE_CFG_BASE(sys_id) + PCIE_CTRL_INTF_REG_BASE(ctrl_id);

	val = mmio_read_32(ctrl_reg_base + PCIE_CTRL_DBI_ATU_ADDR_REG);
	val &= (~PCIE_CTRL_DBI_ATU_ADDR_HIGH_RD_X16_MASK);
	val &= (~PCIE_CTRL_DBI_ATU_ADDR_HIGH_WR_X16_MASK);
	if (atu_elbi == PCIE_CONFIG_ATU) {
		val |= 0x300;
		val |= (0x300 << 16);
	} else {
		val |= 0x200;
		val |= (0x200 << 16);
	}
	mmio_write_32((ctrl_reg_base + PCIE_CTRL_DBI_ATU_ADDR_REG), val);
	mmio_write_32((ctrl_reg_base + 0x104), 0x0);
	mmio_write_32((ctrl_reg_base + 0x108), 0x0);

}

void pcie_config_slv_range(uint32_t sys_id, uint32_t ctrl_id)
{
	uint64_t reg_base = 0;

	if (sys_id == PCIE_SUBSYS) {
		if (ctrl_id == 5)
			ctrl_id = 0;
		else
			ctrl_id = ctrl_id + 1;
	}
	reg_base = PCIE_CFG_BASE(sys_id) + PCIE_TOP_SYS_REG_BASE(ctrl_id);

	if (sys_id == PCIE_SUBSYS) {
		mmio_write_32((reg_base + PCIE_TOP_SYS_AXI0_32BAR_START_ADDR_REG), 0x40000000);
		mmio_write_32((reg_base + PCIE_TOP_SYS_AXI0_32BAR_END_ADDR_REG), 0x7fffffff);

		mmio_write_32((reg_base + PCIE_TOP_SYS_AXI0_BAR_START_LOW_ADDR_REG), 0x0);
		mmio_write_32((reg_base + PCIE_TOP_SYS_AXI0_BAR_START_HIGH_ADDR_REG), 0x80);
		mmio_write_32((reg_base + PCIE_TOP_SYS_AXI0_BAR_END_LOW_ADDR_REG), 0xffffffff);
		mmio_write_32((reg_base + PCIE_TOP_SYS_AXI0_BAR_END_HIGH_ADDR_REG), 0x83);
	} else {
		mmio_write_32((reg_base + SSPERI_TOP_SYS_AXI0_32BAR_START_ADDR_REG), 0x20000000);
		mmio_write_32((reg_base + SSPERI_TOP_SYS_AXI0_32BAR_END_ADDR_REG), 0x3fffffff);

		mmio_write_32((reg_base + SSPERI_TOP_SYS_AXI0_BAR_START_LOW_ADDR_REG), 0x0);
		mmio_write_32((reg_base + SSPERI_TOP_SYS_AXI0_BAR_START_HIGH_ADDR_REG), 0x60);
		mmio_write_32((reg_base + SSPERI_TOP_SYS_AXI0_BAR_END_LOW_ADDR_REG), 0xffffffff);
		mmio_write_32((reg_base + SSPERI_TOP_SYS_AXI0_BAR_END_HIGH_ADDR_REG), 0x63);
	}
}


void pcie_resize_bar0(uint32_t sys_id, uint32_t ctrl_id)
{
	uint32_t val = 0;
	uint64_t dbi_reg_base = 0;

	if (ctrl_id == 5)
		dbi_reg_base = PCIE_CFG_BASE(sys_id) + PCIE_X16_DBI_OFFSET;
	else
		dbi_reg_base = PCIE_CFG_BASE(sys_id) + PCIE_CTRL_DBI_REG_BASE(ctrl_id);

	/* enable DBI_RO_WR_EN */
	val = mmio_read_32(dbi_reg_base + 0x8bc);
	val = (val & 0xfffffffe) | 0x1;
	mmio_write_32((dbi_reg_base + 0x8bc), val);

	pr_info("before reg bar0 value is: 0x%x\n", mmio_read_32((dbi_reg_base) + 0x10));
	pr_info("before reg bar1 value is: 0x%x\n", mmio_read_32((dbi_reg_base) + 0x14));
	/* resize bar0 */
	mmio_write_32(dbi_reg_base + 0x10, 0x0);
	pcie_switch_to_cs2(sys_id, ctrl_id, PCIE_CONFIG_CS2);
	mmio_write_32(dbi_reg_base + 0x10, 0xffffff);
	pcie_switch_to_cs2(sys_id, ctrl_id, PCIE_CONFIG_DBI);

	pr_info("after reg bar0 value is: 0x%x\n", mmio_read_32((dbi_reg_base) + 0x10));
	pr_info("after reg bar1 value is: 0x%x\n", mmio_read_32((dbi_reg_base) + 0x14));

	/* disable DBI_RO_WR_EN */
	val = mmio_read_32(dbi_reg_base + 0x8bc);
	val &= 0xfffffffe;
	mmio_write_32((dbi_reg_base + 0x8bc), val);
}

void pcie_config_bar0_bar1_atu(uint32_t sys_id, uint32_t ctrl_id)
{
	uint64_t atu_reg_base = 0;
	uint64_t cfg_reg_addr = 0;

	if ((sys_id == PCIE_SUBSYS) && (ctrl_id == 5)) {
		atu_reg_base = PCIE_CFG_BASE(sys_id) + PCIE_X16_ATU_OFFSET;
		cfg_reg_addr = PCIE_CFG_BASE(sys_id) + PCIE_X16_DBI_OFFSET;
	}  else {
		atu_reg_base = PCIE_CFG_BASE(sys_id) + PCIE_CTRL_ATU_REG_BASE(ctrl_id);
		cfg_reg_addr = PCIE_CFG_BASE(sys_id) + PCIE_CTRL_DBI_REG_BASE(ctrl_id);
	}

	mmio_write_32((atu_reg_base + 0x114), (cfg_reg_addr & 0xffffffff));
	mmio_write_32((atu_reg_base + 0x118), 0x50);
	mmio_write_32((atu_reg_base + 0x100), 0x0);
	mmio_write_32((atu_reg_base + 0x104), 0xC0080000);
}

void pcie_config_mps(uint32_t sys_id, uint32_t ctrl_id, uint32_t mps)
{
	uint32_t val = 0;
	uint64_t dbi_reg_base = 0;

	if ((sys_id == PCIE_SUBSYS) && (ctrl_id == 5))
		dbi_reg_base = PCIE_CFG_BASE(sys_id) + PCIE_X16_DBI_OFFSET;
	else
		dbi_reg_base = PCIE_CFG_BASE(sys_id) + PCIE_CTRL_DBI_REG_BASE(ctrl_id);

	val = mmio_read_32(dbi_reg_base + 0x78);
	val &= 0xffffff1f;
	val |= ((mps & 0x7) << 5);
	mmio_write_32((dbi_reg_base + 0x78), val);
}

void pcie_config_mrrs(uint32_t sys_id, uint32_t ctrl_id, uint32_t mrrs)
{
	uint32_t val = 0;
	uint64_t dbi_reg_base = 0;

	if ((sys_id == PCIE_SUBSYS) && (ctrl_id == 5))
		dbi_reg_base = PCIE_CFG_BASE(sys_id) + PCIE_X16_DBI_OFFSET;
	else
		dbi_reg_base = PCIE_CFG_BASE(sys_id) + PCIE_CTRL_DBI_REG_BASE(ctrl_id);

	val = mmio_read_32(dbi_reg_base + 0x78);
	val &= 0xffff8fff;
	val |= ((mrrs & 0x7) << 12);
	mmio_write_32((dbi_reg_base + 0x78), val);
}

void pcie_clear_pasid(uint32_t sys_id, uint32_t ctrl_id)
{
	uint64_t ctrl_reg_base = 0;

	ctrl_reg_base = PCIE_CFG_BASE(sys_id) + PCIE_CTRL_INTF_REG_BASE(ctrl_id);

	//clear PASID
	mmio_write_32((ctrl_reg_base + 0x104), 0x0);
	mmio_write_32((ctrl_reg_base + 0x108), 0x0);
}

void pcie_init(uint32_t sys_id, uint32_t ctrl_id, uint32_t phy_id,
		uint32_t ss_mode, uint32_t dev_type, uint32_t pcie_gen_slt, uint32_t pcie_ln_cnt)
{
	pr_info("sys %d, ctrl %d, dev_type = %d\n", sys_id, ctrl_id, dev_type);
	pr_info("ss_mode %d, gen%d, x%d\n", ss_mode, pcie_gen_slt, pcie_ln_cnt);

	pcie_init_phy(sys_id, ctrl_id, phy_id, ss_mode);

	pcie_wait_core_clk(sys_id, ctrl_id);
	pcie_config_ctrl(sys_id, ctrl_id, pcie_gen_slt, dev_type);
	if (sys_id != SSPERI_SUBSYS)
		pcie_config_eq(sys_id, ctrl_id, pcie_gen_slt, pcie_ln_cnt, dev_type);
	pcie_config_link(sys_id, ctrl_id, pcie_ln_cnt);
	if (dev_type == PCIE_DEV_TYPE_EP) {
		if ((sys_id == 0) && (ctrl_id == 5))
			pcie_resize_bar0(sys_id, ctrl_id);
		pcie_config_bar0_bar1_atu(sys_id, ctrl_id);
		pcie_clear_pasid(sys_id, ctrl_id);
	}
	pcie_enable_ltssm(sys_id, ctrl_id);
	pcie_wait_link(sys_id, ctrl_id);
	udelay(200);
	pcie_check_link_status(sys_id, ctrl_id, pcie_gen_slt, pcie_ln_cnt);
	if (dev_type == PCIE_DEV_TYPE_RC) {
		pcie_config_atu_access_addr(sys_id, ctrl_id, PCIE_CONFIG_ATU);
		pcie_config_slv_range(sys_id, ctrl_id);
	}
	pcie_config_mps(sys_id, ctrl_id, 1);
	pcie_config_mrrs(sys_id, ctrl_id, 1);
}

void sg2380_pcie_init(void)
{
	//pcie_init(1, 0, 0, SSPERI_SERDES_PCIE_PROTOCOL, PCIE_DEV_TYPE_RC,
	//          PCIE_GEN_1, PCIE_LANE_WIDTH_X1);
	//pcie_init(1, 4, 1, SSPERI_SERDES_PCIE_PROTOCOL, PCIE_GEN_3, PCIE_LANE_WIDTH_X1);
	pcie_init(0, 5, 0, PCIE_SERDES_MODE_1_LINK, PCIE_DEV_TYPE_RC,
		  PCIE_GEN_4, PCIE_LANE_WIDTH_X16);
	//pcie_init(0, 4, 0, PCIE_SERDES_MODE_6_LINK, PCIE_GEN_4, PCIE_LANE_WIDTH_X4);
	//pcie_init(0, 0, 0, PCIE_SERDES_MODE_6_LINK, PCIE_GEN_4, PCIE_LANE_WIDTH_X2);
	pr_info("%s, done\n", __func__);
}


void sg2380_pcie_cdma_test(uint32_t sys_id, uint32_t cdma_id, uint32_t mode,
			uint32_t length, uint64_t src_addr, uint64_t dst_addr)
{
	uint32_t val = 0;
	uint32_t val_1 = 0;
	uint32_t timer = 0;
	uint32_t cdma1_enable = 0;
	uint64_t cdma_base = 0x0;
	uint64_t cdma_base1 = 0x0;
	uint64_t tick00 = 0;
	uint64_t tick01 = 0;
	uint64_t tick10 = 0;
	uint64_t tick11 = 0;
	uint64_t time = 0;
	uint64_t time1 = 0;
	uint64_t bw = 0;
	uint64_t bw1 = 0;

	if (mode > 2)
		return;

	pr_info("%s, src addr = 0x%llx, dst addr = 0x%llx\n", __func__, src_addr, dst_addr);
	cdma_base = PCIE_CFG_BASE(sys_id) + PCIE_CDMA_REG_BASE(cdma_id);
	cdma_base1 = PCIE_CFG_BASE(sys_id) + PCIE_CDMA_REG_BASE(1);
	// 1. bit[0], --enable cdma, bit[3] -- enable int;
	mmio_write_32((cdma_base + PCIE_CDMA_CHL_CTRL_REG), 0x9);
	mmio_write_32((cdma_base + PCIE_CDMA_INT_MASK_REG), 0x0);
	//mps 256 bytes
	mmio_write_32((cdma_base + PCIE_CDMA_MAX_PAYLOAD_REG), 0x9);

	// 2. cfg descriptor
	mmio_write_32((cdma_base + PCIE_CDMA_DESC_R878_REG), 0x8);

	if (cdma1_enable == 1) {
		// 1. bit[0], --enable cdma, bit[3] -- enable int;
		mmio_write_32((cdma_base1 + PCIE_CDMA_CHL_CTRL_REG), 0x9);
		mmio_write_32((cdma_base1 + PCIE_CDMA_INT_MASK_REG), 0x0);
		//mps 256 bytes
		mmio_write_32((cdma_base1 + PCIE_CDMA_MAX_PAYLOAD_REG), 0x9);

		// 2. cfg descriptor
		mmio_write_32((cdma_base1 + PCIE_CDMA_DESC_R878_REG), 0x8);
		length = length >> 1;
	}

	if (mode == 0) {
		// D2D
		pr_info("D2D test length 0x%x\n", length);
		if (sys_id == 0) {
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_SRC_ADDR_HI_REG),
				      (0x80000000 | (src_addr >> 32)));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_SRC_ADDR_LO_REG),
				      (src_addr & 0xffffffff));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_DST_ADDR_HI_REG),
				      (0x80000000 | (dst_addr >> 32)));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_DST_ADDR_LO_REG),
				      (dst_addr & 0xffffffff));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_1D_DATA_LEGTH_REG), length);
		} else {
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_SRC_ADDR_HI_REG),
				      (0x90000000 | (src_addr >> 32)));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_SRC_ADDR_LO_REG),
				      (src_addr & 0xffffffff));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_DST_ADDR_HI_REG),
				      (0x90000000 | (dst_addr >> 32)));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_DST_ADDR_LO_REG),
				      (dst_addr & 0xffffffff));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_1D_DATA_LEGTH_REG), length);
		}
	} else if (mode == 1) {
		//D2S
		pr_info("D2S tesst, length = 0x%x\n", length);
		if (sys_id == 0) {
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_SRC_ADDR_HI_REG),
				      (0x80000000 | (src_addr >> 32)));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_SRC_ADDR_LO_REG),
				      (src_addr & 0xffffffff));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_DST_ADDR_HI_REG),
				      (0x90000000 | (dst_addr >> 32)));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_DST_ADDR_LO_REG),
				      (dst_addr & 0xffffffff));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_1D_DATA_LEGTH_REG), length);
		} else {
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_SRC_ADDR_HI_REG),
				      (0x90000000 | (src_addr >> 32)));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_SRC_ADDR_LO_REG),
				      (src_addr & 0xffffffff));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_DST_ADDR_HI_REG),
				      (0x00000000 | (dst_addr >> 32)));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_DST_ADDR_LO_REG),
				      (dst_addr & 0xffffffff));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_1D_DATA_LEGTH_REG), length);
		}
	} else {
		//S2D
		pr_info("S2D tesst, length = 0x%x\n", length);
		if (sys_id == 0) {
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_SRC_ADDR_HI_REG),
				      (0x90000000 | (src_addr >> 32)));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_SRC_ADDR_LO_REG),
				      (src_addr & 0xffffffff));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_DST_ADDR_HI_REG),
				      (0x80000000 | (dst_addr >> 32)));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_DST_ADDR_LO_REG),
				      (dst_addr & 0xffffffff));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_1D_DATA_LEGTH_REG), length);
		} else {
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_SRC_ADDR_HI_REG),
				      (0x00000000 | (src_addr >> 32)));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_SRC_ADDR_LO_REG),
				      (src_addr & 0xffffffff));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_DST_ADDR_HI_REG),
				      (0x90000000 | (dst_addr >> 32)));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_DST_ADDR_LO_REG),
				      (dst_addr & 0xffffffff));
			mmio_write_32((cdma_base + PCIE_CDMA_DESC_1D_DATA_LEGTH_REG), length);
		}
	}
	if (cdma1_enable == 1) {
		if (mode == 0) {
			// D2D
			pr_info("CDMA 1 D2D test length 0x%x\n", length);
			mmio_write_32((cdma_base1 + PCIE_CDMA_DESC_SRC_ADDR_HI_REG),
				      (0x80000000 | (src_addr >> 32)));
			mmio_write_32((cdma_base1 + PCIE_CDMA_DESC_SRC_ADDR_LO_REG),
				      ((src_addr & 0xffffffff) + length));
			mmio_write_32((cdma_base1 + PCIE_CDMA_DESC_DST_ADDR_HI_REG),
				      (0x80000000 | (dst_addr >> 32)));
			mmio_write_32((cdma_base1 + PCIE_CDMA_DESC_DST_ADDR_LO_REG),
				      ((dst_addr & 0xffffffff) + length));
			mmio_write_32((cdma_base1 + PCIE_CDMA_DESC_1D_DATA_LEGTH_REG), length);
		} else if (mode == 1) {
			//D2S
			pr_info("CDMA 1 D2S test, length = 0x%x\n", length);
			mmio_write_32((cdma_base1 + PCIE_CDMA_DESC_SRC_ADDR_HI_REG),
				      (0x80000000 | (src_addr >> 32)));
			mmio_write_32((cdma_base1 + PCIE_CDMA_DESC_SRC_ADDR_LO_REG),
				      ((src_addr & 0xffffffff) + length));
			mmio_write_32((cdma_base1 + PCIE_CDMA_DESC_DST_ADDR_HI_REG),
				      (0xa0000000 | (dst_addr >> 32)));
			mmio_write_32((cdma_base1 + PCIE_CDMA_DESC_DST_ADDR_LO_REG),
				      ((dst_addr & 0xffffffff) + length));
			mmio_write_32((cdma_base1 + PCIE_CDMA_DESC_1D_DATA_LEGTH_REG), length);
		} else {
			//S2D
			pr_info("CDMA 1 S2D test, length = 0x%x\n", length);
			mmio_write_32((cdma_base1 + PCIE_CDMA_DESC_SRC_ADDR_HI_REG),
				      (0xa0000000 | (src_addr >> 32)));
			mmio_write_32((cdma_base1 + PCIE_CDMA_DESC_SRC_ADDR_LO_REG),
				      ((src_addr & 0xffffffff) + length));
			mmio_write_32((cdma_base1 + PCIE_CDMA_DESC_DST_ADDR_HI_REG),
				      (0x80000000 | (dst_addr >> 32)));
			mmio_write_32((cdma_base1 + PCIE_CDMA_DESC_DST_ADDR_LO_REG),
				      ((dst_addr & 0xffffffff) + length));
			mmio_write_32((cdma_base1 + PCIE_CDMA_DESC_1D_DATA_LEGTH_REG), length);
		}
	}

	if (cdma1_enable == 0) {
		val = mmio_read_32(cdma_base + PCIE_CDMA_DESC_R878_REG);
		val |= 0x1;
		mmio_write_32((cdma_base + PCIE_CDMA_DESC_R878_REG), val);
	} else {
		val = mmio_read_32(cdma_base + PCIE_CDMA_DESC_R878_REG);
		val |= 0x1;
		mmio_write_32((cdma_base + PCIE_CDMA_DESC_R878_REG), val);
		if (sys_id == 0) {
			val_1 = mmio_read_32(cdma_base1 + PCIE_CDMA_DESC_R878_REG);
			val_1 |= 0x1;
			mmio_write_32((cdma_base1 + PCIE_CDMA_DESC_R878_REG), val_1);
		}
	}
	tick00 = timer_get_tick();
	tick10 = tick00;
	//pr_info("wait int\n");
	do {
		//udelay(1);
		timer++;
		val = mmio_read_32(cdma_base + PCIE_CDMA_INT_STATUS_REG);
		val &= 0x1; //bit1, desc finished
	} while (val != 1);
	tick01 = timer_get_tick();
	if (cdma1_enable == 1) {
		//pr_info("wait cdma1 int\n");
		do {
			//udelay(1);
			timer++;
			val_1 = mmio_read_32(cdma_base1 + PCIE_CDMA_INT_STATUS_REG);
			val_1 &= 0x1; //bit1, desc finished
		} while (val_1 != 1);
	}
	tick11 = timer_get_tick();
	// 4. W1C
	mmio_write_32((cdma_base + PCIE_CDMA_INT_STATUS_REG), 0x1);

	// 5. disable cdma
	mmio_write_32((cdma_base + PCIE_CDMA_CHL_CTRL_REG), 0x0);

	if (cdma1_enable == 1) {
		// 4. W1C
		mmio_write_32((cdma_base1 + PCIE_CDMA_INT_STATUS_REG), 0x1);
		// 5. disable cdma
		mmio_write_32((cdma_base1 + PCIE_CDMA_CHL_CTRL_REG), 0x0);
	}
	pr_info("tick num: %llx - %llx; %llx - %llx\n", tick00, tick01, tick10, tick11);
	time = timer_tick2us((tick01 - tick00));
	time1 = timer_tick2us((tick11 - tick10));
	bw = length / time;
	bw1 = length / time1;
	pr_info("pcie bw test:cdma0 - 0x%llx - 0x%llx; cdma1 - 0x%llx - 0x%llx\n",
		 time, bw, time1, bw1);
	pr_info("cdma finished!, timer = %d\n", timer);
}

void sg2380_pcie_cdma1_test(uint32_t sys_id, uint32_t cdma_id, uint32_t mode,
			uint32_t length, uint64_t src_addr, uint64_t dst_addr)
{
	uint32_t val = 0;
	uint32_t timer = 0;
	uint64_t cdma_base = 0x0;
	uint64_t tick00 = 0;
	uint64_t tick01 = 0;
	uint64_t time = 0;
	uint64_t bw = 0;

	if (mode > 2)
		return;

	pr_info("%s, src addr = 0x%llx, dst addr = 0x%llx\n", __func__, src_addr, dst_addr);
	cdma_base = PCIE_CFG_BASE(sys_id) + PCIE_CDMA_REG_BASE(cdma_id);
	// 1. bit[0], --enable cdma, bit[3] -- enable int;
	mmio_write_32((cdma_base + PCIE_CDMA_CHL_CTRL_REG), 0x9);
	mmio_write_32((cdma_base + PCIE_CDMA_INT_MASK_REG), 0x0);
	//mps 256 bytes
	mmio_write_32((cdma_base + PCIE_CDMA_MAX_PAYLOAD_REG), 0x9);

	// 2. cfg descriptor
	mmio_write_32((cdma_base + PCIE_CDMA_DESC_R878_REG), 0x8);

	if (mode == 0) {
		// D2D
		pr_info("D2D test length 0x%x\n", length);
		mmio_write_32((cdma_base + PCIE_CDMA_DESC_SRC_ADDR_HI_REG),
			      (0x80000000 | (src_addr >> 32)));
		mmio_write_32((cdma_base + PCIE_CDMA_DESC_SRC_ADDR_LO_REG),
			      (src_addr & 0xffffffff));
		mmio_write_32((cdma_base + PCIE_CDMA_DESC_DST_ADDR_HI_REG),
			      (0x80000000 | (dst_addr >> 32)));
		mmio_write_32((cdma_base + PCIE_CDMA_DESC_DST_ADDR_LO_REG),
			      (dst_addr & 0xffffffff));
		mmio_write_32((cdma_base + PCIE_CDMA_DESC_1D_DATA_LEGTH_REG), length);
	} else if (mode == 1) {
		//D2S
		pr_info("D2S tesst, length = 0x%x\n", length);
		mmio_write_32((cdma_base + PCIE_CDMA_DESC_SRC_ADDR_HI_REG),
			      (0x80000000 | (src_addr >> 32)));
		mmio_write_32((cdma_base + PCIE_CDMA_DESC_SRC_ADDR_LO_REG),
			      (src_addr & 0xffffffff));
		mmio_write_32((cdma_base + PCIE_CDMA_DESC_DST_ADDR_HI_REG),
			      (0x90000000 | (dst_addr >> 32)));
		mmio_write_32((cdma_base + PCIE_CDMA_DESC_DST_ADDR_LO_REG),
			      (dst_addr & 0xffffffff));
		mmio_write_32((cdma_base + PCIE_CDMA_DESC_1D_DATA_LEGTH_REG), length);
	} else {
		//S2D
		pr_info("S2D tesst, length = 0x%x\n", length);
		mmio_write_32((cdma_base + PCIE_CDMA_DESC_SRC_ADDR_HI_REG),
			      (0x90000000 | (src_addr >> 32)));
		mmio_write_32((cdma_base + PCIE_CDMA_DESC_SRC_ADDR_LO_REG),
			      (src_addr & 0xffffffff));
		mmio_write_32((cdma_base + PCIE_CDMA_DESC_DST_ADDR_HI_REG),
			      (0x80000000 | (dst_addr >> 32)));
		mmio_write_32((cdma_base + PCIE_CDMA_DESC_DST_ADDR_LO_REG),
			      (dst_addr & 0xffffffff));
		mmio_write_32((cdma_base + PCIE_CDMA_DESC_1D_DATA_LEGTH_REG), length);
	}

	val = mmio_read_32(cdma_base + PCIE_CDMA_DESC_R878_REG);
	val |= 0x1;
	mmio_write_32((cdma_base + PCIE_CDMA_DESC_R878_REG), val);

	tick00 = timer_get_tick();
	//pr_info("wait int\n");
	do {
		//udelay(1);
		timer++;
		val = mmio_read_32(cdma_base + PCIE_CDMA_INT_STATUS_REG);
		val &= 0x1; //bit1, desc finished
	} while (val != 1);
	tick01 = timer_get_tick();

	// 4. W1C
	mmio_write_32((cdma_base + PCIE_CDMA_INT_STATUS_REG), 0x1);

	// 5. disable cdma
	mmio_write_32((cdma_base + PCIE_CDMA_CHL_CTRL_REG), 0x0);

	pr_info("tick num: %llx - %llx\n", tick00, tick01);
	time = timer_tick2us((tick01 - tick00));
	bw = length / time;
	pr_info("pcie bw test:cdma%d - 0x%llx - 0x%llx\n", cdma_id, time, bw);
	pr_info("cdma finished!, timer = %d\n", timer);
}

