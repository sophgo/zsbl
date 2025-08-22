#include <stdio.h>
#include <timer.h>
#include <string.h>
#include <common/module.h>
#include <common/common.h>
#include <lib/libc/errno.h>

#include <driver/sd/sd.h>

uint8_t __attribute__((aligned(1024))) sd_buf[1024];
int sd_test_rw(void)
{
        int ret;
	int cnt = MMC_BLOCK_SIZE*2;
	int lba = 0;

        for (int i = 0; i < 1024; i++)
                sd_buf[i] = 0x5a;

        ret = mmc_erase_blocks(lba, cnt);
        if (ret != cnt) {
                pr_info("erase mmc failed\n");
		return -EIO;
        }

	ret = mmc_write_blocks(lba, (uintptr_t)sd_buf, cnt);
	if (ret != cnt) {
		pr_info("write mmc failed\n");
		return -EIO;
	}
	memset(sd_buf, 0, sizeof(sd_buf));

	ret = mmc_read_blocks(lba, (uintptr_t)sd_buf, cnt);
	if (ret != cnt) {
		pr_info("read mmc failed\n");
		return -EIO;
	}

	for (int i = 0; i < cnt; i++) {
		if (sd_buf[i] != 0x5a) {
			pr_info("compare mmc data failed\n");
			return -EIO;
		}
	}

	pr_info("mmc wr test ok\n");

	return 0;

}

static int testsd(void)
{
        if (bm_sd_card_detect()) {
                pr_info("sd card insert\n");
        } else {
                pr_info("sd card not insert, please insert sd to continue sd test\n");
                goto end;
        }

        if (!bm_sd_init(SD_USE_PIO)) {
                pr_info("sd card init ok\n");
                sd_test_rw();
        }

end:
	while (1) {
		pr_info("Hello BM1686 A53 System\n");
		mdelay(1000);
	}

	return 0;
}

test_case(testsd);
