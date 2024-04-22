/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __RISCV_CACHE_H__
#define __RISCV_CACHE_H__

void flush_dcache_range(unsigned long start, unsigned long end);
void invalidate_dcache_range(unsigned long start, unsigned long end);
void clean_dcache_range(unsigned long start, unsigned long end);
#endif
