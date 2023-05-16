#ifndef __BOARD_H__
#define __BOARD_H__

#include "platform.h"

#define VERSION_MAJOR "0."
#define VERSION_MINOR "1"
#define ZSBL_VERSION VERSION_MAJOR""VERSION_MINOR

#define DDR_CHANLE_NUM	4
#define DDR_SIZE_ZERO	41

#define DDR0_RESERVED	KERNEL_ADDR

typedef struct {
	uint64_t chip_ddr_size[DDR_CHANLE_NUM];
	uint64_t ddr_start_base[DDR_CHANLE_NUM];
	char *ddr_node_name[DDR_CHANLE_NUM];
} ddr_info;

typedef struct {
	uint64_t mac0;
	uint64_t mac1;
	uint8_t sn[32];
	char *dtb_name;
	uint64_t dtb_addr;
} config_ini;
typedef struct {
	ddr_info ddr_info[SG2042_MAX_CHIP_NUM];
	config_ini config_ini;
	uint8_t multi_sockt_mode;
} board_info;

typedef struct {
	uint8_t reg_ddr_size[DDR_CHANLE_NUM];

} reg_ddr_size;



#endif