#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <lib/container_of.h>
#include <common/common.h>
#include <driver/serial.h>

#include <lib/cli.h>

static LIST_HEAD(device_list);
static int device_count;

int serial_list_devices(void)
{
	struct list_head *p;
	struct device *dev;
	struct serial *serial;
	int i;

	pr_info("%6s %30s %14s\n", "Index", "Device", "Baudrate");

	i = 0;
	list_for_each(p, &device_list) {
		dev = container_of(p, struct device, list_head);
		serial = container_of(dev, struct serial, device);
		pr_info("%6d %30s %14u\n", i, serial->device.name, serial->baudrate);
		++i;
	}

	return i;
}

static void command_show_devices(struct command *c, int argc, const char *argv[])
{
	struct list_head *p;
	struct device *dev;
	struct serial *serial;
	int i;

	console_printf(command_get_console(c),
		       "%6s %30s %14s\n",
		       "Index", "Device", "Baudrate");

	i = 0;
	list_for_each(p, &device_list) {
		dev = container_of(p, struct device, list_head);
		serial = container_of(dev, struct serial, device);
		console_printf(command_get_console(c),
			       "%6d %30s %14u\n",
			       i, serial->device.name, serial->baudrate);
		++i;
	}
}

cli_command(lss, command_show_devices);

struct serial *serial_alloc(void)
{
	struct serial *serial;

	serial = malloc(sizeof(struct serial));

	if (serial)
		memset(serial, 0, sizeof(struct serial));

	return serial;
}

void serial_free(struct serial *serial)
{
	free(serial);
}

void serial_add(struct serial *serial)
{
	list_add_tail(&serial->device.list_head, &device_list);
}

void serial_remove(struct serial *serial)
{
	list_del(&serial->device.list_head);
}

int serial_count(void)
{
	return list_count_nodes(&device_list);
}

struct serial *serial_first(void)
{
	return container_of(list_first_entry_or_null(&device_list, struct device, list_head), struct serial, device);
}

struct serial *serial_next(struct serial *current)
{
	if (list_is_last(&current->device.list_head, &device_list))
		return NULL;

	return container_of(list_next_entry(&current->device, list_head), struct serial, device);
}

struct serial *serial_find_by_name(const char *name)
{
	struct device *dev;

	dev = device_find_by_name(&device_list, name);
	if (dev)
		return container_of(dev, struct serial, device);

	return NULL;
}

void serial_set_user_data(struct serial *serial, void *data)
{
	serial->data = data;
}

void *serial_get_user_data(struct serial *serial)
{
	return serial->data;
}

int serial_input(struct serial *serial)
{
	return serial->ops->input(serial);
}

int serial_output(struct serial *serial, uint8_t c)
{
	return serial->ops->output(serial, c);
}

static struct serial *serial_stdio;

static int serial_stdio_input(void)
{
	return serial_input(serial_stdio);
}
static void serial_stdio_output(int ch)
{
	serial_output(serial_stdio, ch);
}

int serial_register(struct serial *serial)
{
	/* check mandatory */
	if (!serial->ops) {
		pr_err("no device operations\n");
		return -EINVAL;
	}

	if (!serial->ops->input || !serial->ops->output) {
		pr_err("no input//output operations\n");
		return -EINVAL;
	}

	/* append s0, s1 ... in front of original device name */
	sprintf(serial->device.name, "s%d", device_count);

	pr_info("%s register serial device\n", serial->device.name);

	serial_add(serial);

	if (serial->stdio) {
		serial_stdio = serial;
		register_stdio(serial_stdio_input, serial_stdio_output);
	}

	++device_count;

	return 0;
}

void serial_unregister(struct serial *serial)
{
	serial_remove(serial);
}
