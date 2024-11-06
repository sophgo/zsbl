#ifndef __DRIVER_H__
#define __DRIVER_H__

struct driver {
	struct list_head list_head;
	const char *name;
};

#endif
