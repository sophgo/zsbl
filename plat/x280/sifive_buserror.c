/* Copyright 2020 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <sifive_metal.h>
#include <sifive_buserror0.h>
#include <sifive_platform.h>
#include <sifive_cpu.h>
#include <sifive_io.h>
#include <sbi/riscv_asm.h>
#include <sbi/riscv_encoding.h>

void metal_buserror_init(struct metal_buserror *beu)
{
	struct metal_cpu *cpu = metal_cpu_get(csr_read(CSR_MHARTID));
	if (cpu != NULL) {
		struct metal_buserror *beu = metal_cpu_get_buserror(cpu);
		if (beu != NULL) {
			metal_buserror_set_event_enabled(
				beu, METAL_BUSERROR_EVENT_ALL, true);
		}
	}
}

int metal_buserror_set_event_enabled(struct metal_buserror *beu,
				     metal_buserror_event_t events,
				     bool enabled)
{
	uintptr_t base = __metal_driver_sifive_buserror0_control_base(beu);
	if (base == (uintptr_t)NULL) {
		return 1;
	}
	if (!(events & METAL_BUSERROR_EVENT_ANY)) {
		return 2;
	}

printf("%s: base is 0x%lx\n", __func__, base);

	uintptr_t reg_enable = base + METAL_SIFIVE_BUSERROR0_ENABLE;
printf("%s: %d\n", __func__, __LINE__);
	if (enabled) {
		__METAL_ACCESS_ONCE((__metal_io_u8 *)reg_enable) |= events;
	} else {
		__METAL_ACCESS_ONCE((__metal_io_u8 *)reg_enable) &= ~events;
	}
printf("%s: %d\n", __func__, __LINE__);
	if (!(events & __METAL_ACCESS_ONCE((__metal_io_u8 *)reg_enable))) {
		return __METAL_ACCESS_ONCE((__metal_io_u8 *)reg_enable);
	}

	return 0;
}
