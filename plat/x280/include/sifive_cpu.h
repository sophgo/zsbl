/* Copyright 2018 SiFive, Inc */

/* SPDX-License-Identifier: Apache-2.0 */

/*! @file cpu.h
 *  @brief API for accessing CPU capabilities.
 */

#ifndef METAL__CPU_H
#define METAL__CPU_H

#include <sifive_interrupt.h>
#include <stdint.h>

struct metal_cpu;

/*!
 * @brief Function signature for exception handlers
 */
typedef void (*metal_exception_handler_t)(struct metal_cpu *cpu, int ecode);
struct metal_buserror *__metal_driver_cpu_get_buserror(struct metal_cpu *cpu);

struct metal_cpu_vtable {
    unsigned long long (*mcycle_get)(struct metal_cpu *cpu);
    unsigned long long (*mcycle_timebase_get)(struct metal_cpu *cpu);
    unsigned long long (*mtime_get)(struct metal_cpu *cpu);
    unsigned long long (*mtime_timebase_get)(struct metal_cpu *cpu);
    int (*mtimecmp_set)(struct metal_cpu *cpu, unsigned long long time);
    struct metal_interrupt *(*tmr_controller_interrupt)(struct metal_cpu *cpu);
    int (*get_tmr_interrupt_id)(struct metal_cpu *cpu);
    struct metal_interrupt *(*sw_controller_interrupt)(struct metal_cpu *cpu);
    int (*get_sw_interrupt_id)(struct metal_cpu *cpu);
    int (*set_sw_ipi)(struct metal_cpu *cpu, int hartid);
    int (*clear_sw_ipi)(struct metal_cpu *cpu, int hartid);
    int (*get_msip)(struct metal_cpu *cpu, int hartid);
    struct metal_interrupt *(*controller_interrupt)(struct metal_cpu *cpu);
    int (*exception_register)(struct metal_cpu *cpu, int ecode,
                              metal_exception_handler_t handler);
    int (*get_ilen)(struct metal_cpu *cpu, uintptr_t epc);
    uintptr_t (*get_epc)(struct metal_cpu *cpu);
    int (*set_epc)(struct metal_cpu *cpu, uintptr_t epc);
    struct metal_buserror *(*get_buserror)(struct metal_cpu *cpu);
    void *(*get_cpu_specific)(struct metal_cpu *cpu);
    int (*is_worldguard_aware)(struct metal_cpu *cpu);
};

/*! @brief A device handle for a CPU hart
 */
struct metal_cpu {
    const struct metal_cpu_vtable *vtable;
    void *cpu_specific;
};

/*! @brief Get a reference to a CPU hart
 *
 * @param hartid The ID of the desired CPU hart
 * @return A pointer to the CPU device handle
 */
struct metal_cpu *metal_cpu_get(unsigned int hartid);

/*! @brief Get the hartid of the CPU hart executing this function
 *
 * @return The hartid of the current CPU hart */
int metal_cpu_get_current_hartid(void);

/*! @brief Get a reference to RTC timer interrupt controller
 *
 * Get a reference to the interrupt controller for the real-time clock
 * interrupt. The controller returned by this function must be initialized
 * before any interrupts are registered or enabled with it.
 *
 * @param cpu The CPU device handle
 * @return A pointer to the timer interrupt handle
 */
__inline__ struct metal_interrupt *
metal_cpu_timer_interrupt_controller(struct metal_cpu *cpu) {
    return cpu->vtable->tmr_controller_interrupt(cpu);
}

/*!
 * @brief Get the interrupt controller for the CPU
 *
 * Get the CPU interrupt controller. The controller returned by this
 * function must be initialized before any interrupts are registered
 * or enabled and before any exception handlers are registered with
 * this CPU.
 *
 * @param cpu The CPU device handle
 * @return The handle for the CPU interrupt controller
 */
__inline__ struct metal_interrupt *
metal_cpu_interrupt_controller(struct metal_cpu *cpu) {
    return cpu->vtable->controller_interrupt(cpu);
}

/*!
 * @brief Get the handle for the hart's bus error unit
 *
 * @param cpu The CPU device handle
 * @return A pointer to the bus error unit handle
 */
__inline__ struct metal_buserror *
metal_cpu_get_buserror(struct metal_cpu *cpu) {
    return cpu->vtable->get_buserror(cpu);
}

#endif
