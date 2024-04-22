#include <ecdc.h>
#include <stdio.h>
#include <lib/mmio.h>
#include <framework/common.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <driver/serial/ns16550.h>
#include <sbi/sbi_string.h>
#include <cli.h>
#include <riscv_cache.h>

#define	CMD_DATA_SIZE_ERR (-1)

typedef unsigned long (*read_fun) (unsigned long);
typedef void (*print_fun)(unsigned long);

static struct ecdc_console *console;
int console_exit = 0;
read_fun read_addr;
print_fun print;

struct command {
	const char *name, *alias, *usage;
	ecdc_callback_fn fn;
};

static inline int is_lower(char c)
{
	return (c >= 'a' && c <= 'z');
}

static inline int is_digit(char c)
{
	return (c >= '0' && c <= '9');
}

static inline int is_xdigit(char c)
{
	return (is_digit(c) ||
		(c >= 'A' && c <= 'F') ||
		(c >= 'a' && c <= 'f'));
}

static inline char _toupper(char c)
{

	return (c - 0x20 * ((c >= 'a') && (c <= 'z')));
}

static int console_getc(void *console_hint)
{
	int err;

	err = uart_getc();
	return err >= 0 ? err : ECDC_GETC_EOF;
}

static void console_putc(void *console_hint, char c)
{
	uart_putc(c);
}

static const char * const cmd_writemem_usage =
"write mem\n"
"input error, please input: mw addr size pattern\n";

static void cmd_writemem(void *hint, int argc, char const *argv[])
{
	unsigned long addr, size;
	uint32_t pattern;
	unsigned long i;

	if (argc > 3) {
		addr = strtoul(argv[1], NULL, 16);
		size = strtoul(argv[2], NULL, 16);
		pattern = strtoul(argv[3], NULL, 16);

		for (i = 0; i < size; i += 4)
			mmio_write_32(addr + i, pattern);
	} else {
		printf(cmd_writemem_usage);
	}
};

static void cmd_go(void *hint, int argc, char const *argv[])
{

	((void (*)(void))strtoul(argv[1], NULL, 16))();
}

void print_byte(unsigned long value)
{
	printf("0x%02lx  ", value);
}

void print_word(unsigned long value)
{
	printf("0x%04lx  ", value);
}

void print_dword(unsigned long value)
{
	printf("0x%08lx  ", value);
}

void print_qword(unsigned long value)
{
	printf("0x%016lx  ", value);
}

int cmd_get_data_size(const char *arg)
{
	switch (arg[0]) {
	case 'b':
		read_addr = (read_fun)mmio_read_8;
		print = print_byte;
		return 1;
	case 'w':
		read_addr = (read_fun)mmio_read_16;
		print = print_word;
		return 2;
	case 'd':
		read_addr = (read_fun)mmio_read_32;
		print = print_dword;
		return 4;
	case 'q':
		read_addr = (read_fun)mmio_read_64;
		print = print_qword;
		return 8;
	default:
		return CMD_DATA_SIZE_ERR;
	}

}

static const char * const cmd_readmem_usage =
"read mem\n"
"input error, please input: mr [b, w, d, q] addr count\n";

static void cmd_readmem(void *hint, int argc, char const *argv[])
{
		unsigned long addr, size, length, value, i;
		uint32_t count, all_size;
		unsigned long j = 0;
		uint32_t max_line_size = 16;

		if (argc > 3) {
			size = cmd_get_data_size(argv[1]);

			if (size == -1) {
				pr_err("command input error\n");
				return;
			}

			count = max_line_size / size;

			addr = strtoul(argv[2], NULL, 16);
			length = strtoul(argv[3], NULL, 16);

			all_size = length * size;

			for (i = 0; i < all_size; i += size) {
				value = read_addr(addr + i);

				if ((j % count) == 0)
					printf("0x%010lx:  ", addr + i);

				print(value);

				if ((++j % count) == 0)
					printf("\n");

			}
			printf("\n");

		} else {
			printf(cmd_readmem_usage);
		}

};

static void cmd_exit(void *hint, int argc, char const *argv[])
{
	console_exit = 1;
}

static struct command command_list[] = {
	{"mr", NULL, NULL, cmd_readmem},
	{"mw", NULL, NULL, cmd_writemem},
	{"go", NULL, NULL, cmd_go},
	{"exit", NULL, NULL, cmd_exit},
};

int console_init(void)
{
	int i;


	console = ecdc_alloc_console(NULL, console_getc, console_putc, 128, 4);
	if (console == NULL) {
		printf("create console failed\n");
		return -1;
	}
	ecdc_configure_console(console, ECDC_MODE_ANSI, ECDC_SET_LOCAL_ECHO);
	for (i = 0; i < ARRAY_SIZE(command_list); ++i) {
		if (command_list[i].name)
			ecdc_alloc_command(NULL, console,
					   command_list[i].name,
					   command_list[i].fn);
		if (command_list[i].alias)
			ecdc_alloc_command(NULL, console,
					   command_list[i].alias,
					   command_list[i].fn);
	}

	ecdc_alloc_list_command(console, "list");

	return 0;
}

void console_poll(void)
{
	ecdc_pump_console(console);
}
