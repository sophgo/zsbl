#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arch.h>

#include <lib/console.h>
#include <lib/cli.h>

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
