/*
 * FatFs -> VFS adapter (thin, read-only).
 *
 * Wraps the existing FatFs library as a vfs_fs_type. Entries are not
 * pre-built: the on-demand lookup() resolves each name via f_stat and
 * materializes a tree node whose priv holds the full FatFs path
 * ("N:/dir/file"). File I/O maps to f_open/f_read/f_close.
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <common/vfs.h>
#include <common/module.h>
#include <driver/blkdev.h>
#include <lib/fatio_dev.h>
#include <ff.h>

#define FATFS_PATH_MAX	256

struct fatfs_mount {
	FATFS fatfs;
	struct fatio_dev *fdev;
	struct blkdev *blkdev;
	int volid;
};

/* ---- FatFs disk glue -> blkdev ---- */

static long fat_read(struct fatio_dev *fdev, unsigned long offset,
		     unsigned long size, void *buf)
{
	return blkdev_read((struct blkdev *)fdev->data, offset, size, buf);
}

static long fat_write(struct fatio_dev *fdev, unsigned long offset,
		      unsigned long size, void *buf)
{
	return blkdev_write((struct blkdev *)fdev->data, offset, size, buf);
}

static struct fatio_ops fat_ops = {
	.read = fat_read,
	.write = fat_write,
};

/* ---- file operations ---- */

static int fatfs_open(struct vfs_file *file)
{
	FIL *fil;

	fil = malloc(sizeof(*fil));
	if (!fil)
		return -ENOMEM;

	if (f_open(fil, (const char *)file->node->priv, FA_READ) != FR_OK) {
		free(fil);
		return -EIO;
	}

	file->private_data = fil;

	return 0;
}

static ssize_t fatfs_read(struct vfs_file *file, void *buf, size_t len)
{
	FIL *fil = file->private_data;
	UINT br = 0;

	if (f_lseek(fil, file->pos) != FR_OK)
		return -EIO;
	if (f_read(fil, buf, len, &br) != FR_OK)
		return -EIO;

	return (ssize_t)br;
}

static int fatfs_close(struct vfs_file *file)
{
	FIL *fil = file->private_data;

	if (fil) {
		f_close(fil);
		free(fil);
		file->private_data = NULL;
	}

	return 0;
}

static const struct vfs_file_ops fatfs_fops = {
	.open = fatfs_open,
	.read = fatfs_read,
	.close = fatfs_close,
};

/* ---- on-demand lookup ---- */

static struct vfs_node *fatfs_lookup(struct vfs_mount *mnt, struct vfs_node *dir,
				     const char *name)
{
	char path[FATFS_PATH_MAX];
	FILINFO info;

	(void)mnt;

	if (snprintf(path, sizeof(path), "%s/%s",
		     (const char *)dir->priv, name) >= (int)sizeof(path))
		return NULL;

	if (f_stat(path, &info) != FR_OK)
		return NULL;

	if (info.fattrib & AM_DIR)
		return vfs_mknod(dir, name, VFS_NODE_DIR, NULL, strdup(path));

	return vfs_mknod(dir, name, VFS_NODE_REG, &fatfs_fops, strdup(path));
}

static const struct vfs_super_ops fatfs_sops = {
	.lookup = fatfs_lookup,
};

/* ---- mount ---- */

static int fatfs_mount(struct vfs_mount *mnt, const char *source, void *data)
{
	struct fatfs_mount *fm;
	char prefix[8];
	int ret;

	(void)source;

	if (!data)
		return -EINVAL;

	fm = malloc(sizeof(*fm));
	if (!fm)
		return -ENOMEM;
	memset(fm, 0, sizeof(*fm));

	fm->blkdev = data;
	fm->fdev = fatio_alloc();
	if (!fm->fdev) {
		ret = -ENOMEM;
		goto err_free;
	}
	fm->fdev->ops = &fat_ops;
	fm->fdev->data = data;

	ret = fatio_register(fm->fdev);
	if (ret)
		goto err_free;
	fm->volid = fm->fdev->id;

	snprintf(prefix, sizeof(prefix), "%d:", fm->volid);
	if (f_mount(&fm->fatfs, prefix, 1) != FR_OK) {
		ret = -EIO;	/* not a FAT volume */
		goto err_free;
	}

	mnt->root = vfs_alloc_dir_node("fatfs");
	if (!mnt->root) {
		ret = -ENOMEM;
		goto err_unmount;
	}
	mnt->root->priv = strdup(prefix);
	mnt->sops = &fatfs_sops;
	mnt->priv = fm;

	return 0;

err_unmount:
	f_mount(NULL, prefix, 0);
err_free:
	free(fm->fdev);
	free(fm);
	return ret;
}

static struct vfs_fs_type fatfs_type = {
	.name = "fatfs",
	.flags = VFS_FS_REQUIRES_DEV,
	.mount = fatfs_mount,
};

static int fatfs_init(void)
{
	return vfs_register_filesystem(&fatfs_type);
}

subsys_init(fatfs_init);
