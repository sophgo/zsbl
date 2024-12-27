#include <framework/common.h>
#include <driver/nvmem.h>
#include <driver/platform.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "config.h"

/* 88 is cell index
 * 2 is double bits
 * 4 is 32bits-per-cell
 */
#define EFUSE_DRAM_INFO_OFFSET	(88 / 2 * 4)

#define GB(n)	((n) * 1024 * 1024 * 1024)
#define MT(n)	((n) * 1000 * 1000)

static const char *vendor[] = { "Micron", "Hynix", "CX", "" };
static const uint64_t capacity[] = { GB(16UL), GB(8UL), 0, 0 };
static const uint64_t data_rate[] = { MT(8533UL), MT(9600UL), 0, 0 };
static const uint64_t channel_number[] = {8, 4, 0, 0};
static const uint64_t channel_map[] = {
	(1 << 1) | (1 << 2) | (1 << 5) | (1 << 6),
	(1 << 0) | (1 << 3) | (1 << 4) | (1 << 7),
	0,
	0,
};

int get_dram_info(struct dram_info *dram_info)
{
	uint32_t flags = 0;
	struct nvmem *nvmem;

	nvmem = nvmem_find_by_name("efuse0");
	if (!nvmem) {
		memset(dram_info, 0, sizeof(struct dram_info));
		pr_err("eFuse0 not found\n");
		return -ENODEV;
	}

	nvmem_read(nvmem, EFUSE_DRAM_INFO_OFFSET, sizeof(flags), &flags);

	/* check valid */
	if (((flags >> 12) & 0x03) == 0) {
		pr_info("No DRAM information in eFuse, using defaults\n");
		dram_info->vendor = "Hynix";
		dram_info->capacity = GB(8UL);
		dram_info->data_rate = MT(8533UL);
		dram_info->channel_number = 8;
		dram_info->channel_map = 0xff;
		return 0;
	}

	dram_info->vendor = vendor[flags & 0x03];
	dram_info->capacity = capacity[(flags >> 2) & 0x03];
	dram_info->data_rate = data_rate[(flags >> 4) & 0x03];
	dram_info->channel_number = channel_number[(flags >> 6) & 0x03];

	if (dram_info->channel_number == 4)
		dram_info->channel_map = channel_map[(flags >> 10) & 0x03];
	else
		dram_info->channel_map = 0xff;

	return 0;
}

