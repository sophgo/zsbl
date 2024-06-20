#include <stdio.h>
#include <stdbool.h>
#include <lib/mmio.h>
#include <memmap.h>
#include <iommu.h>
#include <string.h>

//#define SG2380_IOMMU_DEBUG
#define IOMMU_BRIDGE_NUM	5

//#define SG2380_IOMMU_DIRECT_MAP
#ifdef SG2380_IOMMU_DIRECT_MAP
#define MAX_DEV_NUM		20
#define IOMMU_DDTP_ADDR		0x140000000
#define IOMMU_DDTP_SIZE		0x1000
#define LEAF_DDT_ENTRY_SIZE	32
#define PGD_ROOT		IOMMU_DDTP_ADDR + IOMMU_DDTP_SIZE
#define phys_to_ppn(va)  (((va) >> 2) & (((1ULL << 44) - 1) << 10))
#define PAGE_OFFSET	12
#define PAGE_SIZE	(1 << PAGE_OFFSET)
#define VPN3_OFFSET	39
#define VPN3_MASK	0x1ff
#define VPN2_OFFSET	30
#define VPN2_MASK	0x1ff
#define VPN1_OFFSET	21
#define VPN1_MASK	0x1ff
#define VPN0_OFFSET	12
#define VPN0_MASK	0x1ff
typedef uint64_t pte_t;
#endif

struct devid_info {
	const char *name;
	uintptr_t reg;
	uint32_t shift;
	uint32_t mask;
	uintptr_t devid;
};

struct iommu_bridge{
	const char *name;
	uintptr_t base;
	bool bypass;
	bool prefetch;

	struct devid_info *devid;
	int dev_num;
};

static void sg2380_iommu_prefetch_enable(uintptr_t base, bool enable)
{
	if (enable)
		mmio_setbits_32(base + IOMMU_PREFETCH_CFG, PREFETCH_ENABLE);
	else
		mmio_clrbits_32(base + IOMMU_PREFETCH_CFG, PREFETCH_ENABLE);
}

static void __attribute__((unused)) sg2380_iommu_cfg_pref_va_stride(uintptr_t base)
{
	uint64_t start_reg, end_reg;
	uint64_t reg;

	start_reg = base + IOMMU_PREF_CFG_VA_STRIDE_START;
	end_reg = base + IOMMU_PREF_CFG_VA_STRIDE_END;

	for (reg = start_reg; reg <= end_reg; reg+=4)
		mmio_write_32(reg, 0x04040404);

	return;
}

static void sg2380_iommu_set_devid(uintptr_t addr, uint32_t shift, uint32_t mask, uint32_t devid)
{
	mmio_clrsetbits_32(addr, mask << shift, devid << shift);
}

static void sg2380_iommu_bypass_enable(uintptr_t base, bool enable)
{
	if (enable)
		mmio_setbits_32(base + IOMMU_GLOBAL_CFG, BYPASS_ENABLE);
	else
		mmio_clrbits_32(base + IOMMU_GLOBAL_CFG, BYPASS_ENABLE);
}

static struct devid_info left_devid_table[] = {

};

static struct devid_info right_devid_table[] = {

};

static struct devid_info gpu_devid_table[] = {
	{"dma0_ar", GPU_SYS_BASE + 0x28, 0, 0x1f, TMP_DEVID10},
	{"dma1_ar", GPU_SYS_BASE + 0x28, 5, 0x1f, TMP_DEVID11},
	{"dma2_ar", GPU_SYS_BASE + 0x28, 10, 0x1f, TMP_DEVID12},
	{"dma3_ar", GPU_SYS_BASE + 0x28, 15, 0x1f, TMP_DEVID13},
	{"dma4_ar", GPU_SYS_BASE + 0x28, 20, 0x1f, TMP_DEVID14},
	{"dma5_ar", GPU_SYS_BASE + 0x28, 25, 0x1f, TMP_DEVID15},
	{"dma6_ar", GPU_SYS_BASE + 0x2c, 0, 0x1f, TMP_DEVID16},
	{"dma7_ar", GPU_SYS_BASE + 0x2c, 5, 0x1f, TMP_DEVID17},

	{"dma0_aw", GPU_SYS_BASE + 0x30, 0, 0x1f, TMP_DEVID10},
	{"dma1_aw", GPU_SYS_BASE + 0x30, 5, 0x1f, TMP_DEVID11},
	{"dma2_aw", GPU_SYS_BASE + 0x30, 10, 0x1f, TMP_DEVID12},
	{"dma3_aw", GPU_SYS_BASE + 0x30, 15, 0x1f, TMP_DEVID13},
	{"dma4_aw", GPU_SYS_BASE + 0x30, 20, 0x1f, TMP_DEVID14},
	{"dma5_aw", GPU_SYS_BASE + 0x30, 25, 0x1f, TMP_DEVID15},
	{"dma6_aw", GPU_SYS_BASE + 0x34, 0, 0x1f, TMP_DEVID16},
	{"dma7_aw", GPU_SYS_BASE + 0x34, 5, 0x1f, TMP_DEVID17},
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
		.base = LEFT_IOMMU_BASE,
		.bypass = true,
		.prefetch = true,
		.devid = left_devid_table,
		.dev_num = ARRAY_SIZE(left_devid_table),
	},
	{
		.name = "right iommu",
		.base = RIGHT_IOMMU_BASE,
		.bypass = true,
		.prefetch = true,
		.devid = right_devid_table,
		.dev_num = ARRAY_SIZE(right_devid_table),
	},
	{
		.name = "gpu iommu",
		.base = GPU_IOMMU_BASE,
		.bypass = true,
		.prefetch = false,
		.devid = gpu_devid_table,
		.dev_num = ARRAY_SIZE(gpu_devid_table),
	},
	{
		.name = "PCIe iommu",
		.base = PCIE_IOMMU_BASE,
		.bypass = true,
		.prefetch = false,
		.devid = pcie_devid_table,
		.dev_num = ARRAY_SIZE(pcie_devid_table),
	},
	{
		.name = "ssperi iommu",
		.base = SSPERI_IOMMU_BASE,
		.bypass = true,
		.prefetch = false,
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

		value = mmio_read_32(bridge[i].base + IOMMU_GLOBAL_CFG);
		printf("%s bypass %s\n", bridge[i].name, (value & BYPASS_ENABLE) ? "enable" : "disable");

		value = mmio_read_32(bridge[i].base + IOMMU_PREFETCH_CFG);
		printf("%s prefetch %s\n", bridge[i].name, (value & PREFETCH_ENABLE) ? "enable" : "disable");

		printf("device number %d\n", bridge[i].dev_num);
		for (j = 0; j < bridge[i].dev_num; j++) {
			devid = &bridge[i].devid[j];
			value = mmio_read_32(devid->reg);
			printf("%s device id: %d\n", devid->name, (value >> devid->shift) & devid->mask);
		}
	}
}
#endif

#ifdef SG2380_IOMMU_DIRECT_MAP
static int current_index = 0;
static uint64_t alloc_one_page(void)
{
	uint64_t pa;

	pa = PGD_ROOT + current_index * (1 << PAGE_OFFSET);
	memset((void *)pa, 0, PAGE_SIZE);
	current_index++;

	return pa;
}

static void sg2380_iommu_create_sv48_pt(uint64_t va,  uint64_t pa, uint64_t size)
{
	uint64_t tmp, tmp1, tmp2;
	pte_t *root, *level1, *level2, *level3;
	int i, j, k;
	uint64_t pa_l;

	pa_l = pa;
	root = (pte_t *)alloc_one_page();
	for (i = 0; i < PAGE_SIZE / sizeof(pte_t); i++) {
		root[i] = 0;
	}

	level1 = (pte_t *)alloc_one_page();
	root[(va >> VPN3_OFFSET) & VPN3_MASK] = ((uint64_t)level1 >> 2) | 0x1;

	tmp = va;
	for (i = 0; i < round_up(size, 1 << VPN2_OFFSET) / (1 << VPN2_OFFSET); i++) {
		level2 = (pte_t *)alloc_one_page();
		level1[(tmp >> VPN2_OFFSET) & VPN2_MASK] = ((uint64_t)level2 >> 2) | 0x1;

		tmp1 = tmp;
		for (j = 0; j < PAGE_SIZE / sizeof(pte_t); j++) {
			level3 = (pte_t *)alloc_one_page();
			level2[(tmp1 >> VPN1_OFFSET) & VPN1_MASK] = ((uint64_t)level3 >> 2) | 0x1;

			tmp2 = tmp1;
			for (k = 0; k < PAGE_SIZE / sizeof(pte_t); k++) {
				level3[(tmp2 >> VPN0_OFFSET) & VPN0_MASK] = (uint64_t)(pa_l >> 2) | 0xdf;
				tmp2 += (1 << VPN0_OFFSET);
				pa_l += (1 << VPN0_OFFSET);
			}
			tmp1 += (1 << VPN1_OFFSET);
		}
		tmp += (1 << VPN2_OFFSET);
	}

	return;
}

static void sg2380_iommu_set_dc(uintptr_t dc_addr)
{
	mmio_write_64(dc_addr + TC_OFFSET, TC_V);
	mmio_write_64(dc_addr + FSC_OFFSET, ((PGD_ROOT) >> 12) | SATP_MODE_48);
}

static void sg2380_iommu_direct_map(uint64_t va, uint64_t pa, uint64_t size)
{
	int dev_id;
	uintptr_t addr;

	memset((void *)IOMMU_DDTP_ADDR, 0x0, PAGE_SIZE);

	sg2380_iommu_create_sv48_pt(va, pa, size);

	for (dev_id = 1; dev_id < MAX_DEV_NUM; dev_id++) {
		addr = IOMMU_DDTP_ADDR + dev_id * LEAF_DDT_ENTRY_SIZE;
		sg2380_iommu_set_dc(addr);
	}
}

static void sg2380_iommu_set_ddtp(uintptr_t base)
{
	uint64_t ddtp;

	ddtp = DDTP_MODE_1LVL | phys_to_ppn(IOMMU_DDTP_ADDR);
	mmio_write_64(base + REG_DDTP, ddtp);
}
#endif

void sg2380_iommu_init(void)
{
	int i, j;
	struct devid_info *devid;

	printf("sg2380 iommu init\n");
#ifdef SG2380_IOMMU_DIRECT_MAP
	sg2380_iommu_direct_map(0xffc0000000, 0x80000000, 0x40000000);
#endif

	for(i = 0; i < IOMMU_BRIDGE_NUM; i++)
	{
		if (bridge[i].bypass)
			continue;

		sg2380_iommu_bypass_enable(bridge[i].base, bridge[i].bypass);
		sg2380_iommu_prefetch_enable(bridge[i].base, bridge[i].prefetch);

		for (j = 0; j < bridge[i].dev_num; j++) {
			devid = &bridge[i].devid[j];
			sg2380_iommu_set_devid(devid->reg, devid->shift,
					       devid->mask, devid->devid);
		}
#ifdef SG2380_IOMMU_DIRECT_MAP
		sg2380_iommu_set_ddtp(bridge[i].base);
#endif
		//sg2380_iommu_cfg_pref_va_stride(bridge[i].base);
	}

#ifdef SG2380_IOMMU_DEBUG
	sg2380_dump_reg();
#endif

	return;
}
