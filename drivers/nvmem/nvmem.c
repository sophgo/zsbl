#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <lib/container_of.h>
#include <common/common.h>
#include <driver/nvmem.h>

#include <lib/cli.h>

static LIST_HEAD(device_list);
static int device_count;

int nvmem_list_devices(void)
{
	struct list_head *p;
	struct device *dev;
	struct nvmem *nvmem;
	int i;

	pr_info("%6s %40s %14s %14s %20s\n", "Index", "Device", "Cell Bits", "Cell Count", "Alias");

	i = 0;
	list_for_each(p, &device_list) {
		dev = container_of(p, struct device, list_head);
		nvmem = container_of(dev, struct nvmem, device);
		pr_info("%6d %40s %14u %14u %20s\n", i, nvmem->device.name,
				nvmem->cell_bits, nvmem->cell_count,
				nvmem->device.alias);
		++i;
	}

	return i;
}

static void command_show_devices(struct command *c, int argc, const char *argv[])
{
	struct list_head *p;
	struct device *dev;
	struct nvmem *nvmem;
	int i;

	console_printf(command_get_console(c),
		       "%6s %40s %14s %14s %20s\n",
		       "Index", "Device", "Cell Bits", "Cell Count", "Alias");

	i = 0;
	list_for_each(p, &device_list) {
		dev = container_of(p, struct device, list_head);
		nvmem = container_of(dev, struct nvmem, device);
		console_printf(command_get_console(c),
			       "%6d %40s %14u %14u %20s\n", i, nvmem->device.name,
			       nvmem->cell_bits, nvmem->cell_count,
			       nvmem->device.alias);
		++i;
	}
}

cli_command(lsnvmem, command_show_devices);

struct nvmem *nvmem_alloc(void)
{
	struct nvmem *nvmem;

	nvmem = malloc(sizeof(struct nvmem));

	if (nvmem)
		memset(nvmem, 0, sizeof(struct nvmem));

	return nvmem;
}

void nvmem_free(struct nvmem *nvmem)
{
	free(nvmem);
}

void nvmem_add(struct nvmem *nvmem)
{
	list_add_tail(&nvmem->device.list_head, &device_list);
}

void nvmem_remove(struct nvmem *nvmem)
{
	list_del(&nvmem->device.list_head);
}

int nvmem_count(void)
{
	return list_count_nodes(&device_list);
}

struct nvmem *nvmem_first(void)
{
	return container_of(list_first_entry_or_null(&device_list, struct device, list_head), struct nvmem, device);
}

struct nvmem *nvmem_next(struct nvmem *current)
{
	if (list_is_last(&current->device.list_head, &device_list))
		return NULL;

	return container_of(list_next_entry(&current->device, list_head), struct nvmem, device);
}

struct nvmem *nvmem_find_by_name(const char *name)
{
	struct device *dev;

	dev = device_find_by_name(&device_list, name);
	if (dev)
		return container_of(dev, struct nvmem, device);

	return NULL;
}

void nvmem_set_user_data(struct nvmem *nvmem, void *data)
{
	nvmem->data = data;
}

void *nvmem_get_user_data(struct nvmem *nvmem)
{
	return nvmem->data;
}

long nvmem_read(struct nvmem *nvmem, unsigned long offset, unsigned long size, void *buf)
{
	return nvmem->ops->read(nvmem, offset, size, buf);
}

long nvmem_write(struct nvmem *nvmem, unsigned long offset, unsigned long size, void *buf)
{
	return nvmem->ops->write(nvmem, offset, size, buf);
}

int nvmem_register(struct nvmem *nvmem)
{
	/* check mandatory */
	if (nvmem->cell_bits == 0 || nvmem->cell_count == 0) {
		pr_err("cell_bits or cell_count is mandatory\n");
		return -EINVAL;
	}

	if (!nvmem->ops) {
		pr_err("no device operations\n");
		return -EINVAL;
	}

	if (!nvmem->ops->read || !nvmem->ops->write) {
		pr_err("no read//write operations\n");
		return -EINVAL;
	}

	nvmem_add(nvmem);

	++device_count;

	return 0;
}

void nvmem_unregister(struct nvmem *nvmem)
{
	nvmem_remove(nvmem);
}
