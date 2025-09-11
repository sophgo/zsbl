#include <stdio.h>
#include <string.h>
#include <libfdt.h>
#include <errno.h>

#include <driver/dtb.h>
#include <driver/platform.h>
#include <driver/interrupt.h>
#include <lib/container_of.h>
#include <common/common.h>
#include <common/module.h>

#include <lib/cli.h>

static LIST_HEAD(device_list);

int platform_list_devices(void)
{
	struct list_head *p;
	struct device *dev;
	struct platform_device *pdev;
	int i;

	pr_info("%6s %40s %14s %14s %20s %20s\n", "Index", "Device", "Base", "Size", "Driver", "Alias");

	i = 0;
	list_for_each(p, &device_list) {
		dev = container_of(p, struct device, list_head);
		pdev = container_of(dev, struct platform_device, device);
		pr_info("%6d %40s %14lx %14lx %20s %20s\n",
				i, pdev->device.name,
				pdev->reg_base, pdev->reg_size,
				(pdev->pdrv) ? pdev->pdrv->driver.name : "",
				pdev->device.alias);
		++i;
	}

	return i;
}

static void command_show_devices(struct command *c, int argc, const char *argv[])
{
	struct list_head *p;
	struct device *dev;
	struct platform_device *pdev;
	int i;

	console_printf(command_get_console(c),
		       "%6s %40s %14s %14s %20s %20s\n",
		       "Index", "Device", "Base", "Size", "Driver", "Alias");

	i = 0;
	list_for_each(p, &device_list) {
		dev = container_of(p, struct device, list_head);
		pdev = container_of(dev, struct platform_device, device);
		console_printf(command_get_console(c),
			       "%6d %40s %14lx %14lx %20s %20s\n",
			       i, pdev->device.name,
			       pdev->reg_base, pdev->reg_size,
			       (pdev->pdrv) ? pdev->pdrv->driver.name : "",
			       pdev->device.alias);
		++i;
	}
}

cli_command(lspdev, command_show_devices);

struct platform_device *platform_device_alloc(void)
{
	struct platform_device *pdev;

	pdev = malloc(sizeof(struct platform_device));

	if (pdev)
		memset(pdev, 0, sizeof(struct platform_device));

	pdev->device.status = DEVICE_STATUS_NEW;

	return pdev;
}

void platform_device_free(struct platform_device *pdev)
{
	free(pdev);
}

void platform_device_add(struct platform_device *pdev)
{
	list_add_tail(&pdev->device.list_head, &device_list);
}

void platform_device_remove(struct platform_device *pdev)
{
	list_del(&pdev->device.list_head);
}

int platform_device_count(void)
{
	return list_count_nodes(&device_list);
}

struct platform_device *platform_device_find_by_name(const char *name)
{
	struct device *dev;

	dev = device_find_by_name(&device_list, name);
	if (dev)
		return container_of(dev, struct platform_device, device);

	return NULL;
}

void platform_device_set_user_data(struct platform_device *pdev, void *data)
{
	pdev->data = data;
}

void *platform_device_get_user_data(struct platform_device *pdev)
{
	return pdev->data;
}

/* platform driver */

static LIST_HEAD(driver_list);

static void command_show_drivers(struct command *c, int argc, const char *argv[])
{
	struct list_head *p;
	struct driver *drv;
	struct platform_driver *pdrv;
	int i;

	console_printf(command_get_console(c),
		       "%6s %40s %40s\n",
		       "Index", "Name", "Match");

	i = 0;
	list_for_each(p, &driver_list) {
		drv = container_of(p, struct driver, list_head);
		pdrv = container_of(drv, struct platform_driver, driver);
		console_printf(command_get_console(c),
			       "%6d %40s %40s\n",
			       i, pdrv->driver.name, pdrv->of_match_table->compatible);
		++i;
	}
}

cli_command(lspdrv, command_show_drivers);


int platform_driver_register(struct platform_driver *pdrv)
{
	if (pdrv->driver.name[0] == 0) {
		pr_debug("no driver name\n");
		return -EINVAL;
	}

	list_add_tail(&pdrv->driver.list_head, &driver_list);

	return 0;
}

int platform_list_drivers(void)
{
	struct list_head *p;
	struct platform_driver *pdrv;
	struct driver *drv;
	int i;

	i = 0;
	list_for_each(p, &driver_list) {
		drv = container_of(p, struct driver, list_head);
		pdrv = container_of(drv, struct platform_driver, driver);
		pr_info("%d: %s\n", i, pdrv->driver.name);
		++i;
	}

	return i;

}

/* parse dtb */
int platform_init(void)
{
	pr_debug("platform subsystem driver init\n");
	return 0;
}

subsys_init(platform_init);

/* show debug info when matching */
#ifdef CONFIG_DEBUG
static void debug_device_driver_matching(struct platform_device *pdev, struct platform_driver *pdrv)
{

	const void *prop;
	int len;

	prop = fdt_getprop(pdev->of, pdev->of_node_offset, "compatible", &len);

	pr_info("Matching device %s with driver %s\n",
		(const char *)prop, pdrv->of_match_table->compatible);
}
#endif

static int bind_driver(struct platform_device *pdev, struct platform_driver *pdrv)
{
	int i;
	int err;

	if (!pdrv->of_match_table) {
		pr_warn("no match table found in driver %s\n", pdrv->driver.name);
		return -ENOTSUP;
	}


	for (i = 0; pdrv->of_match_table[i].compatible[0]; ++i) {
#ifdef CONFIG_DEBUG
		debug_device_driver_matching(pdev, pdrv);
#endif
		if (fdt_node_check_compatible(pdev->of, pdev->of_node_offset,
					pdrv->of_match_table[i].compatible) == 0)
			break;
	}

	if (pdrv->of_match_table[i].compatible[0] == 0) {
		pr_debug("can not find driver for device %s\n", pdev->device.name);
		return -ENOTSUP;
	}

	pr_debug("device %s bind to driver %s\n",
			pdev->device.name, pdrv->driver.name);

	/* bind driver to device */
	pdev->pdrv = pdrv;
	pdev->match = &pdrv->of_match_table[i];
	pdev->device.status = DEVICE_STATUS_BOUND;

	/* customer probe */
	err = pdrv->probe(pdev);
	if (err)
		pdev->device.status = DEVICE_STATUS_ERROR;
	else
		pdev->device.status = DEVICE_STATUS_OK;

	return err;
}

static const struct of_device_id *match_table(struct platform_device *pdev)
{
	/* iterate platform drivers */
	struct list_head *p;
	struct platform_driver *pdrv;
	struct driver *drv;
	int err;

	list_for_each(p, &driver_list) {
		drv = container_of(p, struct driver, list_head);
		pdrv = container_of(drv, struct platform_driver, driver);
		err = bind_driver(pdev, pdrv);
		if (err == 0)
			return pdev->match;
	}

	pr_debug("no driver matching device %s\n", pdev->device.name);
	return NULL;

}

static struct platform_device *platform_device_create(void *dtb, int node_offset)
{
	struct platform_device *pdev;
	const struct fdt_property *prop;
	int plen;
	const char *compatible;
	const char *node_name;
	int address_cells, size_cells, parent_node_offset;
	uint64_t reg_base, reg_size;

	prop = fdt_get_property(dtb, node_offset, "compatible", &plen);

	compatible = prop->data;

	prop = fdt_get_property(dtb, node_offset, "reg", &plen);
	if (!prop) {
		pr_warn("%s no register address specified\n", compatible);
		return NULL;
	}

	parent_node_offset = fdt_parent_offset(dtb, node_offset);
	if (parent_node_offset < 0) {
		pr_err("no parent node, incorrect reg property\n");
		return NULL;
	}

	address_cells = fdt_address_cells(dtb, parent_node_offset);

	if (address_cells < 0) {
		pr_warn("parent node doesn't specific address cells, assume address cells is 2\n");
		address_cells = 2;
	}

	size_cells = fdt_size_cells(dtb, parent_node_offset);

	if (size_cells < 0) {
		pr_warn("parent node doesn't specific size cells, assume size cells is 2\n");
		size_cells = 2;
	}

	if ((address_cells + size_cells) * 4 != plen && address_cells * 4 != plen) {
		pr_err("invalid reg property\n");
		return NULL;
	}

	if (address_cells == 2) {
		reg_base = fdt64_to_cpu(*(volatile uint64_t *)prop->data);
	} else if (address_cells == 1) {
		reg_base = fdt32_to_cpu(*(volatile uint32_t *)prop->data);
	} else {
		pr_err("address_cells %d is not supported\n", address_cells);
		return NULL;
	}

	if (address_cells * 4 == plen || size_cells == 0) {
		reg_size = 0;
	} else if (size_cells == 2) {
		reg_size = fdt64_to_cpu(*(volatile uint64_t *)
					((uint32_t *)prop->data + address_cells));
	} else if (size_cells == 1) {
		reg_size = fdt32_to_cpu(*(volatile uint32_t *)
					((uint32_t *)prop->data + address_cells));
	} else {
		pr_err("size_cells %d is not supported\n", size_cells);
		return NULL;
	}

	pdev = platform_device_alloc();

	if (!pdev) {
		pr_err("cannot allocate platform device\n");
		return NULL;
	}

	/* attach device nodes */
	pdev->of = dtb;
	pdev->of_node_offset  = node_offset;

	pdev->reg_base = reg_base;
	pdev->reg_size = reg_size;

	plen = sizeof(pdev->device.name);
	node_name = fdt_get_name(dtb, node_offset, &plen);
	if (node_name)
		strcpy(pdev->device.name, node_name);
	else
		snprintf(pdev->device.name, sizeof(pdev->device.name), "%s.%lx", compatible, pdev->reg_base);

	/* get alias */
	/* instead of searching aliases node in dtb, we use an alias property in each node.
	 * parsing alias is too expensive and not reasonable
	 */
	prop = fdt_get_property(dtb, node_offset, "alias", &plen);
	if (prop)
		strcpy(pdev->device.alias, prop->data);
	else
		pdev->device.alias[0] = 0;

	platform_device_add(pdev);

	return pdev;
}

static int platform_probe(void)
{
	int offset;
	int plen;
	struct platform_device *pdev;
	const struct fdt_property *prop;
	void *dtb = dtb_get_base();

	pr_debug("dtb base: %08lx, %lu Bytes\n", (unsigned long)dtb, dtb_get_size());

	offset = 0;

	while (true) {
		offset = fdt_next_node(dtb, offset, NULL);
		if (offset < 0)
			break;

		/* filter out nodes without reg property */
		prop = fdt_get_property(dtb, offset, "reg", &plen);
		if (!prop) {
			pr_debug("offset: %d, no reg property\n", offset);
			continue;
		}

		prop = fdt_get_property(dtb, offset, "compatible", &plen);
		if (!prop) {
			pr_debug("offset: %d, no compatible property\n", offset);
			continue;
		}

		pr_debug("offset: %d, compatible: %s, strlen: %d\n", offset, prop->data, plen);

		pdev = platform_device_create(dtb, offset);
		if (!pdev) {
			pr_err("create device failed for node offset %d\n", offset);
			continue;
		}

		/* find drivers for this device */
		match_table(pdev);
	}

	return 0;
}

subsys_probe(platform_probe);

const struct of_device_id *platform_get_match_id(struct platform_device *pdev)
{
	return pdev->match;
}

struct platform_device *platform_device_find_by_of(int node_offset)
{
	struct list_head *p;
	struct device *dev;
	struct platform_device *pdev;

	list_for_each(p, &device_list) {
		dev = container_of(p, struct device, list_head);
		pdev = container_of(dev, struct platform_device, device);
		if (pdev->of_node_offset == node_offset)
			return pdev;
	}

	return NULL;
}

int platform_get_irq(struct platform_device *pdev, int n)
{
	void *dtb = dtb_get_base();
	uint32_t irq_chip_phandle;
	int plen;
	const struct fdt_property *prop;
	int irq = -ENODEV, hwirq = -ENODEV;
	// int i;
	// int irq_vector[16];
	int nirq;
	int irq_chip_of_offset;
	struct platform_device *irq_chip_pdev;
	char irq_chip_name[DEVICE_NAME_MAX];
	struct irq_chip *irq_chip;
	int err;

	prop = fdt_get_property(dtb, pdev->of_node_offset, "interrupts-extended", &plen);
	/* check irq count overflow */
	nirq = plen / 8;

	if (n >= nirq) {
		pr_err("Requested index out of resource\n");
		return -ENODEV;
	}

	/* get phandle of a irq */
	irq_chip_phandle = fdt32_to_cpu(*(uint32_t *)(((uint64_t *)prop->data) + n));
	irq_chip_of_offset = fdt_node_offset_by_phandle(dtb, irq_chip_phandle);
	if (irq_chip_of_offset < 0)
		return -EINVAL;

	/* get hardware irq number */
	hwirq = fdt32_to_cpu(*((uint32_t *)(((uint64_t *)prop->data) + n) + 1));

	/* get platform device instance of this of node */
	irq_chip_pdev = platform_device_find_by_of(irq_chip_of_offset);
	if (!irq_chip_pdev) {
		pr_info("Cannot get platform device instance of this irq chip\n");
		return -ENODEV;
	}

	err = snprintf(irq_chip_name, sizeof(irq_chip_name), "irq:%s", irq_chip_pdev->device.name);
	if (err == sizeof(irq_chip_name))
		pr_warn("Truncated irq chip name\n");

	irq_chip = irq_get_chip_by_name(irq_chip_name);
	if (!irq_chip) {
		pr_err("Cannot find irq chip of name %s\n", irq_chip_name);
		return -ENODEV;
	}

	irq = irq_get(irq_chip, hwirq);

	return irq;
}
