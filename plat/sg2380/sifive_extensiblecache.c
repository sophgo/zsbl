/* Copyright 2023 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <io.h>
#include <sifive_extensiblecache0.h>
#include <machine.h>

/**
 * Defines
 */
/* Clock gating states */
#define CG_ENABLE 0
#define CG_DISABLE !CG_ENABLE

/* Naturally Aligned Power-of-Two encoding for an address range */
#define ALIGN_ADDR(addr, size) ((addr) & ~((size >> 5UL) - 1UL))
#define NAPOT_RANGE(size) (((size)-1UL) >> 6UL)
#define NAPOT_ADDR(addr, size)                         \
	ALIGN_ADDR((uintptr_t)addr, (uintptr_t)size) | \
		NAPOT_RANGE((uintptr_t)size)

/* Full Physical Address encoding for lock/flush operations */
#define FULL_LOCK_PA EC_CACHE_LOCK_REGION_PA_MASK
#define FULL_FLUSH_PA EC_CACHE_FLUSH_ADDRESS_PA_MASK

/**
 * Typedefs
 */
/*! @brief Slice's description structure */
struct slice_desc {
	unsigned long base; /**< The control port base address of a slice */
	size_t size; /**< The control port size of a slice */
};

/**
 * Private constants
 */
static const struct slice_desc _SLICES[SIFIVE_EXTENSIBLECACHE0_SLICE_COUNT] =
	METAL_SIFIVE_EXTENSIBLECACHE0_SLICES;

int sifive_extensiblecache0_clock_gating_enable(
	size_t slice_index, enum sifive_extensiblecache0_cg_options option)
{
	if (!(slice_index < SIFIVE_EXTENSIBLECACHE0_SLICE_COUNT)) {
		return -1;
	}

	unsigned cg_option;

	switch (option) {
	case SIFIVE_EXTENSIBLECACHE0_CG_SLICE_LEVEL:
		cg_option = EC_CHICKENBIT_CG_SLICE_LEVEL_MASK;
		break;
	case SIFIVE_EXTENSIBLECACHE0_CG_ART:
		cg_option = EC_CHICKENBIT_CG_ART_MASK;
		break;
	case SIFIVE_EXTENSIBLECACHE0_CG_CONTROL_BLOCK:
		cg_option = EC_CHICKENBIT_CG_CONTROL_BLOCK_MASK;
		break;
	case SIFIVE_EXTENSIBLECACHE0_CG_PERF_MONITOR:
		cg_option = EC_CHICKENBIT_CG_PERF_MONITOR_MASK;
		break;
	case SIFIVE_EXTENSIBLECACHE0_CG_BUCKET_MONITOR:
		cg_option = EC_CHICKENBIT_CG_BUCKET_MONITOR_MASK;
		break;
	case SIFIVE_EXTENSIBLECACHE0_CG_PPAMO:
		cg_option = EC_CHICKENBIT_CG_PPAMO_MASK;
		break;
	default:
		return -1;
	}

	EXTENSIBLECACHE_Type *const ec =
		(EXTENSIBLECACHE_Type *)_SLICES[slice_index].base;

	ec->CHICKENBIT = PREP_FIELD(ec->CHICKENBIT, cg_option, CG_ENABLE);

	return 0;
}

/**
 * Functions definitions
 */
int sifive_extensiblecache0_init(void)
{
	for (unsigned idx = 0U; idx < SIFIVE_EXTENSIBLECACHE0_SLICE_COUNT;
	     ++idx) {
		sifive_extensiblecache0_clock_gating_enable(
			idx, SIFIVE_EXTENSIBLECACHE0_CG_SLICE_LEVEL);
		sifive_extensiblecache0_clock_gating_enable(
			idx, SIFIVE_EXTENSIBLECACHE0_CG_ART);
		sifive_extensiblecache0_clock_gating_enable(
			idx, SIFIVE_EXTENSIBLECACHE0_CG_CONTROL_BLOCK);
		sifive_extensiblecache0_clock_gating_enable(
			idx, SIFIVE_EXTENSIBLECACHE0_CG_PERF_MONITOR);
		sifive_extensiblecache0_clock_gating_enable(
			idx, SIFIVE_EXTENSIBLECACHE0_CG_BUCKET_MONITOR);
		sifive_extensiblecache0_clock_gating_enable(
			idx, SIFIVE_EXTENSIBLECACHE0_CG_PPAMO);
	}
	return 0;
}
