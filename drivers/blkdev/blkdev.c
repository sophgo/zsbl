#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <lib/container_of.h>
#include <common/common.h>
#include <driver/blkdev.h>

#include <lib/cli.h>

static LIST_HEAD(device_list);
static int device_count;

int blkdev_list_devices(void)
{
	struct list_head *p;
	struct device *dev;
	struct blkdev *blkdev;
	int i;

	pr_info("%6s %40s %14s %14s %20s\n", "Index", "Device", "Block Size", "Total Size", "Alias");

	i = 0;
	list_for_each(p, &device_list) {
		dev = container_of(p, struct device, list_head);
		blkdev = container_of(dev, struct blkdev, device);
		pr_info("%6d %40s %14lu %14lu %20s\n", i, blkdev->device.name,
				blkdev->block_size, blkdev->total_size, blkdev->device.alias);
		++i;
	}

	return i;
}

static void command_show_devices(struct command *c, int argc, const char *argv[])
{
	struct list_head *p;
	struct device *dev;
	struct blkdev *blkdev;
	int i;

	console_printf(command_get_console(c),
		       "%6s %40s %14s %14s %20s\n",
		       "Index", "Device", "Block Size", "Total Size", "Alias");

	i = 0;
	list_for_each(p, &device_list) {
		dev = container_of(p, struct device, list_head);
		blkdev = container_of(dev, struct blkdev, device);
		console_printf(command_get_console(c),
			       "%6d %40s %14lu %14lu %20s\n", i, blkdev->device.name,
			       blkdev->block_size, blkdev->total_size, blkdev->device.alias);
		++i;
	}
}

cli_command(lsblk, command_show_devices);

struct blkdev *blkdev_alloc(void)
{
	struct blkdev *blkdev;

	blkdev = malloc(sizeof(struct blkdev));

	if (blkdev)
		memset(blkdev, 0, sizeof(struct blkdev));

	return blkdev;
}

void blkdev_free(struct blkdev *blkdev)
{
	free(blkdev);
}

void blkdev_add(struct blkdev *blkdev)
{
	list_add_tail(&blkdev->device.list_head, &device_list);
}

void blkdev_remove(struct blkdev *blkdev)
{
	list_del(&blkdev->device.list_head);
}

int blkdev_count(void)
{
	return list_count_nodes(&device_list);
}

struct blkdev *blkdev_first(void)
{
	return container_of(list_first_entry_or_null(&device_list, struct device, list_head), struct blkdev, device);
}

struct blkdev *blkdev_next(struct blkdev *current)
{
	if (list_is_last(&current->device.list_head, &device_list))
		return NULL;

	return container_of(list_next_entry(&current->device, list_head), struct blkdev, device);
}

struct blkdev *blkdev_find_by_name(const char *name)
{
	struct device *dev;

	dev = device_find_by_name(&device_list, name);
	if (dev)
		return container_of(dev, struct blkdev, device);

	return NULL;
}

void blkdev_set_user_data(struct blkdev *blkdev, void *data)
{
	blkdev->data = data;
}

void *blkdev_get_user_data(struct blkdev *blkdev)
{
	return blkdev->data;
}

long blkdev_read(struct blkdev *blkdev, unsigned long offset, unsigned long size, void *buf)
{
	return blkdev->ops->read(blkdev, offset, size, buf);
}

long blkdev_write(struct blkdev *blkdev, unsigned long offset, unsigned long size, void *buf)
{
	return blkdev->ops->write(blkdev, offset, size, buf);
}

int blkdev_register(struct blkdev *blkdev)
{
	/* check mandatory */
	if (blkdev->block_size == 0 || blkdev->total_size == 0) {
		pr_err("block_size or total_size is mandatory\n");
		return -EINVAL;
	}

	if (!blkdev->ops) {
		pr_err("no device operations\n");
		return -EINVAL;
	}

	if (!blkdev->ops->read || !blkdev->ops->write) {
		pr_err("no read//write operations\n");
		return -EINVAL;
	}

	blkdev->tmp_buf = malloc(blkdev->block_size);
	if (!blkdev->tmp_buf) {
		pr_err("allocate temp buffer failed\n");
		return -ENOMEM;
	}

	blkdev_add(blkdev);

	++device_count;

	return 0;
}

void blkdev_unregister(struct blkdev *blkdev)
{
	blkdev_remove(blkdev);

	free(blkdev->tmp_buf);
	blkdev->tmp_buf = NULL;
}
