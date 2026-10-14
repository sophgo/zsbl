#include <common/common.h>
#include <driver/nvmem.h>
#include <driver/platform.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "config.h"

/* 88 is cell index
 * 4 is 32bits-per-cell
 */
#define EFUSE_DRAM_INFO_INDEX		(88)
#define EFUSE_CELL_SIZE			(4)
#define EFUSE_DRAM_INFO_OFFSET_0	(EFUSE_DRAM_INFO_INDEX * EFUSE_CELL_SIZE)
#define EFUSE_DRAM_INFO_OFFSET_1	((EFUSE_DRAM_INFO_INDEX + 1) * EFUSE_CELL_SIZE)

#define EFUSE_MISC_INFO_INDEX		(94)
#define EFUSE_MISC_INFO_OFFSET_0	(EFUSE_MISC_INFO_INDEX * EFUSE_CELL_SIZE)
#define EFUSE_MISC_INFO_OFFSET_1	((EFUSE_MISC_INFO_INDEX + 1) * EFUSE_CELL_SIZE)

#define SCS_CONFIG 0x804
#define PUBKEY_HASH 0x830

#define GB(n)	((n) * 1024 * 1024 * 1024)
#define MT(n)	((n) * 1000 * 1000)

static const char *vendor[] = { "Micron", "Hynix", "CXMT", "" };
static const uint64_t capacity[] = { GB(16UL), GB(8UL), 0, 0 };
static const uint64_t data_rate[] = { MT(8533UL), MT(9600UL), 0, 0 };
static const uint64_t channel_number[] = {8, 4, 0, 0};
static const uint64_t channel_map[] = {
	(1 << 1) | (1 << 2) | (1 << 5) | (1 << 6),
	(1 << 0) | (1 << 3) | (1 << 4) | (1 << 7),
	0,
	0,
};

static int secure_boot(void)
{
	struct nvmem *nvmem;
	unsigned int secure_boot_enable;

	nvmem = nvmem_find_by_name("efuse0");

	nvmem_read(nvmem, SCS_CONFIG, sizeof(secure_boot_enable), &secure_boot_enable);
	secure_boot_enable = (secure_boot_enable & 0xc) >> 2;

	/* test secure boot enable set 1 */
	if (secure_boot_enable) {
		pr_info("secure boot start!\n");
		return 1;
	}
	return secure_boot_enable;
}

int get_info_in_efuse(struct config *cfg)
{
	uint32_t flags = 0;
	uint32_t flags0 = 0;
	uint32_t flags1 = 0;
	struct nvmem *nvmem;
	struct dram_info *dram_info;

	dram_info = &cfg->dram;

	nvmem = nvmem_find_by_name("efuse1");
	if (!nvmem) {
		memset(dram_info, 0, sizeof(struct dram_info));
		pr_err("eFuse1 not found\n");
		return -ENODEV;
	}

	nvmem_read(nvmem, EFUSE_MISC_INFO_OFFSET_0, sizeof(flags0), &flags0);
	nvmem_read(nvmem, EFUSE_MISC_INFO_OFFSET_1, sizeof(flags1), &flags1);

	flags = flags0 | flags1;

	/* get conner */
	cfg->conner = (flags >> 16) & 0xf;
	if (cfg->conner < 0 || cfg->conner >= CHIP_CONNER_MAX)
		cfg->conner = CHIP_CONNER_TT;

	/* tpu available */
	cfg->tpu_avl = ((flags >> 15) & 1) ? false : true;

	/* check if SN is burn to DDR information area by mistake */
	if ((flags >> 14) & 1) {
		/* yes, SN is burn to DDR information area by mistake */
		dram_info->vendor = "Micron";
		dram_info->capacity = GB(16UL);
		dram_info->data_rate = MT(8533UL);
		dram_info->channel_number = 8;
		dram_info->channel_map = 0xff;
	} else {
		nvmem_read(nvmem, EFUSE_DRAM_INFO_OFFSET_0, sizeof(flags0), &flags0);
		nvmem_read(nvmem, EFUSE_DRAM_INFO_OFFSET_1, sizeof(flags1), &flags1);

		flags = flags0 | flags1;

		/* check valid */
		if (((flags >> 12) & 0x03) == 0 || ((flags >> 12) & 0x03) == 0x03) {
			pr_info("No DRAM information in eFuse, using defaults\n");
			dram_info->vendor = "Micron";
			dram_info->capacity = GB(16UL);
			dram_info->data_rate = MT(8533UL);
			dram_info->channel_number = 8;
			dram_info->channel_map = 0xff;
		} else {
			dram_info->vendor = vendor[flags & 0x03];
			dram_info->capacity = capacity[(flags >> 2) & 0x03];
			dram_info->data_rate = data_rate[(flags >> 4) & 0x03];
			dram_info->channel_number = channel_number[(flags >> 6) & 0x03];

			if (dram_info->channel_number == 4)
				dram_info->channel_map = channel_map[(flags >> 10) & 0x03];
			else
				dram_info->channel_map = 0xff;
		}
	}

	cfg->secure = secure_boot();

	return 0;
}

void read_pubkey_hash(void *data)
{
	int i;
	struct nvmem *nvmem;
	nvmem = nvmem_find_by_name("efuse0");

	for (i = 0; i < 8; i++)
		nvmem_read(nvmem,
			   PUBKEY_HASH + i * 4, sizeof(uint32_t), &((uint32_t *)data)[i]);

	const uint8_t pubkey_hash[32] = {
		0x40, 0xdb, 0xfb, 0x30, 0x89, 0xc4, 0xc7, 0xb6,
		0x49, 0x2e, 0x15, 0xd3, 0x2d, 0xf3, 0xa4, 0x5f,
		0x8a, 0x3e, 0xc0, 0xc4, 0xb1, 0x8c, 0xd9, 0x03,
		0xf0, 0x57, 0x7e, 0xf9, 0x49, 0x13, 0x3c, 0x5c,
	};

	memcpy(data, pubkey_hash, sizeof(pubkey_hash));
}

