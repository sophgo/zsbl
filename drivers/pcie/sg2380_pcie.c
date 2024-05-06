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

	if (sys_id == PCIE_SUBSYS)
		reg_base = PCIE_CFG_BASE(sys_id) + PCIE_CTRL_INTF_REG_BASE(ctrl_id);
	else
		reg_base = PCIE_CFG_BASE(sys_id) + SSPERI_CTRL_INTF_REG_BASE(ctrl_id);

	//cfg button rstn
	val = mmio_read_32(reg_base + PCIE_CTRL_RST_CTRL_0_REG);
	val |= (1 << PCIE_CTRL_RST_CTRL_0_BUTTON_RSTN_BIT);
	mmio_write_32((reg_base + PCIE_CTRL_RST_CTRL_0_REG), val);
}

static void pcie_config_power_reset(uint32_t sys_id, uint32_t ctrl_id)
{
	uint32_t val = 0;
	uint64_t reg_base = 0;

	if (sys_id == PCIE_SUBSYS)
		reg_base = PCIE_CFG_BASE(sys_id) + PCIE_CTRL_INTF_REG_BASE(ctrl_id);
	else
		reg_base = PCIE_CFG_BASE(sys_id) + SSPERI_CTRL_INTF_REG_BASE(ctrl_id);

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

	if (sys_id == PCIE_SUBSYS)
		base_addr = PCIE_PHY_INTF_REG_BASE;
	else
		base_addr = PCIE_CFG_BASE(sys_id) + SSPERI_PHY_INTF_REG_BASE(phy_id);

	//default  pciex8 need cfg
	//cfg  phy0_upcs_pwr_stable
	val = mmio_read_32(base_addr + PHY_INTF_REG_RX400);
	val |= (0x1 << PHY_INTF_REG_RX000_UPCS_PWR_STABLE_BIT);
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

}

void pcie_config_ss_mode(uint32_t sys_id, uint32_t phy_id, uint32_t ss_mode)
{
	uint32_t val = 0;
	uint64_t base_addr = 0;

	if (sys_id == PCIE_SUBSYS)
		base_addr = PCIE_PHY_INTF_REG_BASE;
	else
		base_addr = PCIE_CFG_BASE(sys_id) + SSPERI_PHY_INTF_REG_BASE(phy_id);

	val = mmio_read_32(base_addr + PHY_INTF_REG_RX410);
	val &= (~PHY_INTF_REG_RX410_SS_MODE_MASK);
	val |= ss_mode & PHY_INTF_REG_RX410_SS_MODE_MASK;
	mmio_write_32((base_addr + PHY_INTF_REG_RX410), val);
}

void pcie_config_phy_bl_bypass(uint32_t sys_id, uint32_t phy_id,
			uint32_t sram_bypass_mode)
{
	uint32_t val = 0;
	uint64_t reg_base = 0;
	uint32_t reg_addr = 0;

	if (sys_id == PCIE_SUBSYS)
		reg_base = PCIE_PHY_INTF_REG_BASE;
	else
		reg_base = PCIE_CFG_BASE(sys_id) + SSPERI_PHY_INTF_REG_BASE(phy_id);

	reg_addr = PHY_INTF_REG_RX000 + (phy_id * 0x100);

	//cfg phy0 phy0_sram_bypass bypass
	val = mmio_read_32(reg_base + reg_addr);
	val &= (~PHY_INTF_REG_RX000_PHY_X_SRAM_BYPASS_MASK);
	val |= (sram_bypass_mode << PHY_INTF_REG_RX000_PHY_X_SRAM_BYPASS_BIT);
	mmio_write_32((reg_base + reg_addr), val);
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

	if (sys_id == PCIE_SUBSYS)
		pcie_sii_base = PCIE_CFG_BASE(sys_id) + PCIE_SII_INTF_REG_BASE(ctrl_id);
	else
		pcie_sii_base = PCIE_CFG_BASE(sys_id) + SSPERI_SII_INTF_REG_BASE(ctrl_id);

	do {
		udelay(10);
		val = mmio_read_32(pcie_sii_base + 0x5c); //GEN_CTRL_4
		val = (val >> 8) & 0x1; //bit8, pcie_rst_n
	} while (val != 1);
}

void pcie_config_link(uint32_t sys_id, uint32_t ctrl_id, uint32_t pcie_ln_cnt)
{
	uint32_t val = 0;
	uint64_t base_addr = 0;

	base_addr = PCIE_CFG_BASE(sys_id) + PCIE_CTRL_DBI_REG_BASE(ctrl_id);

	//config lane_count
	val = mmio_read_32(base_addr + 0x8c0);
	val = (val & 0xffffffc0) | pcie_ln_cnt;
	mmio_write_32((base_addr + 0x8c0), val);

	//config eq bypass highest rate disable
	val = mmio_read_32(base_addr + 0x1c0);
	val |= 0x1;
	mmio_write_32((base_addr + 0x1c0), val);
}

void pcie_config_ctrl(uint32_t sys_id, uint32_t ctrl_id,
			uint32_t pcie_gen_slt, uint32_t pcie_dev_type)
{
	uint32_t val = 0;
	uint64_t sii_reg_base = 0;
	uint64_t dbi_reg_base = 0;

	if (sys_id == PCIE_SUBSYS)
		sii_reg_base = PCIE_CFG_BASE(sys_id) + PCIE_SII_INTF_REG_BASE(ctrl_id);
	else
		sii_reg_base = PCIE_CFG_BASE(sys_id) + SSPERI_SII_INTF_REG_BASE(ctrl_id);

	dbi_reg_base = PCIE_CFG_BASE(sys_id) + PCIE_CTRL_DBI_REG_BASE(ctrl_id);

	//config device_type
	val = mmio_read_32(sii_reg_base + PCIE_SII_GENERAL_CTRL1_REG);
	val &= (~PCIE_SII_GENERAL_CTRL1_DEVICE_TYPE_MASK);
	val |= (pcie_dev_type << 9);
	mmio_write_32((sii_reg_base + PCIE_SII_GENERAL_CTRL1_REG), val);

	//config Directed Speed Change	Writing '1' to this field instructs the LTSSM to initiate
	//a speed change to Gen2 or Gen3 after the link is initialized at Gen1 speed
	val = mmio_read_32(dbi_reg_base + 0x80c);
	val = val | 0x20000;
	mmio_write_32((dbi_reg_base + 0x80c), val);

	//config generation_select-pcie_cap_target_link_speed
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

	if (sys_id == PCIE_SUBSYS)
		pcie_sii_base = PCIE_CFG_BASE(sys_id) + PCIE_SII_INTF_REG_BASE(ctrl_id);
	else
		pcie_sii_base = PCIE_CFG_BASE(sys_id) + SSPERI_SII_INTF_REG_BASE(ctrl_id);

	//config app_ltssm_enable
	val = mmio_read_32(pcie_sii_base + PCIE_SII_GENERAL_CTRL3_REG);
	val |= 0x1;
	mmio_write_32((pcie_sii_base + PCIE_SII_GENERAL_CTRL3_REG), val);
}

void pcie_wait_link(uint32_t sys_id, uint32_t ctrl_id)
{
	uint32_t val = 0;
	uint64_t pcie_sii_base = 0;

	if (sys_id == PCIE_SUBSYS)
		pcie_sii_base = PCIE_CFG_BASE(sys_id) + PCIE_SII_INTF_REG_BASE(ctrl_id);
	else
		pcie_sii_base = PCIE_CFG_BASE(sys_id) + SSPERI_SII_INTF_REG_BASE(ctrl_id);

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

	if (sys_id == PCIE_SUBSYS)
		pcie_sii_base = PCIE_CFG_BASE(sys_id) + PCIE_SII_INTF_REG_BASE(ctrl_id);
	else
		pcie_sii_base = PCIE_CFG_BASE(sys_id) + SSPERI_SII_INTF_REG_BASE(ctrl_id);

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
	pcie_config_ss_mode(sys_id, phy_id, ss_mode);
	pcie_config_phy_bl_bypass(sys_id, phy_id, PCIE_PHY_NO_SRAM_BYPASS);
	pcie_config_button_reset(sys_id, ctrl_id);
	pcie_config_power_reset(sys_id, ctrl_id);
	pcie_config_phy_pwr_stable(sys_id, phy_id);
#ifdef PCIE_PLD_TEST
	udelay(10);
#else
	pcie_wait_sram_init_done(sys_id, phy_id);
	pcie_config_exload_phy(sys_id, phy_id);
	if (ss_mode == 0) {
		pcie_wait_sram_init_done(sys_id, 1);
		pcie_config_exload_phy(sys_id, 1);
	}
#endif
}

void pcie_init(uint32_t sys_id, uint32_t ctrl_id, uint32_t phy_id,
		uint32_t ss_mode, uint32_t pcie_gen_slt, uint32_t pcie_ln_cnt)
{
	pr_info("sys %d, ctrl %d,\n", sys_id, ctrl_id);
	pr_info("ss_mode %d, gen%d, x%d\n", ss_mode, pcie_gen_slt, pcie_ln_cnt);

	pcie_init_phy(sys_id, ctrl_id, phy_id, ss_mode);

	pcie_wait_core_clk(sys_id, ctrl_id);
	pcie_config_ctrl(sys_id, ctrl_id, pcie_gen_slt, PCIE_DEV_TYPE_RC);
	//pcie_config_eq(sys_id, ctrl_id);
	pcie_config_link(sys_id, ctrl_id, pcie_ln_cnt);
	//pcie_config_rc_cap(sys_id, ctrl_id pcie_ln_cnt);
	pcie_enable_ltssm(sys_id, ctrl_id);
	pcie_wait_link(sys_id, ctrl_id);
	udelay(200);
	pcie_check_link_status(sys_id, ctrl_id, pcie_gen_slt, pcie_ln_cnt);
	//pcie_config_axi_route(c2c_id, SOC_WORK_MODE_SERVER);
	//pcie_config_mps(c2c_id, wrapper_id, phy_id, 1);
	//pcie_config_mrrs(c2c_id, wrapper_id, phy_id, 1);
	//pcie_clear_slv_mapping(c2c_id, wrapper_id, SOC_WORK_MODE_SERVER);
	//pcie_config_slv_mapping(c2c_id, wrapper_id, phy_id, SOC_WORK_MODE_SERVER);
	//pcie_config_wrapper(c2c_id, wrapper_id);
}

void sg2380_pcie_init(void)
{
	pcie_init(0, 6, 0, PCIE_SERDES_MODE_1_LINK, PCIE_GEN_4, PCIE_LANE_WIDTH_X16);
	pr_info("%s, done\n", __func__);
}
