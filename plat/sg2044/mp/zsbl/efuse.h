#ifndef __EFUSE_H__
#define __EFUSE_H__

#include "config.h"

int get_info_in_efuse(struct config *cfg);
void show_dram_info(struct dram_info *dram_info);

void read_pubkey_hash(void *pubkey_dig);

#endif
