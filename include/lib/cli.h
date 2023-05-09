/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014 Google, Inc
 * Simon Glass <sjg@chromium.org>
 */

#ifndef __CLI_H_
#define __CLI_H_

#include <u-boot.h>

#define CLI_HOT_KEY	'f'

extern char console_buffer[];

/**
 * Go into the command loop
 *
 * This will return if we get a timeout waiting for a command. See
 * CONFIG_BOOT_RETRY_TIME.
 */
void cli_simple_loop(void);

/**
 * cli_readline() - read a line into the console_buffer
 *
 * This is a convenience function which calls cli_readline_into_buffer().
 *
 * @prompt: Prompt to display
 * @return command line length excluding terminator, or -ve on error
 */
int cli_readline(const char *const prompt);

/**
 * @bootdelay: seconds to wait for pressing key to enter command line, -1
 * for not waiting for key pressing and just boot, 0 for entering command
 * line directly.
 * @return 1 if enter command line, or 0 to continue booting
 */
int abortboot_single_key(int bootdelay);

/**
 * Go into the command loop
 *
 * bootdelay = -1: must press hot key before power up
 * bootdelay = 0: enter cli directly
 * bootdelay > 0: count down by second for pressing hot key
 */
static inline void cli_loop(int bootdelay)
{
	if (abortboot_single_key(bootdelay))
		cli_simple_loop();
}

#define test_cmd(cmd)	strcmp(argv[0], cmd) == 0
unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base);
int plat_cli_cmd_process(int flag, int argc, char *const argv[]);

#endif
