#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#define OPENSBI_ADDR		0x80000000
#define DEVICETREE_ADDR		0x88000000
#define KERNEL_ADDR		0x80200000
#define RAMFS_ADDR		0x8b000000

#define PLAT_MAX_CHIP_NUM	1
#define BOOT_FROM_SD_FIRST	BIT(1)

#if defined(CONFIG_TARGET_EMULATOR)

#define UART_BAUDRATE		115200
#define UART_PCLK		200000000
#define SD_CLOCK		400000000
#define SDCARD_INIT_FREQ	(200 * 1000)
#define SDCARD_TRAN_FREQ	(25 * 1000 * 1000)

#elif defined(CONFIG_TARGET_PALLADIUM)

#define UART_BAUDRATE		9600
#define UART_PCLK		153600
#define SD_CLOCK		400000000
#define SDCARD_INIT_FREQ	(200 * 1000)
#define SDCARD_TRAN_FREQ	(25 * 1000 * 1000)

#elif defined(CONFIG_TARGET_FPGA)

#define UART_BAUDRATE		9600
#define UART_PCLK		10000000
#define SD_CLOCK		10000000
#define SDCARD_INIT_FREQ	(200 * 1000)
#define SDCARD_TRAN_FREQ	(6 * 1000 * 1000)

#elif defined(CONFIG_TARGET_ASIC)

#define UART_BAUDRATE		115200
#define UART_PCLK		200000000
#define SD_CLOCK		400000000
#define SDCARD_INIT_FREQ	(200 * 1000)
#define SDCARD_TRAN_FREQ	(25 * 1000 * 1000)

#else
#error "no target specified"
#endif

#endif

