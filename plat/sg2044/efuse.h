#ifndef __EFUSE_H__
#define __EFUSE_H__

#include "config.h"

int get_dram_info(struct dram_info *dram_info);
void show_dram_info(struct dram_info *dram_info);

#endif
