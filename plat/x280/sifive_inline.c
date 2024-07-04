/* Copyright 2019-2022 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */
/* ----------------------------------- */
/* ----------------------------------- */

#ifndef ASSEMBLY

#ifndef METAL_INLINE_H
#define METAL_INLINE_H

#include "sifive_metal.h"

/* --------------------- fixed_factor_clock ------------ */


/* --------------------- sifive_clint0 ------------ */
extern __inline__ unsigned long __metal_driver_sifive_clint0_control_base(struct metal_interrupt *controller);
extern __inline__ unsigned long __metal_driver_sifive_clint0_control_size(struct metal_interrupt *controller);
extern __inline__ int __metal_driver_sifive_clint0_num_interrupts(struct metal_interrupt *controller);
extern __inline__ struct metal_interrupt * __metal_driver_sifive_clint0_interrupt_parents(struct metal_interrupt *controller, int idx);
extern __inline__ int __metal_driver_sifive_clint0_interrupt_lines(struct metal_interrupt *controller, int idx);


/* --------------------- cpu ------------ */
extern __inline__ int __metal_driver_cpu_hartid(struct metal_cpu *cpu);
extern __inline__ unsigned long long __metal_driver_cpu_mtime_frequency(struct metal_cpu *cpu);
extern __inline__ unsigned long long __metal_driver_cpu_clock_frequency(struct metal_cpu *cpu);
extern __inline__ struct metal_interrupt * __metal_driver_cpu_interrupt_controller(struct metal_cpu *cpu);
extern __inline__ int __metal_driver_cpu_num_pmp_regions(struct metal_cpu *cpu);
extern __inline__ struct metal_buserror * __metal_driver_cpu_buserror(struct metal_cpu *cpu);
extern __inline__ int __metal_driver_cpu_is_worldguard_aware(struct metal_cpu *cpu);


/* --------------------- sifive_plic1 ------------ */
extern __inline__ unsigned long __metal_driver_sifive_plic1_control_base(struct metal_interrupt *controller);
extern __inline__ unsigned long __metal_driver_sifive_plic1_control_size(struct metal_interrupt *controller);
extern __inline__ int __metal_driver_sifive_plic1_num_interrupts(struct metal_interrupt *controller);
extern __inline__ int __metal_driver_sifive_plic1_max_priority(struct metal_interrupt *controller);
extern __inline__ struct metal_interrupt * __metal_driver_sifive_plic1_interrupt_parents(struct metal_interrupt *controller, int idx);
extern __inline__ int __metal_driver_sifive_plic1_interrupt_lines(struct metal_interrupt *controller, int idx);
extern __inline__ int __metal_driver_sifive_plic1_context_ids(int hartid);


/* --------------------- sifive_buserror0 ------------ */
extern __inline__ uintptr_t __metal_driver_sifive_buserror0_control_base(const struct metal_buserror *beu);
extern __inline__ struct metal_interrupt * __metal_driver_sifive_buserror0_interrupt_parent(const struct metal_buserror *beu);
extern __inline__ int __metal_driver_sifive_buserror0_interrupt_id(const struct metal_buserror *beu);


/* --------------------- sifive_local_external_interrupts0 ------------ */

/* --------------------- sifive_gpio0 ------------ */


/* --------------------- sifive_gpio_button ------------ */


/* --------------------- sifive_gpio_led ------------ */


/* --------------------- sifive_gpio_switch ------------ */


/* --------------------- sifive_i2c0 ------------ */


/* --------------------- sifive_prci0 ------------ */


/* --------------------- sifive_pwm0 ------------ */


/* --------------------- sifive_remapper2 ------------ */


/* --------------------- sifive_rtc0 ------------ */


/* --------------------- sifive_spi0 ------------ */


/* --------------------- sifive_test0 ------------ */


/* --------------------- sifive_trace0 ------------ */


/* --------------------- sifive_uart0 ------------ */


/* --------------------- sifive_simuart0 ------------ */


/* --------------------- sifive_wdog0 ------------ */


/* --------------------- sifive_fe310_g000_hfrosc ------------ */


/* --------------------- sifive_fe310_g000_hfxosc ------------ */


/* --------------------- sifive_fe310_g000_lfrosc ------------ */


/* --------------------- sifive_fe310_g000_pll ------------ */


/* --------------------- fe310_g000_prci ------------ */


/* From clint@2000000 */
struct __metal_driver_riscv_clint0 __metal_dt_clint_2000000 = {
    .controller.vtable = &__metal_driver_vtable_riscv_clint0.clint_vtable,
    .init_done = 0,
};

/* From cpu@0 */
struct __metal_driver_cpu __metal_dt_cpu_0 = {
    .cpu.vtable = &__metal_driver_vtable_cpu.cpu_vtable,
    .hpm_count = 0,
};

/* From cpu@1 */
struct __metal_driver_cpu __metal_dt_cpu_1 = {
    .cpu.vtable = &__metal_driver_vtable_cpu.cpu_vtable,
    .hpm_count = 0,
};

/* From cpu@2 */
struct __metal_driver_cpu __metal_dt_cpu_2 = {
    .cpu.vtable = &__metal_driver_vtable_cpu.cpu_vtable,
    .hpm_count = 0,
};

/* From cpu@3 */
struct __metal_driver_cpu __metal_dt_cpu_3 = {
    .cpu.vtable = &__metal_driver_vtable_cpu.cpu_vtable,
    .hpm_count = 0,
};

/* From cpu@4 */
struct __metal_driver_cpu __metal_dt_cpu_4 = {
    .cpu.vtable = &__metal_driver_vtable_cpu.cpu_vtable,
    .hpm_count = 0,
};

/* From cpu@5 */
struct __metal_driver_cpu __metal_dt_cpu_5 = {
    .cpu.vtable = &__metal_driver_vtable_cpu.cpu_vtable,
    .hpm_count = 0,
};

/* From cpu@6 */
struct __metal_driver_cpu __metal_dt_cpu_6 = {
    .cpu.vtable = &__metal_driver_vtable_cpu.cpu_vtable,
    .hpm_count = 0,
};

/* From cpu@7 */
struct __metal_driver_cpu __metal_dt_cpu_7 = {
    .cpu.vtable = &__metal_driver_vtable_cpu.cpu_vtable,
    .hpm_count = 0,
};

/* From interrupt_controller */
struct __metal_driver_riscv_cpu_intc __metal_dt_cpu_0_interrupt_controller = {
    .controller.vtable = &__metal_driver_vtable_riscv_cpu_intc.controller_vtable,
    .init_done = 0,
};

/* From interrupt_controller */
struct __metal_driver_riscv_cpu_intc __metal_dt_cpu_1_interrupt_controller = {
    .controller.vtable = &__metal_driver_vtable_riscv_cpu_intc.controller_vtable,
    .init_done = 0,
};

/* From interrupt_controller */
struct __metal_driver_riscv_cpu_intc __metal_dt_cpu_2_interrupt_controller = {
    .controller.vtable = &__metal_driver_vtable_riscv_cpu_intc.controller_vtable,
    .init_done = 0,
};

/* From interrupt_controller */
struct __metal_driver_riscv_cpu_intc __metal_dt_cpu_3_interrupt_controller = {
    .controller.vtable = &__metal_driver_vtable_riscv_cpu_intc.controller_vtable,
    .init_done = 0,
};

/* From interrupt_controller */
struct __metal_driver_riscv_cpu_intc __metal_dt_cpu_4_interrupt_controller = {
    .controller.vtable = &__metal_driver_vtable_riscv_cpu_intc.controller_vtable,
    .init_done = 0,
};

/* From interrupt_controller */
struct __metal_driver_riscv_cpu_intc __metal_dt_cpu_5_interrupt_controller = {
    .controller.vtable = &__metal_driver_vtable_riscv_cpu_intc.controller_vtable,
    .init_done = 0,
};

/* From interrupt_controller */
struct __metal_driver_riscv_cpu_intc __metal_dt_cpu_6_interrupt_controller = {
    .controller.vtable = &__metal_driver_vtable_riscv_cpu_intc.controller_vtable,
    .init_done = 0,
};

/* From interrupt_controller */
struct __metal_driver_riscv_cpu_intc __metal_dt_cpu_7_interrupt_controller = {
    .controller.vtable = &__metal_driver_vtable_riscv_cpu_intc.controller_vtable,
    .init_done = 0,
};

/* From interrupt_controller@c000000 */
struct __metal_driver_sifive_plic1 __metal_dt_interrupt_controller_c000000 = {
    .controller.vtable = &__metal_driver_vtable_sifive_plic1.plic_vtable,
    .init_done = 0,
};

/* From bus_error_unit@1700000 */
struct metal_buserror __metal_dt_bus_error_unit_1700000 = {
	.__no_empty_structs = 0,
};

/* From bus_error_unit@1704000 */
struct metal_buserror __metal_dt_bus_error_unit_1704000 = {
	.__no_empty_structs = 0,
};

/* From bus_error_unit@1708000 */
struct metal_buserror __metal_dt_bus_error_unit_1708000 = {
	.__no_empty_structs = 0,
};

/* From bus_error_unit@170c000 */
struct metal_buserror __metal_dt_bus_error_unit_170c000 = {
	.__no_empty_structs = 0,
};

/* From bus_error_unit@1710000 */
struct metal_buserror __metal_dt_bus_error_unit_1710000 = {
	.__no_empty_structs = 0,
};

/* From bus_error_unit@1714000 */
struct metal_buserror __metal_dt_bus_error_unit_1714000 = {
	.__no_empty_structs = 0,
};

/* From bus_error_unit@1718000 */
struct metal_buserror __metal_dt_bus_error_unit_1718000 = {
	.__no_empty_structs = 0,
};

/* From bus_error_unit@171c000 */
struct metal_buserror __metal_dt_bus_error_unit_171c000 = {
	.__no_empty_structs = 0,
};

#endif /* METAL_INLINE_H*/
#endif /* ! ASSEMBLY */
