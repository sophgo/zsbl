/* Copyright 2021 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <sifive_platform.h>

#ifdef METAL_SIFIVE_CCACHE1

#include <sifive_metal.h>
#include <sifive_ccache1_regs.h>
#include <sifive_ccache1.h>
#include <stdint.h>

/* Macros to access memory mapped registers */
#define REGW(x) *(volatile uint32_t *)(METAL_SIFIVE_CCACHE1_0_BASE_ADDRESS + x)

#define REGD(x) *(volatile uint64_t *)(METAL_SIFIVE_CCACHE1_0_BASE_ADDRESS + x)

/* Macros to specify register bit shift */
#define REG_SHIFT_4 4
#define REG_SHIFT_8 8
#define REG_SHIFT_16 16
#define REG_SHIFT_24 24

#define SIFIVE_CCACHE1_BYTE_MASK 0xFFUL

#if METAL_SIFIVE_CCACHE1_INTERRUPTS_COUNT != 0
static int sifive_ccache1_interrupts[] = METAL_SIFIVE_CCACHE1_INTERRUPTS;
#endif

/* Linker symbols to calculate LIM allocated size */
/* TODO: */
#if 0
extern char metal_segment_lim_target_start, metal_segment_lim_target_end;
/* Linker symbols for the bss section */
extern char __ld_bss_start, __ld_bss_end;
#endif

int sifive_ccache1_init(void) {
    sifive_ccache1_config config;

    /* Get cache configuration data */
    sifive_ccache1_get_config(&config);

    /* Enable Clock Gating Feature */
    sifive_ccache1_clock_gating_enable(SIFIVE_CCACHE1_CLOCK_GATE_WHOLE);
    sifive_ccache1_clock_gating_enable(SIFIVE_CCACHE1_CLOCK_GATE_REGIONAL);

#if 0
    int lim_size =
        &metal_segment_lim_target_end - &metal_segment_lim_target_start;

    if (lim_size) { /* Do not enable cache ways, corresponding to LIM area in
                       use. */
        while (lim_size > 0) {
            lim_size -= (config.block_size * config.num_sets * config.num_bank);
            config.num_ways--;
        }
    }

    /* Sanity check */
    int ccache_size =
        config.block_size * config.num_sets * config.num_bank * config.num_ways;
    char *tmp;

    tmp = &__ld_bss_start;
    if ((tmp >= &metal_segment_lim_target_start) &&
        (tmp < (&metal_segment_lim_target_start + ccache_size))) {
        return -1;
    }

    tmp = &__ld_bss_end;
    if ((tmp >= &metal_segment_lim_target_start) &&
        (tmp < (&metal_segment_lim_target_start + ccache_size))) {
        return -1;
    }
#endif

    /* Enable ways */
    return sifive_ccache1_set_enabled_ways(config.num_ways);
}

void sifive_ccache1_get_config(sifive_ccache1_config *config) {
    uint32_t val;

    if (config) /* Check for NULL */
    {
        val = REGW(METAL_SIFIVE_CCACHE1_CONFIG);

        /* Populate cache configuration data */
        config->num_bank = (val & SIFIVE_CCACHE1_BYTE_MASK);
        config->num_ways = ((val >> REG_SHIFT_8) & SIFIVE_CCACHE1_BYTE_MASK);
        /* no. of sets, block size is 2's power of register value
        (2 << (value-1)) */
        config->num_sets =
            2 << (((val >> REG_SHIFT_16) & SIFIVE_CCACHE1_BYTE_MASK) - 1);
        config->block_size =
            2 << (((val >> REG_SHIFT_24) & SIFIVE_CCACHE1_BYTE_MASK) - 1);
    }
}

uint32_t sifive_ccache1_get_enabled_ways(void) {

    uint32_t val = 0;

    val = SIFIVE_CCACHE1_BYTE_MASK & REGW(METAL_SIFIVE_CCACHE1_WAYENABLE);

    /* The stored number is the way index, so increment by 1 */
    val++;

    return val;
}

int sifive_ccache1_set_enabled_ways(uint32_t ways) {

    int ret = 0;

    /* We can't decrease the number of enabled ways */
    if (sifive_ccache1_get_enabled_ways() > ways) {
        ret = -1;
    } else {
        /* The stored value is the index, so subtract one */
        uint32_t value = 0xFF & (ways - 1);

        /* Set the number of enabled ways */
        REGW(METAL_SIFIVE_CCACHE1_WAYENABLE) = value;

        /* Make sure the number of ways was set correctly */
        if (sifive_ccache1_get_enabled_ways() != ways) {
            ret = -2;
        }
    }

    return ret;
}

void sifive_ccache1_inject_ecc_error(uint32_t bitindex,
                                     sifive_ccache1_ecc_errtype_t type) {
    /* Induce ECC error at given bit index and location */
    REGW(METAL_SIFIVE_CCACHE1_ECCINJECTERROR) =
        (uint32_t)(((type & 0x01) << REG_SHIFT_16) | (bitindex & 0xFF));
}

void sifive_ccache1_flush(uintptr_t flush_addr) {
    /* Block memory access until operation completed */
    __asm volatile("fence rw, io" : : : "memory");

#if __riscv_xlen == 32
    REGW(METAL_SIFIVE_CCACHE1_FLUSH32) = flush_addr >> REG_SHIFT_4;
#else
    REGD(METAL_SIFIVE_CCACHE1_FLUSH64) = flush_addr;
#endif

    __asm volatile("fence io, rw" : : : "memory");
}

uintptr_t sifive_ccache1_get_ecc_fix_addr(sifive_ccache1_ecc_errtype_t type) {
    uintptr_t addr = 0;

    switch (type) {
        /* Get most recently ECC corrected address */
    case SIFIVE_CCACHE1_DATA:
        addr = (uintptr_t)REGD(METAL_SIFIVE_CCACHE1_DATECCFIXLOW);
        break;

    case SIFIVE_CCACHE1_DIR:
        addr = (uintptr_t)REGD(METAL_SIFIVE_CCACHE1_DIRECCFIXLOW);
        break;
    }

    return addr;
}

uint32_t sifive_ccache1_get_ecc_fix_count(sifive_ccache1_ecc_errtype_t type) {
    uint32_t count = 0;

    switch (type) {
        /* Get number of times ECC errors were corrected */
    case SIFIVE_CCACHE1_DATA:
        count = REGW(METAL_SIFIVE_CCACHE1_DATECCFIXCOUNT);
        break;

    case SIFIVE_CCACHE1_DIR:
        count = REGW(METAL_SIFIVE_CCACHE1_DIRECCFIXCOUNT);
        break;
    }

    return count;
}

uintptr_t sifive_ccache1_get_ecc_fail_addr(sifive_ccache1_ecc_errtype_t type) {
    uintptr_t addr = 0;

    switch (type) {
        /*  Get address location of most recent uncorrected ECC error */
    case SIFIVE_CCACHE1_DATA:
        addr = (uintptr_t)REGD(METAL_SIFIVE_CCACHE1_DATECCFAILLOW);
        break;

    case SIFIVE_CCACHE1_DIR:
        addr = (uintptr_t)REGD(METAL_SIFIVE_CCACHE1_DIRECCFAILLOW);
        break;
    }

    return addr;
}

uint32_t sifive_ccache1_get_ecc_fail_count(sifive_ccache1_ecc_errtype_t type) {
    uint32_t count = 0;

    switch (type) {
        /* Get number of times ECC errors were not corrected */
    case SIFIVE_CCACHE1_DATA:
        count = REGW(METAL_SIFIVE_CCACHE1_DATECCFAILCOUNT);
        break;

    case SIFIVE_CCACHE1_DIR:
        count = REGW(METAL_SIFIVE_CCACHE1_DIRECCFAILCOUNT);
        break;
    }

    return count;
}

uint64_t sifive_ccache1_get_way_mask(uint32_t master_id) {
    uint64_t val = 0;

    /* Get way mask for given master ID */
    val = REGD(METAL_SIFIVE_CCACHE1_WAYMASK0 + master_id * 8);

    return val;
}

int sifive_ccache1_set_way_mask(uint32_t master_id, uint64_t waymask) {

    /* Set way mask for given master ID */
    REGD(METAL_SIFIVE_CCACHE1_WAYMASK0 + master_id * 8) = waymask;

    return 0;
}

void sifive_ccache1_set_pmevent_selector(uint32_t counter, uint64_t mask) {

#if METAL_SIFIVE_CCACHE1_PERFMON_COUNTERS > 0
    if (counter < METAL_SIFIVE_CCACHE1_PERFMON_COUNTERS) {

        /* Set event selector for specified L2 event counter */
        REGD(METAL_SIFIVE_CCACHE1_PMEVENTSELECT0 + counter * 8) = mask;
    }
#endif
    return;
}

uint64_t sifive_ccache1_get_pmevent_selector(uint32_t counter) {
    uint64_t val = 0;

#if METAL_SIFIVE_CCACHE1_PERFMON_COUNTERS > 0
    if (counter < METAL_SIFIVE_CCACHE1_PERFMON_COUNTERS) {

        /* Get event selector for specified L2 event counter */
        val = REGD(METAL_SIFIVE_CCACHE1_PMEVENTSELECT0 + counter * 8);
    }
#endif
    return val;
}

void sifive_ccache1_clr_pmevent_counter(uint32_t counter) {

#if METAL_SIFIVE_CCACHE1_PERFMON_COUNTERS > 0
    if (counter < METAL_SIFIVE_CCACHE1_PERFMON_COUNTERS) {
        /* Clear specified L2 event counter */
        REGD(METAL_SIFIVE_CCACHE1_PMEVENTCOUNTER0 + counter * 8) = 0;
    }
#endif
    return;
}

uint64_t sifive_ccache1_get_pmevent_counter(uint32_t counter) {
#if __riscv_xlen == 32
    uint32_t vh = 0, vh1 = 0, vl = 0;
#else
    uint64_t val = 0;
#endif
#if METAL_SIFIVE_CCACHE1_PERFMON_COUNTERS > 0
    if (counter < METAL_SIFIVE_CCACHE1_PERFMON_COUNTERS) {
        /* Set counter register offset */
        counter *= 8;

#if __riscv_xlen == 32
        do {
            vh = REGW(METAL_SIFIVE_CCACHE1_PMEVENTCOUNTER0 + counter + 4);
            vl = REGW(METAL_SIFIVE_CCACHE1_PMEVENTCOUNTER0 + counter);
            vh1 = REGW(METAL_SIFIVE_CCACHE1_PMEVENTCOUNTER0 + counter + 4);
        } while (vh != vh1);
#else
        val = REGD(METAL_SIFIVE_CCACHE1_PMEVENTCOUNTER0 + counter);
#endif
    }
#endif
#if __riscv_xlen == 32
    return ((((unsigned long long)vh) << 32) | vl);
#else
    return val;
#endif
}

void sifive_ccache1_set_client_filter(uint64_t mask) {

    /* Set clients to be excluded from performance monitoring */
    REGD(METAL_SIFIVE_CCACHE1_PMCLIENTFILTER) = mask;
}

uint64_t sifive_ccache1_get_client_filter(void) {
    uint64_t val = 0;

    /* Get currently active client filter mask */
    val = REGD(METAL_SIFIVE_CCACHE1_PMCLIENTFILTER);

    return val;
}

int sifive_ccache1_get_interrupt_id(uint32_t src) {
    int ret = 0;

#if METAL_SIFIVE_CCACHE1_INTERRUPTS_COUNT != 0
    if (src < (uint32_t)sizeof(sifive_ccache1_interrupts) / sizeof(int)) {
        ret = sifive_ccache1_interrupts[src];
    }
#endif

    return ret;
}

struct metal_interrupt *sifive_ccache1_interrupt_controller(void) {
    return METAL_SIFIVE_CCACHE1_INTERRUPT_PARENT;
}


void sifive_ccache1_clock_gating_enable(sifive_ccache1_clock_gate_featuretype_t feature_type) {

    volatile COMPOSABLECACHE_Type *const ccache_map = (volatile COMPOSABLECACHE_Type *const) METAL_SIFIVE_CCACHE1_0_BASE_ADDRESS;
    switch (feature_type) {
        case SIFIVE_CCACHE1_CLOCK_GATE_WHOLE:
            ccache_map->FEATUREDISABLE.b.DISABLETRUNKCLOCKGATE = 0; // clear trunk clock gating feature disable bit
            break;

        case SIFIVE_CCACHE1_CLOCK_GATE_REGIONAL:
            ccache_map->FEATUREDISABLE.b.DISABLECLOCKGATE = 0; // clear regional clock gating feature disable bit
            break;

        case SIFIVE_CCACHE1_CLOCK_GATE_REGIONAL_AGGRESSIVE:
            ccache_map->FEATUREDISABLE.b.DISABLECLOCKGATESLOW = 0; // clear aggressive regional clock gating feature disable bit
            ccache_map->FEATUREDISABLE.b.DISABLECLOCKGATE = 0; // ... and clear regional clock gating feature disable bit
            break;

        default:
            return;
    }
}

void sifive_ccache1_clock_gating_disable(sifive_ccache1_clock_gate_featuretype_t feature_type) {

    volatile COMPOSABLECACHE_Type *const ccache_map = (volatile COMPOSABLECACHE_Type *const) METAL_SIFIVE_CCACHE1_0_BASE_ADDRESS;
    switch (feature_type) {
        case SIFIVE_CCACHE1_CLOCK_GATE_WHOLE:
            ccache_map->FEATUREDISABLE.b.DISABLETRUNKCLOCKGATE = 1; // set trunk clock gating feature disable bit
            break;

        case SIFIVE_CCACHE1_CLOCK_GATE_REGIONAL:
            ccache_map->FEATUREDISABLE.b.DISABLECLOCKGATE = 1; // set regional clock gating feature disable bit
            break;

        case SIFIVE_CCACHE1_CLOCK_GATE_REGIONAL_AGGRESSIVE:
            ccache_map->FEATUREDISABLE.b.DISABLECLOCKGATESLOW = 1; // set aggressive regional clock gating feature disable bit
            ccache_map->FEATUREDISABLE.b.DISABLECLOCKGATE = 1; // ... and set regional clock gating feature disable bit
            break;

        default:
            return;
    }
}

bool sifive_ccache1_clock_gating_get_state(sifive_ccache1_clock_gate_featuretype_t feature_type) {

    volatile COMPOSABLECACHE_Type *const ccache_map = (volatile COMPOSABLECACHE_Type *const) METAL_SIFIVE_CCACHE1_0_BASE_ADDRESS;
    switch (feature_type) {
        case SIFIVE_CCACHE1_CLOCK_GATE_WHOLE:
            return !ccache_map->FEATUREDISABLE.b.DISABLETRUNKCLOCKGATE; // inverse logic

        case SIFIVE_CCACHE1_CLOCK_GATE_REGIONAL:
            return !ccache_map->FEATUREDISABLE.b.DISABLECLOCKGATE;  // inverse logic

        case SIFIVE_CCACHE1_CLOCK_GATE_REGIONAL_AGGRESSIVE:
            return !ccache_map->FEATUREDISABLE.b.DISABLECLOCKGATESLOW; // inverse logic

        default:
            return 0;
    }
}

#endif

typedef int no_empty_translation_units;
