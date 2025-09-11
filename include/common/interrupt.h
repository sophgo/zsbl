#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <lib/list.h>
#include <driver/device.h>

enum {
	IRQ_NONE = 0,
	IRQ_HANDLED,
};

struct irq_data;
struct irq_chip;

struct irq_chip_ops {
	void (*irq_enable)(struct irq_chip *irqc, unsigned int hwirq);
	void (*irq_disable)(struct irq_chip *irqc, unsigned int hwirq);
	void (*irq_ack)(struct irq_chip *irqc, unsigned int hwirq);
	void (*irq_eoi)(struct irq_chip *irqc, unsigned int hwirq);
	void (*irq_retrigger)(struct irq_chip *irqc, unsigned int hwirq);
	int (*irq_claim)(struct irq_chip *irqc);
};

/* only linar logic irq number to hardware irq number is supported */
struct irq_chip {
	const struct irq_chip_ops *ops;
	void *priv;
	unsigned int hwirq_start, irq_count;
	unsigned int irq_start;
	struct irq_chip *parent;
	struct device device;
	struct device *pdev;
};

/*
 * a hwirq may have multiple irq_data with different handler or private data
 * the share the same hardware irq.
 */
struct irq_data {
	unsigned int hwirq;
	unsigned int irq;
	struct irq_chip *parent;

	int (*handler)(struct irq_data *irqd, void *priv);
	void *handler_data;

	struct list_head list;
};

struct irq_data *irq_request(unsigned int irq, const char *name,
			     int (*func)(struct irq_data *irqd, void *priv),
			     void *priv);

void irq_free(unsigned int irq, int (*func)(struct irq_data *irqd, void *priv), void *priv);

void irq_enable(unsigned int irq);
void irq_disable(unsigned int irq);
void irq_retrigger(unsigned int irq);

int irq_register_chip(struct irq_chip *irqc);
unsigned int irq_get(struct irq_chip *irqc, unsigned int hwirq);

struct irq_chip *irq_get_chip_by_name(const char *name);

#endif
