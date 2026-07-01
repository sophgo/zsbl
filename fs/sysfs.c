#include <common/vfs.h>
#include <common/module.h>
#include <driver/device.h>

#include <errno.h>

static int sysfs_mount(struct vfs_mount *mnt, const char *source, void *data)
{
	struct vfs_node *root;
	struct vfs_node *devices;
	struct device *dev;

	(void)source;
	(void)data;

	root = vfs_alloc_dir_node("sysfs");
	if (!root)
		return -ENOMEM;

	devices = vfs_mkdir(root, "devices");
	if (!devices)
		return -ENOMEM;

	/* one empty directory per device instance in the global registry */
	for (dev = device_global_first(); dev; dev = device_global_next(dev))
		vfs_mkdir(devices, dev->name);

	mnt->root = root;

	return 0;
}

static struct vfs_fs_type sysfs_type = {
	.name = "sysfs",
	.mount = sysfs_mount,
};

static int sysfs_setup(void)
{
	int ret;

	ret = vfs_register_filesystem(&sysfs_type);
	if (ret)
		return ret;

	if (!vfs_mkdir(vfs_root(), "sys"))
		return -ENOMEM;

	return vfs_mount(NULL, "/sys", "sysfs", NULL);
}

late_init(sysfs_setup);
