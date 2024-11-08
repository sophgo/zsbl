/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <driver/sd/mmc.h>
#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#if 0
#define IO_BUFFER_SIZE	4096

static uint8_t __attribute__((aligned(512))) temp_buf[IO_BUFFER_SIZE];

static int MMC_disk_read(BYTE *buff,	DWORD sector,	UINT count)
{
	size_t ret;
	int remain = count * MMC_BLOCK_SIZE;
	int curr, curr_aligned, ptr;

	if (((uintptr_t)buff & MMC_BLOCK_MASK) == 0) {
		ret = mmc_read_blocks(sector, (uintptr_t)buff, count * MMC_BLOCK_SIZE);
		assert(ret == count * MMC_BLOCK_SIZE);
		return count;
	}

	ptr = 0;
	while (remain) {
		curr = remain <= IO_BUFFER_SIZE ? remain : IO_BUFFER_SIZE;
		if ((curr & MMC_BLOCK_MASK) == 0)
			curr_aligned = curr;
		else
			curr_aligned = (curr / MMC_BLOCK_SIZE + 1) * MMC_BLOCK_SIZE;
		ret = mmc_read_blocks(sector, (uintptr_t)temp_buf, curr_aligned);
		assert(ret == curr_aligned);
		memcpy(buff + ptr, temp_buf, curr);
		ptr += curr;
		remain -= curr;
		sector += (curr_aligned / MMC_BLOCK_SIZE);
	}
	return count;
}
#endif

#include <lib/fatio_dev.h>
#include <framework/common.h>
#include <errno.h>

struct fatio_dev *fatio_list[FF_VOLUMES];

int fatio_register(struct fatio_dev *fdev)
{
	int i;

	/* read and write ops are mandatory */
	if (!fdev->ops || !fdev->ops->read || !fdev->ops->write) {
		pr_err("ops: 0x%lx, read: 0x%lx, write: 0x%lx\n",
				(unsigned long)fdev->ops, (unsigned long)fdev->ops->read,
				(unsigned long)fdev->ops->write);
		pr_err("read/write operations are mandatory\n");
		return -EINVAL;
	}

	for (i = 0; ARRAY_SIZE(fatio_list); ++i) {
		if (fatio_list[i] == NULL)
			break;
	}

	if (i == ARRAY_SIZE(fatio_list)) {
		pr_err("no space left in fatio_list");
		return -ENOMEM;
	}

	fdev->id = i;

	fatio_list[i] = fdev;

	return 0;
}


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	struct fatio_ops *fops;
	struct fatio_dev *fdev;

	if (pdrv >= ARRAY_SIZE(fatio_list))
		return STA_NODISK;

	fdev = fatio_list[pdrv];

	if (!fdev)
		return STA_NODISK;

	fops = fdev->ops;

	if (!fops->status)
		return 0;

	return fops->status(fdev);
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	struct fatio_ops *fops;
	struct fatio_dev *fdev;

	if (pdrv >= ARRAY_SIZE(fatio_list))
		return STA_NODISK;

	fdev = fatio_list[pdrv];

	if (!fdev)
		return STA_NODISK;

	fops = fdev->ops;

	if (!fops->init)
		return 0;

	return fops->init(fdev);
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	struct fatio_ops *fops;
	struct fatio_dev *fdev;
	unsigned long offset, size;

	if (pdrv >= ARRAY_SIZE(fatio_list))
		return STA_NODISK;

	fdev = fatio_list[pdrv];

	if (!fdev)
		return STA_NODISK;

	fops = fdev->ops;

	offset = sector * FF_MAX_SS;
	size = count * FF_MAX_SS;

	if (fops->read(fdev, offset, size, buff) == size)
		return RES_OK;

	return RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	struct fatio_ops *fops;
	struct fatio_dev *fdev;
	unsigned long offset, size;

	if (pdrv >= ARRAY_SIZE(fatio_list))
		return STA_NODISK;

	fdev = fatio_list[pdrv];

	if (!fdev)
		return STA_NODISK;

	fops = fdev->ops;

	offset = sector * FF_MAX_SS;
	size = count * FF_MAX_SS;

	if (fops->write(fdev, offset, size, buff) == size)
		return RES_OK;

	return RES_ERROR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	struct fatio_ops *fops;
	struct fatio_dev *fdev;

	if (pdrv >= ARRAY_SIZE(fatio_list))
		return STA_NODISK;

	fdev = fatio_list[pdrv];

	if (!fdev)
		return STA_NODISK;

	fops = fdev->ops;

	if (!fops->ioctl)
		return RES_ERROR;

	if (fops->ioctl(fdev, cmd, buff))
		return RES_ERROR;

	return RES_OK;
}

