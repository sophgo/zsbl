#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#define VERSION_PLATFORM	"2"

#define CSR_MCOR         0x7c2
#define CSR_MHCR         0x7c1
#define CSR_MCCR2        0x7c3
#define CSR_MHINT        0x7c5
#define CSR_MHINT2       0x7cc
#define CSR_MHINT4       0x7ce
#define CSR_MXSTATUS     0x7c0
#define CSR_PLIC_BASE    0xfc1
#define CSR_MRMR         0x7c6
#define CSR_MRVBR        0x7c7
#define CSR_MCOUNTERWEN  0x7c9
#define CSR_MCPUID       0xfc0
#define CSR_MSMPR        0x7f3

#define HART_COUNT		128
#define HART_PER_CHIP		64
#define MAX_CHIP_NUM		2
#define CLUSTER_PER_CHIP 	16

#define MAX_PHY_ADDR_WIDTH	40
#define CHIP_ADDR_SPACE		(1UL << 39)
#define CHIP_ADDR_BASE(n)	((n) * CHIP_ADDR_SPACE)
#define CHIP_HARTID_BASE(n)	((n) * HART_PER_CHIP)

#define CORES_PER_CLUSTER	4

#define DDR_CHANNEL_NUM		4
#define DDR_SIZE_ZERO		41

#define BOOT_FROM_SD_FIRST      BIT(0)
#define MULTI_SOCKET_MODE       BIT(15)

#endif
