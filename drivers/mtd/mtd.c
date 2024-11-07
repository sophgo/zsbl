#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <lib/container_of.h>
#include <framework/common.h>
#include <driver/mtd.h>

static LIST_HEAD(device_list);
static int device_count;

int mtd_list_devices(void)
{
	struct list_head *p;
	struct device *dev;
	struct mtd *mtd;
	int i;

	pr_info("%6s %30s %14s %14s\n", "Index", "Device", "Block Size", "Total Size");

	i = 0;
	list_for_each(p, &device_list) {
		dev = container_of(p, struct device, list_head);
		mtd = container_of(dev, struct mtd, device);
		pr_info("%6d %30s %14lu %14lu\n", i, mtd->device.name,
				mtd->block_size, mtd->total_size);
		++i;
	}

	return i;
}

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
	struct mtd *mtd;
	struct device *dev;
	struct list_head *p;

	list_for_each(p, &device_list) {
		dev = container_of(p, struct device, list_head);
		mtd = container_of(dev, struct mtd, device);
		if (strcmp(mtd->device.name, name) == 0) {
			return mtd;
		}
	}

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

int mtd_open(struct mtd *mtd)
{
	return mtd->ops->open(mtd);
}

long mtd_read(struct mtd *mtd, unsigned long offset, unsigned long size, void *buf)
{
	/* FIXME: support unaligned access */
	return mtd->ops->read(mtd, offset, size, buf);
}

long mtd_write(struct mtd *mtd, unsigned long offset, unsigned long size, void *buf)
{
	/* FIXME: support unaligned access */
	return mtd->ops->write(mtd, offset, size, buf);
}

int mtd_close(struct mtd *mtd)
{
	return mtd->ops->close(mtd);
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

	if (!mtd->ops->read || !mtd->ops->write) {
		pr_err("no read//write operations\n");
		return -EINVAL;
	}

	if (!mtd->ops->open || !mtd->ops->close)
		pr_warn("no open//close operations\n");

	mtd->tmp_buf = malloc(mtd->block_size);
	if (!mtd->tmp_buf) {
		pr_err("allocate temp buffer failed\n");
		return -ENOMEM;
	}

	/* append mtd0, mtd1 ... in front of original device name */
	sprintf(mtd->device.name, "mtd%d-%s", device_count, mtd->suffix);

	mtd_add(mtd);

	++device_count;

	return 0;
}

void mtd_unregister(struct mtd *mtd)
{
	mtd_remove(mtd);

	free(mtd->tmp_buf);
	mtd->tmp_buf = NULL;
}
