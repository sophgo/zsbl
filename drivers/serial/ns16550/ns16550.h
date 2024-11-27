#ifndef __NS16550_H__
#define __NS16550_H__

struct ns16550 {
	unsigned long base;
	unsigned int baudrate;
	unsigned int pclk;
	unsigned int reg_io_width;
	unsigned int reg_shift;
};

int ns16550_putc(struct ns16550 *ndev, uint8_t ch);
int ns16550_getc(struct ns16550 *ndev);
int ns16550_init(struct ns16550 *ndev);

#endif
