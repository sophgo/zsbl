#include <stdio.h>
#include <timer.h>
#include <string.h>
#include <framework/module.h>
#include <framework/common.h>
#include <lib/libc/errno.h>
#include <driver/sd/sd.h>

#include <ff.h>

static FATFS SDC_FS;
FRESULT f_ret;
FIL fp;

int sd_device_init()
{
	if (bm_sd_card_detect()) {
                pr_info("sd card insert\n");
        } else {
                pr_info("sd card not insert, please insert sd to continue sd test\n");
                return -1;
        }

        if (!bm_sd_init(SD_USE_PIO)) {
                pr_info("sd card init ok\n");
		return 0;
        } else {
		return -1;
	}
}
int read_boot_file()
{
	unsigned int rd_num;
	FRESULT ret;
	FILINFO fileinfo;
	uint8_t buf[64] = {0};

	ret = f_stat("0:fip.bin", &fileinfo);
	if (ret == FR_OK) {
		printf("file size is %d\n", fileinfo.fsize);
	}

	ret = f_read(&fp, buf, fileinfo.fsize, &rd_num);
	if (ret == FR_OK) {
		pr_info("read %d bytes, %s\n", rd_num, buf);
	} else {
		pr_info("read failed %d\n", ret);
	}

	return 0;
}
static int testfat32(void)
{
	int ret;
	ret = sd_device_init();
	if (ret == -1) {
		pr_info("sd init failed\n");
		goto end;
	}

	f_ret = f_mount(&SDC_FS, "0:", 1);
	if (f_ret == FR_OK) {
		f_ret = f_open(&fp, "0:fip.bin", FA_READ);
		if (f_ret == FR_OK) {
			pr_info("boot from SD\n");
			read_boot_file();
			f_close(&fp);
		} else {
			pr_info("fip not found on SD\n");
		}
		f_unmount("0:");
	} else {
		pr_info("mount failed\n");
	}

end:
	while (1) {
		pr_info("Hello BM1686 A53 System\n");
		mdelay(1000);
	}

	return 0;
}

test_case(testfat32);
