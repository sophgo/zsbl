#ifndef _ACCDEV_H_
#define _ACCDEV_H_

#include <driver/device.h>
#include <lib/list.h>

struct acc_dev;

struct acc_param {
	unsigned int size;
	const char *func;
	char *name;
	unsigned char *value;
};

struct acc_ops {
	int (*open_dev)(struct acc_dev *dev);
	void (*release_dev)(struct acc_dev *dev);
	int (*set_param)(struct acc_dev *dev, struct acc_param *param);
	int (*get_param)(struct acc_dev *dev, struct acc_param *param);
	int (*do_call)(struct acc_dev *dev, struct acc_param *param);
	int (*wait_done)(struct acc_dev *dev, struct acc_param *param);
	int (*set_flag)(struct acc_dev *dev, struct acc_param *param);
	int (*test_flag)(struct acc_dev *dev, struct acc_param *param);
};

struct acc_dev {
	char name[32];
	struct device device;
	struct acc_ops *ops;

	void *priv;
};

int acc_register(struct acc_dev *dev);
void acc_unregister(struct acc_dev *dev);
int acc_list_devices(void);
struct acc_dev *acc_first(void);
struct acc_dev *acc_next(struct acc_dev *cur);
void acc_free(struct acc_dev *dev);
struct acc_dev *acc_alloc(void);
struct acc_dev *acc_find_by_name(const char *name);

void acc_release_dev(struct acc_dev *dev);
int acc_open_dev(struct acc_dev *dev);
int acc_set_param(struct acc_dev *dev, struct acc_param *param);
int acc_get_param(struct acc_dev *dev, struct acc_param *param);
int acc_do_call(struct acc_dev *dev, struct acc_param *param);
//long acc_wait_done(struct acc_dev *dev, struct acc_param *param);
int acc_set_flag(struct acc_dev *dev, struct acc_param *param);
int acc_test_flag(struct acc_dev *dev, struct acc_param *param);

#endif