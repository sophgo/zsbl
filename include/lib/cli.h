#ifndef __CLI_H__
#define __CLI_H__

#include <lib/console.h>

unsigned long cli_loop(unsigned int wait);
int cli_exit(void);
int cli_add_command(const char *name, command_callback fn, void *priv);

struct cli_command {
	char *name;
	command_callback callback;
	void *priv;
};

#define cli_command(nm, cb)			\
	static const struct cli_command			\
	__attribute__((section(".cli_command"), used))	\
	__cli_command_ ##nm = {			\
		.name = #nm,				\
		.callback = cb,			\
		.priv = NULL				\
	}

#endif
