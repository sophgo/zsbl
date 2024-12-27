#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <lib/list.h>

#define DEVICE_NAME_MAX	(64)

struct device {
	struct list_head list_head;
	char name[DEVICE_NAME_MAX];
	char alias[DEVICE_NAME_MAX];
};

struct device *device_find_by_name(struct list_head *device_list, const char *name);
void device_set_child_name(struct device *child, struct device *parent, const char *sys);

#endif
