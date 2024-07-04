/* Copyright 2018 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <sifive_shutdown.h>

// TODO:
void metal_shutdown(int code) {
    while (1) {
        __asm__ volatile("nop");
    }
}
