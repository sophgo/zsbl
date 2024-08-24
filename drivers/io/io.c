#include <stdio.h>

#include <framework/common.h>
#include <driver/io/io.h>
#include <driver/emmc/sg_emmc.h>
#include <driver/spi-flash/sg_spif.h>
#include <string.h>

enum {
	DPT_MAGIC	= 0x55aa55aa,
};

const char* IO_DEVICE_NAMES[] = {
    "SD",
    "SPIFLASH",
	"SPINAND",
    "EMMC",
    "MAX",
};

static IO_DEV *io_device[IO_DEVICE_MAX] = {0};

int io_device_register(uint32_t dev_num, IO_DEV *dev)
{
	if (dev_num >= IO_DEVICE_MAX) {
		pr_err("%s err dev num\n", __FUNCTION__);
		return -1;
	}

	io_device[dev_num] = dev;

	return 0;
}

int io_device_unregister(uint32_t dev_num)
{
	if (dev_num >= IO_DEVICE_MAX) {
		pr_err("%s err dev num\n", __FUNCTION__);
		return -1;
	}

	io_device[dev_num] = NULL;

	return 0;
}

IO_DEV *set_current_io_device(uint32_t dev_num)
{
	if (dev_num >= IO_DEVICE_MAX) {
		pr_err("%s err dev num\n", __FUNCTION__);
		return NULL;
	}

	if (io_device[dev_num] != NULL) {
		return io_device[dev_num];
	} else {
		pr_err("dont register %d io device\n", dev_num);
		return NULL;
	}
}
const char* get_io_device_name(IO_DEVICE device)
{
	if (device >= IO_DEVICE_SD && device < IO_DEVICE_MAX) {
        return IO_DEVICE_NAMES[device];
    } else {
        return "Unknown device\n";
    }
}
int get_current_io_device(IO_DEV **io_dev, uint32_t dev_num)
{
	if (io_device[dev_num]) {
		*io_dev = io_device[dev_num];
		return 0;
	} else {
		pr_err("register io device failed\n");
		return -1;
	}
}


static int mango_get_part_info(uint32_t addr, struct part_info *info, int dev_num)
{
	int ret;

	if (dev_num == IO_DEVICE_EMMC)
		ret = bm_emmc_read_data(addr, (uint64_t)info, sizeof(struct part_info));
	else
		ret = plat_spif_read((uint64_t)info, addr, sizeof(struct part_info), dev_num);

	if (ret)
		return ret;

	if (info->magic != DPT_MAGIC) {
		pr_err("bad partition magic\n");
		return -1;
	}

	return ret;
}

static int mango_get_part_info_by_name(uint32_t addr, const char *name, struct part_info *info,
			       int dev_num)
{
	int ret;

	do {
		ret = mango_get_part_info(addr, info, dev_num);

		if (dev_num != IO_DEVICE_SPINAND)
			addr += sizeof(struct part_info);
		else
			addr += 4 * 1024;

	} while (!ret && strcmp(info->name, name));

	return ret;
}

int device_get_file_info(BOOT_FILE *boot_file, int id, FILINFO *f_info,
				  int dev_num, uint64_t table_addr)
{
	int ret;
	struct part_info p_info;

	ret = mango_get_part_info_by_name(table_addr, boot_file[id].name[dev_num],
					 &p_info, dev_num);
	if (ret) {
		pr_err("failed to get %s partition info\n", boot_file[id].name[dev_num]);
		return ret;
	}

	f_info->fsize = p_info.size;
	boot_file[id].len = p_info.size;
	boot_file[id].offset = p_info.offset;

	pr_debug("load %s image from sf 0x%x to memory 0x%lx size %d\n",
		 boot_file[id].name[dev_num], boot_file[id].offset, boot_file[id].addr, boot_file[id].len);

	return 0;
}
