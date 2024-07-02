/**
 * Registers definition of SIFIVE PLIC IP
 *
 * @file sifive_plic1_regs.h
 * @copyright 2020-2021 SiFive, Inc
 * @copyright SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>
#include "sifive_defs.h"

/* Want to keep this file as it is, even if line exceed 80 chars */
/* clang-format off */

#ifdef __cplusplus
extern "C" {
#endif

#define PLIC_MAX_CORES_CTX      15872U

/* Max number of interrupt */
#define PLIC_MAX_INT            1024U

/* Max number of bits for MAX interrupt */
#define PLIC_U32_MAX     (PLIC_MAX_INT / 32)

typedef struct _PLIC_IE_CTX {
    __IOM   uint32_t    ENABLE[PLIC_U32_MAX];   /**< Offset: 0x0 (R/W) */
} PLIC_IE_CTX_Type;

/**
 * Structure type to access STATUS Bits
 */
typedef union _PLIC_CLCK_GATE_FEATURE_DISABLE {
    struct {
        uint8_t STATUS:1;       /**< bit:     0  enable clock gating disable feature */
        uint8_t _reserved0:7;   /**< bit:  7..1  (reserved) */
    } b;                        /**< Structure used for bit access */
    uint8_t B;                  /**< Structure used for Byte access */
} PLIC_CLCK_GATE_FEATURE_DISABLE_Type;

typedef struct _PLIC_CLCK_GATE {
    __IOM   PLIC_CLCK_GATE_FEATURE_DISABLE_Type     FEATURE_DISABLE;   /**< Offset: 0x0 (R/W) */
    uint8_t             _reserved0[3U];    /**< Offset: 0x1 */
} PLIC_CLCK_GATE_Type;

typedef struct _PLIC_IC_CTX {
    __IOM   uint32_t    PRIORITY_THRESHOLD;  /**< Offset: 0x000 (R/W) */
    __IOM   uint32_t    CLAIM_COMPLETE;      /**< Offset: 0x004 (R/W) */
            uint32_t    _reserved0[1022U];   /**< Offset: 0x008 */
} PLIC_IC_CTX_Type;

typedef struct _PLIC {
    __IOM   uint32_t            SRC_N_PRIORITY[PLIC_MAX_INT];   /**< Offset: 0x00_0000 (R/W)
                                                                     SRC_N_PRIORITY */
    __IM    uint32_t            PENDING_INT[PLIC_U32_MAX];      /**< Offset: 0x00_1000 (R/W)
                                                                     PENDING_INT Bits */
            uint32_t            _reserved1[992U];               /**< Offset: 0x00_1080 */
    __IOM   PLIC_IE_CTX_Type    IE_CTX[PLIC_MAX_CORES_CTX];     /**< Offset: 0x00_2000 (R/W) */
            uint32_t            _reserved2[13312U];             /**< Offset: 0x1F_2000 */
    __IOM   PLIC_CLCK_GATE_Type CLOCK_GATE;                     /**< Offset: 0x1F_F000 (R/W) */
            uint32_t            _reserved3[1023U];              /**< Offset: 0x1F_F004 */
    __IOM   PLIC_IC_CTX_Type    IC_CTX[PLIC_MAX_CORES_CTX];     /**< Offset: 0x20_0000 (R/W) */
} PLIC_Type;

/* PLIC SRC_N_PRIORITY Bits */
#define PLIC_SRC_N_PRIORITY_Pos         0U
#define PLIC_SRC_N_PRIORITY_Msk         0xFFFFFFFFU

/* PLIC PENDING_INT Bits */
#define PLIC_PENDING_INT_Pos            0U
#define PLIC_PENDING_INT_Msk            0xFFFFFFFFU

/* PLIC PLIC_IE_CTX ENABLE Bits */
#define PLIC_PLIC_IE_CTX_ENABLE_Pos     0U
#define PLIC_PLIC_IE_CTX_ENABLE_Msk     0xFFFFFFFFU

/**
 * Structure type to access THRESHOLD Bits
 */
typedef union _PLIC_IC_CTX_PRIORITY_THRESHOLD {
    struct {
        uint32_t THRESHOLD:3;   /**< bit:  2..0  Sets the priority threshold */
        uint32_t _reserved0:29; /**< bit:  3..31  (reserved) */
    } b;                        /**< Structure used for bit access */
    uint32_t w;                 /**< Structure used for word access */
} PLIC_IC_CTX_PRIORITY_THRESHOLD_Type;

/* PLIC PLIC_IC_CTX PRIORITY_THRESHOLD Bits */
#define PLIC_PLIC_IC_CTX_PRIORITY_THRESHOLD_THRESHOLD_Pos         0U
#define PLIC_PLIC_IC_CTX_PRIORITY_THRESHOLD_THRESHOLD_Msk         7U

/**
 * Structure type to access CLAIM_COMPLETE Bits
 */
typedef union _PLIC_IC_CTX_CLAIM_COMPLETE {
    struct {
        uint32_t ID:11;         /**< bit:   0..10  Sets the priority threshold */
        uint32_t _reserved0:21; /**< bit:  11..31  (reserved) */
    } b;                        /**< Structure used for bit access */
    uint32_t w;                 /**< Structure used for word access */
} PLIC_IC_CTX_PRIORITY_CLAIM_COMPLETE_Type;


/* PLIC CLCK_GATE FEATURE_DISABLE Bits */
#define PLIC_CLCK_GATE_FEATURE_DISABLE_STATUS_Pos         0U
#define PLIC_CLCK_GATE_FEATURE_DISABLE_STATUS_Msk         1U

/**
 * PLIC descriptor for a RISCV hart.
 */
struct plic_descriptor {
    /** Count of supported execution levels */
    unsigned int pd_nblevel;
    /** Enable context absolute base address for each priviledge mode */
    uintptr_t    pd_ie_ctx_base[4U];
    /** Claim/complete absolute base address for each priviledge mode */
    uintptr_t    pd_ic_ctx_base[4U];
};

#ifdef __cplusplus
}
#endif
