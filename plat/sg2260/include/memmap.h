#ifndef __MEMMAP_H__
#define __MEMMAP_H__

#define SYS_CTRL_BASE			0x7050000000
#define TOP_BASE			SYS_CTRL_BASE
#define SG2260_TOP_BASE		TOP_BASE

#define REG_TOP_CONF_INFO	0x4
#define BOOT_SEL_ADDR	(TOP_BASE + REG_TOP_CONF_INFO)
#define BOOT_FROM_SRAM  (1 << 2)

#if defined(CONFIG_TPU_SCALAR)
#define CLINT_MHART_ID  0x6844001000 // used for identify tp id
#endif

#define REG_DDR_SIZE    0X54
#define DDR_SIZE_ADDR   (TOP_BASE + REG_DDR_SIZE)
#define MP0_STATUS	0x380
#define MP0_STATUS_ADDR (TOP_BASE + MP0_STATUS)
#define MP0_CONTROL     0X384
#define MP0_CONTROL_ADDR	(TOP_BASE + MP0_CONTROL)

#define TOP_GP_REG0     (TOP_BASE + 0x1c0)
#define TOP_GP_REG1     (TOP_BASE + 0x1c4)
#define RUNTIME_STATUS  TOP_GP_REG1
#define TOP_GP_REG2     (TOP_BASE + 0x1c8)
#define BOARD_TYPE_REG  TOP_GP_REG2
#define TOP_GP_REG3     (TOP_BASE + 0x1cc)
#define TOP_GP_REG4     (TOP_BASE + 0x1d0)
#define TOP_GP_REG5     (TOP_BASE + 0x1d4)
#define TOP_GP_REG6     (TOP_BASE + 0x1d8)
#define TOP_GP_REG7     (TOP_BASE + 0x1dc)
#define TOP_GP_REG8     (TOP_BASE + 0x1e0)
#define TOP_GP_REG9     (TOP_BASE + 0x1e4)
#define TOP_GP_REG10    (TOP_BASE + 0x1e8)

#define SDIO_BASE	0x703000B000
#define SPIFLASH_1_BASE	0X7002180000
#define EFUSE_0_BASE	0x7030000000
#define TOP_RESET_BASE	0x7030013000

#define SD_RESET_INDEX  28

#if defined(CONFIG_TPU_SCALAR)
#define UART_BASE	0x7030002000
#else
#define UART_BASE	0x7030001000
#endif

#define UART_REG_WIDTH  32
#if defined(CONFIG_TARGET_EMULATOR)

#define UART_PCLK	153600
#define UART_BAUDRATE	9600

#elif defined(CONFIG_TARGET_PALLADIUM)

#define UART_PCLK	153600
#define UART_BAUDRATE	9600

#elif defined(CONFIG_TARGET_FPGA)

#define UART_PCLK	25000000
#define UART_BAUDRATE	115200

#elif defined(CONFIG_TARGET_ASIC)

#define UART_PCLK	500000000
#define UART_BAUDRATE	115200

#else
#error "no target specified"
#endif

#endif
