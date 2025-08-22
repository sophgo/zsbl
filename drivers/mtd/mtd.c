#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <lib/container_of.h>
#include <common/common.h>
#include <driver/mtd.h>

#include <lib/cli.h>

static LIST_HEAD(device_list);
static int device_count;

int mtd_list_devices(void)
{
	struct list_head *p;
	struct device *dev;
	struct mtd *mtd;
	int i;

	pr_info("%6s %40s %14s %14s %20s\n", "Index", "Device", "Block Size", "Total Size", "Alias");

	i = 0;
	list_for_each(p, &device_list) {
		dev = container_of(p, struct device, list_head);
		mtd = container_of(dev, struct mtd, device);
		pr_info("%6d %40s %14lu %14lu %20s\n", i, mtd->device.name,
				mtd->block_size, mtd->total_size, mtd->device.alias);
		++i;
	}

	return i;
}

static void command_show_devices(struct command *c, int argc, const char *argv[])
{
	struct list_head *p;
	struct device *dev;
	struct mtd *mtd;
	int i;

	console_printf(command_get_console(c),
		       "%6s %40s %14s %14s %20s\n",
		       "Index", "Device", "Block Size", "Total Size", "Alias");

	i = 0;
	list_for_each(p, &device_list) {
		dev = container_of(p, struct device, list_head);
		mtd = container_of(dev, struct mtd, device);
		console_printf(command_get_console(c),
			       "%6d %40s %14lu %14lu %20s\n", i, mtd->device.name,
			       mtd->block_size, mtd->total_size, mtd->device.alias);
		++i;
	}
}

cli_command(lsmtd, command_show_devices);

struct mtd *mtd_alloc(void)
{
	struct mtd *mtd;

	mtd = malloc(sizeof(struct mtd));

	if (mtd)
		memset(mtd, 0, sizeof(struct mtd));

	return mtd;
}

void mtd_free(struct mtd *mtd)
{
	free(mtd);
}

void mtd_add(struct mtd *mtd)
{
	list_add_tail(&mtd->device.list_head, &device_list);
}

void mtd_remove(struct mtd *mtd)
{
	list_del(&mtd->device.list_head);
}

int mtd_count(void)
{
	return list_count_nodes(&device_list);
}

struct mtd *mtd_first(void)
{
	return container_of(list_first_entry_or_null(&device_list, struct device, list_head), struct mtd, device);
}

struct mtd *mtd_next(struct mtd *current)
{
	if (list_is_last(&current->device.list_head, &device_list))
		return NULL;

	return container_of(list_next_entry(&current->device, list_head), struct mtd, device);
}

struct mtd *mtd_find_by_name(const char *name)
{
	struct device *dev;

	dev = device_find_by_name(&device_list, name);
	if (dev)
		return container_of(dev, struct mtd, device);

	return NULL;
}

void mtd_set_user_data(struct mtd *mtd, void *data)
{
	mtd->data = data;
}

void *mtd_get_user_data(struct mtd *mtd)
{
	return mtd->data;
}

long mtd_read(struct mtd *mtd, unsigned long offset, unsigned long size, void *buf)
{
	return mtd->ops->read(mtd, offset, size, buf);
}

int mtd_register(struct mtd *mtd)
{
	/* check mandatory */
	if (mtd->block_size == 0 || mtd->total_size == 0) {
		pr_err("block_size or total_size is mandatory\n");
		return -EINVAL;
	}

	if (!mtd->ops) {
		pr_err("no device operations\n");
		return -EINVAL;
	}

	if (!mtd->ops->read) {
		pr_err("no read operations\n");
		return -EINVAL;
	}

	mtd_add(mtd);

	++device_count;

	return 0;
}

void mtd_unregister(struct mtd *mtd)
{
	mtd_remove(mtd);
}
