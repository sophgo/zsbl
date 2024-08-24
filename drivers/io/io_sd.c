#include <stdio.h>
#include <framework/common.h>
#include <framework/module.h>
#include <plat.h>
#include <platform.h>
#include <memmap.h>
#include <lib/mmio.h>

#include <driver/io/io.h>
#include <driver/sd/sd.h>

static int sd_device_open(char *name, uint8_t mode);
static int sd_device_read(BOOT_FILE *boot_file, int id, uint64_t len);
static int sd_device_get_file_info(BOOT_FILE *boot_file, int id, FILINFO *info,
				  int dev_num, uint64_t table_addr);
static int sd_device_close(void);
static int sd_device_destroy(void);

static IO_DEV sd_io_device = {
	.type = IO_DEVICE_SD,
	.func = {
		.open = sd_device_open,
		.read = sd_device_read,
		.close = sd_device_close,
		.get_file_info = sd_device_get_file_info,
		.destroy = sd_device_destroy,
	},
};

int sd_device_init(void)
{
	FRESULT f_ret;
	if ((!(get_boot_sel() & SKIP_SD)) && bm_sd_card_detect()
	&& (!bm_sd_init(SD_USE_PIO))) {
		pr_debug("sd card init ok\n");

		f_ret = f_mount(&sd_io_device.SDC_FS, "0:", 1);
		if (f_ret == FR_OK) {
			pr_debug("sd card mount ok\n");
		} else {
			pr_err("sd card mount failed\n");
			return -1;
		}

		return 0;
	}

	return -1;
}

static int sd_device_destroy(void)
{
	FRESULT f_ret;

	f_ret = f_unmount("0:");
	if (f_ret == FR_OK) {
		pr_debug("unmount sd ok\n");

		__asm__ __volatile__ ("fence.i"::);
		sg2380_top_reset();

		return 0;
	}

	return -1;
}

static int sd_device_open(char *name, uint8_t mode)
{
	FRESULT f_ret;
	f_ret = f_open(&sd_io_device.fp, name, mode);
	if (f_ret == FR_OK) {
		pr_debug("open %s ok\n", name);
		return 0;
	} else {
		pr_err("open %s failed\n", name);
		return -1;
	}
}

static int sd_device_read(BOOT_FILE *boot_file, int id, uint64_t len)
{
	FRESULT ret;
	unsigned int rd_num;

	ret = f_read(&sd_io_device.fp, (void *)boot_file[id].addr, len, &rd_num);
	if (ret == FR_OK) {
		pr_debug("read %d bytes\n", rd_num);
		return 0;
	} else {
		pr_err("read failed %d\n", ret);
		return -1;
	}
}

static int sd_device_close(void)
{
	FRESULT f_ret;
	f_ret = f_close(&sd_io_device.fp);
	if (f_ret == FR_OK) {
		pr_debug("close file ok\n");
		return 0;
	} else {
		pr_err("close file failed\n");
		return -1;
	}
}

static int sd_device_get_file_info(BOOT_FILE *boot_file, int id, FILINFO *info,
				  int dev_num, uint64_t table_addr)
{
	FRESULT f_ret;

	f_ret = f_stat(boot_file[id].name[dev_num], info);
	if (f_ret == FR_OK) {
		pr_info("%s file size is %d\n", boot_file[id].name[dev_num], info->fsize);
		boot_file[id].len = info->fsize;
		return 0;
	} else {
		pr_err("get file info failed\n");
		return -1;
	}
}

int sd_io_device_register(void)
{
	if (io_device_register(IO_DEVICE_SD, &sd_io_device)) {
		pr_err("sd io register failed\n");
		return -1;
	}

	return 0;
}
