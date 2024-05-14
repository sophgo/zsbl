// SPDX-License-Identifier: GPL-2.0

#define L1_CACHE_BYTES 64

void flush_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i = start & ~(L1_CACHE_BYTES - 1);

	for (; i < end; i += L1_CACHE_BYTES)
		asm volatile ("cbo.flush (%0)": "=r" (i) : :);
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a0") = start & ~(L1_CACHE_BYTES - 1);

	for (; i < end; i += L1_CACHE_BYTES)
		asm volatile ("cbo.inval (%0)": "=r" (i) : :);
}

void clean_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a0") = start & ~(L1_CACHE_BYTES - 1);

	for (; i < end; i += L1_CACHE_BYTES)
		asm volatile ("cbo.clean (%0)": "=r" (i) : :);
}
