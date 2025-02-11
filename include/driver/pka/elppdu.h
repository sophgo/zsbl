// ------------------------------------------------------------------------
//
//                (C) COPYRIGHT 2012 - 2015 SYNOPSYS, INC.
//                          ALL RIGHTS RESERVED
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  version 2 as published by the Free Software Foundation.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <https://gnu.org/licenses/>.
//
// ------------------------------------------------------------------------

#ifndef ELPPDU_H_
#define ELPPDU_H_

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

// Macro to print messages to user
#define ELPHW_PRINT  printf

// macro to yield CPU (when in a busy loop)
// this is a placeholder which does not actually yield, user must update
// this macro
#define CPU_YIELD() 1

// DMAable address type, usually can be some equivalent of uint32_t but in Linux it must be dma_addr_t
// since on 64-bit and PAE boxes the pointers used by coherant allocation are 64-bits (even though only the lower 32 bits are used)
#define PDU_DMA_ADDR_T               uint32_t

// Debug modifier for printing, in linux adding KERN_DEBUG makes the output only show up in debug logs (avoids /var/log/messages)
#define ELPHW_PRINT_DEBUG

// these are for IRQ contexts (can block IRQs)
// USER MUST SUPPLY THESE, the following definitions are an EXAMPLE only
#define PDU_LOCK_TYPE                volatile int
#define PDU_INIT_LOCK(lock)          *lock = 0
#define PDU_LOCK(lock, flags)        do { while (*lock) { CPU_YIELD(); }; *lock = 1; } while (0)
#define PDU_UNLOCK(lock, flags)      *lock = 0

// these are for bottom half BH contexts (cannot block IRQs)
// USER MUST SUPPLY THESE, the following definitions are an EXAMPLE only
#define PDU_LOCK_TYPE_BH             volatile int
#define PDU_INIT_LOCK_BH(lock)       *lock = 0
#define PDU_LOCK_BH(lock)            do { while (*lock) { CPU_YIELD(); }; *lock = 1; } while (0)
#define PDU_UNLOCK_BH(lock)          *lock = 0





/**** Platform Generic, do not modify anything below here ****/
#define PDU_IRQ_EN_GLBL (1UL<<31)
#define PDU_IRQ_EN_VSPACC(x) (1UL<<x)
#define PDU_IRQ_EN_RNG  (1UL<<16)
#define PDU_IRQ_EN_PKA  (1UL<<17)
#define PDU_IRQ_EN_RE   (1UL<<18)
#define PDU_IRQ_EN_KEP  (1UL<<19)
#define PDU_IRQ_EN_EA   (1UL<<20)
#define PDU_IRQ_EN_MPM  (1UL<<21)
#ifdef  PDU_DUAL_MPM
   #define PDU_IRQ_EN_MPM1 (1UL<<22)
#endif

#include "elppdu_error.h"

typedef uint32_t u32;

typedef struct {
   unsigned minor,
            major,
            version,
            qos,
            is_spacc,
            is_pdu,
            is_hsm,
            aux,
            vspacc_idx,
            partial,
            project;
} spacc_version_block;

typedef struct {
   unsigned num_ctx,
            num_rc4_ctx,
            num_vspacc,
            ciph_ctx_page_size,
            hash_ctx_page_size,
            dma_type,
            cmd0_fifo_depth,
            cmd1_fifo_depth,
            cmd2_fifo_depth,
            stat_fifo_depth;
} spacc_config_block;

typedef struct {
   unsigned minor,
            major,
            is_rng,
            is_pka,
            is_re,
            is_kep,
            is_ea,
            is_mpm;
} pdu_config_block;

typedef struct {
   unsigned minor,
            major,
            paradigm,
            num_ctx,
            ctx_page_size;
} hsm_config_block;

typedef struct {
   uint32_t            clockrate;
   spacc_version_block spacc_version;
   spacc_config_block  spacc_config;
   pdu_config_block    pdu_config;
   hsm_config_block    hsm_config;
} pdu_info;

void pdu_io_write32(void *addr, unsigned long val);
void pdu_io_cached_write32(void *addr, unsigned long val, uint32_t *cache);
unsigned long pdu_io_read32(void *addr);

void pdu_to_dev32(void *addr, uint32_t *src, unsigned long nword);
void pdu_from_dev32(uint32_t *dst, void *addr, unsigned long nword);
void pdu_to_dev32_big(void *addr, const unsigned char *src, unsigned long nword);
void pdu_from_dev32_big(unsigned char *dst, void *addr, unsigned long nword);
void pdu_to_dev32_little(void *addr, const unsigned char *src, unsigned long nword);
void pdu_from_dev32_little(unsigned char *dst, void *addr, unsigned long nword);
void pdu_from_dev32_s(unsigned char *dst, void *addr, unsigned long nword, int endian);
void pdu_to_dev32_s(void *addr, const unsigned char *src, unsigned long nword, int endian);

void *pdu_malloc(unsigned long n);
void pdu_free(void *p);

int pdu_mem_init(void *device);
void pdu_mem_deinit(void *device);

int pdu_error_code(int code);

int pdu_get_version(void *dev, pdu_info *inf);

void spdu_boot_trng(pdu_info *info, unsigned long pdu_base);

#endif

