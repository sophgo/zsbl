/* Copyright 2018 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#ifndef METAL__DRIVERS__SIFIVE_PLIC1_H
#define METAL__DRIVERS__SIFIVE_PLIC1_H

#include <sifive_compiler.h>
#include <riscv_cpu.h>

#ifdef METAL_SIFIVE_PLIC1

#define METAL_PLIC_SOURCE_MASK 0x1F
#define METAL_PLIC_SOURCE_SHIFT 5
#define METAL_PLIC_SOURCE_PRIORITY_SHIFT 2
#define METAL_PLIC_SOURCE_PENDING_SHIFT 0

struct __metal_driver_vtable_sifive_plic1 {
    struct metal_interrupt_vtable plic_vtable;
};

__METAL_DECLARE_VTABLE(__metal_driver_vtable_sifive_plic1)

#ifndef __METAL_PLIC_SUBINTERRUPTS
#define __METAL_PLIC_SUBINTERRUPTS 0
#endif

struct __metal_driver_sifive_plic1 {
    struct metal_interrupt controller;
    int init_done;
    metal_interrupt_handler_t metal_exint_table[__METAL_PLIC_SUBINTERRUPTS];
    __metal_interrupt_data metal_exdata_table[__METAL_PLIC_SUBINTERRUPTS];
};

#endif /* METAL_SIFIVE_PLIC1 */
#endif /* METAL__DRIVERS__SIFIVE_PLIC1_H */
