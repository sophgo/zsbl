#include <stdint.h>
#include <errno.h>
#include <arch.h>
#include <common/common.h>

#include "ns16550.h"

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

#define	RBR	0	/* 0x00 Data register */
#define	IER	1	/* 0x04 Interrupt Enable Register */
#define	FCR	2	/* 0x08 FIFO Control Register */
#define	LCR	3	/* 0x0C Line control register */
#define	MCR	4	/* 0x10 Line control register */
#define	LSR	5	/* 0x14 Line Status Register */
#define	MSR	6	/* 0x18 Modem Status Register */
#define	SPR	7	/* 0x20 Scratch Register */

#define THR	RBR
#define IIR	FCR
#define DLL	RBR
#define DLM	IER

/* reg is an index, not an offset */
static uint32_t read_reg(struct ns16550 *ndev, uint32_t reg)
{
	switch (ndev->reg_io_width) {
		case 1:
			return readb(ndev->base + (reg << ndev->reg_shift));
		case 2:
			return readw(ndev->base + (reg << ndev->reg_shift));
		default:
			return readl(ndev->base + (reg << ndev->reg_shift));
	}
}

static void write_reg(struct ns16550 *ndev, uint32_t reg, uint32_t val)
{
	switch (ndev->reg_io_width) {
		case 1:
			writeb(val, ndev->base + (reg << ndev->reg_shift));
			break;
		case 2:
			writew(val, ndev->base + (reg << ndev->reg_shift));
			break;
		default:
			writel(val, ndev->base + (reg << ndev->reg_shift));
	}
}

int ns16550_putc(struct ns16550 *ndev, uint8_t ch)
{
	while (!(read_reg(ndev, LSR) & UART_LSR_THRE))
		;

	write_reg(ndev, RBR, ch);

	return ch;

}

int ns16550_getc(struct ns16550 *ndev)
{
	if (!(read_reg(ndev, LSR) & UART_LSR_DR))
		return -EAGAIN;

	return read_reg(ndev, RBR);
}

int ns16550_init(struct ns16550 *ndev)
{
	unsigned long divisor;

	divisor = ndev->pclk / (16 * ndev->baudrate);

	write_reg(ndev, LCR, read_reg(ndev, LCR) | UART_LCR_DLAB | UART_LCR_8N1);
	write_reg(ndev, DLL, divisor & 0xff);
	write_reg(ndev, DLM, (divisor >> 8) & 0xff);
	write_reg(ndev, LCR, read_reg(ndev, LCR) & (~UART_LCR_DLAB));
	write_reg(ndev, IER, 0);
	write_reg(ndev, MCR, UART_MCRVAL);
	write_reg(ndev, FCR, UART_FCR_DEFVAL);

	return 0;
}

