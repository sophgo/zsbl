// SPDX-License-Identifier: GPL-2.0

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <lib/console.h>

// Size of the control sequence buffer. There is no upper bound specified
// in a standard, but we can assume that we won't have to process any of the
// really complex ones. That is mostly the terminals problem
#define CS_BUFFER_SIZE	16
#define DEFAULT_PROMPT	"# "

typedef void (*console_state_fn)(struct console *);

struct console {
	// Command linked list root pointer
	struct command *root;

	// Argument line storage
	char *arg_line;
	size_t arg_line_size;
	size_t arg_line_write_index;

	// Argument pointer storage
	const char **argv;
	size_t max_argc;

	// Console read / write
	struct serial *dev;

	int snoop_char;

	// State machine
	console_state_fn state;

	// Control sequence handling
	char cs_buffer[CS_BUFFER_SIZE];
	size_t cs_write_index;

	// Flags and settings
	bool f_local_echo;
	enum console_mode mode;

	// Prompt
	char *prompt;
};

static struct command *
locate_command(struct console *console, const char *name)
{
	struct command *ret = NULL;

	if (console != NULL) {
		struct command *node = NULL;

		for (node = console->root; node != NULL; node = node->next) {
			if (strcmp(node->name, name) == 0) {
				ret = node;

				break;
			}
		}
	}

	return ret;
}

static struct command *
get_last_command(struct console *console)
{
	struct command *last_command = NULL;

	if (console != NULL) {
		for (last_command = console->root;
		     last_command != NULL;
		     last_command = last_command->next) {

			if (last_command->next == NULL)
				break;
		}
	}

	return last_command;
}

static struct command *
get_previous_command(struct command *command)
{
	struct command *prev = NULL;

	do {
		if (command == NULL)
			break;

		struct console *console = command->console;

		if (console == NULL)
			break;

		if (console->root == command)
			break;

		struct command *node = NULL;

		for (node = console->root; node != NULL; node = node->next) {
			if (command == node->next) {
				prev = node;
				break;
			}
		}

	} while (0);

	return prev;
}

static void
register_command(struct console *console,
		 struct command *command)
{
	if (console != NULL) {
		command->console = console;
		command->next = NULL;
		struct command *last_command = get_last_command(console);

		if (last_command == NULL)
			console->root = command;
		else
			last_command->next = command;
	}
}

static void
unregister_command(struct command *command)
{
	do {
		if (command == NULL)
			break;

		struct console *console = command->console;

		if (console == NULL)
			break;

		if (console->root == command)
			console->root = command->next;
		else {
			struct command *prev = get_previous_command(command);

			if (prev != NULL)
				prev->next = command->next;
		}

		command->console = NULL;
		command->next = NULL;
	} while (0);
}

static char *
find_first_non_whitesapce(char *str)
{
	char *out_ptr = NULL;

	if (str == NULL)
		goto out;

	while (*str != '\0') {
		switch (*str) {
		case '\x20':
		case '\x09':
		case '\x0A':
		case '\x0B':
		case '\x0C':
		case '\x0D':
			++str;
			break;
		default:
			out_ptr = str;
			goto out;
		}
	}

out:
	return out_ptr;
}


static char *
find_fist_whitespace(char *str)
{
	char *out_ptr = NULL;

	if (str == NULL)
		goto out;

	while (*str != '\0') {
		switch (*str) {
		case '\x20':
		case '\x09':
		case '\x0A':
		case '\x0B':
		case '\x0C':
		case '\x0D':
			out_ptr = str;
			goto out;
		default:
			++str;
			break;
		}
	}

out:
	return out_ptr;
}
// --------------------------------------------------------- Terminal functions
// ---------------------------- Newline

static void
term_put_ansi_newline(struct console *console)
{
	serial_output(console->dev, '\r');
	serial_output(console->dev, '\n');
}

static inline void
term_put_newline(struct console *console)
{
	switch (console->mode) {
	case CONSOLE_MODE_ANSI:
	default:
		term_put_ansi_newline(console);
		break;
	}
}

// -------------------------- Backspace

static inline void
term_backspace_ansi(struct console *console)
{
	serial_output(console->dev, '\x08'); // BS
	serial_output(console->dev, '\x20'); // SP
	serial_output(console->dev, '\x08'); // BS
}

static inline void
term_backspace(struct console *console)
{
	switch (console->mode) {
	case CONSOLE_MODE_ANSI:
	default:
		term_backspace_ansi(console);
		break;
	}
}

//------------------- Character writing

static void
term_puts(struct console *console, const char *str)
{
	if (str != NULL) {
		while (*str != '\0') {
			if ('\n' == *str)
				term_put_newline(console);
			else if ('\x08' == *str)
				term_backspace(console);
			else
				serial_output(console->dev, *str);
			++str;
		}
	}
}

static inline void
term_putc(struct console *console, char c)
{
	if ('\n' == c)
		term_put_newline(console);
	else if ('\x08' == c)
		term_backspace(console);
	else
		serial_output(console->dev, c);
}

static inline void
term_puts_raw(struct console *console, const char *str)
{
	if (str != NULL) {
		while (*str != '\0') {
			serial_output(console->dev, *str);
			++str;
		}
	}
}

static inline void
term_putc_raw(struct console *console, char c)
{
	serial_output(console->dev, c);
}

// ------------------ Character reading

static inline int
term_getc_raw(struct console *console)
{
	int ret = console->snoop_char;

	console->snoop_char = EOF;

	if (ret == EOF) {
		ret = serial_input(console->dev);
		if (ret < 0)
			ret = EOF;
	}

	return ret;
}

static inline void
term_set_snoop_char(struct console *console, char c)
{
	console->snoop_char = c;
}

// ---------------------------------------------------- State machine functions

static void
state_start_new_command(struct console *console);

static void
state_read_input(struct console *console);

static void
state_parse_escape_sequence_ansi(struct console *console)
{
	// TODO: Stub
	// This would be a great spot to handle arrow keys
	console->state = state_read_input;
}

static void
state_read_escape_sequence(struct console *console)
{
	bool abort_sequence = false;
	bool parse_sequence = false;

	int loop_limit;

	for (loop_limit = 0; loop_limit < 8; ++loop_limit) {
		int new_char = term_getc_raw(console);

		if (new_char == EOF)
			break;

		char in = new_char;

		if (('\x18' == in) || ('\x1A' == in)) {
			// CAN or SUB
			abort_sequence = true;
			break;
		} else if (console->cs_write_index < CS_BUFFER_SIZE) {
			console->cs_buffer[console->cs_write_index] = in;
			size_t index = console->cs_write_index;

			++console->cs_write_index;

			if (index == 1) {
				if ('[' == in) {
					// Control sequence introducer, this will be a 3 or more
					// character sequence
				} else if (('\x40' <= in) && ('\x5F' >= in)) {
					// End of a two character escape sequence
					parse_sequence = true;
					break;
				}
			} else if (index > 1) {
				if (('\x40' <= in) && ('\x7E' >= in)) {
					// End of a multi character sequence
					parse_sequence = true;
					break;
				}
			}
		} else {
			// Dump the sequence, parsing it would be meaningless
			term_set_snoop_char(console, in);
			abort_sequence = true;
			break;
		}
	}

	if (abort_sequence) {
		size_t i;

		for (i = 0; i < console->cs_write_index; ++i)
			term_putc_raw(console, console->cs_buffer[i]);
		console->cs_write_index = 0;
		console->state = state_read_input;
	} else if (parse_sequence) {
		switch (console->mode) {
		case CONSOLE_MODE_ANSI:
		default:
			console->state = state_parse_escape_sequence_ansi;
			break;
		}
	}
}

static void
state_parse_input(struct console *console)
{
	if (console == NULL)
		return;

	// Clear argv
	{
		size_t i;

		for (i = 0; i < console->max_argc; ++i)
			console->argv[i] = NULL;
	}
	// Split
	size_t argc = 0;

	{
		char *arg_line = console->arg_line;

		arg_line[console->arg_line_write_index] = '\0';

		size_t i = 0;

		for (; i < console->max_argc; ++i) {
			char *start = find_first_non_whitesapce(arg_line);

			if (start == NULL)
				// Reached the \0 at the end of the string
				break;

			console->argv[i] = start;
			argc = i + 1;

			char *stop = find_fist_whitespace(start);

			if (stop == NULL)
				// Reached the \0 at the end of the string
				break;
			*stop = '\0';
			arg_line = stop + 1;
		}
	}

	// Search for handler
	if (argc > 0) {
		struct command *command = locate_command(console, console->argv[0]);

		if (command != NULL)
			// Found
			command->callback(command, argc, console->argv);
		else {
			term_puts(console, "'");
			term_puts(console, console->argv[0]);
			term_puts(console, "' not found\n");
		}
	}

	console->state = state_start_new_command;
}

static void
state_read_input(struct console *console)
{
	int loop_limit;

	for (loop_limit = 0; loop_limit < 8; ++loop_limit) {
		int new_char = term_getc_raw(console);

		if (new_char == EOF)
			break;

		bool local_echo = false;

		char in = new_char;

		if ('\r' == in) {
			term_put_newline(console);
			console->state = state_parse_input;
			break;
		} else if ('\x00' == in) {
			// Nothing to do here
		} else if (('\x08' == in) || ('\x7F' == in)) {
			// Backspace
			// This stuff gets super weird, apparently everyone on the planet
			// screwed up how backspace and delete work. See
			// http://www.ibb.net/~anne/keyboard.html for the full train wreck
			if (console->arg_line_write_index > 0) {
				term_backspace(console);
				--console->arg_line_write_index;
			}
		} else if ('\x1B' == in) {
			// Start of a control sequence
			term_set_snoop_char(console, in);
			console->cs_write_index = 0;
			console->state = state_read_escape_sequence;
		} else if ('\x1F' < in) {
			// Non-control sequence characters
			if (console->arg_line_write_index < console->arg_line_size) {
				console->arg_line[console->arg_line_write_index] = in;
				++console->arg_line_write_index;
				local_echo = console->f_local_echo;
			}
		}

		if (local_echo)
			term_putc_raw(console, in);
	}
}

static void
state_start_new_command(struct console *console)
{
	// Clear input string
	console->arg_line_write_index = 0;

	// Clear argv
	{
		size_t i;

		for (i = 0; i < console->max_argc; ++i)
			console->argv[i] = NULL;
	}

	// Set new state to read user input
	if (console->prompt == NULL)
		term_puts(console, DEFAULT_PROMPT);
	else
		term_puts(console, console->prompt);

	console->state = state_read_input;
}

// ---------------------------------------------------------- Build in commands

static void
built_in_list_command(struct command *cmd, int argc, char const *argv[])
{
	(void) argc;
	(void) argv;

	struct console *console = cmd->console;

	struct command *command;

	for (command = console->root; command != NULL; command = command->next) {
		term_puts(console, command->name);
		term_put_newline(console);
	}
}

// ----------------------------------------------------------- Public functions

struct console *
console_alloc_console(struct serial *dev,
		      size_t max_arg_line_length,
		      size_t max_arg_count)
{
	struct console *console =
		(struct console *) malloc(sizeof(struct console));

	if (console == NULL)
		goto out;

	console->root = NULL;
	console->dev = dev;
	console->snoop_char = EOF;
	console->state = state_start_new_command;
	console->prompt	 = NULL;

	if (max_arg_line_length < 16)
		max_arg_line_length = 16;
	console->arg_line_size = max_arg_line_length;
	console->arg_line_write_index = 0;

	size_t alloc_size = (max_arg_line_length + 1) * sizeof(char);

	console->arg_line = (char *) malloc(alloc_size);
	if (console->arg_line == NULL)
		goto out_arg_line_fail;

	if (max_arg_count < 1)
		max_arg_count = 1;
	console->max_argc = max_arg_count;

	console->argv = (const char **) malloc(sizeof(char *) * max_arg_count);
	if (console->argv == NULL)
		goto out_argv_fail;

	size_t i;

	for (i = 0; i < max_arg_count; ++i)
		console->argv[i] = NULL;
	// Initialize control sequence
	console->cs_write_index = 0;
	for (i = 0; i < CS_BUFFER_SIZE; ++i)
		console->cs_buffer[i] = '\x00';

	// Set default settings
	console_configure_console(console, CONSOLE_MODE_ANSI, CONSOLE_SET_LOCAL_ECHO);

	// Success
	goto out;

out_argv_fail:
	free(console->arg_line);

out_arg_line_fail:
	free(console);
	console = NULL;

out:
	return console;
}

void
console_free_console(struct console *console)
{
	if (console != NULL) {
		while (console->root != NULL)
			unregister_command(console->root);

		free(console->argv);
		free(console->arg_line);
		free(console->prompt);
		free(console);
	}
}

void
console_pump_console(struct console *console)
{
	if (console != NULL)
		console->state(console);
}

void
console_configure_console(struct console *console,
			  enum console_mode mode,
			  int flags)
{
	if (console != NULL) {
		console->mode = mode;
		console->f_local_echo = (CONSOLE_SET_LOCAL_ECHO & flags) ? true : false;
	}
}

struct command *
console_alloc_command(void *command_hint,
		      struct console *console,
		      const char *command_name,
		      command_callback callback)
{
	struct command *command = NULL;

	do {
		if (command_name == NULL)
			break;

		if (locate_command(console, command_name) != NULL)
			break;

		command = (struct command *) malloc(sizeof(struct command));
		if (command == NULL)
			break;
		command->console = NULL;
		command->next = NULL;
		command->callback = callback;
		command->hint = command_hint;

		command->name = strdup(command_name);

		register_command(console, command);
	} while (0);

	return command;
}

void
console_free_command(struct command *command)
{
	if (command != NULL) {
		unregister_command(command);

		free(command->name);
		free(command);
	}
}

struct command *
console_alloc_list_command(struct console *console,
			   const char *command_name)
{
	return console_alloc_command(console,
				     console,
				     command_name,
				     built_in_list_command);
}

char const *
console_replace_prompt(struct console *console,
		       char const *prompt)
{
	if ((console == NULL) || (prompt == NULL))
		return NULL;

	// If the prompt was previously not set, it will be NULL which will not
	// have any ill side effects when calling free
	free(console->prompt);

	console->prompt = strdup(prompt);
	return console->prompt;
}

void
console_free_prompt(struct console *console)
{
	free(console->prompt);
	console->prompt = NULL;
}

void
console_putc(struct console *console, char c)
{
	if (console != NULL)
		term_putc(console, c);
}

void
console_puts(struct console *console, const char *str)
{
	if (console != NULL)
		term_puts(console, str);
}

int console_printf(struct console *console, const char *format, ...)
{
	char buf[256];
	va_list args;
	int err;

	va_start(args, format);
	err = vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);

	console_puts(console, buf);

	return err;
}

struct console *command_get_console(struct command *cmd)
{
	return cmd->console;
}
