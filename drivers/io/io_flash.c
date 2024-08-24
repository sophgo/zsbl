#include <stdio.h>
#include <framework/common.h>
#include <framework/module.h>
#include <driver/io/io.h>
#include <driver/spi-flash/sg_spif.h>
#include <memmap.h>

int flash_device_open(char *name, uint8_t mode);
int flash_device_read(BOOT_FILE *boot_file, int id, uint64_t len);
int flash_device_get_file_info(BOOT_FILE *boot_file, int id, FILINFO *info,
			      int dev_num, uint64_t table_addr);
int flash_device_close(void);
int flash_device_destroy(void);

static IO_DEV flash_io_device = {
	.type = IO_DEVICE_SPIFLASH,
	.func = {
		.open = flash_device_open,
		.read = flash_device_read,
		.close = flash_device_close,
		.get_file_info = flash_device_get_file_info,
		.destroy = flash_device_destroy,
	},
};

int flash_device_open(char *name, uint8_t mode)
{

	return 0;
}

int flash_device_read(BOOT_FILE *boot_file, int id, uint64_t len)
{
	int ret;

	ret = mango_load_from_sf(boot_file[id].addr, boot_file[id].offset, len);
	if (ret) {
		pr_err("failed to load image form spi flash\n");
		return ret;
	}

	return 0;
}

int flash_device_get_file_info(BOOT_FILE *boot_file, int id, FILINFO *f_info,
			      int dev_num, uint64_t table_addr)
{
	return device_get_file_info(boot_file, id, f_info, dev_num, table_addr);
}

int flash_device_close(void)
{

	return 0;
}

int flash_device_destroy(void)
{
	return 0;
}

int flash_io_device_register(void)
{
	if (io_device_register(IO_DEVICE_SPIFLASH, &flash_io_device)) {
		pr_err("flash io register failed\n");
		return -1;
	}

	return 0;
}
