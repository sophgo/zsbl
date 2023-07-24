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
int sbi_printf(const char *format, ...);
/* Definitions of physical drive number for each drive */
// #define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		0	/* Example: Map MMC/SD card to physical drive 1 */
// #define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

#define IO_BUFFER_SIZE	4096

static uint8_t __attribute__((aligned(512))) temp_buf[4096];

static int MMC_disk_status(void)
{
	return 0;
}

static int MMC_disk_initialize(void)
{
	assert((FF_MIN_SS == FF_MAX_SS) &&
		(FF_MIN_SS == MMC_BLOCK_SIZE));
	return 0;
}

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




/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_MMC :
		result = MMC_disk_status();

		// translate the reslut code here
		stat = result;

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_MMC :
		result = MMC_disk_initialize();

		// translate the reslut code here
		stat = result;

		return stat;
	}
	return STA_NOINIT;
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
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_MMC :
		// translate the arguments here

		result = MMC_disk_read(buff, sector, count);

		// translate the reslut code here
		if (result == count)
			res = RES_OK;
		else
			res = RES_ERROR;

		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0
static int MMC_disk_write(const BYTE *buff,	DWORD sector,	UINT count)
{
	return RES_WRPRT;
}


DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_MMC :
		// translate the arguments here

		result = MMC_disk_write(buff, sector, count);

		// translate the reslut code here

		if (result == count)
			res = RES_OK;
		else
			res = RES_ERROR;

		return res;
	}

	return RES_PARERR;
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
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_MMC :

		// Process of the command for the MMC/SD card
		res = result = 0;

		return res;
	}

	return RES_PARERR;
}

