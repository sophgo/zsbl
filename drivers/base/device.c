#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <common/common.h>
#include <driver/device.h>
#include <lib/container_of.h>

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
