#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <common/common.h>
#include <driver/device.h>
#include <lib/container_of.h>
#include <lib/cli.h>

static LIST_HEAD(device_global_list);

struct device *device_find_by_name(struct list_head *device_list, const char *name)
{
	struct device *dev;
	struct list_head *p;

	list_for_each(p, device_list) {
		dev = container_of(p, struct device, list_head);
		if (strcmp(dev->name, name) == 0)
			return dev;
		if (strcmp(dev->alias, name) == 0)
			return dev;
	}

	return NULL;
}

void device_set_child_name(struct device *child, struct device *parent, const char *sys)
{
	int err;
	err = snprintf(child->name, sizeof(child->name), "%s:%s", sys, parent->name);
	if (err == sizeof(child->name))
		pr_warn("Truncated device names\n");

	strcpy(child->alias, parent->alias);
}

int device_register(struct device *dev)
{
	if (!dev)
		return -EINVAL;

	if (device_find_global(dev->name))
		pr_warn("device name %s already registered\n", dev->name);

	list_add_tail(&dev->global_node, &device_global_list);

	return 0;
}

void device_unregister(struct device *dev)
{
	if (dev)
		list_del(&dev->global_node);
}

struct device *device_global_first(void)
{
	return list_first_entry_or_null(&device_global_list,
					struct device, global_node);
}

struct device *device_global_next(struct device *cur)
{
	if (list_is_last(&cur->global_node, &device_global_list))
		return NULL;

	return list_next_entry(cur, global_node);
}

struct device *device_find_global(const char *name)
{
	struct device *dev;
	struct list_head *p;

	list_for_each(p, &device_global_list) {
		dev = container_of(p, struct device, global_node);
		if (strcmp(dev->name, name) == 0)
			return dev;
		if (strcmp(dev->alias, name) == 0)
			return dev;
	}

	return NULL;
}

static void command_lsdev(struct command *c, int argc, const char *argv[])
{
	struct device *dev;
	int i = 0;

	console_printf(command_get_console(c), "%6s %40s %20s %8s\n",
		       "Index", "Device", "Alias", "Status");

	for (dev = device_global_first(); dev; dev = device_global_next(dev)) {
		console_printf(command_get_console(c), "%6d %40s %20s %8d\n",
			       i, dev->name, dev->alias, dev->status);
		++i;
	}
}

cli_command(lsdev, command_lsdev);
