/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SG_IOMMU_H__
#define __SG_IOMMU_H__

#include <lib/utils_def.h>

#define IOMMU_GLOBAL_CFG        0x0
#define BYPASS_ENABLE_OFFSET     BIT(0)

void sg2380_iommu_init(void);

#endif	/* __SG_IOMMU_H__ */
