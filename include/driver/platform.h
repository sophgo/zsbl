#ifndef __PLATFORM_DEVICE_H__
#define __PLATFORM_DEVICE_H__

#include <driver/device.h>
#include <driver/driver.h>
#include <lib/list.h>

#define PLATFORM_COMPATIBLE_MAX	64

struct of_device_id {
	char compatible[PLATFORM_COMPATIBLE_MAX];
	const void *data;
};

struct platform_driver;

struct platform_device {
	struct device device;
	const struct of_device_id *match;

	struct platform_driver *pdrv;

	void *of;
	int of_node_offset;

	unsigned long reg_base;
	unsigned long reg_size;

	void *data;
};

struct platform_driver {
	int (*probe)(struct platform_device *pdev);

	struct driver driver;

	const struct of_device_id *of_match_table;
};

int platform_driver_register(struct platform_driver *pdrv);
int platform_list_drivers(void);
int platform_list_devices(void);
const struct of_device_id *platform_get_match_id(struct platform_device *pdev);
int platform_get_irq(struct platform_device *pdev, int n);

void platform_device_set_user_data(struct platform_device *pdev, void *data);
void *platform_device_get_user_data(struct platform_device *pdev);

#endif
