#ifndef __CACHE_H__
#define __CACHE_H__

void flush_dcache_range(unsigned long start, unsigned long end);
void invalidate_dcache_range(unsigned long start, unsigned long end);
void clean_dcache_range(unsigned long start, unsigned long end);
void sync_icache(void);

#endif
