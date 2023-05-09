#include <stdio.h>

#include <framework/common.h>
#include <driver/io/io.h>

static IO_DEV *io_device[IO_DEVICE_MAX] = {0};

int io_device_register(uint32_t dev_num, IO_DEV *dev)
{
	if (dev_num >= IO_DEVICE_MAX) {
		pr_err("%s err dev num\n", __FUNCTION__);
		return -1;
	}

	io_device[dev_num] = dev;

	return 0;
}

int io_device_unregister(uint32_t dev_num)
{
	if (dev_num >= IO_DEVICE_MAX) {
		pr_err("%s err dev num\n", __FUNCTION__);
		return -1;
	}

	io_device[dev_num] = NULL;

	return 0;
}

IO_DEV *set_current_io_device(uint32_t dev_num)
{
	if (dev_num >= IO_DEVICE_MAX) {
		pr_err("%s err dev num\n", __FUNCTION__);
		return NULL;
	}

	if (io_device[dev_num] != NULL) {
		return io_device[dev_num];
	} else {
		pr_err("dont register %d io device\n", dev_num);
		return NULL;
	}
}