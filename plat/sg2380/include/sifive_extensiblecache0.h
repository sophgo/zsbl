/* Copyright 2023 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#ifndef METAL__DRIVERS__SIFIVE_EXTENSIBLECACHE0_H
#define METAL__DRIVERS__SIFIVE_EXTENSIBLECACHE0_H

/*!
 * @file sifive_extensiblecache0.h
 *
 * @brief A basic extensible cache controller API
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <sifive_metal.h>
#include <sifive_extensiblecache0_regs.h>

/*! @brief Data array configuration */
struct sifive_extensiblecache0_data_config {
    size_t data_capacity; /**< Data buffer capacity in bytes */
    size_t num_banks;     /**< Number of banks for data array */
    size_t line_size;     /**< Cacheline size in bytes */
};

/*! @brief Tag array configuration */
struct sifive_extensiblecache0_tag_config {
    size_t tag_capacity;
    size_t num_banks;
    size_t associativity;
};

/*! @brief Clock gating options */
enum sifive_extensiblecache0_cg_options {
    SIFIVE_EXTENSIBLECACHE0_CG_SLICE_LEVEL,
    SIFIVE_EXTENSIBLECACHE0_CG_ART,
    SIFIVE_EXTENSIBLECACHE0_CG_CONTROL_BLOCK,
    SIFIVE_EXTENSIBLECACHE0_CG_PERF_MONITOR,
    SIFIVE_EXTENSIBLECACHE0_CG_BUCKET_MONITOR,
    SIFIVE_EXTENSIBLECACHE0_CG_PPAMO,
};

/*! @brief Flush command options */
enum sifive_extensiblecache0_flush_options {
    SIFIVE_EXTENSIBLECACHE0_FLUSH_CLEAN,
    SIFIVE_EXTENSIBLECACHE0_FLUSH_CLEAN_INV,
};

int sifive_extensiblecache0_init(void);

/*! @brief Enable clock gating
 * @param slice_index Slice's index [0..SIFIVE_EXTENSIBLECACHE0_SLICE_COUNT - 1]
 * @param option Clock gating option
 * @return @c 0 if no error, a negative error code otherwise
 */
int sifive_extensiblecache0_clock_gating_enable(size_t slice_index,
                    enum sifive_extensiblecache0_cg_options option);

#endif /* METAL__DRIVERS__SIFIVE_EXTENSIBLECACHE0_H */
