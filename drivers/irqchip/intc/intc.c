#include <common/ictl.h>
#include <common/module.h>
#include <irq.h>
#include <arch.h>
#include <memmap.h>
#include <string.h>

#define INTC_ICTL_NUM	2
#define INTC_IRQ_NUM	(ICT_IRQ_NUM * INTC_ICTL_NUM)

static spinlock_t lock;
static struct irqaction action[INTC_IRQ_NUM];
static void *ictl[INTC_ICTL_NUM] = {
	(void *)ICTL0_BASE, (void *)ICTL1_BASE,
};

int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
		const char *name, void *priv)
{
	unsigned long spin_lock_irq_flags;

	spin_lock_irqsave(&lock, spin_lock_irq_flags);

	action[irq].handler = handler;
	action[irq].flags = flags;
	action[irq].irq = irq;
	action[irq].priv = priv;
	strncpy(action[irq].name, name, sizeof(action[irq].name));

	enable_irq(irq);

	spin_unlock_irqrestore(&lock, spin_lock_irq_flags);

	return 0;
}

void enable_irq(unsigned int irq)
{
	unsigned long flags;

	/* ictl0 63 is the irq input port from ictl1 */
	if (irq == ICT_IRQ_NUM - 1)
		return;

	spin_lock_irqsave(&lock, flags);

	if (irq < ICT_IRQ_NUM)
		ictl_clear_irq_mask_bits(ictl[0], 1UL << irq);
	else
		ictl_clear_irq_mask_bits(ictl[1], 1UL << (irq - ICT_IRQ_NUM));

	spin_unlock_irqrestore(&lock, flags);
}

void disable_irq(unsigned int irq)
{
	unsigned long flags;

	/* ictl0 63 is the irq input port from ictl1 */
	if (irq == ICT_IRQ_NUM - 1)
		return;

	spin_lock_irqsave(&lock, flags);

	if (irq < ICT_IRQ_NUM)
		ictl_set_irq_mask_bits(ictl[0], 1UL << irq);
	else
		ictl_set_irq_mask_bits(ictl[1], 1UL << (irq - ICT_IRQ_NUM));

	spin_unlock_irqrestore(&lock, flags);
}

static void irq_handler(void)
{
	u64 ictl0_final, ictl1_final;
	int i;
	struct irqaction *act;

	ictl0_final = ictl_get_irq_final_status(ictl[0]);

	/* process ictl1 first */
	if (ictl0_final & (1UL << (ICT_IRQ_NUM - 1))) {
		ictl1_final = ictl_get_irq_final_status(ictl[1]);
		for (i = 0; i < ICT_IRQ_NUM; ++i) {
			if (ictl1_final & (1 << i)) {
				act = action + i + ICT_IRQ_NUM;
				act->handler(act->irq, act->priv);
			}
		}
		ictl0_final &= ~(1UL << (ICT_IRQ_NUM - 1));
	}

	if (ictl0_final) {
		for (i = 0; i < ICT_IRQ_NUM - 1; ++i) {
			if (ictl0_final & (1 << i)) {
				act = action + i;
				act->handler(act->irq, act->priv);
			}
		}
	}
}

static int intc_init(void)
{
	spin_lock_init(&lock);

	ictl_init(ictl[0]);
	ictl_init(ictl[1]);

	/* enable all interrupts, interrupt enable/disable relay on
	 * ictl_set_irq_mask
	 */
	ictl_set_irq_enable(ictl[0], 0xffffffffffffffffUL);
	ictl_set_irq_enable(ictl[1], 0xffffffffffffffffUL);

	/* always enable ictl0 irq 63 */
	ictl_clear_irq_mask_bits(ictl[0], 1UL << (ICT_IRQ_NUM - 1));

	set_handle_irq(irq_handler);
	return 0;
}

// arch_init(intc_init);
