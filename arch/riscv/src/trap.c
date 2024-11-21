#include <asm.h>
#include <encoding.h>
#include <trap.h>
#include <stdio.h>

#include <framework/common.h>

static int misa_extension_imp(char ext)
{
	unsigned long misa = csr_read(CSR_MISA);

	if (misa) {
		if ('A' <= ext && ext <= 'Z')
			return misa & (1 << (ext - 'A'));
		if ('a' <= ext && ext <= 'z')
			return misa & (1 << (ext - 'a'));

	}
	return 0;
}

#define misa_extension(c)\
({\
	_Static_assert(((c >= 'A') && (c <= 'Z')),\
		"The parameter of misa_extension must be [A-Z]");\
	misa_extension_imp(c);\
})

static void trap_error(const char *msg,
		ulong mcause, ulong mtval, ulong mtval2,
		ulong mtinst, struct trap_regs *regs)
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

	const char *cause;

	if (mcause & (1UL << 63))
		cause = "Interrupt";
	else if (mcause >= ARRAY_SIZE(exception_cause_list))
		cause = "Unknown Cause";
	else
		cause = exception_cause_list[mcause];

	printf("\n%s\n", msg);

	printf("Hart%d, %s\n", hartid, cause);

	printf("%8s 0x%016lx    %8s 0x%016lx\n", "mcause", mcause, "mstatus", regs->mstatus);
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

void trap_handler(struct trap_regs *regs)
{
	ulong mcause = csr_read(CSR_MCAUSE);
	ulong mtval = csr_read(CSR_MTVAL), mtval2 = 0, mtinst = 0;

	if (misa_extension('H')) {
		mtval2 = csr_read(CSR_MTVAL2);
		mtinst = csr_read(CSR_MTINST);
	}

	trap_error("Exception Occur", mcause, mtval, mtval2, mtinst, regs); }
