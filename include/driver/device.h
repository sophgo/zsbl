#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <lib/list.h>

#define DEVICE_NAME_MAX	(64)

/*
 * DEVICE_STATUS_NEW: Newly created
 * DEVICE_STATUS_BOUND: Device driver is bound to this device
 * DEVICE_STATUS_OK: Finish initilaze and no error
 * DEVICE_STATUS_ERROR: Failed when trying to init this device
 */
enum device_status {
	DEVICE_STATUS_NEW,
	DEVICE_STATUS_BOUND,
	DEVICE_STATUS_OK,
	DEVICE_STATUS_ERROR,
};

struct device {
	struct list_head list_head;
	char name[DEVICE_NAME_MAX];
	char alias[DEVICE_NAME_MAX];
	enum device_status status;
};

struct device *device_find_by_name(struct list_head *device_list, const char *name);
void device_set_child_name(struct device *child, struct device *parent, const char *sys);

#endif
