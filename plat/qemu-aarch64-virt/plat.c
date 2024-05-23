#include <stdint.h>
#include <mmu.h>
#include <cache.h>
#include <irq.h>
#include <arch.h>
#include <framework/module.h>
#include <framework/common.h>

struct mm_region default_memory_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x40000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = 0x08000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

int is_normal_memory(const void *addr)
{
	return true;
}

static int qemu_init(void)
{
	pr_debug("enable interrupt\n");
	arch_local_irq_enable();
	irq_init();
	return 0;
}

plat_init(qemu_init);

