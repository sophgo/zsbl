#ifndef __BOOTDEV_H__
#define __BOOTDEV_H__

#include <driver/device.h>
#include <lib/list.h>

struct bootdev;

struct bootdev_ops {
	long (*load)(struct bootdev *bootdev, const char *file, void *buf);
	long (*get_file_size)(struct bootdev *bootdev, const char *file);
};

/* priority: the smaller one load first */
struct bootdev {
	struct device device;
	struct bootdev_ops *ops;

	unsigned long size;

	int priority;

	void *data;
};

struct bootdev *bootdev_alloc(void);
int bootdev_register(struct bootdev *bootdev);
int bootdev_list_devices(void);
void bootdev_free(struct bootdev *bootdev);
void bootdev_add(struct bootdev *bootdev);
long bootdev_get_file_size(struct bootdev *bootdev, const char *file);
long bootdev_load(struct bootdev *bootdev, const char *file, void *buf);
int bdm_set_priority(const char *devname, int priority);
long bdm_load(const char *file, void *buf);
long bdm_get_file_size(const char *file);

#endif
