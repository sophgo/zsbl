#ifndef __IRQ_H__
#define __IRQ_H__

/* irq */
#define IRQ_LEVEL   0
#define IRQ_EDGE    3

#define IRQF_TRIGGER_NONE    0x00000000
#define IRQF_TRIGGER_RISING  0x00000001
#define IRQF_TRIGGER_FALLING 0x00000002
#define IRQF_TRIGGER_HIGH    0x00000004
#define IRQF_TRIGGER_LOW     0x00000008
#define IRQF_TRIGGER_MASK   (IRQF_TRIGGER_HIGH | IRQF_TRIGGER_LOW | \
		                 IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING)

#define IRQ_NAME_MAX	32

typedef int (*irq_handler_t)(int irqn, void *priv);

struct irqaction {
	irq_handler_t handler;
	unsigned int flags;
	unsigned int irq;
	void *priv;
	char name[IRQ_NAME_MAX];
};

int request_irq(unsigned int irqn, irq_handler_t handler, unsigned long flags,
		const char *name, void *priv);

void disable_irq(unsigned int irqn);
void enable_irq(unsigned int irqn);

void irq_trigger(int irqn);
void irq_clear(int irqn);
int irq_get_nums(void);

void irq_init(void);

#endif
