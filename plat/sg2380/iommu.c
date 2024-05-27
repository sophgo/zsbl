#include <stdio.h>
#include <stdbool.h>
#include <lib/mmio.h>
#include <memmap.h>
#include <iommu.h>

//#define SG2380_IOMMU_DEBUG

#define IOMMU_BRIDGE_NUM	5

struct devid_info {
	const char *name;
	uintptr_t reg;
	uint32_t shift;
	uint32_t mask;
	uintptr_t devid;
};

struct iommu_bridge{
	const char *name;
	uintptr_t bypass_enable_reg;
	bool bypass;

	struct devid_info *devid;
	int dev_num;
};

static void sg2380_iommu_set_devid(uintptr_t addr, uint32_t shift, uint32_t devid)
{
	mmio_clrsetbits_32(addr, DEVID_MASK << shift, devid << shift);
}

static void sg2380_iommu_bypass_enable(uintptr_t addr, bool enable)
{
	if (enable)
		mmio_setbits_32(addr, BYPASS_ENABLE);
	else
		mmio_clrbits_32(addr, BYPASS_ENABLE);
}

static struct devid_info left_devid_table[] = {

};

static struct devid_info right_devid_table[] = {

};

static struct devid_info gpu_devid_table[] = {

};

static struct devid_info pcie_devid_table[] = {

};

static struct devid_info ssperi_devid_table[] = {
	{"eth0_aw", SSPERI_SYS_TOP + ETH01_AW_DEVID_REG, ETH0_SHIFT, DEVID_MASK, ETH0_DEVID},
	{"eth1_aw", SSPERI_SYS_TOP + ETH01_AW_DEVID_REG, ETH1_SHIFT, DEVID_MASK, ETH1_DEVID},
	{"eth2_aw", SSPERI_SYS_TOP + ETH23_AW_DEVID_REG, ETH2_SHIFT, DEVID_MASK, ETH2_DEVID},
	{"eth3_aw", SSPERI_SYS_TOP + ETH23_AW_DEVID_REG, ETH3_SHIFT, DEVID_MASK, ETH3_DEVID},
	{"eth4_aw", SSPERI_SYS_TOP + ETH45_AW_DEVID_REG, ETH4_SHIFT, DEVID_MASK, ETH4_DEVID},
	{"eth5_aw", SSPERI_SYS_TOP + ETH45_AW_DEVID_REG, ETH5_SHIFT, DEVID_MASK, ETH5_DEVID},
	{"sata_aw", SSPERI_SYS_TOP + SATA_CDMA_AW_DEVID_REG, SATA_SHIFT, DEVID_MASK, SATA_DEVID},
	{"cdma_aw", SSPERI_SYS_TOP + SATA_CDMA_AW_DEVID_REG, CDMA_SHIFT, DEVID_MASK, CDMA_DEVID},

	{"eth0_ar", SSPERI_SYS_TOP + ETH01_AR_DEVID_REG, ETH0_SHIFT, DEVID_MASK, ETH0_DEVID},
	{"eth1_ar", SSPERI_SYS_TOP + ETH01_AR_DEVID_REG, ETH1_SHIFT, DEVID_MASK, ETH1_DEVID},
	{"eth2_ar", SSPERI_SYS_TOP + ETH23_AR_DEVID_REG, ETH2_SHIFT, DEVID_MASK, ETH2_DEVID},
	{"eth3_ar", SSPERI_SYS_TOP + ETH23_AR_DEVID_REG, ETH3_SHIFT, DEVID_MASK, ETH3_DEVID},
	{"eth4_ar", SSPERI_SYS_TOP + ETH45_AR_DEVID_REG, ETH4_SHIFT, DEVID_MASK, ETH4_DEVID},
	{"eth5_ar", SSPERI_SYS_TOP + ETH45_AR_DEVID_REG, ETH5_SHIFT, DEVID_MASK, ETH5_DEVID},
	{"sata_ar", SSPERI_SYS_TOP + SATA_CDMA_AR_DEVID_REG, SATA_SHIFT, DEVID_MASK, SATA_DEVID},
	{"cdma_ar", SSPERI_SYS_TOP + SATA_CDMA_AR_DEVID_REG, CDMA_SHIFT, DEVID_MASK, CDMA_DEVID},
};

static const struct iommu_bridge bridge[IOMMU_BRIDGE_NUM] = {
	{
		.name = "left iommu",
		.bypass_enable_reg = LEFT_IOMMU_BASE + IOMMU_GLOBAL_CFG,
		.bypass = true,
		.devid = left_devid_table,
		.dev_num = ARRAY_SIZE(left_devid_table),
	},
	{
		.name = "right iommu",
		.bypass_enable_reg = RIGHT_IOMMU_BASE + IOMMU_GLOBAL_CFG,
		.bypass = true,
		.devid = right_devid_table,
		.dev_num = ARRAY_SIZE(right_devid_table),
	},
	{
		.name = "gpu iommu",
		.bypass_enable_reg = GPU_IOMMU_BASE + IOMMU_GLOBAL_CFG,
		.bypass = true,
		.devid = gpu_devid_table,
		.dev_num = ARRAY_SIZE(gpu_devid_table),
	},
	{
		.name = "PCIe iommu",
		.bypass_enable_reg = PCIE_IOMMU_BASE + IOMMU_GLOBAL_CFG,
		.bypass = true,
		.devid = pcie_devid_table,
		.dev_num = ARRAY_SIZE(pcie_devid_table),
	},
	{
		.name = "ssperi iommu",
		.bypass_enable_reg = SSPERI_IOMMU_BASE + IOMMU_GLOBAL_CFG,
		.bypass = true,
		.devid = ssperi_devid_table,
		.dev_num = ARRAY_SIZE(ssperi_devid_table),
	},
};

#ifdef SG2380_IOMMU_DEBUG
static void sg2380_dump_reg(void)
{
	int i, j;
	uint32_t value;
	struct devid_info *devid;

	for(i = 0; i < IOMMU_BRIDGE_NUM; i++)
	{
		if (bridge[i].bypass)
			continue;

		value = mmio_read_32(bridge[i].bypass_enable_reg);
		printf("%s bypass %s\n", bridge[i].name, (value & BYPASS_ENABLE) ? "enable" : "disable");
		printf("device number %d\n", bridge[i].dev_num);

		for (j = 0; j < bridge[i].dev_num; j++) {
			devid = &bridge[i].devid[j];
			value = mmio_read_32(devid->reg);
			printf("%s device id: %d\n", devid->name, (value >> devid->shift) & devid->mask);
		}
	}
}
#endif

void sg2380_iommu_init(void)
{
	int i, j;
	struct devid_info *devid;

	printf("sg2380 iommu init\n");

	for(i = 0; i < IOMMU_BRIDGE_NUM; i++)
	{
		if (bridge[i].bypass)
			continue;

		sg2380_iommu_bypass_enable(bridge[i].bypass_enable_reg,
					   bridge[i].bypass);

		for (j = 0; j < bridge[i].dev_num; j++) {
			devid = &bridge[i].devid[j];
			sg2380_iommu_set_devid(devid->reg, devid->shift, devid->devid);
		}
	}

#ifdef SG2380_IOMMU_DEBUG
	sg2380_dump_reg();
#endif

	return;
}
