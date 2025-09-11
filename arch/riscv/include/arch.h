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

#define readb(a)	(*(volatile u8 *)(a))
#define readw(a)	(*(volatile u16 *)(a))
#define readl(a)	(*(volatile u32 *)(a))
#define readq(a)	(*(volatile u64 *)(a))

#define writeb(v, a)	(*(volatile u8 *)(a) = (v))
#define writew(v, a)	(*(volatile u16 *)(a) = (v))
#define writel(v, a)	(*(volatile u32 *)(a) = (v))
#define writeq(v, a)	(*(volatile u64 *)(a) = (v))

#ifdef CONFIG_SMP
#error "smp not supported yet"
#endif

void arch_disable_local_irq(void);
void arch_enable_local_irq(void);
void arch_disable_local_timer_irq(void);
void arch_enable_local_timer_irq(void);
void arch_disable_local_external_irq(void);
void arch_enable_local_external_irq(void);

#endif

#endif
