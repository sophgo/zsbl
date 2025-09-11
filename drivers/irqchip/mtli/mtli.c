#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <arch.h>

#include <common/module.h>
#include <common/common.h>
#include <driver/platform.h>
#include <driver/interrupt.h>
#include <lib/cli.h>

struct mtli {
	unsigned long base;
	int irq_start, irq_count;
};

struct mtli mtli;

static void mtli_set_pending(struct mtli *mtli, int n)
{
	writel(1, mtli->base + n * 4);
}

static void mtli_clear_pending(struct mtli *mtli, int n)
{
	writel(0, mtli->base + n * 4);
}

static void mtli_trigger_pending(struct mtli *mtli, int n)
{
	mtli_set_pending(mtli, n);
	mtli_clear_pending(mtli, n);
}

int mtli_device_irq_handler(struct irq_data *irqd, void *priv)
{
	pr_info("MTLI device irq happen\n");
	return 0;
}

static int probe(struct platform_device *pdev)
{
	int irq;

	irq = platform_get_irq(pdev, 0);

	irq_request(irq, "mtli-device", mtli_device_irq_handler, NULL);

	mtli.base = pdev->reg_base;

	platform_device_set_user_data(pdev, &mtli);

	return 0;
}

static struct of_device_id match_table[] = {
	{
		.compatible = "sophgo,mtli",
		.data = (void *)NULL,
	},
	{},
};

static struct platform_driver driver = {
	.driver.name = "mtli",
	.probe = probe,
	.of_match_table = match_table,
};

static int mtli_module_init(void)
{
	platform_driver_register(&driver);
	return 0;
}

module_init(mtli_module_init);

static void command_mtli_trigger(struct command *c, int argc, const char *argv[])
{
	int n;

	if (argc > 2) {
		console_printf(command_get_console(c), "Invalid usage: mtli msi-index\n");
		return;
	}

	n = strtoul(argv[1], NULL, 0);

	console_printf(command_get_console(c), "Trigger %d MSI\n", n);

	mtli_trigger_pending(&mtli, n);
}

cli_command(mtli, command_mtli_trigger);

