#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <mmu.h>
#include <cache.h>
#include <irq.h>
#include <arch.h>
#include <timer.h>
#include <memmap.h>
#include <platform.h>
#include <firmware_top.h>
#include <framework/module.h>
#include <framework/common.h>
#include <framework/efuse.h>
#include <framework/ictl.h>

/* porting for merged code from arm9 firmware */
static inline void cpu_write32(unsigned long addr, uint32_t value)
{
	writel(value, addr);
}

static inline uint32_t cpu_read32(unsigned long addr)
{
	return readl(addr);
}

static inline void shm_write32(unsigned long index, uint32_t value)
{
	cpu_write32(SHARE_REG_BASE + index * 4, value);
}

static inline uint32_t shm_read32(unsigned long index)
{
	return cpu_read32(SHARE_REG_BASE + index * 4);
}

static int work_mode;

int get_work_mode(void)
{
	return work_mode;
}

#ifdef CONFIG_TARGET_EMULATOR
static inline void mark_test_result(uint32_t magic)
{
	writel(magic, 0x50017f10);
}
static inline void mark_test_progress(u32 magic)
{
	writel(magic, 0x50017f30);
}
#else
#define mark_test_resul(...)	do {} while (0)
#define mark_test_progress(...)	do {} while (0)
#endif

/* end porting merged code */

static u32 pcie_msi_data = 0x0;
static u32 pcie_msi_addr = 0x0;

static void *ictl[] = {
	(void *)ICTL0_BASE, (void *)ICTL1_BASE,
};

void setup_pcie(void)
{
	u32 msi_upper_addr = 0x0;
	u32 msi_low_addr = 0x0;
	u32 remote_msi_iatu_base_addr = 0x0;
	u32 chip_id = 0x0;
	u32 function_num = 0x0;
	u32 value = 0x0;
	u32 pcie_msi_low;

	pr_debug("setup pcie\n");

	chip_id = cpu_read32(0x5fb80000);
	chip_id = (chip_id >> 28);
	chip_id &= 0x7;
	if (chip_id > 0)
		function_num = chip_id -1;
	pr_debug("chip id is 0x%x, function_num = 0x%x\n", chip_id, function_num);

	/* set remap table to write msi outbound iatu, use remap table 0x5fbf_1xxx */
	if (chip_id > 1) {
		remote_msi_iatu_base_addr += (0x5fbf1000 + (chip_id - 1) * 0x200);
		cpu_write32(0x5fb8f00c, 0x5fb00000 & ~0xfff);   // write low adddr;
		cpu_write32(0x5fb8f008, (0x3 << 8));
		cpu_write32(0x5fb8f05c, 0x5fb00000 & ~0xfff);  // read low adddr;
		cpu_write32(0x5fb8f058, (0x3 << 8));
	} else {
		remote_msi_iatu_base_addr = 0x5fb00000;
	}
	printf("remote_msi_iatu_base_addr = 0x%x\n", remote_msi_iatu_base_addr);

	/*set outbound iatu*/
	value = cpu_read32(remote_msi_iatu_base_addr);
	value &= ~(0x7 << 20);
	value |= (function_num << 20);
	cpu_write32(remote_msi_iatu_base_addr, value);

	value = cpu_read32(remote_msi_iatu_base_addr + 0x4);
	value |= (0x1 << 31);
	cpu_write32(remote_msi_iatu_base_addr + 0x4, value);

	msi_low_addr = cpu_read32(0x5fb8f004);
	pcie_msi_low = msi_low_addr;
	msi_low_addr &= (~0xfff);
	cpu_write32(remote_msi_iatu_base_addr + 0x8, msi_low_addr);
	cpu_write32(remote_msi_iatu_base_addr + 0x14, msi_low_addr);
	cpu_write32(remote_msi_iatu_base_addr + 0x10, msi_low_addr + 0xfff);

	cpu_write32(0x5fb8f004, msi_low_addr);
	msi_upper_addr = cpu_read32(0x5fb8f000);
	cpu_write32(remote_msi_iatu_base_addr + 0x18, msi_upper_addr);
	msi_upper_addr &= 0xf;
	msi_upper_addr |= ((0x3 << 8) | function_num << 5 | 0x1 << 4);
	cpu_write32(remote_msi_iatu_base_addr + 0xc, msi_upper_addr);

	cpu_write32(0x5fb8f000, msi_upper_addr);

	pcie_msi_data = cpu_read32(0x5fa0103c);
	pcie_msi_low &= (0xfff);
	pcie_msi_addr = 0x5fbf0000 + pcie_msi_low;

	shm_write32(SHARE_REG_FW_STATUS, ARM9_START_STEP_PCIE_TABLE_INIT);
}

static void irq_handler(void)
{
	/* mask all external interrupt */
	ictl_set_irq_mask(ictl[0], 0xffffffffffffffff);
	ictl_set_irq_mask(ictl[1], 0xffffffffffffffff);
	/* send pcie interrupt */
	if (get_work_mode() == WORK_MODE_PCIE)
		cpu_write32(pcie_msi_addr, pcie_msi_data);
}

static void fiq_handler(void)
{
	/* clear fiq source */
	writel(1 << 14, TOP_GP14_CLR);
	while (true)
		asm volatile ("wfi");
}

struct mm_region default_memory_map[] = {
	{
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 0x06000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			PTE_BLOCK_NON_SHARE |
			PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{
		/* SPI Flash */
		.virt = 0x06000000UL,
		.phys = 0x06000000UL,
		.size = 0x01000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			PTE_BLOCK_INNER_SHARE
	},
	{
		.virt = 0x07000000UL,
		.phys = 0x07000000UL,
		.size = 0x09000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			PTE_BLOCK_NON_SHARE |
			PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{
		/* AXI SRAM */
		.virt = 0x10000000UL,
		.phys = 0x10000000UL,
		.size = 0x00200000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			PTE_BLOCK_NON_SHARE |
			PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{
		.virt = 0x10200000UL,
		.phys = 0x10200000UL,
		.size = 0xEFE00000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			PTE_BLOCK_NON_SHARE |
			PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{
		.virt = MEMORY_BASE,
		.phys = MEMORY_BASE,
		.size = MEMORY_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			PTE_BLOCK_INNER_SHARE
	},
	{
		/* List terminator */
		0,
	}
};

static struct tpu_runtime tpu_runtime = {0};

struct tpu_runtime *get_tpu_runtime(void)
{
	return &tpu_runtime;
}

/* used by firmware core */
void fw_log(char *fmt, ...)
{
	u32 index = shm_read32(SHARE_REG_ARM9FW_LOG_WP);
	index = index % LOG_BUFFER_SIZE;
	char *ptr = (char *)(LOG_BUFFER_ADDR + index);
	char log_buffer[LOG_LINE_SIZE - 8] = "";
	va_list p;
	va_start(p,fmt);
	vsnprintf(log_buffer, LOG_LINE_SIZE, fmt, p);
	unsigned long long timestamp = timer_tick2us(timer_get_tick());
	printf("[%llx] %s", timestamp, log_buffer);
	snprintf(ptr, LOG_LINE_SIZE,
		 "[%llx] %s", timestamp, log_buffer);
	va_end(p);
	index = index + LOG_LINE_SIZE;
	shm_write32(SHARE_REG_ARM9FW_LOG_WP, index);
}

void setup_intc(void)
{
	pr_debug("setup interrupt\n");
	ictl_init(ictl[0]);
	ictl_init(ictl[1]);
	ictl_set_irq_enable(ictl[0], 0);
	ictl_set_irq_enable(ictl[1], 0);
	ictl_set_irq_mask(ictl[0], 0);
	ictl_set_irq_mask(ictl[1], 0);

	ictl_set_fiq_enable(ictl[0], (1 << 6));
	ictl_set_fiq_mask(ictl[0], 0xffffffff & ~(1 << 6));

	/* register irq and fiq */
	set_handle_irq(irq_handler);
	set_handle_fiq(fiq_handler);
	arch_local_irq_enable();
	arch_local_fiq_enable();

	shm_write32(SHARE_REG_FW_STATUS, ARM9_START_STEP_UNMASK_ALL_INTC);
}

void setup_npu(void)
{
	int bad_npu0, bad_npu1, bad_npu2;

	pr_debug("setup npu\n");

	shm_write32(SHARE_REG_FW_STATUS, ARM9_START_STEP_ENTER_BMDNN);
	u32 v1 =  efuse_read_cell(EFUSE_BASE, 80);
	u32 v2 =  efuse_read_cell(EFUSE_BASE, 81);
	shm_write32(SHARE_REG_FW_STATUS, ARM9_START_STEP_GET_BAD_NPU);
	pr_debug("v1 = %u,v2 = %u\n", v1, v2);
	u32 v = v1 | v2;
	if ((v & 0x0FF80000) != 0)
		bad_npu2 = ((0x07Fc0000 & v) >>18) - 1;
	else
		bad_npu2 = 0;
	if ((0x0003FE00 & v) != 0)
		bad_npu1 = ((0x0003FE00 & v) >>9) - 1;
	else
		bad_npu1 = 0;
	if ((0x0000001FF & v) != 0)
		bad_npu0 = (0x0000001FF & v) - 1;
	else
		bad_npu0 = 0;

	tpu_runtime.nodechip_idx = 0;
	tpu_runtime.bad_npu_idx0 = bad_npu0;  //originally BAD_NPU0,
	tpu_runtime.bad_npu_idx1 = bad_npu1;
	tpu_runtime.bad_npu_idx2 = bad_npu2;
	mark_test_progress(0xbadabada);
	mark_test_progress(bad_npu0);
	mark_test_progress(bad_npu1);
	mark_test_progress(bad_npu2);
}

int is_normal_memory(const void *addr)
{
	return (unsigned long)addr >= MEMORY_BASE &&
		(unsigned long)addr < MEMORY_SIZE;
}

int bm1686_plat_init(void)
{
#if defined(CONFIG_PLAT_BM1686_FORCE_WORK_MODE_SOC)
	work_mode = WORK_MODE_SOC;
#elif defined(CONFIG_PLAT_BM1686_FORCE_WORK_MODE_PCIE)
	work_mode = WORK_MODE_PCIE;
#else
	work_mode = shm_read32(SHARE_REG_ARM9FW_MODE);
#endif

#if 0
	setup_npu();
#endif

	if (get_work_mode() == WORK_MODE_PCIE)
		setup_pcie();

	setup_intc();

	return 0;
}

#ifndef CONFIG_TARGET_EMULATOR
plat_init(bm1686_plat_init);
#endif

