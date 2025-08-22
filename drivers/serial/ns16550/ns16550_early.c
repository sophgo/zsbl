#include <stdint.h>
#include <errno.h>
#include <arch.h>
#include <libfdt.h>
#include <common/module.h>
#include <common/common.h>
#include <driver/platform.h>
#include <driver/serial.h>

#include "ns16550.h"

static struct ns16550 ns16550_early_port;

static int ns16550_stdio_input(void)
{
	return ns16550_getc(&ns16550_early_port);
}
static void ns16550_stdio_output(int ch)
{
	ns16550_putc(&ns16550_early_port, ch);
}

static int ns16550_early_init(void)
{
	ns16550_early_port.base = CONFIG_DRIVER_SERIAL_NS16550_EARLY_SERIAL_BASE;
	ns16550_early_port.baudrate = CONFIG_DRIVER_SERIAL_NS16550_EARLY_SERIAL_BAUDRATE;
	ns16550_early_port.pclk = CONFIG_DRIVER_SERIAL_NS16550_EARLY_SERIAL_CLOCK;
	ns16550_early_port.reg_io_width = CONFIG_DRIVER_SERIAL_NS16550_EARLY_SERIAL_REG_IO_WIDTH;
	ns16550_early_port.reg_shift = CONFIG_DRIVER_SERIAL_NS16550_EARLY_SERIAL_REG_SHIFT;

	ns16550_init(&ns16550_early_port);
	register_stdio(ns16550_stdio_input, ns16550_stdio_output);

	return 0;
}

early_init(ns16550_early_init);
