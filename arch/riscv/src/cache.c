// SPDX-License-Identifier: GPL-2.0

#define L1_CACHE_BYTES 64

static inline void sync_is(void)
{
	asm volatile (".long 0x01b0000b");
}

void flush_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a0") = start & ~(L1_CACHE_BYTES - 1);

	for (; i < end; i += L1_CACHE_BYTES)
		asm volatile (".long 0x0275000b"); /* dcache.civa a0 */

	sync_is();
}

void invalidate_dcache_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a0") = start & ~(L1_CACHE_BYTES - 1);

	for (; i < end; i += L1_CACHE_BYTES)
		asm volatile (".long 0x2A5000B"); /* dcache.ipa a0 */

	sync_is();
}

void clean_dcache_range(unsigned long start, unsigned long end)
{
	sync_is();
}
