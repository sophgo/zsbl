#include <console.h>
#include <driver/serial/ns16550.h>
#include <framework/common.h>

static int console_uart_getc(void *console_hint);
static void console_uart_putc(void *console_hint, char c);
static struct ecdc_console *uart_console;

static CONSOLE uart_dev = {
	.console_getc= console_uart_getc,
	.console_putc= console_uart_putc,
};


static int console_uart_getc(void *console_hint)
{
	int err;

	err = uart_getc();
	return err >= 0 ? err : ECDC_GETC_EOF;
}

static void console_uart_putc(void *console_hint, char c)
{
	uart_putc(c);
}

int uart_console_set()
{
	uart_dev.console = uart_console;

	if (dev_console_set(CONSOLE_UART, &uart_dev)) {
		pr_err("uart console set failed\n");

		return -1;
	}

	return 0;
}
