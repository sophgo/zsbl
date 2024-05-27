/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SG_IOMMU_H__
#define __SG_IOMMU_H__

#include <lib/utils_def.h>

/* iommu wrapper register */
#define IOMMU_GLOBAL_CFG	0x2000 + 0x0
#define BYPASS_ENABLE		BIT(0)

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

void sg2380_iommu_init(void);

#endif	/* __SG_IOMMU_H__ */
