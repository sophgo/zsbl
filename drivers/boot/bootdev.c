#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <cache.h>

#include <driver/bootdev.h>
#include <common/common.h>
#include <lib/container_of.h>

#include <lib/cli.h>

static LIST_HEAD(device_list);

int bootdev_list_devices(void)
{
	struct list_head *p;
	struct device *dev;
	struct bootdev *bootdev;
	int i;

	pr_info("%6s %40s %14s %8s %20s\n", "Index", "Device", "Size", "Priority", "Alias");

	i = 0;
	list_for_each(p, &device_list) {
		dev = container_of(p, struct device, list_head);
		bootdev = container_of(dev, struct bootdev, device);
		pr_info("%6d %40s %14lu %8d %20s\n",
				i, bootdev->device.name, bootdev->size,
				bootdev->priority, bootdev->device.alias);
		++i;
	}

	return i;
}

static void command_show_devices(struct command *c, int argc, const char *argv[])
{
	struct list_head *p;
	struct device *dev;
	struct bootdev *bootdev;
	int i;

	console_printf(command_get_console(c),
		       "%6s %40s %14s %8s %20s\n",
		       "Index", "Device", "Size", "Priority", "Alias");

	i = 0;
	list_for_each(p, &device_list) {
		dev = container_of(p, struct device, list_head);
		bootdev = container_of(dev, struct bootdev, device);
		console_printf(command_get_console(c),
			       "%6d %40s %14lu %8d %20s\n",
			       i, bootdev->device.name, bootdev->size,
			       bootdev->priority, bootdev->device.alias);
		++i;
	}
}

cli_command(lsbdev, command_show_devices);

struct bootdev *bootdev_alloc(void)
{
	struct bootdev *bootdev;

	bootdev = malloc(sizeof(struct bootdev));

	if (bootdev)
		memset(bootdev, 0, sizeof(struct bootdev));

	return bootdev;
}

void bootdev_free(struct bootdev *bootdev)
{
	free(bootdev);
}

void bootdev_add(struct bootdev *bootdev)
{
	list_add_tail(&bootdev->device.list_head, &device_list);
}

void bootdev_remove(struct bootdev *bootdev)
{
	list_del(&bootdev->device.list_head);
}

int bootdev_count(void)
{
	return list_count_nodes(&device_list);
}

struct bootdev *bootdev_find_by_name(const char *name)
{
	struct device *dev;

	dev = device_find_by_name(&device_list, name);
	if (dev)
		return container_of(dev, struct bootdev, device);

	return NULL;
}

void bootdev_set_user_data(struct bootdev *bootdev, void *data)
{
	bootdev->data = data;
}

void *bootdev_get_user_data(struct bootdev *bootdev)
{
	return bootdev->data;
}

long bootdev_load(struct bootdev *bootdev, const char *file, void *buf)
{
	long err;

	err = bootdev->ops->load(bootdev, file, buf);

	if (err <= 0)
		return err;

	/* zsbl may load executables for none coherent coprocessors, so flush data cache any way */
	flush_dcache_range((unsigned long)buf, ((unsigned long)buf) + err);

	return err;
}

long bootdev_get_file_size(struct bootdev *bootdev, const char *file)
{
	return bootdev->ops->get_file_size(bootdev, file);
}

int bootdev_register(struct bootdev *bootdev)
{
	/* check mandatory */
	if (bootdev->device.name[0] == 0) {
		pr_err("device need a name\n");
		return -EINVAL;
	}

	if (bootdev->size == 0) {
		pr_err("device size is mandatory\n");
		return -EINVAL;
	}

	if (!bootdev->ops) {
		pr_err("no device operations\n");
		return -EINVAL;
	}

	if (!bootdev->ops->get_file_size || !bootdev->ops->load) {
		pr_err("no get_file_size and load operations\n");
		return -EINVAL;
	}

	bootdev_add(bootdev);

	return 0;
}

void bootdev_unregister(struct bootdev *bootdev)
{
	bootdev_remove(bootdev);
}

struct bootdev *bootdev_first(void)
{
	return container_of(list_first_entry_or_null(&device_list, struct device, list_head), struct bootdev, device);
}

struct bootdev *bootdev_next(struct bootdev *current)
{
	if (list_is_last(&current->device.list_head, &device_list))
		return NULL;

	return container_of(list_next_entry(&current->device, list_head), struct bootdev, device);
}

/* boot device manager */

/* priority: the smaller one load first */
static void sort_device(void)
{
	struct bootdev *cur, *next, *smallest;

	for (cur = bootdev_first(); cur; cur = bootdev_next(cur)) {
		smallest = cur;
		for (next = bootdev_next(cur); next; next = bootdev_next(next)) {
			if (next) {
				if (next->priority < smallest->priority) {
					smallest = next;
				}
			}
		}
		if (smallest != cur) {
			list_swap(&cur->device.list_head, &smallest->device.list_head);
			cur = smallest;
		}
	}
}

int bdm_set_priority(const char *devname, int priority)
{
	struct bootdev *bootdev;

	bootdev = bootdev_find_by_name(devname);
	if(!bootdev) {
		pr_err("boot device %s not found\n", devname);
		return -ENODEV;
	}

	bootdev->priority = priority;
	sort_device();

	return 0;
}

long bdm_load(const char *file, void *buf)
{
	struct bootdev *bootdev;
	long err;

	pr_info("Loading %-29s ", file);

	for (bootdev = bootdev_first(); bootdev; bootdev = bootdev_next(bootdev)) {

		err = bootdev_load(bootdev, file, buf);
		if (err > 0) {
			pr_info("[%s] (%ld bytes)\n",
					bootdev->device.alias[0] ? bootdev->device.alias : bootdev->device.name, err);
			return err;
		}
		pr_debug("load %s from %s failed, try next boot device\n", file, bootdev->device.name);
	}

	pr_err("\nCan not load %s from any boot devices\n", file);

	pr_err("Available boot devices\n");
	bootdev_list_devices();

	return -EIO;
}

/* iterate every boot deivces, making sure buffers allocated by user can be used to any devices */
long bdm_get_file_size(const char *file)
{
	struct bootdev *bootdev;
	long max_size = -ENOENT;
	long size;

	for (bootdev = bootdev_first(); bootdev; bootdev = bootdev_next(bootdev)) {

		size = bootdev_get_file_size(bootdev, file);

		/* if get_file_size failed, an nagtive value will return */
		if (size > max_size)
			max_size = size;
	}

	return max_size;
}

