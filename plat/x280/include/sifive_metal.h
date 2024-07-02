/* Copyright 2019-2022 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */
/* ----------------------------------- */
/* ----------------------------------- */

#ifndef ASSEMBLY

#include <sifive_platform.h>

#ifdef __METAL_MACHINE_MACROS

#ifndef MACROS_IF_METAL_H
#define MACROS_IF_METAL_H

#define __METAL_CLINT_NUM_PARENTS 16

#ifndef __METAL_CLINT_NUM_PARENTS
#define __METAL_CLINT_NUM_PARENTS 0
#endif
#define __METAL_PLIC_SUBINTERRUPTS 141

#define __METAL_PLIC_NUM_PARENTS 16


#endif /* MACROS_IF_METAL_H*/

#else /* ! __METAL_MACHINE_MACROS */

#ifndef MACROS_ELSE_METAL_H
#define MACROS_ELSE_METAL_H

#define __METAL_CLINT_2000000_INTERRUPTS 16

#define METAL_MAX_CLINT_INTERRUPTS 16

#define __METAL_CLINT_NUM_PARENTS 16

#define __METAL_INTERRUPT_CONTROLLER_C000000_INTERRUPTS 16

#define __METAL_PLIC_SUBINTERRUPTS 141

#define METAL_MAX_PLIC_INTERRUPTS 16

#define __METAL_PLIC_NUM_PARENTS 16

#define METAL_SIFIVE_CCACHE1_INTERRUPTS_COUNT 4

#define METAL_SIFIVE_CCACHE1_INTERRUPTS { 1, 2, 3, 4, }

#define METAL_SIFIVE_CCACHE1_INTERRUPT_PARENT &__metal_dt_interrupt_controller_c000000.controller

#define METAL_CACHE_DRIVER_PREFIX sifive_ccache1

#define METAL_SIFIVE_CCACHE1_PERFMON_COUNTERS 6

#define __METAL_DT_SIFIVE_CCACHE1_HANDLE (struct metal_cache *)NULL

#define METAL_MAX_LOCAL_EXT_INTERRUPTS 0

#define __METAL_GLOBAL_EXTERNAL_INTERRUPTS_INTERRUPTS 127

#define METAL_MAX_GLOBAL_EXT_INTERRUPTS 127

#define METAL_MAX_GPIO_INTERRUPTS 0

#define METAL_MAX_I2C0_INTERRUPTS 0

#define METAL_SIFIVE_L2PF2_BASE_ADDR {\
	METAL_SIFIVE_L2PF2_0_BASE_ADDRESS,\
	METAL_SIFIVE_L2PF2_1_BASE_ADDRESS,\
	METAL_SIFIVE_L2PF2_2_BASE_ADDRESS,\
	METAL_SIFIVE_L2PF2_3_BASE_ADDRESS,\
	METAL_SIFIVE_L2PF2_4_BASE_ADDRESS,\
	METAL_SIFIVE_L2PF2_5_BASE_ADDRESS,\
	METAL_SIFIVE_L2PF2_6_BASE_ADDRESS,\
	METAL_SIFIVE_L2PF2_7_BASE_ADDRESS,\
	}

#define METAL_SIFIVE_L2PF2_QUEUE_ENTRIES {\
	METAL_SIFIVE_L2PF2_0_QUEUE_ENTRIES,\
	METAL_SIFIVE_L2PF2_1_QUEUE_ENTRIES,\
	METAL_SIFIVE_L2PF2_2_QUEUE_ENTRIES,\
	METAL_SIFIVE_L2PF2_3_QUEUE_ENTRIES,\
	METAL_SIFIVE_L2PF2_4_QUEUE_ENTRIES,\
	METAL_SIFIVE_L2PF2_5_QUEUE_ENTRIES,\
	METAL_SIFIVE_L2PF2_6_QUEUE_ENTRIES,\
	METAL_SIFIVE_L2PF2_7_QUEUE_ENTRIES,\
	}

#define METAL_SIFIVE_L2PF2_WINDOW_BITS {\
	METAL_SIFIVE_L2PF2_0_WINDOW_BITS,\
	METAL_SIFIVE_L2PF2_1_WINDOW_BITS,\
	METAL_SIFIVE_L2PF2_2_WINDOW_BITS,\
	METAL_SIFIVE_L2PF2_3_WINDOW_BITS,\
	METAL_SIFIVE_L2PF2_4_WINDOW_BITS,\
	METAL_SIFIVE_L2PF2_5_WINDOW_BITS,\
	METAL_SIFIVE_L2PF2_6_WINDOW_BITS,\
	METAL_SIFIVE_L2PF2_7_WINDOW_BITS,\
	}

#define METAL_SIFIVE_L2PF2_DISTANCE_BITS {\
	METAL_SIFIVE_L2PF2_0_DISTANCE_BITS,\
	METAL_SIFIVE_L2PF2_1_DISTANCE_BITS,\
	METAL_SIFIVE_L2PF2_2_DISTANCE_BITS,\
	METAL_SIFIVE_L2PF2_3_DISTANCE_BITS,\
	METAL_SIFIVE_L2PF2_4_DISTANCE_BITS,\
	METAL_SIFIVE_L2PF2_5_DISTANCE_BITS,\
	METAL_SIFIVE_L2PF2_6_DISTANCE_BITS,\
	METAL_SIFIVE_L2PF2_7_DISTANCE_BITS,\
	}

#define METAL_SIFIVE_L2PF2_STREAMS {\
	METAL_SIFIVE_L2PF2_0_STREAMS,\
	METAL_SIFIVE_L2PF2_1_STREAMS,\
	METAL_SIFIVE_L2PF2_2_STREAMS,\
	METAL_SIFIVE_L2PF2_3_STREAMS,\
	METAL_SIFIVE_L2PF2_4_STREAMS,\
	METAL_SIFIVE_L2PF2_5_STREAMS,\
	METAL_SIFIVE_L2PF2_6_STREAMS,\
	METAL_SIFIVE_L2PF2_7_STREAMS,\
	}

#define METAL_SIFIVE_L2PF2_DISTANCE_DEFAULT 4U

#define METAL_SIFIVE_L2PF2_HITCACHE_THRD_DEFAULT 4U

#define METAL_SIFIVE_L2PF2_HITMSHR_THRD_DEFAULT 4U

#define METAL_SIFIVE_L2PF2_LIN_TO_EXP_THRD_DEFAULT 5U

#define METAL_SIFIVE_L2PF2_WINDOW_DEFAULT 6U

#define METAL_SIFIVE_L2PF2_QFULLNESS_THRD_DEFAULT 15U

#define METAL_SIFIVE_L2PF2_MAX_ALLOWED_DIST_DEFAULT 24U

#define METAL_SIFIVE_L2PF2_SCALAR_LOAD_SUPPORTEN_DEFAULT 1U

#define METAL_SIFIVE_L2PF2_SCALAR_STORE_SUPPORTEN_DEFAULT 1U

#define METAL_SIFIVE_L2PF2_VECTOR_LOAD_SUPPORTEN_DEFAULT 1U

#define METAL_SIFIVE_L2PF2_VECTOR_STORE_SUPPORTEN_DEFAULT 1U

#define METAL_SIFIVE_L2PF2_0_QUEUE_ENTRIES 0

#define METAL_SIFIVE_L2PF2_0_WINDOW_BITS 0

#define METAL_SIFIVE_L2PF2_0_DISTANCE_BITS 0

#define METAL_SIFIVE_L2PF2_0_STREAMS 0

#define METAL_SIFIVE_L2PF2_1_QUEUE_ENTRIES 0

#define METAL_SIFIVE_L2PF2_1_WINDOW_BITS 0

#define METAL_SIFIVE_L2PF2_1_DISTANCE_BITS 0

#define METAL_SIFIVE_L2PF2_1_STREAMS 0

#define METAL_SIFIVE_L2PF2_2_QUEUE_ENTRIES 0

#define METAL_SIFIVE_L2PF2_2_WINDOW_BITS 0

#define METAL_SIFIVE_L2PF2_2_DISTANCE_BITS 0

#define METAL_SIFIVE_L2PF2_2_STREAMS 0

#define METAL_SIFIVE_L2PF2_3_QUEUE_ENTRIES 0

#define METAL_SIFIVE_L2PF2_3_WINDOW_BITS 0

#define METAL_SIFIVE_L2PF2_3_DISTANCE_BITS 0

#define METAL_SIFIVE_L2PF2_3_STREAMS 0

#define METAL_SIFIVE_L2PF2_4_QUEUE_ENTRIES 0

#define METAL_SIFIVE_L2PF2_4_WINDOW_BITS 0

#define METAL_SIFIVE_L2PF2_4_DISTANCE_BITS 0

#define METAL_SIFIVE_L2PF2_4_STREAMS 0

#define METAL_SIFIVE_L2PF2_5_QUEUE_ENTRIES 0

#define METAL_SIFIVE_L2PF2_5_WINDOW_BITS 0

#define METAL_SIFIVE_L2PF2_5_DISTANCE_BITS 0

#define METAL_SIFIVE_L2PF2_5_STREAMS 0

#define METAL_SIFIVE_L2PF2_6_QUEUE_ENTRIES 0

#define METAL_SIFIVE_L2PF2_6_WINDOW_BITS 0

#define METAL_SIFIVE_L2PF2_6_DISTANCE_BITS 0

#define METAL_SIFIVE_L2PF2_6_STREAMS 0

#define METAL_SIFIVE_L2PF2_7_QUEUE_ENTRIES 0

#define METAL_SIFIVE_L2PF2_7_WINDOW_BITS 0

#define METAL_SIFIVE_L2PF2_7_DISTANCE_BITS 0

#define METAL_SIFIVE_L2PF2_7_STREAMS 0

#define METAL_PL2CACHE_DRIVER_PREFIX sifive_pl2cache0

#define METAL_SIFIVE_PL2CACHE0_BASE_ADDR {\
				METAL_SIFIVE_PL2CACHE0_0_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE0_1_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE0_2_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE0_3_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE0_4_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE0_5_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE0_6_BASE_ADDRESS,\
				METAL_SIFIVE_PL2CACHE0_7_BASE_ADDRESS,\
				}

#define METAL_SIFIVE_PL2CACHE0_PERFMON_COUNTERS 6

#define __METAL_DT_SIFIVE_PL2CACHE0_HANDLE (struct metal_cache *)NULL

#define METAL_MAX_PWM0_INTERRUPTS 0

#define METAL_MAX_PWM0_NCMP 0

#define METAL_MAX_UART_INTERRUPTS 0

#define METAL_MAX_SIMUART_INTERRUPTS 0


#include <riscv_clint0.h>
#include <riscv_cpu.h>
#include <sifive_plic1.h>
#include <sifive_buserror0.h>
#include <sifive_ccache1.h>
#include <sifive_l2pf2.h>
#include <sifive_pl2cache0.h>


/* From clint@2000000 */
extern struct __metal_driver_riscv_clint0 __metal_dt_clint_2000000;

/* From cpu@0 */
extern struct __metal_driver_cpu __metal_dt_cpu_0;

/* From cpu@1 */
extern struct __metal_driver_cpu __metal_dt_cpu_1;

/* From cpu@2 */
extern struct __metal_driver_cpu __metal_dt_cpu_2;

/* From cpu@3 */
extern struct __metal_driver_cpu __metal_dt_cpu_3;

/* From cpu@4 */
extern struct __metal_driver_cpu __metal_dt_cpu_4;

/* From cpu@5 */
extern struct __metal_driver_cpu __metal_dt_cpu_5;

/* From cpu@6 */
extern struct __metal_driver_cpu __metal_dt_cpu_6;

/* From cpu@7 */
extern struct __metal_driver_cpu __metal_dt_cpu_7;

extern struct __metal_driver_riscv_cpu_intc __metal_dt_cpu_0_interrupt_controller;

extern struct __metal_driver_riscv_cpu_intc __metal_dt_cpu_1_interrupt_controller;

extern struct __metal_driver_riscv_cpu_intc __metal_dt_cpu_2_interrupt_controller;

extern struct __metal_driver_riscv_cpu_intc __metal_dt_cpu_3_interrupt_controller;

extern struct __metal_driver_riscv_cpu_intc __metal_dt_cpu_4_interrupt_controller;

extern struct __metal_driver_riscv_cpu_intc __metal_dt_cpu_5_interrupt_controller;

extern struct __metal_driver_riscv_cpu_intc __metal_dt_cpu_6_interrupt_controller;

extern struct __metal_driver_riscv_cpu_intc __metal_dt_cpu_7_interrupt_controller;

/* From interrupt_controller@c000000 */
extern struct __metal_driver_sifive_plic1 __metal_dt_interrupt_controller_c000000;

/* From bus_error_unit@1700000 */
extern struct metal_buserror __metal_dt_bus_error_unit_1700000;

/* From bus_error_unit@1704000 */
extern struct metal_buserror __metal_dt_bus_error_unit_1704000;

/* From bus_error_unit@1708000 */
extern struct metal_buserror __metal_dt_bus_error_unit_1708000;

/* From bus_error_unit@170c000 */
extern struct metal_buserror __metal_dt_bus_error_unit_170c000;

/* From bus_error_unit@1710000 */
extern struct metal_buserror __metal_dt_bus_error_unit_1710000;

/* From bus_error_unit@1714000 */
extern struct metal_buserror __metal_dt_bus_error_unit_1714000;

/* From bus_error_unit@1718000 */
extern struct metal_buserror __metal_dt_bus_error_unit_1718000;

/* From bus_error_unit@171c000 */
extern struct metal_buserror __metal_dt_bus_error_unit_171c000;

/* From cache_controller@2010000 */
extern struct __metal_driver_sifive_ccache1 __metal_dt_cache_controller_2010000;


/* --------------------- sifive_clint0 ------------ */
static __inline__ unsigned long __metal_driver_sifive_clint0_control_base(struct metal_interrupt *controller)
{
	if ((uintptr_t)controller == (uintptr_t)&__metal_dt_clint_2000000) {
		return METAL_RISCV_CLINT0_2000000_BASE_ADDRESS;
	}
	else {
		return 0;
	}
}

static __inline__ unsigned long __metal_driver_sifive_clint0_control_size(struct metal_interrupt *controller)
{
	if ((uintptr_t)controller == (uintptr_t)&__metal_dt_clint_2000000) {
		return METAL_RISCV_CLINT0_2000000_SIZE;
	}
	else {
		return 0;
	}
}

static __inline__ int __metal_driver_sifive_clint0_num_interrupts(struct metal_interrupt *controller)
{
	if ((uintptr_t)controller == (uintptr_t)&__metal_dt_clint_2000000) {
		return METAL_MAX_CLINT_INTERRUPTS;
	}
	else {
		return 0;
	}
}

static __inline__ struct metal_interrupt * __metal_driver_sifive_clint0_interrupt_parents(struct metal_interrupt *controller, int idx)
{
	if (idx == 0) {
		return (struct metal_interrupt *)&__metal_dt_cpu_0_interrupt_controller.controller;
	}
	else if (idx == 1) {
		return (struct metal_interrupt *)&__metal_dt_cpu_0_interrupt_controller.controller;
	}
	else if (idx == 2) {
		return (struct metal_interrupt *)&__metal_dt_cpu_1_interrupt_controller.controller;
	}
	else if (idx == 3) {
		return (struct metal_interrupt *)&__metal_dt_cpu_1_interrupt_controller.controller;
	}
	else if (idx == 4) {
		return (struct metal_interrupt *)&__metal_dt_cpu_2_interrupt_controller.controller;
	}
	else if (idx == 5) {
		return (struct metal_interrupt *)&__metal_dt_cpu_2_interrupt_controller.controller;
	}
	else if (idx == 6) {
		return (struct metal_interrupt *)&__metal_dt_cpu_3_interrupt_controller.controller;
	}
	else if (idx == 7) {
		return (struct metal_interrupt *)&__metal_dt_cpu_3_interrupt_controller.controller;
	}
	else if (idx == 8) {
		return (struct metal_interrupt *)&__metal_dt_cpu_4_interrupt_controller.controller;
	}
	else if (idx == 9) {
		return (struct metal_interrupt *)&__metal_dt_cpu_4_interrupt_controller.controller;
	}
	else if (idx == 10) {
		return (struct metal_interrupt *)&__metal_dt_cpu_5_interrupt_controller.controller;
	}
	else if (idx == 11) {
		return (struct metal_interrupt *)&__metal_dt_cpu_5_interrupt_controller.controller;
	}
	else if (idx == 12) {
		return (struct metal_interrupt *)&__metal_dt_cpu_6_interrupt_controller.controller;
	}
	else if (idx == 13) {
		return (struct metal_interrupt *)&__metal_dt_cpu_6_interrupt_controller.controller;
	}
	else if (idx == 14) {
		return (struct metal_interrupt *)&__metal_dt_cpu_7_interrupt_controller.controller;
	}
	else if (idx == 15) {
		return (struct metal_interrupt *)&__metal_dt_cpu_7_interrupt_controller.controller;
	}
	else {
		return NULL;
	}
}

static __inline__ int __metal_driver_sifive_clint0_interrupt_lines(struct metal_interrupt *controller, int idx)
{
	if (idx == 0) {
		return 3;
	}
	else if (idx == 1) {
		return 7;
	}
	else if (idx == 2) {
		return 3;
	}
	else if (idx == 3) {
		return 7;
	}
	else if (idx == 4) {
		return 3;
	}
	else if (idx == 5) {
		return 7;
	}
	else if (idx == 6) {
		return 3;
	}
	else if (idx == 7) {
		return 7;
	}
	else if (idx == 8) {
		return 3;
	}
	else if (idx == 9) {
		return 7;
	}
	else if (idx == 10) {
		return 3;
	}
	else if (idx == 11) {
		return 7;
	}
	else if (idx == 12) {
		return 3;
	}
	else if (idx == 13) {
		return 7;
	}
	else if (idx == 14) {
		return 3;
	}
	else if (idx == 15) {
		return 7;
	}
	else {
		return 0;
	}
}


/* --------------------- cpu ------------ */
static __inline__ int __metal_driver_cpu_hartid(struct metal_cpu *cpu)
{
	if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_0) {
		return 0;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_1) {
		return 1;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_2) {
		return 2;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_3) {
		return 3;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_4) {
		return 4;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_5) {
		return 5;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_6) {
		return 6;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_7) {
		return 7;
	}
	else {
		return -1;
	}
}

static __inline__ unsigned long long __metal_driver_cpu_mtime_frequency(struct metal_cpu *cpu)
{
	if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_0) {
#ifdef MTIME_RATE_HZ_DEF
		return MTIME_RATE_HZ_DEF;
#else
		return 32768ULL;
#endif
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_1) {
#ifdef MTIME_RATE_HZ_DEF
		return MTIME_RATE_HZ_DEF;
#else
		return 32768ULL;
#endif
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_2) {
#ifdef MTIME_RATE_HZ_DEF
		return MTIME_RATE_HZ_DEF;
#else
		return 32768ULL;
#endif
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_3) {
#ifdef MTIME_RATE_HZ_DEF
		return MTIME_RATE_HZ_DEF;
#else
		return 32768ULL;
#endif
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_4) {
#ifdef MTIME_RATE_HZ_DEF
		return MTIME_RATE_HZ_DEF;
#else
		return 32768ULL;
#endif
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_5) {
#ifdef MTIME_RATE_HZ_DEF
		return MTIME_RATE_HZ_DEF;
#else
		return 32768ULL;
#endif
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_6) {
#ifdef MTIME_RATE_HZ_DEF
		return MTIME_RATE_HZ_DEF;
#else
		return 32768ULL;
#endif
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_7) {
#ifdef MTIME_RATE_HZ_DEF
		return MTIME_RATE_HZ_DEF;
#else
		return 32768ULL;
#endif
	}
	else {
		return 0;
	}
}

static __inline__ unsigned long long __metal_driver_cpu_clock_frequency(struct metal_cpu *cpu)
{
	if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_0) {
		return 1000000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_1) {
		return 0;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_2) {
		return 0;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_3) {
		return 0;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_4) {
		return 0;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_5) {
		return 0;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_6) {
		return 0;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_7) {
		return 0;
	}
	else {
		return 0;
	}
}

static __inline__ struct metal_interrupt * __metal_driver_cpu_interrupt_controller(struct metal_cpu *cpu)
{
	if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_0) {
		return &__metal_dt_cpu_0_interrupt_controller.controller;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_1) {
		return &__metal_dt_cpu_1_interrupt_controller.controller;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_2) {
		return &__metal_dt_cpu_2_interrupt_controller.controller;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_3) {
		return &__metal_dt_cpu_3_interrupt_controller.controller;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_4) {
		return &__metal_dt_cpu_4_interrupt_controller.controller;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_5) {
		return &__metal_dt_cpu_5_interrupt_controller.controller;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_6) {
		return &__metal_dt_cpu_6_interrupt_controller.controller;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_7) {
		return &__metal_dt_cpu_7_interrupt_controller.controller;
	}
	else {
		return NULL;
	}
}

static __inline__ int __metal_driver_cpu_num_pmp_regions(struct metal_cpu *cpu)
{
	if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_0) {
		return 8;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_1) {
		return 8;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_2) {
		return 8;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_3) {
		return 8;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_4) {
		return 8;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_5) {
		return 8;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_6) {
		return 8;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_7) {
		return 8;
	}
	else {
		return 0;
	}
}

static __inline__ struct metal_buserror * __metal_driver_cpu_buserror(struct metal_cpu *cpu)
{
	if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_0) {
		return &__metal_dt_bus_error_unit_1700000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_1) {
		return &__metal_dt_bus_error_unit_1704000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_2) {
		return &__metal_dt_bus_error_unit_1708000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_3) {
		return &__metal_dt_bus_error_unit_170c000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_4) {
		return &__metal_dt_bus_error_unit_1710000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_5) {
		return &__metal_dt_bus_error_unit_1714000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_6) {
		return &__metal_dt_bus_error_unit_1718000;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_7) {
		return &__metal_dt_bus_error_unit_171c000;
	}
	else {
		return NULL;
	}
}

static __inline__ int __metal_driver_cpu_is_worldguard_aware(struct metal_cpu *cpu)
{
	if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_0) {
		return 0;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_1) {
		return 0;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_2) {
		return 0;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_3) {
		return 0;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_4) {
		return 0;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_5) {
		return 0;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_6) {
		return 0;
	}
	else if ((uintptr_t)cpu == (uintptr_t)&__metal_dt_cpu_7) {
		return 0;
	}
	else {
		return 0;
	}
}



/* --------------------- sifive_plic1 ------------ */
static __inline__ unsigned long __metal_driver_sifive_plic1_control_base(struct metal_interrupt *controller)
{
	if ((uintptr_t)controller == (uintptr_t)&__metal_dt_interrupt_controller_c000000) {
		return METAL_SIFIVE_PLIC1_C000000_BASE_ADDRESS;
	}
	else {
		return 0;
	}
}

static __inline__ unsigned long __metal_driver_sifive_plic1_control_size(struct metal_interrupt *controller)
{
	if ((uintptr_t)controller == (uintptr_t)&__metal_dt_interrupt_controller_c000000) {
		return METAL_SIFIVE_PLIC1_C000000_SIZE;
	}
	else {
		return 0;
	}
}

static __inline__ int __metal_driver_sifive_plic1_num_interrupts(struct metal_interrupt *controller)
{
	if ((uintptr_t)controller == (uintptr_t)&__metal_dt_interrupt_controller_c000000) {
		return METAL_SIFIVE_PLIC1_C000000_RISCV_NDEV;
	}
	else {
		return 0;
	}
}

static __inline__ int __metal_driver_sifive_plic1_max_priority(struct metal_interrupt *controller)
{
	if ((uintptr_t)controller == (uintptr_t)&__metal_dt_interrupt_controller_c000000) {
		return METAL_SIFIVE_PLIC1_C000000_RISCV_MAX_PRIORITY;
	}
	else {
		return 0;
	}
}

static __inline__ struct metal_interrupt * __metal_driver_sifive_plic1_interrupt_parents(struct metal_interrupt *controller, int idx)
{
	if (idx == 0) {
		return (struct metal_interrupt *)&__metal_dt_cpu_0_interrupt_controller.controller;
	}
	else if (idx == 1) {
		return (struct metal_interrupt *)&__metal_dt_cpu_0_interrupt_controller.controller;
	}
	else if (idx == 2) {
		return (struct metal_interrupt *)&__metal_dt_cpu_1_interrupt_controller.controller;
	}
	else if (idx == 3) {
		return (struct metal_interrupt *)&__metal_dt_cpu_1_interrupt_controller.controller;
	}
	else if (idx == 4) {
		return (struct metal_interrupt *)&__metal_dt_cpu_2_interrupt_controller.controller;
	}
	else if (idx == 5) {
		return (struct metal_interrupt *)&__metal_dt_cpu_2_interrupt_controller.controller;
	}
	else if (idx == 6) {
		return (struct metal_interrupt *)&__metal_dt_cpu_3_interrupt_controller.controller;
	}
	else if (idx == 7) {
		return (struct metal_interrupt *)&__metal_dt_cpu_3_interrupt_controller.controller;
	}
	else if (idx == 8) {
		return (struct metal_interrupt *)&__metal_dt_cpu_4_interrupt_controller.controller;
	}
	else if (idx == 9) {
		return (struct metal_interrupt *)&__metal_dt_cpu_4_interrupt_controller.controller;
	}
	else if (idx == 10) {
		return (struct metal_interrupt *)&__metal_dt_cpu_5_interrupt_controller.controller;
	}
	else if (idx == 11) {
		return (struct metal_interrupt *)&__metal_dt_cpu_5_interrupt_controller.controller;
	}
	else if (idx == 12) {
		return (struct metal_interrupt *)&__metal_dt_cpu_6_interrupt_controller.controller;
	}
	else if (idx == 13) {
		return (struct metal_interrupt *)&__metal_dt_cpu_6_interrupt_controller.controller;
	}
	else if (idx == 14) {
		return (struct metal_interrupt *)&__metal_dt_cpu_7_interrupt_controller.controller;
	}
	else if (idx == 15) {
		return (struct metal_interrupt *)&__metal_dt_cpu_7_interrupt_controller.controller;
	}
	else {
		return NULL;
	}
}

static __inline__ int __metal_driver_sifive_plic1_interrupt_lines(struct metal_interrupt *controller, int idx)
{
	if (idx == 0) {
		return 11;
	}
	else if (idx == 1) {
		return 9;
	}
	else if (idx == 2) {
		return 11;
	}
	else if (idx == 3) {
		return 9;
	}
	else if (idx == 4) {
		return 11;
	}
	else if (idx == 5) {
		return 9;
	}
	else if (idx == 6) {
		return 11;
	}
	else if (idx == 7) {
		return 9;
	}
	else if (idx == 8) {
		return 11;
	}
	else if (idx == 9) {
		return 9;
	}
	else if (idx == 10) {
		return 11;
	}
	else if (idx == 11) {
		return 9;
	}
	else if (idx == 12) {
		return 11;
	}
	else if (idx == 13) {
		return 9;
	}
	else if (idx == 14) {
		return 11;
	}
	else if (idx == 15) {
		return 9;
	}
	else {
		return 0;
	}
}

static __inline__ int __metal_driver_sifive_plic1_context_ids(int hartid)
{
	if (hartid == 0) {
		return 0;
	}
	else if (hartid == 1) {
		return 2;
	}
	else if (hartid == 2) {
		return 4;
	}
	else if (hartid == 3) {
		return 6;
	}
	else if (hartid == 4) {
		return 8;
	}
	else if (hartid == 5) {
		return 10;
	}
	else if (hartid == 6) {
		return 12;
	}
	else if (hartid == 7) {
		return 14;
	}
	else {
		return -1;
	}
}



/* --------------------- sifive_buserror0 ------------ */
static __inline__ uintptr_t __metal_driver_sifive_buserror0_control_base(const struct metal_buserror *buserror)
{
	if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1700000) {
		return METAL_SIFIVE_BUSERROR0_1700000_BASE_ADDRESS;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1704000) {
		return METAL_SIFIVE_BUSERROR0_1704000_BASE_ADDRESS;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1708000) {
		return METAL_SIFIVE_BUSERROR0_1708000_BASE_ADDRESS;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_170c000) {
		return METAL_SIFIVE_BUSERROR0_170C000_BASE_ADDRESS;
	}
	else {
		return 0;
	}
}

static __inline__ struct metal_interrupt * __metal_driver_sifive_buserror0_interrupt_parent(const struct metal_buserror *buserror)
{
	if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1700000) {
		return (struct metal_interrupt *)&__metal_dt_interrupt_controller_c000000.controller;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1704000) {
		return NULL;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1708000) {
		return NULL;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_170c000) {
		return NULL;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1710000) {
		return NULL;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1714000) {
		return NULL;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1718000) {
		return NULL;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_171c000) {
		return NULL;
	}
	else {
		return NULL;
	}
}

static __inline__ int __metal_driver_sifive_buserror0_interrupt_id(const struct metal_buserror *buserror)
{
	if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1700000) {
		return 133;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1704000) {
		return 134;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1708000) {
		return 135;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_170c000) {
		return 136;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1710000) {
		return 137;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1714000) {
		return 138;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_1718000) {
		return 139;
	}
	else if ((uintptr_t)buserror == (uintptr_t)&__metal_dt_bus_error_unit_171c000) {
		return 140;
	}
	else {
		return 0;
	}
}



/* --------------------- sifive_local_external_interrupts0 ------------ */


/* --------------------- sifive_global_external_interrupts0 ------------ */


/* --------------------- sifive_gpio0 ------------ */


/* --------------------- sifive_gpio_button ------------ */


/* --------------------- sifive_gpio_led ------------ */


/* --------------------- sifive_gpio_switch ------------ */


/* --------------------- sifive_i2c0 ------------ */


/* --------------------- sifive_prci0 ------------ */


/* --------------------- sifive_pwm0 ------------ */


/* --------------------- sifive_remapper2 ------------ */


/* --------------------- sifive_rtc0 ------------ */



/* --------------------- sifive_test0 ------------ */


/* --------------------- sifive_trace0 ------------ */


/* --------------------- sifive_uart0 ------------ */


/* --------------------- sifive_simuart0 ------------ */


/* --------------------- sifive_wdog0 ------------ */


/* --------------------- sifive_fe310_g000_hfrosc ------------ */


/* --------------------- sifive_fe310_g000_hfxosc ------------ */


/* --------------------- sifive_fe310_g000_lfrosc ------------ */


/* --------------------- sifive_fe310_g000_pll ------------ */


/* --------------------- sifive_fe310_g000_prci ------------ */


/* From clint@2000000 */
#define __METAL_DT_RISCV_CLINT0_HANDLE (&__metal_dt_clint_2000000.controller)

#define __METAL_DT_CLINT_2000000_HANDLE (&__metal_dt_clint_2000000.controller)

#define __METAL_DT_MAX_HARTS CONFIG_SMP_NUM

#define __METAL_CPU_0_ICACHE_HANDLE 1

#define __METAL_CPU_0_DCACHE_HANDLE 1

#define __METAL_CPU_1_ICACHE_HANDLE 1

#define __METAL_CPU_1_DCACHE_HANDLE 1

#define __METAL_CPU_2_ICACHE_HANDLE 1

#define __METAL_CPU_2_DCACHE_HANDLE 1

#define __METAL_CPU_3_ICACHE_HANDLE 1

#define __METAL_CPU_3_DCACHE_HANDLE 1

#define __METAL_CPU_4_ICACHE_HANDLE 1

#define __METAL_CPU_4_DCACHE_HANDLE 1

#define __METAL_CPU_5_ICACHE_HANDLE 1

#define __METAL_CPU_5_DCACHE_HANDLE 1

#define __METAL_CPU_6_ICACHE_HANDLE 1

#define __METAL_CPU_6_DCACHE_HANDLE 1

#define __METAL_CPU_7_ICACHE_HANDLE 1

#define __METAL_CPU_7_DCACHE_HANDLE 1

struct __metal_driver_cpu *__metal_cpu_table[] __attribute__((weak))  = {
					&__metal_dt_cpu_0,
					&__metal_dt_cpu_1,
					&__metal_dt_cpu_2,
					&__metal_dt_cpu_3};

/* From interrupt_controller@c000000 */
#define __METAL_DT_SIFIVE_PLIC1_HANDLE (&__metal_dt_interrupt_controller_c000000.controller)

#define __METAL_DT_INTERRUPT_CONTROLLER_C000000_HANDLE (&__metal_dt_interrupt_controller_c000000.controller)

#define __METAL_DT_PMP_HANDLE (&__metal_dt_pmp)

/* From global_external_interrupts */
#define __METAL_DT_SIFIVE_GLOBAL_EXINTR0_HANDLE (&__metal_dt_global_external_interrupts.irc)

#define __METAL_DT_GLOBAL_EXTERNAL_INTERRUPTS_HANDLE (&__metal_dt_global_external_interrupts.irc)

#define __MEE_DT_MAX_GPIOS 0

#define __METAL_DT_MAX_BUTTONS 0

#define __METAL_DT_MAX_LEDS 0

#define __METAL_DT_MAX_SWITCHES 0

#define __METAL_DT_MAX_I2CS 0

#define __METAL_DT_MAX_PWMS 0

#define __METAL_DT_MAX_RTCS 0

#define __METAL_DT_MAX_SPIS 0


#endif /* MACROS_ELSE_METAL_H*/

#endif /* ! __METAL_MACHINE_MACROS */

#endif /* ! ASSEMBLY */
