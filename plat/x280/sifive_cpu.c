/* Copyright 2018 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <sifive_metal.h>
#include <sifive_cpu.h>
#include <stddef.h>

extern __inline__ struct metal_interrupt *
metal_cpu_interrupt_controller(struct metal_cpu *cpu);

int metal_cpu_get_current_hartid() {
#ifdef __riscv
	int mhartid;
	__asm__ volatile("csrr %0, mhartid" : "=r"(mhartid));
	return mhartid;
#endif
}

struct metal_cpu *metal_cpu_get(unsigned int hartid) {
	if (hartid < __METAL_DT_MAX_HARTS) {
		return (struct metal_cpu *)__metal_cpu_table[hartid];
	}
	return NULL;
}

extern __inline__ struct metal_buserror *
metal_cpu_get_buserror(struct metal_cpu *cpu);
