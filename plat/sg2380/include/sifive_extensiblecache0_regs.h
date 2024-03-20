/**
 * Extensible cache registers
 *
 * @file sifive_extensiblecache0_regs.h
 *
 *
 * @copyright (c) 2023 SiFive, Inc
 * @copyright SPDX-License-Identifier: Apache-2.0
 */

#ifndef METAL__DRIVERS__SIFIVE_EXTENSIBLECACHE0_REG_H
#define METAL__DRIVERS__SIFIVE_EXTENSIBLECACHE0_REG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* following defines should be used for structure members */
#define __IM  volatile const    /**< Defines 'read only' structure member permissions */
#define __OM  volatile          /**< Defines 'write only' structure member permissions */
#define __IOM volatile          /**< Defines 'read / write' structure member permissions */

/**
 * Defines
 */
#define SIFIVE_EXTENSIBLECACHE0_LOCK_REGION_COUNT (4U)
/* Address range granule of the lock region */
#define SIFIVE_EXTENSIBLECACHE0_LOCK_REGION_GRANULE (64U)
#define SIFIVE_EXTENSIBLECACHE0_SLICE_COUNT METAL_SIFIVE_EXTENSIBLECACHE0_SLICE_COUNT
/* Bit-fields' masks */
/* Data config's register fields */
#define EC_DATA_CONFIG_BUFFER_CAPACITY_MASK (0xFFFFFFFFULL)
#define EC_DATA_CONFIG_NUM_BANKS_MASK (0xFF00000000ULL)
#define EC_DATA_CONFIG_LINE_SIZE_MASK (0xFF0000000000ULL)
/* Tag config's register fields */
#define EC_TAG_CONFIG_TRACKING_CAPACITY_MASK (0xFFFFFFFFULL)
#define EC_TAG_CONFIG_NUM_BANKS_MASK (0xFF00000000ULL)
#define EC_TAG_CONFIG_ASSOCIATIVITY_MASK (0xFF0000000000ULL)
/* Cache clock gating register fields */
#define EC_CHICKENBIT_CG_SLICE_LEVEL_MASK (1U)
#define EC_CHICKENBIT_CG_ART_MASK (2U)
#define EC_CHICKENBIT_CG_CONTROL_BLOCK_MASK (4U)
#define EC_CHICKENBIT_CG_PERF_MONITOR_MASK (8U)
#define EC_CHICKENBIT_CG_BUCKET_MONITOR_MASK (0x10U)
#define EC_CHICKENBIT_CG_PPAMO_MASK (0x20U)
#define EC_CHICKENBIT_CG_MASK (0x3FU)
/* Slice-Level clock gating hysteresis counter */
#define EC_HYSCOUNTER_SLICE_LEVEL_CG_MASK (0x03E0U)
/* Memory lock region's regiter fields */
#define EC_CACHE_LOCK_REGION_ENABLE_MASK (1ULL)
#define EC_CACHE_LOCK_REGION_PA_MASK (0xFFFFFFFFFFFFE0ULL)
/* Cache flush command's regiter fields */
#define EC_CACHE_FLUSH_CMD_MASK (3U)
/* Cache flush status's regiter fields */
#define EC_CACHE_FLUSH_STATUS_MASK (1U)
/* Cache flush address's regiter fields */
#define EC_CACHE_FLUSH_ADDRESS_PA_MASK (0xFFFFFFFFFFFFE0ULL)

/* Cache flush commands */
#define EC_CACHE_FLUSH_CLEAN_CMD (2U)
#define EC_CACHE_FLUSH_CLEAN_INV_CMD (3U)

/**
 * Extensible cache control port
 */
typedef struct _EXTENSIBLECACHE {
    /** Offset: 0x0000 (R/ ) Information about the Data Array Configuration */
    __IM    uint64_t   DATAARRAYCONFIG;
    /** Offset: 0x0008 (R/ ) Information about the Tag Array Configuration */
    __IM    uint64_t   TAGARRAYCONFIG;
            uint32_t   _reserved0[0x3cU];
    /** Offset: 0x0100 (R/W) Chicken Bit */
    __IOM   uint32_t   CHICKENBIT;
            uint32_t   _reserved1[0xbfU];
    /** Offset: 0x0400 (R/W) Cache lock 0..3 */
    __IOM   uint64_t   MEMLOCK[4U];
            uint32_t   _reserved2[0xf8U];
    /** Offset: 0x0800 ( /W) Cache flush command */
    __OM    uint32_t   CACHEFLUSHCMD;
            uint32_t   _reserved3;
    /** Offset: 0x0808 (R/ ) Cache flush status */
    __IM    uint32_t   CACHEFLUSHSTATUS;
            uint32_t   _reserved4;
    /** Offset: 0x0810 (R/W) Cache flush address */
    __IOM   uint64_t   FLUSHADDR;
            uint32_t   _reserved5[0x3aU];
    /** Offset: 0x0900 (R/W) Hysteresis counter */
    __IOM   uint32_t   HYSCOUNTER;
            uint32_t   _reserved7[0x1dfU];
    /** Offset: 0x1000 (R/W) Error Summary Status */
    __IOM   uint64_t   SUMMARY_STATUS;
    /** Offset: 0x1008 (R/ ) Bank Revision/Personalization */
    __IM    uint64_t   ERROR_BANK_PERSONALIZATION;
    /** Offset: 0x1010 (R/W) Bank Error Entry Status */
    __IOM   uint64_t   ERROR_BANK_ENTRY_STATUS;
    /** Offset: 0x1018 (R/ ) Bank Error Entry Supplemental 1 */
    __IM    uint64_t   ERROR_BANK_SUPPLEMENTAL1;
    /** Offset: 0x1020 (R/ ) Bank Error Entry Supplemental 2 */
    __IM    uint64_t   ERROR_BANK_SUPPLEMENTAL2;
    /** Offset: 0x1028 (R/W) Error Bank Simple Modifier */
    __IOM   uint32_t   ERROR_BANK_SIMPLE_MODS;
            uint32_t   _reserved9[0xf5U];
    /** Offset: 0x1400 (R/W) Error Injection Control 0 */
    __IOM   uint32_t   ERRINJCTRL;
            uint32_t   _reserved10[15U];
    /** Offset: 0x1440 (R/ ) Tracks the number of errors injected */
    __IM    uint32_t   ERRINJCOUNT;
            uint32_t   _reserved11[0x2efU];
    /** Offset: 0x2000 (R/W) Performance event selectors */
    __IOM   uint32_t   PMEVENTSELECT;
            uint32_t   _reserved12[0x7eU];
    /** Offset: 0x2200 (R/W) Performance counter client disable mask */
    __IOM   uint64_t   PMCLIENTFILTER;
    /** Offset: 0x2208 (R/W) performance counter enable mask */
    __IOM   uint64_t   PMCOUNTERINHIBIT;
            uint32_t   _reserved13[0x3cU];
    /** Offset: 0x2300 (R/W) Bucket Monitor Control */
    __IOM   uint64_t   BMCONTROL;
    /** Offset: 0x2308 (R/W) Bucket Monitor event select */
    __IOM   uint64_t   BMEVENTSELECTS;
            uint32_t   _reserved14[0x33cU];
    /** Offset: 0x3000 (R/W) Performance event counters */
    __IOM   uint64_t   PMEVENTCOUNTER;
            uint32_t   _reserved15[0x7eU];
    /** Offset: 0x3200 (R/W) Bucket Monitor counters */
    __IOM   uint32_t   BMEVENTCOUNTERS;
            uint32_t   _reserved16[0x7eU];
} EXTENSIBLECACHE_Type;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* METAL__DRIVERS__SIFIVE_EXTENSIBLECACHE0_REG_H */
