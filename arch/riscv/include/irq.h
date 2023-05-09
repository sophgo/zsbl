#ifndef __IRQ_H__
#define __IRQ_H__

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
