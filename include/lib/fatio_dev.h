#ifndef __FATIO_DEV_H__
#define __FATIO_DEV_H__

struct fatio_dev;

struct fatio_ops {
	int (*status)(struct fatio_dev *dev);
	long (*read)(struct fatio_dev *fdev, unsigned long offset, unsigned long size, void *buf);
	long (*write)(struct fatio_dev *dev, unsigned long offset, unsigned long size, void *buf);
	int (*ioctl)(struct fatio_dev *dev, int cmd, void *buf);
	int (*init)(struct fatio_dev *dev);
};

struct fatio_dev {
	struct fatio_ops *ops;
	int id;
	void *data;
};

int fatio_register(struct fatio_dev *fdev);

#endif
