#ifndef __ARCH_H__
#define __ARCH_H__

#define SMP_CONTEXT_SIZE_SHIFT 7
#define SMP_CONTEXT_SIZE (1 << SMP_CONTEXT_SIZE_SHIFT)
#define SMP_CONTEXT_SP_OFFSET 0
#define SMP_CONTEXT_FN_OFFSET 8
#define SMP_CONTEXT_PRIV_OFFSET 16
#define SMP_CONTEXT_STATCKSIZE_OFFSET 24

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <stdbool.h>
#include <asm.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef __uint128_t uint128_t;

#define readl(a)	(*(volatile u32 *)(a))

#define writel(v, a)	(*(volatile u32 *)(a) = (v))

#ifdef CONFIG_SMP
#error "smp not supported yet"
#endif

#endif

#endif
