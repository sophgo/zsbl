#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>

/* stdinput:
 *	return none-nagtive number means input character
 *	return nagtive number means no input character in buffer
 * stdout:
 *	output character to stdout
 */
void register_stdio(int (*stdinput)(void), void (*stdoutput)(int));
void unregister_stdio(void);

int stdio_input(void);
void stdio_output(int ch);
int stdout_ready(void);

/* log system */
#ifdef DEBUG
#define pr_debug(fmt, ...)	printf(fmt, ##__VA_ARGS__)
#else
#define pr_debug(fmt, ...)	do {} while (0)
#endif
#define pr_info(fmt, ...)	printf(fmt, ##__VA_ARGS__)
#define pr_warn(fmt, ...)	printf(fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...)	printf(fmt, ##__VA_ARGS__)

#define pr_line()		do { pr_info("%s: %d (%s)\n", __func__, __LINE__, __FILE__); } while (0)

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof((a)[0]))
#define ROUND_UP(x, align)	(((x) + ((align) - 1)) & ~((align) - 1))

#endif
