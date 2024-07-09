/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SG_IOMMU_H__
#define __SG_IOMMU_H__

#include <lib/utils_def.h>

/* iommu memory-mapped register layout */
#define REG_DDTP		0x10
#define DDTP_MODE_OFF       0
#define DDTP_MODE_BARE      1
#define DDTP_MODE_1LVL      2
#define DDTP_MODE_2LVL      3
#define DDTP_MODE_3LVL      4

#define TC_V		0x1
#define SATP_MODE_39    0x8000000000000000UL
#define SATP_MODE_48    0x9000000000000000UL

#define TC_OFFSET	0x0
#define IOHGATP_OFFSET	0x8
#define TA_OFFSET	0x10
#define FSC_OFFSET	0x18

/* iommu wrapper register */
#define IOMMU_WRAPPER_OFFSET	0x2000

#define IOMMU_GLOBAL_CFG	IOMMU_WRAPPER_OFFSET + 0x0
#define BYPASS_ENABLE		BIT(0)

#define IOMMU_PREFETCH_CFG	IOMMU_WRAPPER_OFFSET + 0x14
#define PREFETCH_ENABLE		BIT(0)

#define IOMMU_PDS_IF_CFG0	IOMMU_WRAPPER_OFFSET + 0x40
#define PDS_ARQOS_MASK		0xF

#define IOMMU_PREF_CFG_VA_STRIDE_START	IOMMU_WRAPPER_OFFSET + 0x54
#define IOMMU_PREF_CFG_VA_STRIDE_END	IOMMU_WRAPPER_OFFSET + 0x150

/* ssperi sys top register */
#define ETH01_AW_DEVID_REG	0x164
#define ETH23_AW_DEVID_REG	0x168
#define ETH45_AW_DEVID_REG	0x16c
#define SATA_CDMA_AW_DEVID_REG	0x170

#define ETH01_AR_DEVID_REG	0x174
#define ETH23_AR_DEVID_REG	0x178
#define ETH45_AR_DEVID_REG	0x17c
#define SATA_CDMA_AR_DEVID_REG	0x180

#define ETH0_SHIFT	0
#define ETH1_SHIFT	16
#define ETH2_SHIFT	0
#define ETH3_SHIFT	16
#define ETH4_SHIFT	0
#define ETH5_SHIFT	16
#define SATA_SHIFT	0
#define CDMA_SHIFT	16

#define DEVID_MASK	0xFFFF

/* device id */
#define ETH0_DEVID	1
#define ETH1_DEVID	2
#define ETH2_DEVID	3
#define ETH3_DEVID	4
#define ETH4_DEVID	5
#define ETH5_DEVID	6
#define SATA_DEVID	7
#define CDMA_DEVID	8

#define TMP_DEVID10	10
#define TMP_DEVID11	11
#define TMP_DEVID12	12
#define TMP_DEVID13	13
#define TMP_DEVID14	14
#define TMP_DEVID15	15
#define TMP_DEVID16	16
#define TMP_DEVID17	17
#define TMP_DEVID18	18
#define TMP_DEVID19	19

void sg2380_iommu_init(void);

#endif	/* __SG_IOMMU_H__ */
