/* Copyright 2022 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <sifive_platform.h>

#ifdef METAL_SIFIVE_L2PF2

#include <sifive_metal.h>
#include <sifive_l2pf2.h>
#include <stdint.h>

/* Macros to access memory mapped registers. */
#define REGW(x) *((volatile uint32_t *)(l2pf_base[hartid] + x))

/* Macros for register bit masks. */
#define REG_MASK_BITWIDTH_1 0x01
#define REG_MASK_BITWIDTH_2 0x03
#define REG_MASK_BITWIDTH_4 0x0f
#define REG_MASK_BITWIDTH_5 0x1f
#define REG_MASK_BITWIDTH_6 0x3f

/* Macros to specify register bit shift. */
#define REG_BITSHIFT_2 2
#define REG_BITSHIFT_4 4
#define REG_BITSHIFT_8 8
#define REG_BITSHIFT_9 9
#define REG_BITSHIFT_13 13
#define REG_BITSHIFT_14 14
#define REG_BITSHIFT_19 19
#define REG_BITSHIFT_20 20
#define REG_BITSHIFT_21 21
#define REG_BITSHIFT_28 28
#define REG_BITSHIFT_29 29

/* Array of base addresses with HART IDs as the index. */
unsigned long l2pf_base[] = METAL_SIFIVE_L2PF2_BASE_ADDR;
int l2pf_base_len = sizeof(l2pf_base) / sizeof(unsigned long);

/* Array of distance bits with HART IDs as the index. */
unsigned long l2pf_distance_bits[] = METAL_SIFIVE_L2PF2_DISTANCE_BITS;

void sifive_l2pf2_enable(void) {
    int hartid;
    __asm__ volatile("csrr %0, mhartid" : "=r"(hartid));

    if ((hartid < l2pf_base_len) && (l2pf_base[hartid] != 0UL)) {
        uint32_t val = REGW(METAL_SIFIVE_L2PF2_BASIC_CONTROL);

        /* Enable L2 prefetch unit for scalar load. */
        val |= REG_MASK_BITWIDTH_1;

        REGW(METAL_SIFIVE_L2PF2_BASIC_CONTROL) = val;

        val = REGW(METAL_SIFIVE_L2PF2_USER_CONTROL);

        /* Enable L2 prefetch unit for scalar store. */
        val |= (REG_MASK_BITWIDTH_1 << REG_BITSHIFT_19);

        REGW(METAL_SIFIVE_L2PF2_USER_CONTROL) = val;
    }
}

void sifive_l2pf2_disable(void) {
    int hartid;
    __asm__ volatile("csrr %0, mhartid" : "=r"(hartid));

    if ((hartid < l2pf_base_len) && (l2pf_base[hartid] != 0UL)) {
        uint32_t val = REGW(METAL_SIFIVE_L2PF2_BASIC_CONTROL);

        /* Disable L2 prefetch unit for scalar load. */
        val &= ~REG_MASK_BITWIDTH_1;

        REGW(METAL_SIFIVE_L2PF2_BASIC_CONTROL) = val;

        val = REGW(METAL_SIFIVE_L2PF2_USER_CONTROL);

        /* Disable L2 prefetch unit for scalar store */
        val &= ~(REG_MASK_BITWIDTH_1 << REG_BITSHIFT_19);

        // set qFullnessThrd to 0xf
        val |= 0xf;
        // disable vector load
        val &= ~(REG_MASK_BITWIDTH_1 << REG_BITSHIFT_20);
        // disable vector store
        val &= ~(REG_MASK_BITWIDTH_1 << REG_BITSHIFT_21);

        REGW(METAL_SIFIVE_L2PF2_USER_CONTROL) = val;
    }
}

static void _l2pf2_import_basic_ctrl_reg(sifive_l2pf2_config *config,
                                         const uint32_t val,
                                         const uint32_t distance_bits_mask) {
    config->ScalarLoadSupportEn = (val & REG_MASK_BITWIDTH_1);
    config->Dist = ((val >> REG_BITSHIFT_2) & distance_bits_mask);
    config->MaxAllowedDist = ((val >> REG_BITSHIFT_8) & distance_bits_mask);
    config->LinToExpThrd = ((val >> REG_BITSHIFT_14) & distance_bits_mask);
    config->CrossPageEn = ((val >> REG_BITSHIFT_28) & REG_MASK_BITWIDTH_1);
    config->ForgiveThrd = ((val >> REG_BITSHIFT_29) & REG_MASK_BITWIDTH_2);
}

static void _l2pf2_import_user_ctrl_reg(sifive_l2pf2_config *config,
                                        const uint32_t val) {
    config->QFullnessThrd = (val & REG_MASK_BITWIDTH_4);
    config->HitCacheThrd = ((val >> REG_BITSHIFT_4) & REG_MASK_BITWIDTH_5);
    config->HitMSHRThrd = ((val >> REG_BITSHIFT_9) & REG_MASK_BITWIDTH_4);
    config->Window = ((val >> REG_BITSHIFT_13) & REG_MASK_BITWIDTH_6);
    config->ScalarStoreSupportEn =
        ((val >> REG_BITSHIFT_19) & REG_MASK_BITWIDTH_1);
    config->VectorLoadSupportEn =
        ((val >> REG_BITSHIFT_20) & REG_MASK_BITWIDTH_1);
    config->VectorStoreSupportEn =
        ((val >> REG_BITSHIFT_21) & REG_MASK_BITWIDTH_1);
}

void sifive_l2pf2_get_config(sifive_l2pf2_config *config) {
    int hartid;
    __asm__ volatile("csrr %0, mhartid" : "=r"(hartid));
    uint32_t val;

    /* Check for NULL, valid base address. */
    if ((config) && (hartid < l2pf_base_len) && (l2pf_base[hartid] != 0UL)) {
        uint32_t distance_bits_mask = (1 << l2pf_distance_bits[hartid]) - 1;

        /* Get basic control register configuration values. */
        val = REGW(METAL_SIFIVE_L2PF2_BASIC_CONTROL);
        _l2pf2_import_basic_ctrl_reg(config, val, distance_bits_mask);

        /* Get L2 user bits control register configuration values. */
        val = REGW(METAL_SIFIVE_L2PF2_USER_CONTROL);
        _l2pf2_import_user_ctrl_reg(config, val);
    }
}

void sifive_l2pf2_set_config(sifive_l2pf2_config *config) {
    int hartid;
    __asm__ volatile("csrr %0, mhartid" : "=r"(hartid));
    uint32_t val;

    /* Check for NULL, valid base address. */
    if ((config) && (hartid < l2pf_base_len) && (l2pf_base[hartid] != 0UL)) {
        uint32_t distance_bits_mask = (1 << l2pf_distance_bits[hartid]) - 1;

        /* Set basic control register configuration values. */
        val = (uint32_t)(
            (config->ScalarLoadSupportEn & REG_MASK_BITWIDTH_1) |
            ((config->Dist & distance_bits_mask) << REG_BITSHIFT_2) |
            ((config->MaxAllowedDist & distance_bits_mask) << REG_BITSHIFT_8) |
            ((config->LinToExpThrd & distance_bits_mask) << REG_BITSHIFT_14) |
            ((config->CrossPageEn & REG_MASK_BITWIDTH_1) << REG_BITSHIFT_28) |
            ((config->ForgiveThrd & REG_MASK_BITWIDTH_1) << REG_BITSHIFT_29));

        /* Set L2 user bits control register configuration values. */
        REGW(METAL_SIFIVE_L2PF2_BASIC_CONTROL) = val;

        val = (uint32_t)(
            (config->QFullnessThrd & REG_MASK_BITWIDTH_4) |
            ((config->HitCacheThrd & REG_MASK_BITWIDTH_5) << REG_BITSHIFT_4) |
            ((config->HitMSHRThrd & REG_MASK_BITWIDTH_4) << REG_BITSHIFT_9) |
            ((config->Window & REG_MASK_BITWIDTH_6) << REG_BITSHIFT_13) |
            ((config->ScalarStoreSupportEn & REG_MASK_BITWIDTH_1)
             << REG_BITSHIFT_19) |
            ((config->VectorLoadSupportEn & REG_MASK_BITWIDTH_1)
             << REG_BITSHIFT_20) |
            ((config->VectorStoreSupportEn & REG_MASK_BITWIDTH_1)
             << REG_BITSHIFT_21));

        REGW(METAL_SIFIVE_L2PF2_USER_CONTROL) = val;
    }
}

void sifive_l2pf2_init(void) {
    sifive_l2pf2_config config;

    /* Basic control register initial configuration (0x15811). */
    config.ScalarLoadSupportEn = METAL_SIFIVE_L2PF2_SCALAR_LOAD_SUPPORTEN_DEFAULT;
    config.Dist = METAL_SIFIVE_L2PF2_DISTANCE_DEFAULT;
    config.MaxAllowedDist = METAL_SIFIVE_L2PF2_MAX_ALLOWED_DIST_DEFAULT;
    config.LinToExpThrd = METAL_SIFIVE_L2PF2_LIN_TO_EXP_THRD_DEFAULT;

    /* User control register initial configuration (0x38c84e). */
    config.QFullnessThrd = METAL_SIFIVE_L2PF2_QFULLNESS_THRD_DEFAULT;
    config.HitCacheThrd = METAL_SIFIVE_L2PF2_HITCACHE_THRD_DEFAULT;
    config.HitMSHRThrd = METAL_SIFIVE_L2PF2_HITMSHR_THRD_DEFAULT;
    config.Window = METAL_SIFIVE_L2PF2_WINDOW_DEFAULT;
    config.ScalarStoreSupportEn = METAL_SIFIVE_L2PF2_SCALAR_STORE_SUPPORTEN_DEFAULT;
    config.VectorLoadSupportEn = METAL_SIFIVE_L2PF2_VECTOR_LOAD_SUPPORTEN_DEFAULT;
    config.VectorStoreSupportEn = METAL_SIFIVE_L2PF2_VECTOR_STORE_SUPPORTEN_DEFAULT;

    sifive_l2pf2_set_config(&config);
}

#endif

typedef int no_empty_translation_units;
