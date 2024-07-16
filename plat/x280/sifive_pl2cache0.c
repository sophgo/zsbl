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
#define SIFIVE_PL2CACHE0_PL2CONFIG1_REGIONCLOCKDISABLE (1U << 3U)

/* Array of base addresses with HART IDs as the index */
unsigned long pl2cache_base[] = METAL_SIFIVE_PL2CACHE0_BASE_ADDR;

void sifive_pl2cache0_set_cleanEvictEnable_bit(bool val) {
	sifive_pl2cache0_configbits tmp;
	int hartid;

	__asm__ volatile("csrr %0, mhartid" : "=r"(hartid));

	tmp.w = REGW(METAL_SIFIVE_PL2CACHE0_PL2CONFIG0);
	tmp.b.cleanEvictEnable = val;
	REGW(METAL_SIFIVE_PL2CACHE0_PL2CONFIG0) = tmp.w;
}

/* Write to bit 3 of METAL_SIFIVE_PL2CACHE0_PL2CONFIG1 */
void sifive_pl2cache_regional_clock_gate_enable(void)
{
	int hartid;

	__asm__ volatile("csrr %0, mhartid" : "=r"(hartid));
	if (!pl2cache_base[hartid]) {
		return;
	}

	uint32_t tmp;

	tmp = REGW(METAL_SIFIVE_PL2CACHE0_PL2CONFIG1);
	REGW(METAL_SIFIVE_PL2CACHE0_PL2CONFIG1) =
		tmp & (~(SIFIVE_PL2CACHE0_PL2CONFIG1_REGIONCLOCKDISABLE));
}

void sifive_pl2cache0_init(void)
{
	int hartid;

	__asm__ volatile("csrr %0, mhartid" : "=r"(hartid));
	if (!pl2cache_base[hartid]) {
		return;
	}

	sifive_pl2cache_regional_clock_gate_enable();

	sifive_pl2cache0_set_cleanEvictEnable_bit(1);
}
