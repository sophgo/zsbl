#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <lib/list.h>

#define DEVICE_NAME_MAX	(64)

struct device {
	struct list_head list_head;
	char name[DEVICE_NAME_MAX];
};

#endif
