/* Copyright 2018 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <cpu.h>
#include <machine.h>
#include <stddef.h>

#define __METAL_DT_MAX_HARTS	16

struct metal_cpu *metal_cpu_get(unsigned int hartid) {
	if (hartid < __METAL_DT_MAX_HARTS) {
		return (struct metal_cpu *)__metal_cpu_table[hartid];
	}
	return NULL;
}

struct metal_buserror *__metal_driver_cpu_get_buserror(struct metal_cpu *cpu) {
	return __metal_driver_cpu_buserror(cpu);
}

extern __inline__ struct metal_buserror *
metal_cpu_get_buserror(struct metal_cpu *cpu);
