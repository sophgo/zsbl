/* Copyright 2018 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#ifndef METAL__DRIVERS__RISCV_CLINT0_H
#define METAL__DRIVERS__RISCV_CLINT0_H

#include <sifive_compiler.h>
#include <riscv_cpu.h>

struct __metal_driver_vtable_riscv_clint0 {
    struct metal_interrupt_vtable clint_vtable;
};

__METAL_DECLARE_VTABLE(__metal_driver_vtable_riscv_clint0)

struct __metal_driver_riscv_clint0 {
    struct metal_interrupt controller;
    int init_done;
};

int __metal_driver_riscv_clint0_command_request(
    struct metal_interrupt *controller, int command, void *data);

#endif
