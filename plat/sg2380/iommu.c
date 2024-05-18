#include <stdio.h>
#include <stdbool.h>
#include <lib/mmio.h>
#include <memmap.h>
#include <iommu.h>

static void sg2380_iommu_bypass_enable(uintptr_t base, bool enable)
{
	if (enable)
		mmio_setbits_32(base + IOMMU_GLOBAL_CFG, 0x1 << BYPASS_ENABLE_OFFSET);
	else
		mmio_clrbits_32(base + IOMMU_GLOBAL_CFG, 0x1 << BYPASS_ENABLE_OFFSET);
}

void sg2380_iommu_init(void)
{
	printf("sg2380 iommu bypass enable\n");

	sg2380_iommu_bypass_enable(LEFT_IOMMU_BASE, true);
	sg2380_iommu_bypass_enable(RIGHT_IOMMU_BASE, true);
	sg2380_iommu_bypass_enable(GPU_IOMMU_BASE, true);
	sg2380_iommu_bypass_enable(PCIE_IOMMU_BASE, true);
	sg2380_iommu_bypass_enable(SSPERI_IOMMU_BASE, true);

	return;
}
