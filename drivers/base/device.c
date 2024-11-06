#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <framework/common.h>
#include <driver/device.h>
#include <lib/container_of.h>

static LIST_HEAD(device_list);

int device_list_devices(void)
{
	struct list_head *p;
	struct device *dev;
	int i;

	i = 0;
	list_for_each(p, &device_list) {
		dev = container_of(p, struct device, list_head);
		pr_info("%d: %s\n", i, dev->name);
		++i;
	}

	return i;
}

struct device *device_alloc(void)
{
	struct device *dev;

	dev = malloc(sizeof(struct device));

	if (dev)
		memset(dev, 0, sizeof(struct device));

	return dev;
}

void device_free(struct device *dev)
{
	free(dev);
}

void device_add(struct device *dev)
{
	list_add_tail(&dev->list_head, &device_list);
}

void device_remove(struct device *dev)
{
	list_del(&dev->list_head);
}

int device_count(void)
{
	return list_count_nodes(&device_list);
}

struct device *device_find_by_name(const char *name)
{
	struct device *dev;
	struct list_head *p;

	list_for_each(p, &device_list) {
		dev = container_of(p, struct device, list_head);
		if (strcmp(dev->name, name) == 0) {
			return dev;
		}
	}

	return NULL;
}

