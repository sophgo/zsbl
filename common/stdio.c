#include <stdlib.h>

static int (*stdinput_func)(void);
static void (*stdout_func)(int);

void register_stdio(int (*stdinput)(void), void (*stdoutput)(int))
{
	stdinput_func = stdinput;
	stdout_func = stdoutput;
}

int stdio_input(void)
{
	if (!stdinput_func)
		return -1;

	return stdinput_func();
}

void stdio_output(int ch)
{
	if (!stdout_func)
		return;

	if (ch == '\n')
		stdout_func('\r');

	stdout_func(ch);
}

int stdout_ready(void)
{
	return stdout_func != NULL;
}
