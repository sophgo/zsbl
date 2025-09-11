#include <asm.h>
#include <encoding.h>
#include <trap.h>
#include <stdio.h>
#include <timer.h>

#include <common/common.h>
#include <common/thread.h>

unsigned char __attribute__((aligned(16))) trap_stack[TRAP_STACK_SIZE];

static void trap_error(ulong exception_code, struct trap_regs *regs)
{
	const char *exception_cause_list[] = {
		"Instruction address misaligned",
		"Instruction access fault",
		"Illegal instruction",
		"Breakpoint",
		"Load address misaligned",
		"Load access fault",
		"Store/AMO address misaligned",
		"Store/AMO access fault",
		"Environment call from U-mode",
		"Environment call from S-mode",
		"Reserved",
		"Environment call from M-mode",
		"Instruction page fault",
		"Load page fault",
		"Reserved",
		"Store/AMO page fault",
	};

	u32 hartid = current_hartid();
	ulong mtval = csr_read(CSR_MTVAL);
	const char *cause;

	if (exception_code >= ARRAY_SIZE(exception_cause_list))
		cause = "Unknown Cause";
	else
		cause = exception_cause_list[exception_code];

	printf("\nException Occur\n");

	printf("Hart%d, %s\n", hartid, cause);

	printf("%8s 0x%016lx    %8s 0x%016lx\n", "mcause", exception_code, "mstatus", regs->mstatus);
	printf("%8s 0x%016lx    %8s 0x%016lx\n", "mepc", regs->mepc, "mtval", mtval);
	printf("%8s 0x%016lx    %8s 0x%016lx\n", "ra", regs->ra, "sp", regs->sp);
	printf("%8s 0x%016lx    %8s 0x%016lx\n", "gp", regs->gp, "tp", regs->tp);
	printf("%8s 0x%016lx    %8s 0x%016lx\n", "a0", regs->a0, "a1", regs->a1);
	printf("%8s 0x%016lx    %8s 0x%016lx\n", "a2", regs->a2, "a3", regs->a3);
	printf("%8s 0x%016lx    %8s 0x%016lx\n", "a4", regs->a4, "a5", regs->a5);
	printf("%8s 0x%016lx    %8s 0x%016lx\n", "a6", regs->a6, "a7", regs->a7);
	printf("%8s 0x%016lx    %8s 0x%016lx\n", "s0", regs->s0, "s1", regs->s1);
	printf("%8s 0x%016lx    %8s 0x%016lx\n", "s2", regs->s2, "s3", regs->s3);
	printf("%8s 0x%016lx    %8s 0x%016lx\n", "s4", regs->s4, "s5", regs->s5);
	printf("%8s 0x%016lx    %8s 0x%016lx\n", "s6", regs->s6, "s7", regs->s7);
	printf("%8s 0x%016lx    %8s 0x%016lx\n", "s8", regs->s8, "s9", regs->s9);
	printf("%8s 0x%016lx    %8s 0x%016lx\n", "s10", regs->s10, "s11", regs->s11);
	printf("%8s 0x%016lx    %8s 0x%016lx\n", "t0", regs->t0, "t1", regs->t1);
	printf("%8s 0x%016lx    %8s 0x%016lx\n", "t2", regs->t2, "t3", regs->t3);
	printf("%8s 0x%016lx    %8s 0x%016lx\n", "t4", regs->t4, "t5", regs->t5);
	printf("%8s 0x%016lx    %8s 0x%016lx\n", "t6", regs->t6, "zero", 0UL);

	while (1)
		wfi();
}

void __attribute__((weak)) sswi_isr(void)
{
	printf("Supervisor Software Interrupt Happen without Setup Before\n");
}

void __attribute__((weak)) mswi_isr(void)
{
	printf("Machine Software Interrupt Happen without Setup Before\n");
}

void __attribute__((weak)) stimer_isr(void)
{
	printf("Supervisor Timer Interrupt Happen without Setup Before\n");
}

void __attribute__((weak)) mtimer_isr(void)
{
	printf("Machine Timer Interrupt Happen without Setup Before\n");
}

void __attribute__((weak)) sei_isr(void)
{
	printf("Supervisor External Interrupt Happen without Setup Before\n");
}

void __attribute__((weak)) mei_isr(void)
{
	printf("Machine External Interrupt Happen without Setup Before\n");
}

static void unknown_isr(void)
{
	printf("Unknown Interrupt Happen\n");
}

static void (*(isr_table[]))(void) = {
	[1] = sswi_isr,
	[3] = mswi_isr,
	[5] = stimer_isr,
	[7] = mtimer_isr,
	[9] = sei_isr,
	[11] = mei_isr,
};

void arch_install_external_interrupt_handler(void (*exti_handler))
{
	isr_table[11] = exti_handler;
}

static void trap_interrupt(ulong exception_code, struct trap_regs *regs)
{
	void (*isr)(void);

	if (exception_code < ARRAY_SIZE(isr_table))
		isr = isr_table[exception_code];
	else
		isr = NULL;

	if (isr)
		isr();
	else
		unknown_isr();
}

#ifdef CONFIG_MULTI_THREAD
#include <arch_thread.h>
struct trap_regs *trap_handler(struct trap_regs *regs)
{
	struct arch_cpu_ctx ctx;

	ulong mcause = csr_read(CSR_MCAUSE);
	ulong is_interrupt = mcause & (1UL << 63);
	ulong exception_code = mcause & ~(1UL << 63);


	/*
	 * filter out machine call
	 * ecall exception is synchronous, it means error pc is the address where
	 * causes this exception, we need multiple of 4 (size of ecall instruction)
	 */
	if (is_interrupt)
		trap_interrupt(exception_code, regs);
	else if (exception_code != 11)
		trap_error(exception_code, regs);
	else
		regs->mepc += 4;

	ctx.regs = regs;

	return sched_thread(&ctx)->regs;
}
#else
struct trap_regs *trap_handler(struct trap_regs *regs)
{
	ulong mcause = csr_read(CSR_MCAUSE);
	ulong is_interrupt = mcause & (1UL << 63);
	ulong exception_code = mcause & ~(1UL << 63);

	if (is_interrupt)
		trap_interrupt(exception_code, regs);
	else
		trap_error(exception_code, regs);

	return regs;
}
#endif

