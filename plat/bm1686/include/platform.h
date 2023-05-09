#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <stdint.h>

struct tpu_runtime {
	int32_t	nodechip_idx;
	int8_t	bad_npu_idx0;
	int8_t	bad_npu_idx1;
	int8_t	bad_npu_idx2;
	int8_t	bad_npu_idx3;
	int32_t	terminated;
};

struct tpu_runtime *get_tpu_runtime(void);

#endif
