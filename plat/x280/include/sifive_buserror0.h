/* Copyright 2020 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#ifndef METAL__DRIVERS__SIFIVE_BUSERROR0_H
#define METAL__DRIVERS__SIFIVE_BUSERROR0_H

/*!
 * @file sifive_buserror0.h
 *
 * @brief API for configuring the SiFive Bus Error Unit
 */

#include <stdint.h>
#include <stdbool.h>

/*!
 * @brief The set of possible events handled by a SiFive Bus Error Unit
 */
typedef enum {
	/*! @brief No event or error has been detected */
	METAL_BUSERROR_EVENT_NONE = 0,

	/*! @brief A TileLink Bus error has occurred in the I$ */
	METAL_BUSERROR_EVENT_INST_BUS_ERROR = (1 << 1),
	/*! @brief A correctable ECC error has occurred in the I$ or ITIM */
	METAL_BUSERROR_EVENT_INST_CORRECTABLE_ECC_ERROR = (1 << 2),
	/*! @brief An uncorrectable ECC error has occurred in the I$ or ITIM */
	METAL_BUSERROR_EVENT_INST_UNCORRECTABLE_ECC_ERROR = (1 << 3),
	/*! @brief A TileLink load or store bus error has occurred */
	METAL_BUSERROR_EVENT_LOAD_STORE_ERROR = (1 << 5),
	/*! @brief A correctable ECC error has occurred in the D$ or DTIM */
	METAL_BUSERROR_EVENT_DATA_CORRECTABLE_ECC_ERROR = (1 << 6),
	/*! @brief An uncorrectable ECC error has occurred in the D$ or DTIM */
	METAL_BUSERROR_EVENT_DATA_UNCORRECTABLE_ECC_ERROR = (1 << 7),
	/*! @brief A TileLink Bus error has occurred in the Private L2 */
	METAL_BUSERROR_EVENT_PL2_BUS_ERROR = (1 << 9),
	/*! @brief A correctable ECC error has occurred in Private L2 */
	METAL_BUSERROR_EVENT_PL2_CORRECTABLE_ECC_ERROR = (1 << 10),
	/*! @brief An uncorrectable ECC error has occurred in Private L2 */
	METAL_BUSERROR_EVENT_PL2_UNCORRECTABLE_ECC_ERROR = (1 << 11),

	/*! @brief Used to set/clear all interrupts or query/clear all accrued
       events */
	METAL_BUSERROR_EVENT_ALL =
		METAL_BUSERROR_EVENT_INST_BUS_ERROR |
		METAL_BUSERROR_EVENT_INST_CORRECTABLE_ECC_ERROR |
		METAL_BUSERROR_EVENT_INST_UNCORRECTABLE_ECC_ERROR |
		METAL_BUSERROR_EVENT_LOAD_STORE_ERROR |
		METAL_BUSERROR_EVENT_DATA_CORRECTABLE_ECC_ERROR |
		METAL_BUSERROR_EVENT_DATA_UNCORRECTABLE_ECC_ERROR |
		METAL_BUSERROR_EVENT_PL2_BUS_ERROR |
		METAL_BUSERROR_EVENT_PL2_CORRECTABLE_ECC_ERROR |
		METAL_BUSERROR_EVENT_PL2_UNCORRECTABLE_ECC_ERROR,
	METAL_BUSERROR_EVENT_ANY = METAL_BUSERROR_EVENT_ALL,

	/*! @brief A value which is impossible for the bus error unit to report.
     * Indicates an error has occurred if provided as a return value. */
	METAL_BUSERROR_EVENT_INVALID = (1 << 8),
} metal_buserror_event_t;

/*!
 * @brief The handle for a bus error unit
 */
struct metal_buserror {
	uint8_t __no_empty_structs;
};

/*!
 * @brief Initialize a bus error unit
 *
 * Initialization enables all events
 */
void metal_buserror_init();

/*!
 * @brief Enable bus error events
 *
 * Enabling bus error events causes them to be registered as accrued and,
 * if the corresponding interrupt is inabled, trigger interrupts.
 *
 * @param beu The bus error unit handle
 * @param events A mask of error events to enable
 * @param enabled True if the mask should be enabled, false if they should be
 * disabled
 * @return 0 upon success
 */
int metal_buserror_set_event_enabled(struct metal_buserror *beu,
				     metal_buserror_event_t events,
				     bool enabled);

#endif
