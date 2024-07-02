/**
 * COMPOSABLECACHE registers
 *
 * @file sifive_ccache1_regs.h
 * 
 * @note 
 *
 * @copyright (c) 2020-2021 SiFive, Inc
 * @copyright SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>
#include <sifive_defs.h>

/* clang-format off */

/**
 * Structure type to access Composable cache (FEATUREDISABLE)
 */
typedef union _COMPOSABLECACHE_FEATUREDISABLE {
    struct {
        uint32_t DISABLETRUNKCLOCKGATE:1;   /**< bit:      0  Disable trunk clock gating */
        uint32_t DISABLECLOCKGATE:1;        /**< bit:      1  Disable regional clock gating */
        uint32_t DISABLECLOCKGATESLOW:1;    /**< bit:      2  Disable aggressive regional clock gating that may degrade performance */
        uint32_t _reserved0:29;             /**< bit:  3..31  (reserved) */
    } b;                                    /**< Structure used for bit access */
    uint32_t w;                             /**< Structure used for word access */
} COMPOSABLECACHE_FEATUREDISABLE_Type;
 

typedef struct _COMPOSABLECACHE {
    __IM    uint32_t   CONFIG;               /**< Offset: 0x0000 (R/ ) Information about the Cache Configuration */
            uint32_t   _reserved0;
    __IOM   uint32_t   WAYENABLE;            /**< Offset: 0x0008 (R/W) */
            uint32_t   _reserved1[13U];
    __IOM   uint32_t   ECCINJECTERROR;       /**< Offset: 0x0040 (R/W) Inject an ECC Error */
            uint32_t   _reserved2[0x2fU];
    __IM    uint32_t   DIRECCFIXLOW;         /**< Offset: 0x0100 (R/ ) */
    __IM    uint32_t   DIRECCFIXHIGH;        /**< Offset: 0x0104 (R/ ) */
    __IM    uint32_t   DIRECCFIXCOUNT;       /**< Offset: 0x0108 (R/ ) */
            uint32_t   _reserved3[5U];
    __IM    uint32_t   DIRECCFAILLOW;        /**< Offset: 0x0120 (R/ ) */
    __IM    uint32_t   DIRECCFAILHIGH;       /**< Offset: 0x0124 (R/ ) */
    __IM    uint32_t   DIRECCFAILCOUNT;      /**< Offset: 0x0128 (R/ ) */
            uint32_t   _reserved4[5U];
    __IM    uint32_t   DATECCFIXLOW;         /**< Offset: 0x0140 (R/ ) */
    __IM    uint32_t   DATECCFIXHIGH;        /**< Offset: 0x0144 (R/ ) */
    __IM    uint32_t   DATECCFIXCOUNT;       /**< Offset: 0x0148 (R/ ) */
            uint32_t   _reserved5[5U];
    __IM    uint32_t   DATECCFAILLOW;        /**< Offset: 0x0160 (R/ ) */
    __IM    uint32_t   DATECCFAILHIGH;       /**< Offset: 0x0164 (R/ ) */
    __IM    uint32_t   DATECCFAILCOUNT;      /**< Offset: 0x0168 (R/ ) */
            uint32_t   _reserved6[0x25U];
    __OM    uint64_t   FLUSH64;              /**< Offset: 0x0200 ( /W) */
            uint32_t   _reserved7[14U];
    __OM    uint32_t   FLUSH32;              /**< Offset: 0x0240 ( /W) */
            uint32_t   _reserved8[0x16fU];
    __IOM   uint32_t   WAYMASK0;             /**< Offset: 0x0800 (R/W) Master 0 way mask */
            uint32_t   _reserved9;
    __IOM   uint32_t   WAYMASK1;             /**< Offset: 0x0808 (R/W) Master 1 way mask */
            uint32_t   _reserved10;
    __IOM   uint32_t   WAYMASK2;             /**< Offset: 0x0810 (R/W) Master 2 way mask */
            uint32_t   _reserved11;
    __IOM   uint32_t   WAYMASK3;             /**< Offset: 0x0818 (R/W) Master 3 way mask */
            uint32_t   _reserved12;
    __IOM   uint32_t   WAYMASK4;             /**< Offset: 0x0820 (R/W) Master 4 way mask */
            uint32_t   _reserved13;
    __IOM   uint32_t   WAYMASK5;             /**< Offset: 0x0828 (R/W) Master 5 way mask */
            uint32_t   _reserved14;
    __IOM   uint32_t   WAYMASK6;             /**< Offset: 0x0830 (R/W) Master 6 way mask */
            uint32_t   _reserved15;
    __IOM   uint32_t   WAYMASK7;             /**< Offset: 0x0838 (R/W) Master 7 way mask */
            uint32_t   _reserved16;
    __IOM   uint32_t   WAYMASK8;             /**< Offset: 0x0840 (R/W) Master 8 way mask */
            uint32_t   _reserved17[0x1efU];
    __IOM   COMPOSABLECACHE_FEATUREDISABLE_Type   FEATUREDISABLE;       /**< Offset: 0x1000 (R/W) Composable cache */
            uint32_t   _reserved18[0x3ffU];
    __IOM   uint32_t   PMEVENTSELECT0_0;     /**< Offset: 0x2000 (R/W) */
    __IOM   uint32_t   PMEVENTSELECT0_1;     /**< Offset: 0x2004 (R/W) */
    __IOM   uint32_t   PMEVENTSELECT1_0;     /**< Offset: 0x2008 (R/W) */
    __IOM   uint32_t   PMEVENTSELECT1_1;     /**< Offset: 0x200C (R/W) */
    __IOM   uint32_t   PMEVENTSELECT2_0;     /**< Offset: 0x2010 (R/W) */
    __IOM   uint32_t   PMEVENTSELECT2_1;     /**< Offset: 0x2014 (R/W) */
    __IOM   uint32_t   PMEVENTSELECT3_0;     /**< Offset: 0x2018 (R/W) */
    __IOM   uint32_t   PMEVENTSELECT3_1;     /**< Offset: 0x201C (R/W) */
    __IOM   uint32_t   PMEVENTSELECT4_0;     /**< Offset: 0x2020 (R/W) */
    __IOM   uint32_t   PMEVENTSELECT4_1;     /**< Offset: 0x2024 (R/W) */
    __IOM   uint32_t   PMEVENTSELECT5_0;     /**< Offset: 0x2028 (R/W) */
    __IOM   uint32_t   PMEVENTSELECT5_1;     /**< Offset: 0x202C (R/W) */
            uint32_t   _reserved19[0x1f4U];
    __IOM   uint32_t   PMCLIENTFILTER0;      /**< Offset: 0x2800 (R/W) performance counter client disable mask 0 */
            uint32_t   _reserved20[0x1ffU];
    __IOM   uint32_t   PMEVENTCOUNTER0_0;    /**< Offset: 0x3000 (R/W) */
    __IOM   uint32_t   PMEVENTCOUNTER0_1;    /**< Offset: 0x3004 (R/W) */
    __IOM   uint32_t   PMEVENTCOUNTER1_0;    /**< Offset: 0x3008 (R/W) */
    __IOM   uint32_t   PMEVENTCOUNTER1_1;    /**< Offset: 0x300C (R/W) */
    __IOM   uint32_t   PMEVENTCOUNTER2_0;    /**< Offset: 0x3010 (R/W) */
    __IOM   uint32_t   PMEVENTCOUNTER2_1;    /**< Offset: 0x3014 (R/W) */
    __IOM   uint32_t   PMEVENTCOUNTER3_0;    /**< Offset: 0x3018 (R/W) */
    __IOM   uint32_t   PMEVENTCOUNTER3_1;    /**< Offset: 0x301C (R/W) */
    __IOM   uint32_t   PMEVENTCOUNTER4_0;    /**< Offset: 0x3020 (R/W) */
    __IOM   uint32_t   PMEVENTCOUNTER4_1;    /**< Offset: 0x3024 (R/W) */
    __IOM   uint32_t   PMEVENTCOUNTER5_0;    /**< Offset: 0x3028 (R/W) */
    __IOM   uint32_t   PMEVENTCOUNTER5_1;    /**< Offset: 0x302C (R/W) */
} COMPOSABLECACHE_Type;

/**
 * Structure type to access Information about the Cache Configuration (CONFIG)
 */
typedef union _COMPOSABLECACHE_CONFIG {
    struct {
        uint8_t  BANKS;                     /**< bit:   0..7  Number of banks in the cache */
        uint8_t  WAYS;                      /**< bit:  8..15  Number of ways per bank */
        uint8_t  LGSETS;                    /**< bit: 16..23  Base-2 logarithm of the sets per bank */
        uint8_t  LGBLOCKBYTES;              /**< bit: 24..31  Base-2 logarithm of the bytes per cache block */
    } b;                                    /**< Structure used for bit access */
    uint32_t w;                             /**< Structure used for word access */
} COMPOSABLECACHE_CONFIG_Type;

/* COMPOSABLECACHE Information about the Cache Configuration: Number of banks in the cache */
#define COMPOSABLECACHE_CONFIG_BANKS_Pos                            0U
#define COMPOSABLECACHE_CONFIG_BANKS_Msk                            (0xFFU << COMPOSABLECACHE_CONFIG_BANKS_Pos)

/* COMPOSABLECACHE Information about the Cache Configuration: Number of ways per bank */
#define COMPOSABLECACHE_CONFIG_WAYS_Pos                             8U
#define COMPOSABLECACHE_CONFIG_WAYS_Msk                             (0xFFU << COMPOSABLECACHE_CONFIG_WAYS_Pos)

/* COMPOSABLECACHE Information about the Cache Configuration: Base-2 logarithm of the sets per bank */
#define COMPOSABLECACHE_CONFIG_LGSETS_Pos                           16U
#define COMPOSABLECACHE_CONFIG_LGSETS_Msk                           (0xFFU << COMPOSABLECACHE_CONFIG_LGSETS_Pos)

/* COMPOSABLECACHE Information about the Cache Configuration: Base-2 logarithm of the bytes per cache block */
#define COMPOSABLECACHE_CONFIG_LGBLOCKBYTES_Pos                     24U
#define COMPOSABLECACHE_CONFIG_LGBLOCKBYTES_Msk                     (0xFFU << COMPOSABLECACHE_CONFIG_LGBLOCKBYTES_Pos)

/**
 * Structure type to access  (WAYENABLE)
 */
typedef union _COMPOSABLECACHE_WAYENABLE {
    struct {
        uint8_t  WAYENABLE;                 /**< bit:   0..7  The index of the largest way which has been enabled. May only be increased. */
        uint32_t _reserved0:24;             /**< bit:  8..31  (reserved) */
    } b;                                    /**< Structure used for bit access */
    uint32_t w;                             /**< Structure used for word access */
} COMPOSABLECACHE_WAYENABLE_Type;

/* COMPOSABLECACHE : The index of the largest way which has been enabled. May only be increased. */
#define COMPOSABLECACHE_WAYENABLE_WAYENABLE_Pos                     0U
#define COMPOSABLECACHE_WAYENABLE_WAYENABLE_Msk                     0xFFU

/**
 * Structure type to access Inject an ECC Error (ECCINJECTERROR)
 */
typedef union _COMPOSABLECACHE_ECCINJECTERROR {
    struct {
        uint8_t  ECCTOGGLEBIT;              /**< bit:   0..7  Toggle (corrupt) this bit index on the next cache operation */
        uint8_t  _reserved0;                /**< bit:  8..15  (reserved) */
        uint32_t ECCTOGGLETYPE:1;           /**< bit:     16  Toggle (corrupt) a bit in 0=data or 1=directory */
        uint32_t _reserved1:15;             /**< bit: 17..31  (reserved) */
    } b;                                    /**< Structure used for bit access */
    uint32_t w;                             /**< Structure used for word access */
} COMPOSABLECACHE_ECCINJECTERROR_Type;

/* COMPOSABLECACHE Inject an ECC Error: Toggle (corrupt) this bit index on the next cache operation */
#define COMPOSABLECACHE_ECCINJECTERROR_ECCTOGGLEBIT_Pos             0U
#define COMPOSABLECACHE_ECCINJECTERROR_ECCTOGGLEBIT_Msk             (0xFFU << COMPOSABLECACHE_ECCINJECTERROR_ECCTOGGLEBIT_Pos)

/* COMPOSABLECACHE Inject an ECC Error: Toggle (corrupt) a bit in 0=data or 1=directory */
#define COMPOSABLECACHE_ECCINJECTERROR_ECCTOGGLETYPE_Pos            16U
#define COMPOSABLECACHE_ECCINJECTERROR_ECCTOGGLETYPE_Msk            (1U << COMPOSABLECACHE_ECCINJECTERROR_ECCTOGGLETYPE_Pos)

/* COMPOSABLECACHE : The low 32-bits of the most recent address to fail ECC */
#define COMPOSABLECACHE_DIRECCFIXLOW_DIRECCFIXLOW_Pos               0U
#define COMPOSABLECACHE_DIRECCFIXLOW_DIRECCFIXLOW_Msk               0xFFFFFFFFU

/* COMPOSABLECACHE : The high 32-bits of the most recent address to fail ECC */
#define COMPOSABLECACHE_DIRECCFIXHIGH_DIRECCFIXHIGH_Pos             0U
#define COMPOSABLECACHE_DIRECCFIXHIGH_DIRECCFIXHIGH_Msk             0xFFFFFFFFU

/* COMPOSABLECACHE : Reports the number of times an ECC error occured */
#define COMPOSABLECACHE_DIRECCFIXCOUNT_DIRECCFIXCOUNT_Pos           0U
#define COMPOSABLECACHE_DIRECCFIXCOUNT_DIRECCFIXCOUNT_Msk           0xFFFFFFFFU

/* COMPOSABLECACHE : The low 32-bits of the most recent address to fail ECC */
#define COMPOSABLECACHE_DIRECCFAILLOW_DIRECCFAILLOW_Pos             0U
#define COMPOSABLECACHE_DIRECCFAILLOW_DIRECCFAILLOW_Msk             0xFFFFFFFFU

/* COMPOSABLECACHE : The high 32-bits of the most recent address to fail ECC */
#define COMPOSABLECACHE_DIRECCFAILHIGH_DIRECCFAILHIGH_Pos           0U
#define COMPOSABLECACHE_DIRECCFAILHIGH_DIRECCFAILHIGH_Msk           0xFFFFFFFFU

/* COMPOSABLECACHE : Reports the number of times an ECC error occured */
#define COMPOSABLECACHE_DIRECCFAILCOUNT_DIRECCFAILCOUNT_Pos         0U
#define COMPOSABLECACHE_DIRECCFAILCOUNT_DIRECCFAILCOUNT_Msk         0xFFFFFFFFU

/* COMPOSABLECACHE : The low 32-bits of the most recent address to fail ECC */
#define COMPOSABLECACHE_DATECCFIXLOW_DATECCFIXLOW_Pos               0U
#define COMPOSABLECACHE_DATECCFIXLOW_DATECCFIXLOW_Msk               0xFFFFFFFFU

/* COMPOSABLECACHE : The high 32-bits of the most recent address to fail ECC */
#define COMPOSABLECACHE_DATECCFIXHIGH_DATECCFIXHIGH_Pos             0U
#define COMPOSABLECACHE_DATECCFIXHIGH_DATECCFIXHIGH_Msk             0xFFFFFFFFU

/* COMPOSABLECACHE : Reports the number of times an ECC error occured */
#define COMPOSABLECACHE_DATECCFIXCOUNT_DATECCFIXCOUNT_Pos           0U
#define COMPOSABLECACHE_DATECCFIXCOUNT_DATECCFIXCOUNT_Msk           0xFFFFFFFFU

/* COMPOSABLECACHE : The low 32-bits of the most recent address to fail ECC */
#define COMPOSABLECACHE_DATECCFAILLOW_DATECCFAILLOW_Pos             0U
#define COMPOSABLECACHE_DATECCFAILLOW_DATECCFAILLOW_Msk             0xFFFFFFFFU

/* COMPOSABLECACHE : The high 32-bits of the most recent address to fail ECC */
#define COMPOSABLECACHE_DATECCFAILHIGH_DATECCFAILHIGH_Pos           0U
#define COMPOSABLECACHE_DATECCFAILHIGH_DATECCFAILHIGH_Msk           0xFFFFFFFFU

/* COMPOSABLECACHE : Reports the number of times an ECC error occured */
#define COMPOSABLECACHE_DATECCFAILCOUNT_DATECCFAILCOUNT_Pos         0U
#define COMPOSABLECACHE_DATECCFAILCOUNT_DATECCFAILCOUNT_Msk         0xFFFFFFFFU

/* COMPOSABLECACHE : Flush the phsyical address equal to the 64-bit written data from the cache */
#define COMPOSABLECACHE_FLUSH64_FLUSH64_Pos                         0U
#define COMPOSABLECACHE_FLUSH64_FLUSH64_Msk                         0xFFFFFFFFFFFFFFFFU

/* COMPOSABLECACHE : Flush the physical address equal to the 32-bit written data << 4 from the cache */
#define COMPOSABLECACHE_FLUSH32_FLUSH32_Pos                         0U
#define COMPOSABLECACHE_FLUSH32_FLUSH32_Msk                         0xFFFFFFFFU

/**
 * Structure type to access Master 0 way mask (WAYMASK0)
 */
typedef union _COMPOSABLECACHE_WAYMASK0 {
    struct {
        uint8_t  WAYMASK0;                  /**< bit:   0..7  Enable way 0 for Master 0 */
        uint32_t _reserved0:24;             /**< bit:  8..31  (reserved) */
    } b;                                    /**< Structure used for bit access */
    uint32_t w;                             /**< Structure used for word access */
} COMPOSABLECACHE_WAYMASK0_Type;

/* COMPOSABLECACHE Master 0 way mask: Enable way 0 for Master 0 */
#define COMPOSABLECACHE_WAYMASK0_WAYMASK0_Pos                       0U
#define COMPOSABLECACHE_WAYMASK0_WAYMASK0_Msk                       0xFFU

/**
 * Structure type to access Master 1 way mask (WAYMASK1)
 */
typedef union _COMPOSABLECACHE_WAYMASK1 {
    struct {
        uint8_t  WAYMASK1;                  /**< bit:   0..7  Enable way 0 for Master 1 */
        uint32_t _reserved0:24;             /**< bit:  8..31  (reserved) */
    } b;                                    /**< Structure used for bit access */
    uint32_t w;                             /**< Structure used for word access */
} COMPOSABLECACHE_WAYMASK1_Type;

/* COMPOSABLECACHE Master 1 way mask: Enable way 0 for Master 1 */
#define COMPOSABLECACHE_WAYMASK1_WAYMASK1_Pos                       0U
#define COMPOSABLECACHE_WAYMASK1_WAYMASK1_Msk                       0xFFU

/**
 * Structure type to access Master 2 way mask (WAYMASK2)
 */
typedef union _COMPOSABLECACHE_WAYMASK2 {
    struct {
        uint8_t  WAYMASK2;                  /**< bit:   0..7  Enable way 0 for Master 2 */
        uint32_t _reserved0:24;             /**< bit:  8..31  (reserved) */
    } b;                                    /**< Structure used for bit access */
    uint32_t w;                             /**< Structure used for word access */
} COMPOSABLECACHE_WAYMASK2_Type;

/* COMPOSABLECACHE Master 2 way mask: Enable way 0 for Master 2 */
#define COMPOSABLECACHE_WAYMASK2_WAYMASK2_Pos                       0U
#define COMPOSABLECACHE_WAYMASK2_WAYMASK2_Msk                       0xFFU

/**
 * Structure type to access Master 3 way mask (WAYMASK3)
 */
typedef union _COMPOSABLECACHE_WAYMASK3 {
    struct {
        uint8_t  WAYMASK3;                  /**< bit:   0..7  Enable way 0 for Master 3 */
        uint32_t _reserved0:24;             /**< bit:  8..31  (reserved) */
    } b;                                    /**< Structure used for bit access */
    uint32_t w;                             /**< Structure used for word access */
} COMPOSABLECACHE_WAYMASK3_Type;

/* COMPOSABLECACHE Master 3 way mask: Enable way 0 for Master 3 */
#define COMPOSABLECACHE_WAYMASK3_WAYMASK3_Pos                       0U
#define COMPOSABLECACHE_WAYMASK3_WAYMASK3_Msk                       0xFFU

/**
 * Structure type to access Master 4 way mask (WAYMASK4)
 */
typedef union _COMPOSABLECACHE_WAYMASK4 {
    struct {
        uint8_t  WAYMASK4;                  /**< bit:   0..7  Enable way 0 for Master 4 */
        uint32_t _reserved0:24;             /**< bit:  8..31  (reserved) */
    } b;                                    /**< Structure used for bit access */
    uint32_t w;                             /**< Structure used for word access */
} COMPOSABLECACHE_WAYMASK4_Type;

/* COMPOSABLECACHE Master 4 way mask: Enable way 0 for Master 4 */
#define COMPOSABLECACHE_WAYMASK4_WAYMASK4_Pos                       0U
#define COMPOSABLECACHE_WAYMASK4_WAYMASK4_Msk                       0xFFU

/**
 * Structure type to access Master 5 way mask (WAYMASK5)
 */
typedef union _COMPOSABLECACHE_WAYMASK5 {
    struct {
        uint8_t  WAYMASK5;                  /**< bit:   0..7  Enable way 0 for Master 5 */
        uint32_t _reserved0:24;             /**< bit:  8..31  (reserved) */
    } b;                                    /**< Structure used for bit access */
    uint32_t w;                             /**< Structure used for word access */
} COMPOSABLECACHE_WAYMASK5_Type;

/* COMPOSABLECACHE Master 5 way mask: Enable way 0 for Master 5 */
#define COMPOSABLECACHE_WAYMASK5_WAYMASK5_Pos                       0U
#define COMPOSABLECACHE_WAYMASK5_WAYMASK5_Msk                       0xFFU

/**
 * Structure type to access Master 6 way mask (WAYMASK6)
 */
typedef union _COMPOSABLECACHE_WAYMASK6 {
    struct {
        uint8_t  WAYMASK6;                  /**< bit:   0..7  Enable way 0 for Master 6 */
        uint32_t _reserved0:24;             /**< bit:  8..31  (reserved) */
    } b;                                    /**< Structure used for bit access */
    uint32_t w;                             /**< Structure used for word access */
} COMPOSABLECACHE_WAYMASK6_Type;

/* COMPOSABLECACHE Master 6 way mask: Enable way 0 for Master 6 */
#define COMPOSABLECACHE_WAYMASK6_WAYMASK6_Pos                       0U
#define COMPOSABLECACHE_WAYMASK6_WAYMASK6_Msk                       0xFFU

/**
 * Structure type to access Master 7 way mask (WAYMASK7)
 */
typedef union _COMPOSABLECACHE_WAYMASK7 {
    struct {
        uint8_t  WAYMASK7;                  /**< bit:   0..7  Enable way 0 for Master 7 */
        uint32_t _reserved0:24;             /**< bit:  8..31  (reserved) */
    } b;                                    /**< Structure used for bit access */
    uint32_t w;                             /**< Structure used for word access */
} COMPOSABLECACHE_WAYMASK7_Type;

/* COMPOSABLECACHE Master 7 way mask: Enable way 0 for Master 7 */
#define COMPOSABLECACHE_WAYMASK7_WAYMASK7_Pos                       0U
#define COMPOSABLECACHE_WAYMASK7_WAYMASK7_Msk                       0xFFU

/**
 * Structure type to access Master 8 way mask (WAYMASK8)
 */
typedef union _COMPOSABLECACHE_WAYMASK8 {
    struct {
        uint8_t  WAYMASK8;                  /**< bit:   0..7  Enable way 0 for Master 8 */
        uint32_t _reserved0:24;             /**< bit:  8..31  (reserved) */
    } b;                                    /**< Structure used for bit access */
    uint32_t w;                             /**< Structure used for word access */
} COMPOSABLECACHE_WAYMASK8_Type;

/* COMPOSABLECACHE Master 8 way mask: Enable way 0 for Master 8 */
#define COMPOSABLECACHE_WAYMASK8_WAYMASK8_Pos                       0U
#define COMPOSABLECACHE_WAYMASK8_WAYMASK8_Msk                       0xFFU


/* COMPOSABLECACHE Composable cache: Disable trunk clock gating */
#define COMPOSABLECACHE_FEATUREDISABLE_DISABLETRUNKCLOCKGATE_Pos    0U
#define COMPOSABLECACHE_FEATUREDISABLE_DISABLETRUNKCLOCKGATE_Msk    (1U << COMPOSABLECACHE_FEATUREDISABLE_DISABLETRUNKCLOCKGATE_Pos)

/* COMPOSABLECACHE Composable cache: Disable regional clock gating */
#define COMPOSABLECACHE_FEATUREDISABLE_DISABLECLOCKGATE_Pos         1U
#define COMPOSABLECACHE_FEATUREDISABLE_DISABLECLOCKGATE_Msk         (1U << COMPOSABLECACHE_FEATUREDISABLE_DISABLECLOCKGATE_Pos)

/* COMPOSABLECACHE Composable cache: Disable aggressive regional clock gating that may degrade performance */
#define COMPOSABLECACHE_FEATUREDISABLE_DISABLECLOCKGATESLOW_Pos     2U
#define COMPOSABLECACHE_FEATUREDISABLE_DISABLECLOCKGATESLOW_Msk     (1U << COMPOSABLECACHE_FEATUREDISABLE_DISABLECLOCKGATESLOW_Pos)

/* COMPOSABLECACHE : The low 32-bits of performance monitor event select 0 */
#define COMPOSABLECACHE_PMEVENTSELECT0_0_PMEVENTSELECT0LOW_Pos      0U
#define COMPOSABLECACHE_PMEVENTSELECT0_0_PMEVENTSELECT0LOW_Msk      0xFFFFFFFFU

/* COMPOSABLECACHE : The high 32-bits of performance monitor event select 0 */
#define COMPOSABLECACHE_PMEVENTSELECT0_1_PMEVENTSELECT0HIGH_Pos     0U
#define COMPOSABLECACHE_PMEVENTSELECT0_1_PMEVENTSELECT0HIGH_Msk     0xFFFFFFFFU

/* COMPOSABLECACHE : The low 32-bits of performance monitor event select 1 */
#define COMPOSABLECACHE_PMEVENTSELECT1_0_PMEVENTSELECT1LOW_Pos      0U
#define COMPOSABLECACHE_PMEVENTSELECT1_0_PMEVENTSELECT1LOW_Msk      0xFFFFFFFFU

/* COMPOSABLECACHE : The high 32-bits of performance monitor event select 1 */
#define COMPOSABLECACHE_PMEVENTSELECT1_1_PMEVENTSELECT1HIGH_Pos     0U
#define COMPOSABLECACHE_PMEVENTSELECT1_1_PMEVENTSELECT1HIGH_Msk     0xFFFFFFFFU

/* COMPOSABLECACHE : The low 32-bits of performance monitor event select 2 */
#define COMPOSABLECACHE_PMEVENTSELECT2_0_PMEVENTSELECT2LOW_Pos      0U
#define COMPOSABLECACHE_PMEVENTSELECT2_0_PMEVENTSELECT2LOW_Msk      0xFFFFFFFFU

/* COMPOSABLECACHE : The high 32-bits of performance monitor event select 2 */
#define COMPOSABLECACHE_PMEVENTSELECT2_1_PMEVENTSELECT2HIGH_Pos     0U
#define COMPOSABLECACHE_PMEVENTSELECT2_1_PMEVENTSELECT2HIGH_Msk     0xFFFFFFFFU

/* COMPOSABLECACHE : The low 32-bits of performance monitor event select 3 */
#define COMPOSABLECACHE_PMEVENTSELECT3_0_PMEVENTSELECT3LOW_Pos      0U
#define COMPOSABLECACHE_PMEVENTSELECT3_0_PMEVENTSELECT3LOW_Msk      0xFFFFFFFFU

/* COMPOSABLECACHE : The high 32-bits of performance monitor event select 3 */
#define COMPOSABLECACHE_PMEVENTSELECT3_1_PMEVENTSELECT3HIGH_Pos     0U
#define COMPOSABLECACHE_PMEVENTSELECT3_1_PMEVENTSELECT3HIGH_Msk     0xFFFFFFFFU

/* COMPOSABLECACHE : The low 32-bits of performance monitor event select 4 */
#define COMPOSABLECACHE_PMEVENTSELECT4_0_PMEVENTSELECT4LOW_Pos      0U
#define COMPOSABLECACHE_PMEVENTSELECT4_0_PMEVENTSELECT4LOW_Msk      0xFFFFFFFFU

/* COMPOSABLECACHE : The high 32-bits of performance monitor event select 4 */
#define COMPOSABLECACHE_PMEVENTSELECT4_1_PMEVENTSELECT4HIGH_Pos     0U
#define COMPOSABLECACHE_PMEVENTSELECT4_1_PMEVENTSELECT4HIGH_Msk     0xFFFFFFFFU

/* COMPOSABLECACHE : The low 32-bits of performance monitor event select 5 */
#define COMPOSABLECACHE_PMEVENTSELECT5_0_PMEVENTSELECT5LOW_Pos      0U
#define COMPOSABLECACHE_PMEVENTSELECT5_0_PMEVENTSELECT5LOW_Msk      0xFFFFFFFFU

/* COMPOSABLECACHE : The high 32-bits of performance monitor event select 5 */
#define COMPOSABLECACHE_PMEVENTSELECT5_1_PMEVENTSELECT5HIGH_Pos     0U
#define COMPOSABLECACHE_PMEVENTSELECT5_1_PMEVENTSELECT5HIGH_Msk     0xFFFFFFFFU

/**
 * Structure type to access performance counter client disable mask 0 (PMCLIENTFILTER0)
 */
typedef union _COMPOSABLECACHE_PMCLIENTFILTER0 {
    struct {
        uint32_t PMCLIENTFILTER0:1;         /**< bit:      0  performance counter disable mask for client 0 */
        uint32_t PMCLIENTFILTER1:1;         /**< bit:      1  performance counter disable mask for client 1 */
        uint32_t PMCLIENTFILTER2:1;         /**< bit:      2  performance counter disable mask for client 2 */
        uint32_t PMCLIENTFILTER3:1;         /**< bit:      3  performance counter disable mask for client 3 */
        uint32_t PMCLIENTFILTER4:1;         /**< bit:      4  performance counter disable mask for client 4 */
        uint32_t PMCLIENTFILTER5:1;         /**< bit:      5  performance counter disable mask for client 5 */
        uint32_t PMCLIENTFILTER6:1;         /**< bit:      6  performance counter disable mask for client 6 */
        uint32_t PMCLIENTFILTER7:1;         /**< bit:      7  performance counter disable mask for client 7 */
        uint32_t PMCLIENTFILTER8:1;         /**< bit:      8  performance counter disable mask for client 8 */
        uint32_t _reserved0:23;             /**< bit:  9..31  (reserved) */
    } b;                                    /**< Structure used for bit access */
    uint32_t w;                             /**< Structure used for word access */
} COMPOSABLECACHE_PMCLIENTFILTER0_Type;

/* COMPOSABLECACHE performance counter client disable mask 0: Performance counter disable mask for client 0 */
#define COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER0_Pos         0U
#define COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER0_Msk         (1U << COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER0_Pos)

/* COMPOSABLECACHE performance counter client disable mask 0: Performance counter disable mask for client 1 */
#define COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER1_Pos         1U
#define COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER1_Msk         (1U << COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER1_Pos)

/* COMPOSABLECACHE performance counter client disable mask 0: Performance counter disable mask for client 2 */
#define COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER2_Pos         2U
#define COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER2_Msk         (1U << COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER2_Pos)

/* COMPOSABLECACHE performance counter client disable mask 0: Performance counter disable mask for client 3 */
#define COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER3_Pos         3U
#define COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER3_Msk         (1U << COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER3_Pos)

/* COMPOSABLECACHE performance counter client disable mask 0: Performance counter disable mask for client 4 */
#define COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER4_Pos         4U
#define COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER4_Msk         (1U << COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER4_Pos)

/* COMPOSABLECACHE performance counter client disable mask 0: Performance counter disable mask for client 5 */
#define COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER5_Pos         5U
#define COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER5_Msk         (1U << COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER5_Pos)

/* COMPOSABLECACHE performance counter client disable mask 0: Performance counter disable mask for client 6 */
#define COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER6_Pos         6U
#define COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER6_Msk         (1U << COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER6_Pos)

/* COMPOSABLECACHE performance counter client disable mask 0: Performance counter disable mask for client 7 */
#define COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER7_Pos         7U
#define COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER7_Msk         (1U << COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER7_Pos)

/* COMPOSABLECACHE performance counter client disable mask 0: Performance counter disable mask for client 8 */
#define COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER8_Pos         8U
#define COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER8_Msk         (1U << COMPOSABLECACHE_PMCLIENTFILTER0_PMCLIENTFILTER8_Pos)

/* COMPOSABLECACHE : The low 32-bits of performance monitor event counter 0 */
#define COMPOSABLECACHE_PMEVENTCOUNTER0_0_PMEVENTCOUNTER0LOW_Pos    0U
#define COMPOSABLECACHE_PMEVENTCOUNTER0_0_PMEVENTCOUNTER0LOW_Msk    0xFFFFFFFFU

/* COMPOSABLECACHE : The high 32-bits of performance monitor event counter 0 */
#define COMPOSABLECACHE_PMEVENTCOUNTER0_1_PMEVENTCOUNTER0HIGH_Pos   0U
#define COMPOSABLECACHE_PMEVENTCOUNTER0_1_PMEVENTCOUNTER0HIGH_Msk   0xFFFFFFFFU

/* COMPOSABLECACHE : The low 32-bits of performance monitor event counter 1 */
#define COMPOSABLECACHE_PMEVENTCOUNTER1_0_PMEVENTCOUNTER1LOW_Pos    0U
#define COMPOSABLECACHE_PMEVENTCOUNTER1_0_PMEVENTCOUNTER1LOW_Msk    0xFFFFFFFFU

/* COMPOSABLECACHE : The high 32-bits of performance monitor event counter 1 */
#define COMPOSABLECACHE_PMEVENTCOUNTER1_1_PMEVENTCOUNTER1HIGH_Pos   0U
#define COMPOSABLECACHE_PMEVENTCOUNTER1_1_PMEVENTCOUNTER1HIGH_Msk   0xFFFFFFFFU

/* COMPOSABLECACHE : The low 32-bits of performance monitor event counter 2 */
#define COMPOSABLECACHE_PMEVENTCOUNTER2_0_PMEVENTCOUNTER2LOW_Pos    0U
#define COMPOSABLECACHE_PMEVENTCOUNTER2_0_PMEVENTCOUNTER2LOW_Msk    0xFFFFFFFFU

/* COMPOSABLECACHE : The high 32-bits of performance monitor event counter 2 */
#define COMPOSABLECACHE_PMEVENTCOUNTER2_1_PMEVENTCOUNTER2HIGH_Pos   0U
#define COMPOSABLECACHE_PMEVENTCOUNTER2_1_PMEVENTCOUNTER2HIGH_Msk   0xFFFFFFFFU

/* COMPOSABLECACHE : The low 32-bits of performance monitor event counter 3 */
#define COMPOSABLECACHE_PMEVENTCOUNTER3_0_PMEVENTCOUNTER3LOW_Pos    0U
#define COMPOSABLECACHE_PMEVENTCOUNTER3_0_PMEVENTCOUNTER3LOW_Msk    0xFFFFFFFFU

/* COMPOSABLECACHE : The high 32-bits of performance monitor event counter 3 */
#define COMPOSABLECACHE_PMEVENTCOUNTER3_1_PMEVENTCOUNTER3HIGH_Pos   0U
#define COMPOSABLECACHE_PMEVENTCOUNTER3_1_PMEVENTCOUNTER3HIGH_Msk   0xFFFFFFFFU

/* COMPOSABLECACHE : The low 32-bits of performance monitor event counter 4 */
#define COMPOSABLECACHE_PMEVENTCOUNTER4_0_PMEVENTCOUNTER4LOW_Pos    0U
#define COMPOSABLECACHE_PMEVENTCOUNTER4_0_PMEVENTCOUNTER4LOW_Msk    0xFFFFFFFFU

/* COMPOSABLECACHE : The high 32-bits of performance monitor event counter 4 */
#define COMPOSABLECACHE_PMEVENTCOUNTER4_1_PMEVENTCOUNTER4HIGH_Pos   0U
#define COMPOSABLECACHE_PMEVENTCOUNTER4_1_PMEVENTCOUNTER4HIGH_Msk   0xFFFFFFFFU

/* COMPOSABLECACHE : The low 32-bits of performance monitor event counter 5 */
#define COMPOSABLECACHE_PMEVENTCOUNTER5_0_PMEVENTCOUNTER5LOW_Pos    0U
#define COMPOSABLECACHE_PMEVENTCOUNTER5_0_PMEVENTCOUNTER5LOW_Msk    0xFFFFFFFFU

/* COMPOSABLECACHE : The high 32-bits of performance monitor event counter 5 */
#define COMPOSABLECACHE_PMEVENTCOUNTER5_1_PMEVENTCOUNTER5HIGH_Pos   0U
#define COMPOSABLECACHE_PMEVENTCOUNTER5_1_PMEVENTCOUNTER5HIGH_Msk   0xFFFFFFFFU
