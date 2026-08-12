#ifndef __MTDBLOCK_H__
#define __MTDBLOCK_H__

struct mtd;
struct blkdev;

/*
 * Wrap an MTD device as a block device (reads pass straight through to
 * mtd_read; writes are unsupported). Idempotent: if this MTD is already
 * wrapped, the existing blkdev is returned rather than creating a duplicate,
 * so a mounter and the automatic init pass can both call it in any order.
 * Returns NULL on allocation/registration failure.
 */
struct blkdev *mtdblock_create(struct mtd *mtd);

#endif
