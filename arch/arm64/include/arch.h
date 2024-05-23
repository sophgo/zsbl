#ifndef __ARCH_H__
#define __ARCH_H__

#define CONTEXT_FRAME_SIZE	(34 * 8)
#define MPIDR_MT_MASK		(U(1) << 24)
#define MPIDR_CPU_MASK		MPIDR_AFFLVL_MASK
#define MPIDR_CLUSTER_MASK	(MPIDR_AFFLVL_MASK << MPIDR_AFFINITY_BITS)
#define MPIDR_AFFINITY_BITS	(8)
#define MPIDR_AFFLVL_MASK	(0xff)

#define SMP_CONTEXT_SIZE_SHIFT 7
#define SMP_CONTEXT_SP_OFFSET 0
#define SMP_CONTEXT_FN_OFFSET 8
#define SMP_CONTEXT_PRIV_OFFSET 16
#define SMP_CONTEXT_STATCKSIZE_OFFSET 24

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <stdbool.h>
#include <sysreg.h>
#include <rwonce.h>
#include <spinlock.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef __uint128_t uint128_t;

#define read128(_addr, _l64, _h64)			\
	do {						\
		asm volatile ("ldp %0, %1, [%2]"	\
			      : "=r"(_l64), "=r"(_h64)	\
			      : "r"(_addr));		\
	} while (0)

#define write128(_addr, _l64, _h64)			\
	do {						\
		asm volatile ("stp %0, %1, [%2]"	\
			      : "=r"(_l64), "=r"(_h64)	\
			      : "r"(_addr));		\
	} while (0)

#define readl(a)	(*(volatile u32 *)(a))

#define writel(v, a)	(*(volatile u32 *)(a) = (v))

struct context_frame {
	uint64_t x[30];
	uint64_t lr;
	uint64_t sp;
	uint64_t elr;
	uint64_t spsr;
};

_Static_assert(sizeof(struct context_frame) == CONTEXT_FRAME_SIZE,
"CONTEXT_FRAME_SIZE should be the same with sizeof(struct context_frame)");

static inline void arch_local_irq_enable(void)
{
	asm volatile(
		"msr    daifclr, #2             // arch_local_irq_enable"
		:
		:
		: "memory");
}

static inline void arch_local_irq_disable(void)
{
	asm volatile(
		"msr    daifset, #2             // arch_local_irq_disable"
		:
		:
		: "memory");
}

static inline void arch_local_fiq_enable(void)
{
	asm volatile(
		"msr    daifclr, #1             // arch_local_fiq_enable"
		:
		:
		: "memory");
}

static inline void arch_local_fiq_disable(void)
{
	asm volatile(
		"msr    daifset, #1             // arch_local_fiq_disable"
		:
		:
		: "memory");
}

static inline unsigned long arch_local_save_flags(void)
{
	unsigned long flags;

	asm volatile("mrs    %0, daif"
		     : "=r" (flags)
		     :
		     : "memory");

	flags = (flags >> 7) & 1;

	return flags;
}

static inline int arch_irqs_disabled_flags(unsigned long flags)
{
	return flags;
}

static inline unsigned long arch_local_irq_save(void)
{
	unsigned long flags;

	flags = arch_local_save_flags();

	if (!arch_irqs_disabled_flags(flags))
		arch_local_irq_disable();

	return flags;
}

static inline void arch_local_irq_restore(unsigned long flags)
{
	if (arch_irqs_disabled_flags(flags))
		arch_local_irq_disable();
	else
		arch_local_irq_enable();
}

void set_handle_irq(void (*irq_handler)(void));
void set_handle_fiq(void (*fiq_handler)(void));

#endif

#endif
