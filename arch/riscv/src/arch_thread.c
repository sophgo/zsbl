#include <stdlib.h>
#include <string.h>
#include <asm.h>
#include <arch.h>
#include <arch_thread.h>

struct arch_cpu_ctx *
arch_thread_create_cpu_ctx(void *stack_base, unsigned long stack_size,
			   int (*func)(void *), void *arg,
			   void (*end)(int))
{
	struct arch_cpu_ctx *t;

	t = malloc(sizeof(struct arch_cpu_ctx));
	if (!t)
		return NULL;

	t->regs = stack_base + stack_size - sizeof(struct trap_regs);

	/* setup context */
	memset(t->regs, 0, sizeof(*t->regs));

	/* inherit current mstatus */
	t->regs->mstatus = csr_read(CSR_MSTATUS);
	/* restore MPIE with MIE */
	t->regs->mstatus |= ((t->regs->mstatus >> 3) & 1) << 7;
	/* set next privilidge level to machine mode */
	t->regs->mstatus |= (3 << 11);

	/* setup entry point */
	t->regs->mepc = (unsigned long)func;
	/* setup stack top */
	t->regs->sp = (unsigned long)t->regs;
	/* final call after func execut done */
	t->regs->ra = (unsigned long)end;
	/* parameter of thread routine */
	t->regs->a0 = (unsigned long)arg;

	return t;

}

void arch_thread_destroy_cpu_ctx(struct arch_cpu_ctx *ctx)
{
	free(ctx);
}

void arch_thread_save_ctx(struct arch_cpu_ctx *to, struct arch_cpu_ctx *from)
{
	to->regs = from->regs;
}

void arch_thread_yield(void)
{
	/* trigger an exception, schedule will happen in trap_handler */
	asm volatile ("ecall");
}

void arch_preempt_disable(void)
{
	arch_disable_local_irq();
}

void arch_preempt_enable(void)
{

	arch_enable_local_irq();
}
