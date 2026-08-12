/*
 * sophgo partition table -> VFS filesystem ("sgpart").
 *
 * The sophgo flat partition table is a run of fixed-size part_info records,
 * each tagged with DPT_MAGIC, starting at a table offset that is NOT at a
 * fixed position on the medium (unlike a FAT boot sector). It therefore does
 * not self-describe and does not take part in auto-detect: a mounter supplies
 * {blkdev, offset} explicitly via struct sgpart_data.
 *
 * Each partition is exposed as one regular file directly under the mount root.
 * Entries are finite, so all are materialized at mount time (no on-demand
 * lookup). A file's read maps to blkdev_read within the partition window.
 *
 * The record layout mirrors the legacy sgmtd boot path (lib/sgmtd/sgmtd.c);
 * sgpart expresses the same table as a file tree rather than a bootdev.
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <common/vfs.h>
#include <common/module.h>
#include <driver/blkdev.h>
#include <fs/sgpart.h>

#define DPT_MAGIC	0x55aa55aaU

/* Guard against a corrupt table with no terminator running away. */
#define SGPART_MAX_ENTRIES	64

/* On-medium partition record. Must match the sophgo partition table format. */
struct part_info {
	uint32_t magic;
	char name[32];
	uint32_t offset;
	uint32_t size;
	char reserve[4];
	uint64_t lma;
};

/* Per-file private data: the window this partition occupies on the device. */
struct sgpart_file {
	struct blkdev *blkdev;
	unsigned long offset;
	unsigned long size;
};

/* ---- file operations ---- */

static ssize_t sgpart_read(struct vfs_file *file, void *buf, size_t len)
{
	struct sgpart_file *pf = file->node->priv;
	size_t pos = file->pos;

	if (pos >= pf->size)
		return 0;
	if (len > pf->size - pos)
		len = pf->size - pos;

	return blkdev_read(pf->blkdev, pf->offset + pos, len, buf);
}

static const struct vfs_file_ops sgpart_fops = {
	.read = sgpart_read,
};

/* ---- mount ---- */

/* Create one file node for a partition; frees priv on failure. */
static int sgpart_add(struct vfs_node *root, const struct part_info *pi,
		      struct blkdev *blkdev)
{
	struct sgpart_file *pf;
	char name[sizeof(pi->name) + 1];

	/* name[] may not be NUL-terminated on medium; bound the copy */
	memcpy(name, pi->name, sizeof(pi->name));
	name[sizeof(pi->name)] = '\0';
	if (name[0] == '\0')
		return -EINVAL;

	pf = malloc(sizeof(*pf));
	if (!pf)
		return -ENOMEM;

	pf->blkdev = blkdev;
	pf->offset = pi->offset;
	pf->size = pi->size;

	if (!vfs_mknod(root, name, VFS_NODE_REG, &sgpart_fops, pf)) {
		free(pf);		/* duplicate name or error */
		return -EEXIST;
	}

	return 0;
}

static int sgpart_mount(struct vfs_mount *mnt, const char *source, void *data)
{
	struct sgpart_data *sd = data;
	struct vfs_node *root;
	struct part_info pi;
	unsigned long addr;
	int count;

	(void)source;

	if (!sd || !sd->blkdev)
		return -EINVAL;

	/*
	 * Validate the first record before allocating the fs root: the VFS
	 * core frees only the mount object (not mnt->root) if mount() fails,
	 * so a rejecting path must not build a tree. This also implements
	 * "claim or reject" — a missing magic here means this is not an
	 * sgpart table.
	 */
	addr = sd->offset;
	if (blkdev_read(sd->blkdev, addr, sizeof(pi), &pi) != sizeof(pi))
		return -EIO;
	if (pi.magic != DPT_MAGIC)
		return -ENODEV;

	root = vfs_alloc_dir_node("sgpart");
	if (!root)
		return -ENOMEM;

	for (count = 0; count < SGPART_MAX_ENTRIES; count++) {
		if (pi.magic != DPT_MAGIC)
			break;			/* end of table */

		sgpart_add(root, &pi, sd->blkdev);	/* skip bad/dup entries */
		addr += sizeof(pi);

		if (blkdev_read(sd->blkdev, addr, sizeof(pi), &pi) != sizeof(pi))
			break;
	}

	mnt->root = root;

	return 0;
}

static struct vfs_fs_type sgpart_type = {
	.name = "sgpart",
	.flags = 0,			/* not self-describing: explicit mount only */
	.mount = sgpart_mount,
};

static int sgpart_init(void)
{
	return vfs_register_filesystem(&sgpart_type);
}

subsys_init(sgpart_init);
