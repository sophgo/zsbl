#ifndef _CACHE_H
#define _CACHE_H

#include <mmu.h>

void icache_enable(void);
void icache_disable(void);
void dcache_enable(void);
void dcache_disable(void);
int dcache_status(void);

void enable_caches(struct mm_region *mm);
extern void invalidate_dcache_range(unsigned long start, unsigned long stop);
extern void flush_dcache_range(unsigned long start, unsigned long stop);
extern void invalidate_icache_all(void);
extern void invalidate_dcache_all(void);
extern void mmu_change_region_attr(unsigned long addr, unsigned long size,
				   unsigned long attrs);

extern struct mm_region default_memory_map[];

#endif
