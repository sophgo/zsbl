/* Copyright 2021 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#ifndef METAL__DRIVERS__SIFIVE_PL2CACHE0_H
#define METAL__DRIVERS__SIFIVE_PL2CACHE0_H

/*!
 * @file sifive_pl2cache0.h
 *
 * @brief API for configuring the SiFive private L2 cache controller
 */

#include <stdbool.h>
#include <stdint.h>

/*! @brief Cache configuration data */
typedef struct {
    uint32_t num_bank;
    uint32_t num_ways;
    uint32_t num_sets;
    uint32_t block_size;
} sifive_pl2cache0_config;

typedef union _sifive_pl2cache0_configbits {
    struct {
        uint32_t _reserved0 : 3;
        uint32_t cleanEvictEnable : 1;
        uint32_t _reserved1 : 5;
        uint32_t l2AvoidL1LineDisable : 1;
        uint32_t _reserved2 : 6;
        uint32_t softwareEccInjectEnable : 1;
        uint32_t errInjectOnWriteEnable : 1;
        uint32_t errInjectOnReadEnable : 1;
        uint32_t dataUceInjectEnable : 1;
        uint32_t dirUceInjectEnable : 1;
        uint32_t _reserved3 : 11;
    } b;        /**< Structure used for bit access */
    uint32_t w; /**< Structure used for 32bits access */
} sifive_pl2cache0_configbits;

/*! @brief Set the cleanEvictEnable bit of the Private L2 cache controller.
 * @param val boolean parameter true(enable) or false(disable).
 * @return None.*/
void sifive_pl2cache0_set_cleanEvictEnable_bit(bool val);

/*! @brief Enable Private L2 cache regional clock gating.
 * @param None.
 * @return None.*/
void sifive_pl2cache_regional_clock_gate_enable(void);

/*! @brief Initialize Private L2 cache controller.
 * @param None.
 * @return None.*/
void sifive_pl2cache0_init(void);

#endif
