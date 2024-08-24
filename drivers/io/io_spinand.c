#include <stdio.h>
#include <framework/common.h>
#include <framework/module.h>
#include <plat.h>
#include <platform.h>
#include <memmap.h>
#include <lib/mmio.h>
#include <lib/libc/errno.h>

#include <driver/io/io.h>
#include <driver/spi-flash/sg_spif.h>
#include <driver/spinand/spinand.h>

static int spinand_device_open(char *name, uint8_t mode);
static int spinand_device_read(BOOT_FILE *boot_file, int id, uint64_t len);
static int spinand_device_get_file_info(BOOT_FILE *boot_file, int id, FILINFO *info,
				  int dev_num, uint64_t table_addr);
static int spinand_device_close(void);
static int spinand_device_destroy(void);

static IO_DEV spi_nand_io_device = {
	.type = IO_DEVICE_SPINAND,
	.func = {
		.open = spinand_device_open,
		.read = spinand_device_read,
		.close = spinand_device_close,
		.get_file_info = spinand_device_get_file_info,
		.destroy = spinand_device_destroy,
	},
};

static int spinand_device_destroy(void)
{
	return 0;
}

static int spinand_device_open(char *name, uint8_t mode)
{
	return 0;
}

static int spinand_device_read(BOOT_FILE *boot_file, int id, uint64_t len)
{
	uint32_t ret;

	ret = load_from_spinand(boot_file[id].offset, (uint8_t *)boot_file[id].addr, len); 

	if (ret)
		pr_err("failed to load image form spi nand\n");

	return ret;
}

static int spinand_device_close(void)
{
	return 0;
}

static int spinand_device_get_file_info(BOOT_FILE *boot_file, int id, FILINFO *f_info,
				  int dev_num, uint64_t table_addr)
{
	return device_get_file_info(boot_file, id, f_info, dev_num, table_addr); 
}

int spinand_io_device_register(void)
{
	if (io_device_register(IO_DEVICE_SPINAND, &spi_nand_io_device)) {
		pr_err("spinand io register failed\n");
		return -1;
	}

	return 0;
}
