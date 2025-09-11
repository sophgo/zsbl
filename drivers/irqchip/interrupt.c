#include <common/common.h>
#include <common/module.h>

#include <lib/console.h>
#include <lib/cli.h>

#include <driver/interrupt.h>

#include <errno.h>
#include <string.h>
#include <trap.h>
#include <arch.h>

LIST_HEAD(irq_chip_list);
LIST_HEAD(irq_data_list);

unsigned int irq_end;

static unsigned int alloc_irq(unsigned int count)
{
	unsigned int irq = irq_end;

	irq_end += count;

	return irq;
}

struct irq_chip *find_irq_chip(unsigned int irq, unsigned int *hwirq)
{
	struct device *dev;
	struct irq_chip *irqc;

	list_for_each_entry(dev, &irq_chip_list, list_head) {
		irqc = container_of(dev, struct irq_chip, device);
		if (irq >= irqc->irq_start && irq < irqc->irq_start + irqc->irq_count) {
			if (hwirq)
				*hwirq = irqc->hwirq_start + (irq - irqc->irq_start);
			return irqc;
		}
	}
	return NULL;
}

struct irq_data *irq_request(unsigned int irq, const char *name,
			     int (*func)(struct irq_data *irqd, void *priv),
			     void *priv)
{
	struct irq_data *irqd;
	struct irq_chip *irqc;

	irqd = malloc(sizeof(struct irq_data));
	if (!irqd)
		return NULL;

	irqc = find_irq_chip(irq, &irqd->hwirq);
	if (!irqc)
		return NULL;

	irqd->irq = irq;
	irqd->parent = irqc;

	irqd->handler = func;
	irqd->handler_data = priv;

	list_add_tail(&irqd->list, &irq_data_list);

	irqc->ops->irq_enable(irqc, irqd->hwirq);

	return NULL;
}

void irq_free(unsigned int irq, int (*func)(struct irq_data *irqd, void *priv), void *priv)
{
	struct irq_data *irqd;

	list_for_each_entry(irqd, &irq_data_list, list) {
		if (irqd->irq == irq && irqd->handler == func && irqd->handler_data == priv)
			break;
	}

	list_del(&irqd->list);
}

void irq_enable(unsigned int irq)
{
	struct irq_chip *irqc;
	unsigned int hwirq;

	irqc = find_irq_chip(irq, &hwirq);
	if(irqc)
		irqc->ops->irq_enable(irqc, hwirq);
}

void irq_disable(unsigned int irq)
{
	struct irq_chip *irqc;
	unsigned int hwirq;

	irqc = find_irq_chip(irq, &hwirq);
	if(irqc)
		irqc->ops->irq_disable(irqc, hwirq);
}

int irq_register_chip(struct irq_chip *irqc)
{
	int err;

	if (!irqc->pdev)
		return -EINVAL;

	if (irqc->irq_count == 0)
		return -EINVAL;

	err = snprintf(irqc->device.name, sizeof(irqc->device.name), "irq:%s", irqc->pdev->name);
	if (err == sizeof(irqc->device.name)) {
		pr_info("device name too long %d: %s\n", err, irqc->device.name);
		return -ENOMEM;
	}

	irqc->irq_start = alloc_irq(irqc->irq_count);

	pr_info("Add to irq chip list\n");
	list_add_tail(&irqc->device.list_head, &irq_chip_list);

	return 0;
}

struct irq_chip *irq_get_chip_by_name(const char *name)
{
	struct irq_chip *irqc;
	struct device *dev;

	dev = device_find_by_name(&irq_chip_list, name);
	if (dev) {
		irqc = container_of(dev, struct irq_chip, device);
		return irqc;
	}

	return NULL;
}

unsigned int irq_get(struct irq_chip *irqc, unsigned int hwirq)
{
	return irqc->irq_start + hwirq - irqc->hwirq_start;
}

static void irq_handler(void)
{
	struct device *dev;
	struct irq_chip *irqc;
	struct irq_data *irqd;
	int irq, hwirq;

	hwirq = -1;

	list_for_each_entry(dev, &irq_chip_list, list_head) {
		irqc = container_of(dev, struct irq_chip, device);
		hwirq = irqc->ops->irq_claim(irqc);
		if (hwirq >= 0) {
			/* handler irq */
			irq = irq_get(irqc, hwirq);
			break;
		}
	}

	if (hwirq < 0)
		return;

	if (irqc->ops->irq_ack)
		irqc->ops->irq_ack(irqc, hwirq);

	list_for_each_entry(irqd, &irq_data_list, list) {
		if (irqd->irq == irq && irqd->handler)
			if (irqd->handler(irqd, irqd->handler_data) == IRQ_HANDLED)
				return;
	}

	if (irqc->ops->irq_eoi)
		irqc->ops->irq_eoi(irqc, hwirq);
}

int irq_init(void)
{
	arch_install_external_interrupt_handler(irq_handler);
	arch_enable_local_external_irq();
	return 0;
}

void irq_retrigger(unsigned int irq)
{
	struct irq_chip *irqc;
	unsigned int hwirq;

	irqc = find_irq_chip(irq, &hwirq);

	if (irqc)
		irqc->ops->irq_retrigger(irqc, hwirq);
}

subsys_init(irq_init);

static void command_show_devices(struct command *c, int argc, const char *argv[])
{
	struct list_head *p;
	struct device *dev;
	struct irq_chip *irqc;
	int i;

	console_printf(command_get_console(c),
		       "%6s %40s %14s %14s %14s %20s\n",
		       "Index", "Device", "IRQ Start", "HW IRQ Start", "IRQ Count", "Alias");

	i = 0;
	list_for_each(p, &irq_chip_list) {
		dev = container_of(p, struct device, list_head);
		irqc = container_of(dev, struct irq_chip, device);
		console_printf(command_get_console(c),
			       "%6d %40s %14ld %14ld %14ld %20s\n",
			       i, irqc->device.name,
			       irqc->irq_start, irqc->hwirq_start, irqc->irq_count,
			       irqc->device.alias);
		++i;
	}
}

cli_command(lsirqc, command_show_devices);

