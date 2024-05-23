#include <asm.h>
#include <encoding.h>
#include <trap.h>
#include <stdio.h>

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

static void  trap_error(const char *msg,
			    ulong mcause, ulong mtval, ulong mtval2,
			    ulong mtinst, struct trap_regs *regs)
{
	u32 hartid = current_hartid();

	printf("%s: hart%d: %s\n", __func__, hartid, msg);
	printf("%s: hart%d: mcause=0x%" PRILX " mtval=0x%" PRILX "\n",
	       __func__, hartid, mcause, mtval);
	if (misa_extension('H')) {
		printf("%s: hart%d: mtval2=0x%" PRILX
		       " mtinst=0x%" PRILX "\n",
		       __func__, hartid, mtval2, mtinst);
	}
	printf("%s: hart%d: mepc=0x%" PRILX " mstatus=0x%" PRILX "\n",
	       __func__, hartid, regs->mepc, regs->mstatus);
	printf("%s: hart%d: %s=0x%" PRILX " %s=0x%" PRILX "\n", __func__,
	       hartid, "ra", regs->ra, "sp", regs->sp);
	printf("%s: hart%d: %s=0x%" PRILX " %s=0x%" PRILX "\n", __func__,
	       hartid, "gp", regs->gp, "tp", regs->tp);
	printf("%s: hart%d: %s=0x%" PRILX " %s=0x%" PRILX "\n", __func__,
	       hartid, "s0", regs->s0, "s1", regs->s1);
	printf("%s: hart%d: %s=0x%" PRILX " %s=0x%" PRILX "\n", __func__,
	       hartid, "a0", regs->a0, "a1", regs->a1);
	printf("%s: hart%d: %s=0x%" PRILX " %s=0x%" PRILX "\n", __func__,
	       hartid, "a2", regs->a2, "a3", regs->a3);
	printf("%s: hart%d: %s=0x%" PRILX " %s=0x%" PRILX "\n", __func__,
	       hartid, "a4", regs->a4, "a5", regs->a5);
	printf("%s: hart%d: %s=0x%" PRILX " %s=0x%" PRILX "\n", __func__,
	       hartid, "a6", regs->a6, "a7", regs->a7);
	printf("%s: hart%d: %s=0x%" PRILX " %s=0x%" PRILX "\n", __func__,
	       hartid, "s2", regs->s2, "s3", regs->s3);
	printf("%s: hart%d: %s=0x%" PRILX " %s=0x%" PRILX "\n", __func__,
	       hartid, "s4", regs->s4, "s5", regs->s5);
	printf("%s: hart%d: %s=0x%" PRILX " %s=0x%" PRILX "\n", __func__,
	       hartid, "s6", regs->s6, "s7", regs->s7);
	printf("%s: hart%d: %s=0x%" PRILX " %s=0x%" PRILX "\n", __func__,
	       hartid, "s8", regs->s8, "s9", regs->s9);
	printf("%s: hart%d: %s=0x%" PRILX " %s=0x%" PRILX "\n", __func__,
	       hartid, "s10", regs->s10, "s11", regs->s11);
	printf("%s: hart%d: %s=0x%" PRILX " %s=0x%" PRILX "\n", __func__,
	       hartid, "t0", regs->t0, "t1", regs->t1);
	printf("%s: hart%d: %s=0x%" PRILX " %s=0x%" PRILX "\n", __func__,
	       hartid, "t2", regs->t2, "t3", regs->t3);
	printf("%s: hart%d: %s=0x%" PRILX " %s=0x%" PRILX "\n", __func__,
	       hartid, "t4", regs->t4, "t5", regs->t5);
	printf("%s: hart%d: %s=0x%" PRILX "\n", __func__, hartid, "t6",
	       regs->t6);

	while (1)
		wfi();

}

void trap_handler(struct trap_regs *regs)
{
	const char *msg = "trap handler failed";
	ulong mcause = csr_read(CSR_MCAUSE);
	ulong mtval = csr_read(CSR_MTVAL), mtval2 = 0, mtinst = 0;

	if (misa_extension('H')) {
		mtval2 = csr_read(CSR_MTVAL2);
		mtinst = csr_read(CSR_MTINST);
	}

	trap_error(msg, mcause, mtval, mtval2, mtinst, regs); }
