#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <arch.h>

#include <common/module.h>
#include <driver/platform.h>
#include <common/common.h>
#include <driver/interrupt.h>

struct plic {
	struct device *parent;
	unsigned long base;
	unsigned long size;
	struct irq_chip irq_chip;
};

#define PLIC_SP_OFFSET		0x000000
#define PLIC_IP_OFFSET		0x001000
#define PLIC_IE_OFFSET		0x002000
#define PLIC_TC_OFFSET		0x200000

enum {
	PLIC_MODE_M = 0,
	PLIC_MODE_S = 1,
	PLIC_MODE_MAX,
};

#define PLIC_IRQ_MAX		1023

static void __maybe_unused
set_interrupt_source_priority(struct plic *plic, unsigned int hwirq, uint32_t priority)
{
	writel(priority, plic->base + PLIC_SP_OFFSET + hwirq * 4);
}

static void set_contex_threshold(struct plic *plic, int hart, int mode, uint32_t threshold)
{
	unsigned int cpu_id = get_cpu_id();
	unsigned long reg;

	reg = plic->base + PLIC_TC_OFFSET + cpu_id * 0x2000 + mode * 0x1000 + 0;

	writel(threshold, reg);
}

static void set_interrupt_pending(struct plic *plic, unsigned int hwirq, int pending)
{
	unsigned long reg;
	int bit;
	uint32_t value;

	reg = plic->base + PLIC_IP_OFFSET + (hwirq / 32) * 4;
	bit = hwirq % 32;

	value = readl(reg);
	if (pending)
		value |= 1 << bit;
	else
		value &= ~(1 << bit);
	writel(value, reg);
}

static void set_interrupt_enable(struct plic *plic, unsigned int hwirq,
				 int hart, int mode, int enable)
{
	unsigned long reg;
	int bit;
	uint32_t value;

	/* s mode not implemented */
	assert(mode == PLIC_MODE_M);

	reg = plic->base + PLIC_IE_OFFSET + mode * ((PLIC_IRQ_MAX + 1 + 31) / 32) * 4 + (hwirq / 32) * 4;
	bit = hwirq % 32;

	value = readl(reg);
	if (enable)
		value |= 1 << bit;
	else
		value &= ~(1 << bit);
	writel(value, reg);
}

static unsigned int get_claim(struct plic *plic)
{
	unsigned int cpu_id = get_cpu_id();
	int cpu_mode = PLIC_MODE_M;
	unsigned long reg;

	reg = plic->base + PLIC_TC_OFFSET + cpu_id * 0x2000 + cpu_mode * 0x1000 + 4;

	return readl(reg);
}

static void set_claim(struct plic *plic, uint32_t hwirq)
{
	unsigned int cpu_id = get_cpu_id();
	int cpu_mode = PLIC_MODE_M;
	unsigned long reg;

	reg = plic->base + PLIC_TC_OFFSET + cpu_id * 0x2000 + cpu_mode * 0x1000 + 4;

	writel(hwirq, reg);
}

static void plic_enable_irq(struct irq_chip *irqc, unsigned int hwirq)
{
	struct plic *plic = irqc->priv;

	set_interrupt_source_priority(plic, hwirq, 1);
	set_contex_threshold(plic, get_cpu_id(), PLIC_MODE_M, 0);
	set_interrupt_enable(plic, hwirq, get_cpu_id(), PLIC_MODE_M, true);
}

static void plic_disable_irq(struct irq_chip *irqc, unsigned int hwirq)
{
	struct plic *plic = irqc->priv;

	set_interrupt_enable(plic, hwirq, get_cpu_id(), PLIC_MODE_M, false);
}

static void plic_ack_irq(struct irq_chip *irqc, unsigned int hwirq)
{
}

static void plic_eoi_irq(struct irq_chip *irqc, unsigned int hwirq)
{
	set_claim(irqc->priv, hwirq);
}

static void plic_retrigger_irq(struct irq_chip *irqc, unsigned int hwirq)
{
	struct plic *plic = irqc->priv;

	set_interrupt_pending(plic, hwirq, true);
	set_interrupt_pending(plic, hwirq, false);
}

int plic_claim_irq(struct irq_chip *irqc)
{
	unsigned int hwirq;

	hwirq = get_claim(irqc->priv);

	if (hwirq == 0)
		return -1;

	return hwirq;
}

const static struct irq_chip_ops plic_ops = {
	.irq_enable = plic_enable_irq,
	.irq_disable = plic_disable_irq,
	.irq_ack = plic_ack_irq,
	.irq_eoi = plic_eoi_irq,
	.irq_retrigger = plic_retrigger_irq,
	.irq_claim = plic_claim_irq,
};

static int plic_probe(struct platform_device *pdev)
{
	int err = -EINVAL;
	struct plic *plic = malloc(sizeof(struct plic));
	struct irq_chip *irq_chip;

	if (!plic)
		return -ENOMEM;

	plic->base = pdev->reg_base;
	plic->size = pdev->reg_size;

	platform_device_set_user_data(pdev, plic);

	irq_chip = &plic->irq_chip;

	irq_chip->ops = &plic_ops;
	irq_chip->priv = plic;
	irq_chip->hwirq_start = 1;

	/* fixme: should get from fdt, riscv,ndev */
	irq_chip->irq_count = 1023;
	irq_chip->parent = NULL;
	irq_chip->pdev = &pdev->device;

	err = irq_register_chip(irq_chip);
	if (err)
		goto err_register_chip;

	return 0;

err_register_chip:
	free(plic);
	return err;
}

static struct of_device_id match_table[] = {
	{
		.compatible = "sifive,plic-1.0.0",
		.data = (void *)NULL,
	},
	{
		.compatible = "thead,c900-plic",
		.data = (void *)NULL,
	},
	{},
};

static struct platform_driver driver = {
	.driver.name = "plic",
	.probe = plic_probe,
	.of_match_table = match_table,
};

int plic_init(void)
{
	platform_driver_register(&driver);
	return 0;
}

module_init(plic_init);
