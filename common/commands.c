#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arch.h>

#include <lib/console.h>
#include <lib/cli.h>
#include <lib/xmodem.h>
#include <common/common.h>

/* memory read */
static void command_mr(struct command *c, int argc, const char *argv[])
{
	uint64_t addr;

	if (argc != 2) {
		console_printf(c->console, "Invalid arguments\n");
		console_printf(c->console, "Useage: mr ADDRESS\n");
		return;
	}

	addr = strtoul(argv[1], NULL, 0);

	console_printf(c->console, "%08lx: %08x\n", addr, readl(addr));
}

cli_command(mr, command_mr);

static void command_mrq(struct command *c, int argc, const char *argv[])
{
	uint64_t addr;

	if (argc != 2) {
		console_printf(c->console, "Invalid arguments\n");
		console_printf(c->console, "Useage: mr ADDRESS\n");
		return;
	}

	addr = strtoul(argv[1], NULL, 0);

	console_printf(c->console, "%08lx: %08lx\n", addr, readq(addr));
}

cli_command(mrq, command_mrq);

/* memory write */
static void command_mw(struct command *c, int argc, const char *argv[])
{
	uint64_t addr, value;

	if (argc != 3) {
		console_printf(c->console, "Invalid arguments\n");
		console_printf(c->console, "Useage: mr ADDRESS VALUE\n");
		return;
	}

	addr = strtoul(argv[1], NULL, 0);
	value = strtoul(argv[2], NULL, 0);

	writel(value, addr);
}

cli_command(mw, command_mw);

static void command_mwq(struct command *c, int argc, const char *argv[])
{
	uint64_t addr, value;

	if (argc != 3) {
		console_printf(c->console, "Invalid arguments\n");
		console_printf(c->console, "Useage: mr ADDRESS VALUE\n");
		return;
	}

	addr = strtoul(argv[1], NULL, 0);
	value = strtoul(argv[2], NULL, 0);

	writeq(value, addr);
}

cli_command(mwq, command_mwq);

static int memory_test(struct command *c, unsigned long start, unsigned long size, uint64_t pattern)
{
	unsigned long i, end;
	uint64_t n;

	end = start + size;

	for (i = 0; i < size; i += 8) {
		*(volatile uint64_t *)(start + i) = pattern;
		if (i % (128 * 1024 * 1024) == 0)
			console_printf(c->console, "[%012lx - %012lx] : %016lx : %02u%%\r",
				       start, end, pattern, i * 100 / size / 2);
	}

	for (i = 0; i < size; i += 8) {

		if (i % (128 * 1024 * 1024) == 0)
			console_printf(c->console, "[%012lx - %012lx] : %016lx : %02u%%\r",
				       start, end, pattern, 50 + i * 100 / size / 2);

		n = *(volatile uint64_t *)(start + i);
		if (n != pattern) {
			console_printf(c->console,
				       "\nFailed at 0x%lx, expected 0x%016lx, get 0x%016lx\n",
				       start + i, pattern, n);
			return -1;
		}
	}

	console_printf(c->console, "[%012lx - %012lx] : %016lx : PASS\n", start, end, pattern);

	return 0;
}

static unsigned long strtoul_unit(const char *s)
{
	char *end;
	unsigned long n;

	n = strtoul(s, &end, 0);

	switch (*end) {
	case 't':
	case 'T':
		n *= 1024;
	case 'g':
	case 'G':
		n *= 1024;
	case 'm':
	case 'M':
		n *= 1024;
	case 'k':
	case 'K':
		n *= 1024;
	}

	return n;
}

/* memory test */
static void command_mt(struct command *c, int argc, const char *argv[])
{
	int i;
	uint64_t start, size;
	uint64_t pattern[] = {
		0xffffffffffffffffUL,
		0x0000000000000000UL,
		0x5555555555555555UL,
		0xaaaaaaaaaaaaaaaaUL,
	};

	if (argc != 3) {
		console_printf(c->console, "Invalid arguments\n");
		console_printf(c->console, "Useage: mt START SIZE\n");
		console_printf(c->console,
			"    START and SIZE support k/K, m/M, g/G suffix for KiB, MiB and GiB\n");
		return;
	}

	start = strtoul_unit(argv[1]);
	size = strtoul_unit(argv[2]);

	for (i = 0; i < ARRAY_SIZE(pattern); ++i)
		if (memory_test(c, start, size, pattern[i]))
			return;
}

cli_command(mt, command_mt);

/* download */

struct cmd_xm_ctx {
	void *start;
	unsigned long offset;
};

static int cmd_xm_get_char(struct xmodem *xm)
{
	struct command *c = (struct command *)xm->priv;

	return console_getc_raw(c->console);
}

static int cmd_xm_put_char(struct xmodem *xm, int ch)
{
	struct command *c = (struct command *)xm->priv;

	console_putc_raw(c->console, ch);

	return ch;
}

static int cmd_xm_save(void *priv, void *data, unsigned long len)
{
	struct cmd_xm_ctx *cxc = (struct cmd_xm_ctx *)priv;

	memcpy(cxc->start + cxc->offset, data, len);

	cxc->offset += len;

	return 0;
}

static void command_download(struct command *c, int argc, const char *argv[])
{
	struct xmodem xm;
	struct cmd_xm_ctx cxc;
	int err;
	unsigned long start;
	long len;

	err = xmodem_init(&xm, cmd_xm_get_char, cmd_xm_put_char, c);
	if (err) {
		console_printf(c->console, "Failed to init xmodem\n");
		return;
	}

	if (argc != 2) {
		console_printf(c->console, "Invalid arguments\n");
		console_printf(c->console, "Useage: download ADDRESS\n");
		return;
	}

	start = strtoul(argv[1], NULL, 0);

	cxc.offset = 0;
	cxc.start = (void *)start;

	len = xmodem_receive(&xm, cmd_xm_save, &cxc);

	if (len < 0)
		console_printf(c->console, "XMODEM receive failed\n");
	else
		console_printf(c->console, "%l bytes received\n", len);

	return;
}

cli_command(download, command_download);

/* set serial baudrate */
static void command_set_speed(struct command *c, int argc, const char *argv[])
{
	unsigned long baudrate;

	if (argc != 2) {
		console_printf(c->console, "Invalid arguments\n");
		console_printf(c->console, "Useage: speed BAUDRATE\n");
		return;
	}

	baudrate = strtoul(argv[1], NULL, 0);

	console_printf(c->console, "Set speed to %lu\n", baudrate);

	console_set_speed(c->console, baudrate);

	return;
}

cli_command(speed, command_set_speed);

