/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2021-2025, Sophgo Limited and Contributors. All rights reserved.
 *
 */


#ifndef SG2380_PCIE_H
#define SG2380_PCIE_H

#include <stdint.h>
#include <lib/utils_def.h>

#define PCIE_SUBSYS_BASE                        0x50B9000000
#define SSPERI_SUBSYS_BASE                      (PCIE_SUBSYS_BASE + 0x02000000)
#define PCIE_CFG_BASE(sys_id)                   (PCIE_SUBSYS_BASE + ((sys_id) * 0x02000000))

#define PCIE_X16_DBI_OFFSET			0x1000000
#define PCIE_X16_ATU_OFFSET			0x1800000
#define PCIE_CTRL_DBI_REG_BASE(ctrl_id)         (0x00000000 + ((ctrl_id) * 0x100000))
#define PCIE_CTRL_ATU_REG_BASE(ctrl_id)         (0x00080000 + ((ctrl_id) * 0x100000))
#define PCIE_CTRL_INTF_REG_BASE(ctrl_id)        (0xa01000 + ((ctrl_id) * 0x2000))
#define PCIE_SII_INTF_REG_BASE(ctrl_id)         (0xa00000 + ((ctrl_id) * 0x2000))
#define PCIE_PHY_INTF_REG_BASE                  (PCIE_SUBSYS_BASE + 0x870000)
#define PCIE_TOP_SYS_REG_BASE(ctrl_id)          (0x831000 + ((ctrl_id) * 0x20))
#define PCIE_CDMA_REG_BASE(cdma_id)             (0x810000 + ((cdma_id) * 0x1000))
#define SSPERI_PHY_INTF_REG_BASE(phy_id)        (0x880000 + ((phy_id) * 0x70000))


#define C2C_TOP_MSI_GEN_MODE_REG                            0xd8

// PHY INTF REG (PHY TOP)
#define PHY_INTF_REG_RX000                                  0x0
#define PHY_INTF_REG_RX400                                  0x400
#define PHY_INTF_REG_RX410                                  0x410

#define PHY_INTF_REG_RX000_PHY_X_PCS_PWR_STABLE_BIT         10
#define PHY_INTF_REG_RX000_PHY_X_PMA_PWR_STABLE_BIT         11
#define PHY_INTF_REG_RX000_PHY_X_SRAM_BL_BYPASS_BIT         20
#define PHY_INTF_REG_RX000_PHY_X_SRAM_EXT_LD_DONE_BIT       22
#define PHY_INTF_REG_RX400_UPCS_PWR_STABLE_BIT              16


#define PHY_INTF_REG_RX000_PHY_X_SRAM_BYPASS_MASK           GENMASK_32(21, 20)
#define PHY_INTF_REG_RX410_SS_MODE_MASK                     GENMASK_32(3, 0)

//Ssperi PHY INTF REG
#define SSPERI_PHY_INTF_REG_RX000                           0x0
#define SSPERI_PHY_INTF_REG_RX004                           0x0

#define SSPERI_PHY_INTF_REG_RX2b8                           0x2b8

#define SSPERI_PHY_INTF_REG_RX004_PHY0_PCS_PWR_STABLE_BIT   10
#define SSPERI_PHY_INTF_REG_RX004_PHY0_PMA_PWR_STABLE_BIT   11
#define SSPERI_PHY_INTF_REG_RX004_PHY0_SRAM_BL_BYPASS_BIT   21
#define SSPERI_PHY_INTF_REG_RX004_PHY0_SRAM_EXT_LD_DONE_BIT 23

#define SSPERI_PHY_INTF_REG_RX2b8_UPCS_PWR_STABLE_BIT       28

#define SSPERI_PHY_INTF_REG_RX000_SS_MODE_MASK              GENMASK_32(2, 0)
#define SSPERI_PHY_INTF_REG_RX004_PHY0_SRAM_BYPASS_MASK     GENMASK_32(22, 21)



//PCIE CTRL REG
#define PCIE_CTRL_GENERAL_CORE_CTRL                         0x008
#define PCIE_CTRL_RST_CTRL_0_REG                            0x060
#define PCIE_CTRL_DBI_CDM_ADDR_REG                          0x080
#define PCIE_CTRL_DBI_ATU_ADDR_REG                          0x084

#define PCIE_CTRL_GENERAL_CORE_CTRL_DEVICE_TYPE_MASK        GENMASK_32(7, 4)
#define PCIE_CTRL_CS2_CDM_ADDR_HIGH_RD_X16_MASK             GENMASK_32(9, 0)
#define PCIE_CTRL_CS2_CDM_ADDR_HIGH_WR_X16_MASK             GENMASK_32(25, 16)
#define PCIE_CTRL_DBI_ATU_ADDR_HIGH_RD_X16_MASK             GENMASK_32(9, 0)
#define PCIE_CTRL_DBI_ATU_ADDR_HIGH_WR_X16_MASK             GENMASK_32(25, 16)

#define PCIE_CTRL_RST_CTRL_0_BUTTON_RSTN_BIT                0
#define PCIE_CTRL_RST_CTRL_0_PWR_UP_RSTN_BIT                1

//PCIE SII REG
#define PCIE_SII_GENERAL_CTRL1_REG                          0x050
#define PCIE_SII_GENERAL_CTRL3_REG                          0x058

//PCIE TOP SYS REG
#define PCIE_TOP_SYS_AXI0_BAR_START_LOW_ADDR_REG            0x004
#define PCIE_TOP_SYS_AXI0_BAR_START_HIGH_ADDR_REG           0x008
#define PCIE_TOP_SYS_AXI0_BAR_END_LOW_ADDR_REG              0x00C
#define PCIE_TOP_SYS_AXI0_BAR_END_HIGH_ADDR_REG             0x010
#define PCIE_TOP_SYS_AXI0_32BAR_START_ADDR_REG              0x018
#define PCIE_TOP_SYS_AXI0_32BAR_END_ADDR_REG                0x01C

//SSPERI TOP SYS REG
#define SSPERI_TOP_SYS_AXI0_BAR_START_LOW_ADDR_REG          0x004
#define SSPERI_TOP_SYS_AXI0_BAR_START_HIGH_ADDR_REG         0x008
#define SSPERI_TOP_SYS_AXI0_BAR_END_LOW_ADDR_REG            0x00C
#define SSPERI_TOP_SYS_AXI0_BAR_END_HIGH_ADDR_REG           0x010
#define SSPERI_TOP_SYS_AXI0_32BAR_START_ADDR_REG            0x014
#define SSPERI_TOP_SYS_AXI0_32BAR_END_ADDR_REG              0x018

//CDMA REG
#define PCIE_CDMA_CHL_CTRL_REG                              0x800
#define PCIE_CDMA_BW_LMT_EN_REG                             0x804
#define PCIE_CDMA_INT_MASK_REG                              0x808
#define PCIE_CDMA_INT_STATUS_REG                            0x80C
#define PCIE_CDMA_CMD_FIFO_STATUS_REG                       0x810
#define PCIE_CDMA_AW_BW_LMT_REG                             0x814
#define PCIE_CDMA_AR_BW_LMT_REG                             0x818
#define PCIE_CDMA_MAX_PAYLOAD_REG                           0x9F8
#define PCIE_CDMA_DESC_R878_REG                             0x878
#define PCIE_CDMA_DESC_2D_IMG_TOTAL_WIDTH_REG               0x87C
#define PCIE_CDMA_DESC_2D_IMG_SIZE_REG                      0x880
#define PCIE_CDMA_DESC_SRC_ADDR_LO_REG                      0x884
#define PCIE_CDMA_DESC_SRC_ADDR_HI_REG                      0x888
#define PCIE_CDMA_DESC_DST_ADDR_LO_REG                      0x88C
#define PCIE_CDMA_DESC_DST_ADDR_HI_REG                      0x890
#define PCIE_CDMA_DESC_1D_DATA_LEGTH_REG                    0x894

enum pcie_serdes_mode {
	PCIE_SERDES_MODE_6_LINK = 0, //4x2 + 2x4
	PCIE_SERDES_MODE_3_LINK,     //x4 + x4 + x8
	PCIE_SERDES_MODE_1_LINK,     //x16
	PCIE_SERDES_MODE_BUTT
};

enum ssperi_serdes_mode {
	SSPERI_SERDES_PCIE_PROTOCOL = 0,
	SSPERI_SERDES_SATA_PROTOCOL,
	SSPERI_SERDES_ETH_PROTOCOL,
	SSPERI_SERDES_MODE_BUTT
};

enum {
	PCIE_PHY_NO_SRAM_BYPASS = 0,
	PCIE_PHY_SRAM_BYPASS,
	PCIE_PHY_SRAM_BL_BYPASS,
	PCIE_PHY_BYPASS_MODE_BUTT
};

enum pcie_gen {
	PCIE_GEN_1 = 1,
	PCIE_GEN_2,
	PCIE_GEN_3,
	PCIE_GEN_4,
	PCIE_GEN_5,
	PCIE_GEN_6
};

enum pcie_phy_id {
	PCIE_PHY_ID_0 = 0,   // 4lanes
	PCIE_PHY_ID_1,       // 4lanes
	PCIE_PHY_ID_2,
	PCIE_PHY_ID_3,
	PCIE_PHY_ID_BUTT
};

enum pcie_subsys_id {
	PCIE_SUBSYS = 0,
	SSPERI_SUBSYS,
	PCIE_SUBSYS_ID_BUTT
};

enum pcie_ctrl_id {
	PCIE_CTRL_X2_0 = 0,
	PCIE_CTRL_X2_1,
	PCIE_CTRL_X2_2,
	PCIE_CTRL_X4_0,
	PCIE_CTRL_X4_1,
	PCIE_CTRL_X16_0,
	SSPERI_CTRL_X1_6,
	SSPERI_CTRL_X1_7,
	PCIE_C2C_ID_BUTT
};

enum pcie_rst_status {
	PCIE_RST_ASSERT = 0,
	PCIE_RST_DE_ASSERT,
	PCIE_RST_STATUS_BUTT
};

enum pcie_4g_mode {
	PCIE_UP4G_MODE = 0,
	PCIE_DW4G_MODE,
	PCIE_4G_MODE_BUTT
};

struct pcie_slv_addr_tbl {
	uint32_t pcie_up4g_start_addr;
	uint32_t pcie_up4g_end_addr;
	uint32_t pcie_dw4g_start_addr;
	uint32_t pcie_dw4g_end_addr;
};

enum pcie_op_type {
	PCIE_DEV_TYPE_EP = 0,
	PCIE_DEV_TYPE_RC = 4,
	PCIE_DEV_TYPE_BUTT
};

enum pcie_lane_width {
	PCIE_LANE_WIDTH_X1 = 1,
	PCIE_LANE_WIDTH_X2 = 2,
	PCIE_LANE_WIDTH_X4 = 4,
	PCIE_LANE_WIDTH_X8 = 8,
	PCIE_LANE_WIDTH_X16 = 16,
	PCIE_LANE_WIDTH_BUTT
};

enum pcie_max_payload {
	PCIE_MPS_128 = 0,
	PCIE_MPS_256,
	PCIE_MPS_512,
	PCIE_MPS_1024,
	PCIE_MPS_BUTT
};

enum pcie_dbi_cs2 {
	PCIE_CONFIG_DBI = 0,
	PCIE_CONFIG_CS2,
	PCIE_DNI_CS2_BUTT
};

enum pcie_atu_elbi {
	PCIE_CONFIG_ATU = 0,
	PCIE_CONFIG_ELBI,
	PCIE_ATU_ELBI_BUTT
};

enum pcie_cdm_msix {
	PCIE_CONFIG_cdm = 0,
	PCIE_CONFIG_msix,
	PCIE_CDM_MSIX_BUTT
};

struct pcie_eq_preset {
	uint32_t addr;
	uint32_t value;
};
void sg2380_pcie_init(void);
void sg2380_pcie_cdma_test(uint32_t sys_id, uint32_t cdma_id, uint32_t mode,
				uint32_t length, uint64_t src_addr, uint64_t dst_addr);

#endif /* MANGO_PCIE_H */

