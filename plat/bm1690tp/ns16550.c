#include <memmap.h>
#include <stdint.h>

#include <lib/mmio.h>
#include <common/module.h>
#include <common/common.h>

#define UART_REG_WIDTH 32
#define UART_BASE	0x7030002000
#define UART_PCLK	500000000
#define UART_BAUDRATE	115200

#define thr rbr
#define iir fcr
#define dll rbr
#define dlm ier

#if UART_REG_WIDTH == 8
typedef volatile uint8_t ns16550_reg_t;
#elif UART_REG_WIDTH == 16
typedef volatile uint16_t ns16550_reg_t;
#elif UART_REG_WIDTH == 32
typedef volatile uint32_t ns16550_reg_t;
#else
#error "unsupported register width"
#endif

struct ns16550_regs {
	ns16550_reg_t	rbr;	/* 0x00 Data register */
	ns16550_reg_t	ier;	/* 0x04 Interrupt Enable Register */
	ns16550_reg_t	fcr;	/* 0x08 FIFO Control Register */
	ns16550_reg_t	lcr;	/* 0x0C Line control register */
	ns16550_reg_t	mcr;	/* 0x10 Line control register */
	ns16550_reg_t	lsr;	/* 0x14 Line Status Register */
	ns16550_reg_t	msr;	/* 0x18 Modem Status Register */
	ns16550_reg_t	spr;	/* 0x20 Scratch Register */
};

#define UART_LCR_WLS_MSK	0x03	/* character length select mask */
#define UART_LCR_WLS_5		0x00	/* 5 bit character length */
#define UART_LCR_WLS_6		0x01	/* 6 bit character length */
#define UART_LCR_WLS_7		0x02	/* 7 bit character length */
#define UART_LCR_WLS_8		0x03	/* 8 bit character length */
#define UART_LCR_STB		0x04	/* # stop Bits, off=1, on=1.5 or 2) */
#define UART_LCR_PEN		0x08	/* Parity eneble */
#define UART_LCR_EPS		0x10	/* Even Parity Select */
#define UART_LCR_STKP		0x20	/* Stick Parity */
#define UART_LCR_SBRK		0x40	/* Set Break */
#define UART_LCR_BKSE		0x80	/* Bank select enable */
#define UART_LCR_DLAB		0x80	/* Divisor latch access bit */

#define UART_MCR_DTR		0x01	/* DTR	 */
#define UART_MCR_RTS		0x02	/* RTS	 */

#define UART_LSR_THRE		0x20	/* Transmit-hold-register empty */
#define UART_LSR_DR		0x01	/* Receiver data ready */
#define UART_LSR_TEMT		0x40	/* Xmitter empty */

#define UART_FCR_FIFO_EN	0x01	/* Fifo enable */
#define UART_FCR_RXSR		0x02	/* Receiver soft reset */
#define UART_FCR_TXSR		0x04	/* Transmitter soft reset */

#define UART_MCRVAL (UART_MCR_DTR | UART_MCR_RTS)		/* RTS/DTR */
#define UART_FCR_DEFVAL	(UART_FCR_FIFO_EN | UART_FCR_RXSR | UART_FCR_TXSR)
#define UART_LCR_8N1	0x03

struct ns16550_regs *uart = (struct ns16550_regs *)UART_BASE;

void uart_putc(int ch)
{
	while (!(uart->lsr & UART_LSR_THRE))
		;
	uart->rbr= ch;
}

int uart_getc(void)
{
	if (!(uart->lsr & UART_LSR_DR))
		return -1;

	return (int)uart->rbr;
}

int uart_init(void)
{
	unsigned int divisor;

	unsigned int baudrate = UART_BAUDRATE;
	unsigned int pclk = UART_PCLK;

	if(mmio_read_32(CLINT_MHART_ID))
		return 0;

	/* if any interrupt has been enabled, that means this uart controller
	 * may be initialized by some one before, just use it without
	 * reinitializing. such situation occur when main cpu and a53lite share
	 * the same uart port. main cpu should bringup first, reinitialize uart
	 * may cause unpredictable bug, especially disable all interrupts, which
	 * will cause linux running on main cpu lose of interrupts and cannot
	 * type into any character in serial console
	 */
	if (uart->ier == 0) {
		divisor = pclk / (16 * baudrate);

		uart->lcr = uart->lcr | UART_LCR_DLAB | UART_LCR_8N1;
		uart->dll = divisor & 0xff;
		uart->dlm = (divisor >> 8) & 0xff;
		uart->lcr = uart->lcr & (~UART_LCR_DLAB);

		uart->ier = 0;
		uart->mcr = UART_MCRVAL;
		uart->fcr = UART_FCR_DEFVAL;

		uart->lcr = 3;
	}

	register_stdio(uart_getc, uart_putc);

	return 0;
}

early_init(uart_init);
