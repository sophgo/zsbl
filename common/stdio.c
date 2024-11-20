#include <stdlib.h>
#include <stdio.h>

static int (*stdinput_func)(void);
static void (*stdout_func)(int);

void register_stdio(int (*stdinput)(void), void (*stdoutput)(int))
{
	stdinput_func = stdinput;
	stdout_func = stdoutput;
}

void unregister_stdio(void)
{
	stdinput_func = NULL;
	stdout_func = NULL;
}

int stdio_input(void)
{
	if (!stdinput_func)
		return EOF;

	return stdinput_func();
}

int stdio_output(int ch)
{
	if (!stdout_func)
		return EOF;

	if (ch == '\n')
		stdout_func('\r');

	stdout_func(ch);

	return ch;
}

int stdout_ready(void)
{
	return stdout_func != NULL;
}

