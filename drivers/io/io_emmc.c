#include <stdio.h>
#include <framework/common.h>
#include <framework/module.h>
#include <plat.h>
#include <platform.h>
#include <memmap.h>
#include <lib/mmio.h>
#include <lib/libc/errno.h>

#include <driver/io/io.h>
#include <driver/emmc/sg_emmc.h>

static int emmc_device_open(char *name, uint8_t mode);
static int emmc_device_read(BOOT_FILE *boot_file, int id, uint64_t len);
static int emmc_device_get_file_info(BOOT_FILE *boot_file, int id, FILINFO *info,
				  int dev_num, uint64_t table_addr);
static int emmc_device_close(void);
static int emmc_device_destroy(void);

static IO_DEV emmc_io_device = {
	.type = IO_DEVICE_EMMC,
	.func = {
		.open = emmc_device_open,
		.read = emmc_device_read,
		.close = emmc_device_close,
		.get_file_info = emmc_device_get_file_info,
		.destroy = emmc_device_destroy,
	},
};

static int emmc_device_destroy(void)
{
	return 0;
}

static int emmc_device_open(char *name, uint8_t mode)
{
	return 0;
}

static int emmc_device_read(BOOT_FILE *boot_file, int id, uint64_t len)
{
	int ret, mod, div;

	mod = len % EMMC_BLOCK_SIZE;
	div = len / EMMC_BLOCK_SIZE;

	if (mod != 0)
		len = (div + 1) * EMMC_BLOCK_SIZE;

	ret = bm_emmc_read_data(boot_file[id].offset, boot_file[id].addr, len);

	return ret;

}

static int emmc_device_close(void)
{
	return 0;
}

static int emmc_device_get_file_info(BOOT_FILE *boot_file, int id, FILINFO *f_info,
				  int dev_num, uint64_t table_addr)
{
	return device_get_file_info(boot_file, id, f_info, dev_num, table_addr);
}

int emmc_io_device_register(void)
{
	if (io_device_register(IO_DEVICE_EMMC, &emmc_io_device)) {
		pr_err("emmc io register failed\n");
		return -1;
	}

	return 0;
}
