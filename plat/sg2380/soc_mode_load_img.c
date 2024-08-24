// SPDX-License-Identifier: GPL-2.0

#define DEBUG
#include <stdio.h>
#include <timer.h>
#include <string.h>
#include <framework/module.h>
#include <framework/common.h>
#include <lib/libc/errno.h>
#include <driver/sd/sd.h>
#include <driver/io/io_sd.h>
#include <driver/io/io_flash.h>
#include <driver/io/io_emmc.h>
#include <driver/io/io_spinand.h>
#include <driver/emmc/sg_emmc.h>
#include <driver/io/io.h>
#include <driver/spi-flash/sg_spif.h>
#include <driver/spinand/spinand.h>
#include <stdlib.h>
#include <string.h>
#include <ff.h>
#include <platform.h>
#include <memmap.h>
#include <lib/mmio.h>
#include <assert.h>
#include <of.h>
#include <smp.h>
#include <sbi/riscv_asm.h>
#include "spinlock.h"
#include <libfdt.h>
#include <sbi.h>
#include <thread_safe_printf.h>

//#include "sg2380/include/board.h"

#define L1_CACHE_BYTES 64
#define STACK_SIZE 4096

uint64_t table_addr = DISK_PART_TABLE_ADDR;

int get_current_device(void);

static inline void sync_is(void)
{
	asm volatile(".long 0x01b0000b");
}

void wbinv_va_range(unsigned long start, unsigned long end)
{
	register unsigned long i asm("a0") = start & ~(L1_CACHE_BYTES - 1);

	for (; i < end; i += L1_CACHE_BYTES)
		asm volatile(".long 0x0275000b"); /* dcache.civa a0 */

	sync_is();
}

struct core_stack {
	uint8_t stack[STACK_SIZE];
};

static struct core_stack secondary_core_stack[CONFIG_SMP_NUM];

static struct fw_dynamic_info ap_dynamic_info = {
	.magic = FW_DYNAMIC_INFO_MAGIC_VALUE,
	.version = 0,
	.next_addr = KERNEL_ADDR,
	.next_mode = FW_DYNAMIC_INFO_NEXT_MODE_S,
	.boot_hart = 0xffffffffffffffff,
};

void *get_stack_base(int core_id)
{
	return (void *)&secondary_core_stack[core_id];
}

struct fw_dynamic_info *get_ap_fw_info(void)
{
	return &ap_dynamic_info;
}

enum { 
	RP_ID_ZSBL = 0,
    RP_ID_MAX,
};

enum { AP_ID_OPENSBI = 0,
       AP_ID_KERNEL,
       AP_ID_RAMFS,
       AP_ID_DEVICETREE,
       AP_ID_MAX,
};
enum {
	BOOT_SPINOR_PRIORITY = 0,
	BOOT_EMMC_PRIORITY,
	BOOT_SPINAND_PRIORITY,
};
//static --> noraml
BOOT_FILE ap_boot_file[AP_ID_MAX] = {
	[AP_ID_OPENSBI] = {
		.id = AP_ID_OPENSBI,
		.name[IO_DEVICE_SD] = "0:riscv64/fw_dynamic.bin",
		.name[IO_DEVICE_EMMC] = "0:riscv64/fw_dynamic.bin",
		.name[IO_DEVICE_SPIFLASH] = "0:riscv64/fw_dynamic.bin",
		.name[IO_DEVICE_SPINAND] = "0:riscv64/fw_dynamic.bin",
		.addr = OPENSBI_ADDR,
	},
	[AP_ID_KERNEL] = {
		.id = AP_ID_KERNEL,
		.name[IO_DEVICE_SD] = "0:riscv64/riscv64_Image",
		.name[IO_DEVICE_EMMC] = "0:riscv64/riscv64_Image",
		.name[IO_DEVICE_SPIFLASH] = "0:riscv64/riscv64_Image",
		.name[IO_DEVICE_SPINAND] = "0:riscv64/riscv64_Image",
		.addr = KERNEL_ADDR,
	},
	[AP_ID_RAMFS] = {
		.id = AP_ID_RAMFS,
		.name[IO_DEVICE_SD] = "0:riscv64/initrd.img",
		.name[IO_DEVICE_EMMC] = "0:riscv64/initrd.img",
		.name[IO_DEVICE_SPIFLASH] = "0:riscv64/initrd.img",
		.name[IO_DEVICE_SPINAND] = "0:riscv64/initrd.img",
		.addr = RAMFS_ADDR,
	},
	[AP_ID_DEVICETREE] = {
		.id = AP_ID_DEVICETREE,
		.name[IO_DEVICE_SD] = "0:riscv64/ap.dtb",
		.name[IO_DEVICE_EMMC] = "0:riscv64/ap.dtb",
		.name[IO_DEVICE_SPIFLASH] = "0:riscv64/ap.dtb",
		.name[IO_DEVICE_SPINAND] = "0:riscv64/ap.dtb",
		.addr = DEVICETREE_ADDR,
	},
};

// chenym add baohu
static char *ap_img_name_sd[] = {
	[AP_ID_OPENSBI] = "0:riscv64/fw_dynamic.bin",
	[AP_ID_KERNEL] = "0:riscv64/riscv64_Image",
	[AP_ID_RAMFS] = "0:riscv64/initrd.img",
	[AP_ID_DEVICETREE] = "0:riscv64/ap.dtb",
};

static char *ap_img_name_spi[] = {
	[AP_ID_OPENSBI] = "fw_dynamic.bin",
	[AP_ID_KERNEL] = "riscv64_Image",
	[AP_ID_RAMFS] = "initrd.img",
	[AP_ID_DEVICETREE] = "ap.dtb",
};
static char *ap_img_name_spi_nand[] = {
	[AP_ID_OPENSBI] = "fw_dynamic.bin",
	[AP_ID_KERNEL] = "riscv64_Image",
	[AP_ID_RAMFS] = "initrd.img",
	[AP_ID_DEVICETREE] = "ap.dtb",
};
static char *ap_img_name_emmc[] = {
	[AP_ID_OPENSBI] = "fw_dynamic.bin",
	[AP_ID_KERNEL] = "riscv64_Image",
	[AP_ID_RAMFS] = "initrd.img",
	[AP_ID_DEVICETREE] = "ap.dtb",
};

static char **ap_img_name[] = {
	[IO_DEVICE_SD] = ap_img_name_sd,
	[IO_DEVICE_SPIFLASH] = ap_img_name_spi,
	[IO_DEVICE_SPINAND] = ap_img_name_spi_nand,
	[IO_DEVICE_EMMC] = ap_img_name_emmc
};

//chenym add
//board_info sg2380_board_info;

static inline char **get_bootfile_list(int dev_num, char **bootfile[])
{
	if (dev_num < IO_DEVICE_MAX)
		return bootfile[dev_num];

	return NULL;
}

int read_all_img(IO_DEV *io_dev, int dev_num, BOOT_FILE *boot_file, int file_num)
{
	FILINFO info;

	// if (io_dev->func.init()) {
	// 	pr_err("init %s device failed\n", get_io_device_name(io_dev->type));
	// 	goto umount_dev;
	// }

	for (int i = 0; i < file_num; i++) {
		if (boot_file[i].name[dev_num] == NULL)
			continue;

		if (io_dev->func.open(boot_file[i].name[dev_num], FA_READ)) {
			pr_err("open %s failed\n", boot_file[i].name[dev_num]);
			goto close_file;
		}

		if (io_dev->func.get_file_info(boot_file, i, &info, dev_num, table_addr)) {
			pr_err("get %s info failed\n", boot_file[i].name[dev_num]);
			goto close_file;
		}

		/* skip empty file */
		if (info.fsize == 0) {
			pr_warn("%s file length zero, skip it!\n",
				boot_file[i].name[dev_num]);
			continue;
		}

		boot_file[i].len = info.fsize;

		if (io_dev->func.read(boot_file, i, info.fsize)) {
			pr_err("read %s failed\n", boot_file[i].name);
			goto close_file;
		}

		if (io_dev->func.close()) {
			pr_err("close %s failed\n", boot_file[i].name[dev_num]);
			goto umount_dev;
		}

		wbinv_va_range(boot_file[i].addr,
			       boot_file[i].addr + info.fsize);
		__asm__ __volatile__("fence.i" ::);
	}

	io_dev->func.destroy();

	return 0;

close_file:
	io_dev->func.close();
umount_dev:
	io_dev->func.destroy();

	return -1;
}

int boot_device_register(void)
{
	if (sd_io_device_register() || flash_io_device_register()
		|| emmc_io_device_register() || spinand_io_device_register())
		return -1;

	return 0;
}

int build_bootfile_info(int dev_num, BOOT_FILE *boot_file, int file_num)
{
	// char *sd_dtb_name = NULL;
	// char *sd_kernel_name = NULL;
	// char *sd_fw_name = NULL;
	// char *sd_ramfs_name = NULL;
	
	char **imgs =  get_bootfile_list(dev_num, ap_img_name);
	
	
	if (!imgs)
	{
		pr_err("error boot file list\n");
		return -1;
	}
	pr_info("get boot file list successfully");
	
	//addr 是否需要修改？？？
	for (int i = 0; i < file_num; i++)
		boot_file[i].name[dev_num] = imgs[i];

	return 0;
}

//cym add
//read boot_sel
unsigned char get_boot_sel()
{
	char boot_src = mmio_read_32(BOOT_SEL) >> 18;
	return boot_src;
}

int read_boot_file(BOOT_FILE *boot_file, int file_num)
{
	IO_DEV *io_dev;
	int dev_num = get_current_device();
	pr_info("boot from device %s",get_io_device_name(dev_num));
	
	// dev_num = IO_DEVICE_SPIFLASH;
	// write bootfile list inti bootfile
	build_bootfile_info(dev_num, boot_file, file_num);

	io_dev = set_current_io_device(dev_num);
	if (io_dev == NULL) {
		pr_err("set current io device failed, dev_num=%d \n",dev_num);
		return -1;
	}
	
	//需要注意 执行失败会进入，if判断为真
	
	if (read_all_img(io_dev, dev_num, boot_file, file_num)) 
	{
		int i = 0;
		for (i = 0; i < IO_DEVICE_MAX; i++)
		{
			dev_num = i;
			io_dev = set_current_io_device(dev_num);
			if (io_dev == NULL)
			{
				continue;
			}
			if (!read_all_img(io_dev, dev_num, boot_file, file_num))
			{
				pr_info("dev+num=%d read boot file ok\n",dev_num);
				break;
			}else
			{
				pr_err("dev+num=%d read boot file failed\n",dev_num);
			}
			
		}
		if (i == IO_DEVICE_MAX)
		{
			return -1;
		}
		
		
	} else {
		pr_debug("dev %d read boot file ok\n",dev_num);
	}

	return 0;
}

static void soc_mode_secondary_core_fun(void *priv)
{
	__asm__ __volatile__("fence.i" ::);

	jump_to(OPENSBI_ADDR, current_hartid(), DEVICETREE_ADDR,
		(long)get_ap_fw_info());
}

static int soc_mode_jump_to_sbi(void)
{
	unsigned int hartid = current_hartid();

	for (int i = 0; i < CONFIG_SMP_NUM; i++) {
		if (i == hartid)
			continue;
		wake_up_other_core(i, soc_mode_secondary_core_fun, NULL,
				   get_stack_base(i), STACK_SIZE);
	}

	jump_to(OPENSBI_ADDR, current_hartid(), DEVICETREE_ADDR,
		(long)get_ap_fw_info());

	return 0;
}

// chenym add 
// for init memery

int build_board_info (void)
{
	//TODO : read multi_socket_mode

	//TODO : init ddr info

	
	return 0;
}
//chenym add
int get_current_device(void)
{
	IO_DEVICE dev_num = IO_DEVICE_SD;
	uint8_t boot_sel = get_boot_sel();
	uint8_t boot_src = (boot_sel & BOOT_PRIORITY) >> 3;

	//所有设备注册 -- 填充*io_device 数组
	if (boot_device_register())
	{
		pr_err("boot device register failed\n");
		return -1;
	}
	pr_info("boot device register ok \n");
	//使用 BOOT_SEL的第21位和22位标识启动方式
	// 01 EMMC 10 NANDFLASH
	// 11 00 SD? NORFLASH
	if (!(boot_sel & BOOT_FROM_SD_FIRST) &&
	    bm_sd_card_detect()) {
		dev_num = IO_DEVICE_SD;
	} else if ((boot_src == BOOT_EMMC_PRIORITY))
	{
		dev_num = IO_DEVICE_EMMC;
	}else if ((boot_src == BOOT_SPINAND_PRIORITY))
	{
		dev_num = IO_DEVICE_SPINAND;
		
	}else
	{
		dev_num = IO_DEVICE_SPIFLASH;
	}
	return dev_num;
	
}

// copy from zsbl 2042
/*Resize fdt to modify some node, eg: bootargs*/
// int resize_dtb(int delta)
// {
// 	void *fdt;
// 	int size;
// 	int ret = 0;

// 	fdt = (void *)boot_file[ID_DEVICETREE].addr;
// 	size = fdt_totalsize(fdt) + delta;

// 	fdt = realloc(fdt, size);
// 	if (fdt) {
// 		ret = fdt_open_into(fdt, fdt, size);
// 		if (ret != 0)
// 			pr_err("fdt: resize failed, error[%d\n]", ret);
// 		else {
// 			boot_file[ID_DEVICETREE].addr = (uint64_t)fdt;
// 			boot_file[ID_DEVICETREE].len = size;
// 		}
// 	} else {
// 		pr_err("fdt: realloc fdt failed\n");
// 		ret = -1;
// 	}

// 	return ret;
// }

// copy from zsbl 2042
int modify_dtb(void)
{
	// int ret = 0;

	// ret = resize_dtb(4096);
	// if (!ret) {
	// 	modify_ddr_node();
	// 	modify_bootargs();
	// }

	// modify_cpu_node();
	// modify_eth_node();

	return 0;
}


int read_config_file (void)
{
	//TODO 学习 2042 读取配置文件并适配
	

	return 0;
}

int ap_boot(void)
{
	// if (build_board_info)
	// {
	// 	pr_err("build board info failed\n");
	// 	return -1;
	// }
	//pr_info("build board info ok");

	// if (read_config_file())
	// {
	// 	pr_err("read config file failed\n");
	// }
	// pr_info("read config file ok");

	
	
	if (read_boot_file(ap_boot_file, AP_ID_MAX)) {
		pr_err("ap read boot file faile\n");
		assert(0);
	}
	pr_info("read boot_file ok \n");

	if (modify_dtb()) {
		pr_err("modify dtb failed\n");
		assert(0);
	}

	soc_mode_jump_to_sbi();

	return 0;
}

int boot_from_storage(void)
{
	pr_info("boot from storage\n");
	
	ap_boot();

	return 0;
}
