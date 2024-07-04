/* Copyright 2023 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <sifive_metal.h>
#include <sifive_platform.h>
#include <sifive_pl2cache0.h>
#include <stdint.h>

/* Macros to access memory mapped registers */
#define REGW(_x_) *(volatile uint32_t *)(pl2cache_base[hartid] + (_x_))

#define REGD(_x_) *(volatile uint64_t *)(pl2cache_base[hartid] + (_x_))

/* Macros to specify register bit shift */
#define REG_SHIFT_4 4
#define REG_SHIFT_8 8
#define REG_SHIFT_16 16
#define REG_SHIFT_24 24

#define SIFIVE_PL2CACHE0_BYTE_MASK 0xFFUL

/* Array of base addresses with HART IDs as the index */
unsigned long pl2cache_base[] = METAL_SIFIVE_PL2CACHE0_BASE_ADDR;

void sifive_pl2cache0_set_cleanEvictEnable_bit(bool val) {
	sifive_pl2cache0_configbits tmp;
	int hartid;
	__asm__ volatile("csrr %0, mhartid" : "=r"(hartid));

	tmp.w = REGW(METAL_SIFIVE_PL2CACHE0_CONFIGBITS);
	tmp.b.cleanEvictEnable = val;
	REGW(METAL_SIFIVE_PL2CACHE0_CONFIGBITS) = tmp.w;
}

void sifive_pl2cache0_init(void)
{
	sifive_pl2cache0_set_cleanEvictEnable_bit(1);
}
