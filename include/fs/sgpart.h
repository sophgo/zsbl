#ifndef __FS_SGPART_H__
#define __FS_SGPART_H__

struct blkdev;

/*
 * Mount data for the sophgo partition-table filesystem ("sgpart").
 *
 * sgpart does not self-describe: the partition table is not at a fixed offset,
 * so a mounter must supply both the block device and the table's byte offset
 * (read from platform info, e.g. the DTB "sophgo-boot" property). Pass a
 * pointer to this struct as the 'data' argument of vfs_mount(..., "sgpart",
 * data).
 */
struct sgpart_data {
	struct blkdev *blkdev;
	unsigned long offset;		/* byte offset of the partition table */
};

#endif
