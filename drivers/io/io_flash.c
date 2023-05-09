#define DEBUG

#include <stdio.h>
#include <framework/common.h>

#include <driver/io/io.h>
#include <driver/spi-flash/mango_spif.h>

int flash_device_init(void);
int flash_device_open(char *name, uint8_t mode);
int flash_device_read(BOOT_FILE *boot_file, int id, uint64_t len);
int flash_device_get_file_info(BOOT_FILE *boot_file, int id, FILINFO *info);
int flash_device_close(void);
int flash_device_destroy(void);

static IO_DEV flash_io_device = {
	.type = IO_DEVICE_SPIFLASH,
	.func = {
		.init = flash_device_init,
		.open = flash_device_open,
		.read = flash_device_read,
		.close = flash_device_close,
		.get_file_info = flash_device_get_file_info,
		.destroy = flash_device_destroy,
	},
};

int flash_device_init(void)
{
	bm_spi_init(FLASH1_BASE);

	return 0;
}

int flash_device_open(char *name, uint8_t mode)
{

	return 0;
}

int flash_device_read(BOOT_FILE *boot_file, int id, uint64_t len)
{
	int ret;

	ret = mango_load_from_sf(boot_file[id].addr, boot_file[id].offset, len);
	if (ret) {
		pr_debug("failed to load %s image form spi flash\n", boot_file[id].name);
		return ret;
	}

	return 0;
}

int flash_device_get_file_info(BOOT_FILE *boot_file, int id, FILINFO *f_info)
{

	int ret;
	struct part_info info;

	ret = mango_get_part_info_by_name(DISK_PART_TABLE_ADDR, boot_file[id].name, &info);
	if (ret) {
		pr_debug("failed to get %s partition info\n", boot_file[id].name);
		return ret;
	}

	f_info->fsize = info.size;
	boot_file[id].offset = info.offset;

	pr_debug("load %s image from sf 0x%x to memory 0x%lx size %d\n",
		 boot_file[id].name, info.offset, info.lma, info.size);

	return 0;
}

int flash_device_close(void)
{

	return 0;
}

int flash_device_destroy(void)
{
	sp_flash_enable_dmmr();

	return 0;
}

int flash_io_device_register(void)
{
	if (io_device_register(IO_DEVICE_SPIFLASH, &flash_io_device)) {
		pr_debug("sd io register failed\n");
		return -1;
	}

	return 0;
}