#if defined ARCH_ARM64
#include <stdint.h>
#include <string.h>
#include <mmu.h>
#include <cache.h>
#include <irq.h>
#include <arch.h>
#include <timer.h>
#include <memmap.h>
#include <platform.h>
#include <framework/module.h>
#include <framework/common.h>

struct mm_region default_memory_map[] = {
	{
		/* AXI SRAM */
		.virt = 0x10000000UL,
		.phys = 0x10000000UL,
		.size = 0x00200000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
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
		.virt = 0x6000000UL,
		.phys = 0x6000000UL,
		.size = 0x4000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			PTE_BLOCK_NON_SHARE |
			PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{
		/* List terminator */
		0,
	}
};
#endif
