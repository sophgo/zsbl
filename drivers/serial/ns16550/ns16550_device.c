#include <stdint.h>
#include <errno.h>
#include <arch.h>
#include <libfdt.h>
#include <common/module.h>
#include <common/common.h>
#include <driver/platform.h>
#include <driver/serial.h>

#include "ns16550.h"

struct ns16550_device {
	struct ns16550 hw;
	int stdio;
	struct platform_device *pdev;
};

static int ns16550_device_putc(struct serial *sdev, uint8_t ch)
{
	struct ns16550_device *ndev = sdev->data;

	return ns16550_putc(&ndev->hw, ch);
}

static int ns16550_device_getc(struct serial *sdev)
{
	struct ns16550_device *ndev = sdev->data;

	return ns16550_getc(&ndev->hw);
}

static struct serial_ops ops = {
	.input = ns16550_device_getc,
	.output = ns16550_device_putc,
};

static int probe(struct platform_device *pdev)
{
	struct ns16550_device *ndev;
	struct ns16550 *hw;
	struct serial *sdev;
	unsigned int baudrate, pclk;
	unsigned int reg_shift, reg_io_width;
	int stdio;

	pr_info("ns16550 probe\n");

	const struct fdt_property *prop;
	int plen;

	prop = fdt_get_property(pdev->of, pdev->of_node_offset, "speed", &plen);
	if (prop)
		baudrate = fdt32_to_cpu(*(volatile uint32_t *)(prop->data));
	else
		baudrate = 115200;

	prop = fdt_get_property(pdev->of, pdev->of_node_offset, "clock-frequency", &plen);
	if (prop)
		pclk = fdt32_to_cpu(*(volatile uint32_t *)(prop->data));
	else 
		pclk = 500000000;

	prop = fdt_get_property(pdev->of, pdev->of_node_offset, "reg-shift", &plen);
	if (prop)
		reg_shift = fdt32_to_cpu(*(volatile uint32_t *)(prop->data));
	else 
		reg_shift = 2;

	prop = fdt_get_property(pdev->of, pdev->of_node_offset, "reg-io-width", &plen);
	if (prop)
		reg_io_width = fdt32_to_cpu(*(volatile uint32_t *)(prop->data));
	else 
		reg_io_width = 4;

	prop = fdt_get_property(pdev->of, pdev->of_node_offset, "stdio", &plen);
	if (prop)
		stdio = true;
	else 
		stdio = false;

	ndev = malloc(sizeof(struct ns16550_device));
	if (!ndev)
		return -ENOMEM;

	ndev->pdev = pdev;
	ndev->stdio = stdio;

	hw = &ndev->hw;

	hw->base = pdev->reg_base;
	hw->baudrate = baudrate;
	hw->pclk = pclk;
	hw->reg_io_width = reg_io_width;
	hw->reg_shift = reg_shift;

	ns16550_init(hw);

	sdev = serial_alloc();
	if (!sdev)
		return -ENOMEM;

	sdev->ops = &ops;
	sdev->baudrate = baudrate;
	sdev->data = ndev;
	sdev->stdio = stdio;

	return serial_register(sdev);
}

static struct of_device_id match_table[] = {
	{
		.compatible = "snps,dw-apb-uart",
		.data = (void *)NULL,
	},
	{
		.compatible = "ns16550a",
		.data = (void *)NULL,
	},

	{},
};

static struct platform_driver driver = {
	.driver.name = "ns16550",
	.probe = probe,
	.of_match_table = match_table,
};

static int ns16550_device_init(void)
{
	platform_driver_register(&driver);
	return 0;
}

module_init(ns16550_device_init);
