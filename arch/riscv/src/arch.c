#include <arch.h>
#include <asm.h>

#define CSR_MSTATUS_MIE		(1UL << 3)
#define CSR_MIE_MTIE		(1UL << 7)

void arch_disable_local_irq(void)
{
	csr_clear(CSR_MSTATUS, CSR_MSTATUS_MIE);
}

void arch_enable_local_irq(void)
{
	csr_set(CSR_MSTATUS, CSR_MSTATUS_MIE);
}

void arch_disable_local_timer_irq(void)
{
	csr_clear(CSR_MIE, CSR_MIE_MTIE);
}

void arch_enable_local_timer_irq(void)
{
	csr_set(CSR_MIE, CSR_MIE_MTIE);
}
