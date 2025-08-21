#ifndef __ARCH_THREAD_H__
#define __ARCH_THREAD_H__

#include <trap.h>

struct arch_cpu_ctx {
	struct trap_regs *regs;
};

struct arch_cpu_ctx *
arch_thread_create_cpu_ctx(void *stack_base, unsigned long stack_size,
			   int (*func)(void *), void *arg,
			   void (*end)(int));
void arch_thread_destroy_cpu_ctx(struct arch_cpu_ctx *ctx);
void arch_thread_save_ctx(struct arch_cpu_ctx *to, struct arch_cpu_ctx *from);
void arch_thread_yield(void);
void arch_preempt_disable(void);
void arch_preempt_enable(void);

#endif
