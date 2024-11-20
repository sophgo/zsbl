#ifndef __MEMMAP_H__
#define __MEMMAP_H__

#define CLINT_MHART_ID  0x6844001000

#define UART_BASE	0x7030002000

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
