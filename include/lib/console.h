/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <stddef.h>
#include <driver/serial.h>

enum console_mode {
	CONSOLE_MODE_ANSI = 0
};

#define CONSOLE_SET_LOCAL_ECHO	 (1 << 0)

struct console;
struct command;

typedef void (*command_callback)(struct command *cmd, int argc, char const *argv[]);

struct console *console_alloc_console(struct serial *dev, size_t max_arg_line_length,
				      size_t max_arg_count);
void console_free_console(struct console *console);
void console_pump_console(struct console *console);
void console_configure_console(struct console *console, enum console_mode mode, int flags);
char const *console_replace_prompt(struct console *console, char const *prompt);
void console_free_prompt(struct console *console);
struct command * console_alloc_command(void *command_hint, struct console *console,
					 const char *command_name, command_callback callback);
void console_free_command(struct command *command);
struct command *console_alloc_list_command(struct console *console,
					     const char *command_name);
void console_putc(struct console *console, char c);
void console_puts(struct console *console, const char *str);
int console_printf(struct console *console, const char *format, ...);

struct console *command_get_console(struct command *cmd);

#endif
