#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <lib/container_of.h>
#include <framework/common.h>
#include <driver/accdev.h>

static LIST_HEAD(device_list);
static int device_count;

struct acc_dev *acc_alloc(void)
{
	struct acc_dev *accdev;

	accdev = malloc(sizeof(struct acc_dev));

	if (accdev)
		memset(accdev, 0, sizeof(struct acc_dev));

	return accdev;
}

void acc_free(struct acc_dev *dev)
{
	free(dev);
}

int acc_list_devices(void)
{
	struct list_head *p;
	struct device *dev;
	struct acc_dev *acc_dev;
	int i;

	i = 0;
	list_for_each(p, &device_list) {
		dev = container_of(p, struct device, list_head);
		acc_dev = container_of(dev, struct acc_dev, device);
		pr_info("%s ", acc_dev->device.name);
	}
	i++;
	return i;
}

void acc_add(struct acc_dev *dev)
{
	list_add_tail(&dev->device.list_head, &device_list);
}

void acc_remove(struct acc_dev *dev)
{
	list_del(&dev->device.list_head);
}

int acc_register(struct acc_dev *dev)
{
	if (!dev->ops) {
		pr_info("no device ops\n");
		return -EINVAL;
	}
	if (!dev->priv) {
		pr_info("no acc device\n");
		return -EINVAL;
	}

	acc_add(dev);
	device_count++;
	return 0;
}

void acc_unregister(struct acc_dev *dev)
{
	acc_remove(dev);
	device_count--;
}

struct acc_dev *acc_first(void)
{
	return container_of(list_first_entry_or_null(&device_list, struct device, list_head), struct acc_dev, device);
}

struct acc_dev *acc_next(struct acc_dev *cur)
{
	if (list_is_last(&cur->device.list_head, &device_list)) {
		return NULL;
	}
	return container_of(list_next_entry(&cur->device, list_head), struct acc_dev, device);
}

struct acc_dev *acc_find_by_name(const char *name)
{
	struct device *dev;
	struct list_head *p;
	struct acc_dev *acc_dev;

	list_for_each(p, &device_list) {
		dev = container_of(p, struct device, list_head);
		acc_dev = container_of(dev, struct acc_dev, device);
		if (strcmp(acc_dev->name, name) == 0) {
			return acc_dev;
		}
	}
	return NULL;
}

void acc_release_dev(struct acc_dev *dev)
{
	dev->ops->release_dev(dev);	
}

int acc_open_dev(struct acc_dev *dev)
{
	return dev->ops->open_dev(dev);
}

int acc_set_param(struct acc_dev *dev, struct acc_param *param)
{
	return dev->ops->set_param(dev, param);
}

int acc_get_param(struct acc_dev *dev, struct acc_param *param)
{
	return dev->ops->get_param(dev, param);
}

int acc_do_call(struct acc_dev *dev, struct acc_param *param)
{
	return dev->ops->do_call(dev, param);
}

int acc_set_flag(struct acc_dev *dev, struct acc_param *param)
{
	return dev->ops->set_flag(dev, param);
}

int acc_test_flag(struct acc_dev *dev, struct acc_param *param)
{
	return dev->ops->test_flag(dev, param);
}