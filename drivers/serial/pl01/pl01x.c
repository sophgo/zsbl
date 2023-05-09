#include <stdint.h>
#include <memmap.h>

#include <framework/module.h>
#include <framework/common.h>

struct pl01x_regs {
	volatile uint32_t	dr;		/* 0x00 Data register */
	volatile uint32_t	ecr;		/* 0x04 Error clear register (Write) */
	volatile uint32_t	pl010_lcrh;	/* 0x08 Line control register, high byte */
	volatile uint32_t	pl010_lcrm;	/* 0x0C Line control register, middle byte */
	volatile uint32_t	pl010_lcrl;	/* 0x10 Line control register, low byte */
	volatile uint32_t	pl010_cr;	/* 0x14 Control register */
	volatile uint32_t	fr;		/* 0x18 Flag register (Read only) */
	volatile uint32_t	reserved;
};

#define UART_PL01x_FR_TXFE              0x80
#define UART_PL01x_FR_RXFF              0x40
#define UART_PL01x_FR_TXFF              0x20
#define UART_PL01x_FR_RXFE              0x10
#define UART_PL01x_FR_BUSY              0x08
#define UART_PL01x_FR_TMSK              (UART_PL01x_FR_TXFF + UART_PL01x_FR_BUSY)

static struct pl01x_regs *uart = (struct pl01x_regs *)UART_BASE;

static void uart_putc(int ch)
{
	while (uart->fr & UART_PL01x_FR_TXFF)
		;
	uart->dr = ch;
}

static int uart_getc(void)
{
	if (uart->fr & UART_PL01x_FR_RXFE)
		return -1;

	return (int)uart->dr;
}

int uart_init(void)
{
	register_stdio(uart_getc, uart_putc);
	return 0;
}

early_init(uart_init);
