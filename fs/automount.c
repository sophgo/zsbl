/*
 * Block-device automount policy.
 *
 * Enumerates every registered block device and mounts it into the VFS at
 * /blkN, letting the VFS auto-detect the filesystem (vfs_mount with a NULL
 * fstype tries each registered filesystem until one accepts the device).
 * Devices with no recognized filesystem are skipped.
 *
 * This is deliberately separate from any filesystem implementation: device
 * discovery and mount placement are policy, not part of a fs.
 */

#include <stdio.h>

#include <common/vfs.h>
#include <common/module.h>
#include <driver/blkdev.h>

static int automount_setup(void)
{
	struct blkdev *blkdev;
	int i = 0;

	for (blkdev = blkdev_first(); blkdev; blkdev = blkdev_next(blkdev), i++) {
		char name[16];
		char path[24];

		snprintf(name, sizeof(name), "blk%d", i);
		if (!vfs_mkdir(vfs_root(), name))
			continue;

		snprintf(path, sizeof(path), "/%s", name);
		if (vfs_mount(NULL, path, NULL, blkdev))
			vfs_unlink(vfs_root(), name);	/* no recognized fs */
	}

	return 0;
}

late_init(automount_setup);
