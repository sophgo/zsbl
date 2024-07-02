/* Copyright 2022-2023 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#ifndef METAL__ASM_MACROS_H
#define METAL__ASM_MACROS_H

/*! @file asm_macros.h
 *
 * Utility assembly macros.
 */
#ifdef __ASSEMBLY__
#define __ASM_STR(x)	x
#else
#define __ASM_STR(x)	#x
#endif
#if __riscv_xlen == 64
#define __REG_SEL(a, b)	__ASM_STR(a)
#elif __riscv_xlen == 32
#define __REG_SEL(a, b)	__ASM_STR(b)
#else
#error "Unexpected __riscv_xlen"
#endif

#define REG_L		__REG_SEL(ld, lw)
#define REG_S		__REG_SEL(sd, sw)

#if defined(__ASSEMBLER__)

/* define size of a register (in bytes) */
#if __riscv_xlen == 32
    #define REG_SIZE 4
    #define REG_SIZE_LOG2 2
    #define PTR_SIZE .word
#elif __riscv_xlen == 64
    #define REG_SIZE 8
    #define REG_SIZE_LOG2 3
    #define PTR_SIZE .dword
#else // __riscv_xlen == 64
    #error Unsupported XLEN
#endif // __riscv_xlen != 64

/**
 * macro long_load rd, symbol
 * Load four/eight bytes (according to the xlen of the platform) from
 * the symbol given. Note the symbol can be more than 2GB away so use a
 * literal/symbol in the code to load the symbol to the register.
 */
.macro long_load r, s
    j 101f
.balign 8
102:
    .dword \s
101:
    la \r, 102b
    load_x \r, 0(\r)
.endm

/**
 * macro load_x rd, offset_rs
 * Load four or eight bytes (according the xlen of the platform) from memory
 * "offset_rs" address and write them to register "rd"
 */
.macro  load_x rd, offset_rs
    #if __riscv_xlen == 32
    lw      \rd, \offset_rs
    #elif __riscv_xlen == 64
    ld      \rd, \offset_rs
    #endif // __riscv_xlen != 64
    .endm

/**
 * macro store_x rd, offset_rs
 * Store four or eight bytes (according the xlen of the platform) from register
 * "rd" and write them to memory "offset_rs" address
 */
.macro  store_x rd, offset_rs
    #if __riscv_xlen == 32
    sw      \rd, \offset_rs
    #elif __riscv_xlen == 64
    sd      \rd, \offset_rs
    #endif // __riscv_xlen != 64
    .endm

/**
 * macro load_a rd, symbol
 * Load "symbol" address (32 bits or 64 bits according the xlen of the platform)
 * into "rd" register
 */
.macro load_a rd, symbol
#ifdef __riscv_cmodel_compact
    lla.gprel   \rd, \symbol
#else
    la      \rd, \symbol
#endif
    .endm

/**
 * macro load_a_got rd, symbol
 * Load "symbol" address (32 bits or 64 bits according the xlen of the platform)
 * into "rd" register. This macro uses the Global Offset Table (GOT) and is
 * suitable for global symbols.
 */
.macro load_a_got rd, symbol
#ifdef __riscv_cmodel_compact
    la.got.gprel   \rd, \symbol
#else
    la             \rd, \symbol
#endif
    .endm

#endif /* __ASSEMBLER__ */

#endif /* METAL__ASM_MACROS_H */
