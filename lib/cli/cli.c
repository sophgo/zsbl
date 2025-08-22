#include <errno.h>
#include <string.h>

#include <common/common.h>
#include <common/module.h>
#include <driver/serial.h>
#include <lib/console.h>
#include <lib/cli.h>
#include <lib/list.h>
#include <lib/container_of.h>
#include <timer.h>

static LIST_HEAD(console_list);

struct cli_console {
	struct list_head list_head;
	struct console *console;
	struct serial *dev;
};

int cli_list_consoles(void)
{
	struct list_head *p;
	struct cli_console *cli_console;
	int i;

	pr_info("%6s %30s\n", "Index", "Device");

	i = 0;
	list_for_each(p, &console_list) {
		cli_console = container_of(p, struct cli_console, list_head);
		pr_info("%6d %30s\n", i, cli_console->dev->device.name);
		++i;
	}

	return i;
}

struct cli_console *cli_console_first(void)
{
	return list_first_entry_or_null(&console_list, struct cli_console, list_head);
}

struct cli_console *cli_console_next(struct cli_console *current)
{
	if (list_is_last(&current->list_head, &console_list))
		return NULL;

	return list_next_entry(current, list_head);
}

int cli_add_command(const char *name, command_callback fn, void *priv)
{
	struct cli_console *c;
	struct command *cmd;

	for (c = cli_console_first(); c; c = cli_console_next(c)) {
		cmd = console_alloc_command(priv, c->console, name, fn);
		if (!cmd) {
			pr_err("Failed add command to console\n");
		}
	}

	return 0;
}

static int run;

unsigned long cli_loop(unsigned int wait)
{
	struct cli_console *c;
	uint64_t start;
	int ch = 0;

	start = timer_get_tick();

	if (wait) {
		for (c = cli_console_first(); c; c = cli_console_next(c))
			ch = console_printf(c->console, "Press 'j' enter console\n");

		do {
			for (c = cli_console_first(); c; c = cli_console_next(c)) {
				ch = serial_input(c->dev);

				if (ch == 'j' || ch == 'J')
					break;
			}

			if (ch == 'j' || ch == 'J')
				break;

			if (timer_tick2us(timer_get_tick() - start) > wait)
				return wait;

		} while (true);
	}

	run = true;

	while (run) {
		for (c = cli_console_first(); c; c = cli_console_next(c)) {
			console_pump_console(c->console);
		}
	}

	return timer_tick2us(timer_get_tick() - start) + 1;
}

int cli_exit(void)
{
	int tmp;

	tmp = run;
	run = false;

	return tmp;
}

extern struct cli_command __ld_cli_command_start[0], __ld_cli_command_end[0];

static struct cli_console *init_cli_console(struct serial *dev)
{
	struct cli_console *cli_console;
	struct console *console;
	struct cli_command *cli_command;
	struct command *command;

	cli_console = malloc(sizeof(struct cli_console));
	if (!cli_console) {
		pr_err("Failed allocate cli console instance\n");
		return NULL;
	}

	console = console_alloc_console(dev, 128, 10);
	if (!console) {
		pr_err("Create console on device %s failed\n", dev->device.name);
		return NULL;
	}

	cli_console->console = console;
	cli_console->dev = dev;

	/* add all commands */
	console_alloc_list_command(console, "help");

	for (cli_command = __ld_cli_command_start;
	     cli_command != __ld_cli_command_end;
	     ++cli_command) {

		command = console_alloc_command(NULL, console,
						cli_command->name, cli_command->callback);
		if (!command)
			pr_err("Add command %s to console failed\n", cli_command->name);
	}

	return cli_console;
}

int cli_init(void)
{
	struct serial *dev;
	struct cli_console *cli_console;

	for (dev = serial_first(); dev; dev = serial_next(dev)) {
		cli_console = init_cli_console(dev);
		if (!cli_console)
			continue;
		list_add_tail(&cli_console->list_head, &console_list);
	}

	return 0;
}

late_init(cli_init);

static void hello(struct command *c, int argc, const char *argv[])
{
	console_printf(command_get_console(c), "hello\n");
}

cli_command(hello, hello);

static void command_exit(struct command *c, int argc, const char *argv[])
{
	cli_exit();
}

cli_command(exit, command_exit);
