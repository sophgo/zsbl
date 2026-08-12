/*
 * Block-device automount policy.
 *
 * Enumerates every registered block device and mounts it into the VFS, letting
 * the VFS auto-detect the filesystem (vfs_mount with a NULL fstype tries each
 * registered filesystem until one accepts the device). Devices with no
 * recognized filesystem are skipped.
 *
 * Mount-point naming is policy, decided here:
 *   - a device with a non-empty alias (from its DTB "alias" property, inherited
 *     down the device chain) mounts at a semantic /mnt/<alias>, e.g. /mnt/sdcard
 *   - an anonymous device falls back to /blkN
 * The filesystem never sees the mount point; it only builds its own tree.
 *
 * This is deliberately separate from any filesystem implementation: device
 * discovery and mount placement are policy, not part of a fs.
 */

#include <stdio.h>

#include <common/vfs.h>
#include <common/module.h>
#include <driver/blkdev.h>

/* Mount one device at an alias-derived /mnt/<alias>. Returns 0 on success. */
static int mount_named(struct blkdev *blkdev, const char *alias)
{
	char path[8 + DEVICE_NAME_MAX];

	snprintf(path, sizeof(path), "/mnt/%s", alias);

	if (!vfs_mkdir_p(path))
		return -1;

	/* leave the directory in place on failure; it is a shared prefix */
	return vfs_mount(NULL, path, NULL, blkdev);
}

/* Mount one anonymous device at /blkN. Returns 0 on success. */
static int mount_anon(struct blkdev *blkdev, int index)
{
	char name[16];
	char path[24];

	snprintf(name, sizeof(name), "blk%d", index);
	if (!vfs_mkdir(vfs_root(), name))
		return -1;

	snprintf(path, sizeof(path), "/%s", name);
	if (vfs_mount(NULL, path, NULL, blkdev)) {
		vfs_unlink(vfs_root(), name);	/* no recognized fs */
		return -1;
	}

	return 0;
}

static int automount_setup(void)
{
	struct blkdev *blkdev;
	int anon = 0;

	for (blkdev = blkdev_first(); blkdev; blkdev = blkdev_next(blkdev)) {
		const char *alias = blkdev->device.alias;

		if (alias[0]) {
			mount_named(blkdev, alias);
		} else {
			if (mount_anon(blkdev, anon) == 0)
				anon++;
		}
	}

	return 0;
}

late_init(automount_setup);
