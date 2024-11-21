#include <cache.h>

#define L1_CACHE_BYTES 64

static void sync_dcache(void)
{
	/* sync.i */
	asm volatile (".word 0x01a0000b":::);
}

void sync_icache(void)
{
	asm volatile ("fence.i");
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm ("a0");

	/* dcache.civa data cache clean and invalidate by virtual address */
	for (i = start & ~(L1_CACHE_BYTES - 1); i < end; i += L1_CACHE_BYTES)
		asm volatile (".word 0x0275000b":::);

	sync_dcache();
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm ("a0");

	for (i = start & ~(L1_CACHE_BYTES - 1); i < end; i += L1_CACHE_BYTES)
		asm volatile (".word 0x0265000b":::);

	sync_dcache();
}

void clean_dcache_range(unsigned long start, unsigned long end)
{
	flush_dcache_range(start, end);
}
