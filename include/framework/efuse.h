#ifndef __EFUSE_H__
#define __EFUSE_H__

#include <stdint.h>

uint32_t efuse_read_cell(unsigned long base, int offset);

#endif
