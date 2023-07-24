#include <stdio.h>

#define va_start(v, l) __builtin_va_start((v), l)
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg
typedef __builtin_va_list va_list;

int fprintf(FILE *stream, const char *format, ...)
{
        va_list args;
	int retval;

	va_start(args, format);
	retval = vprintf(format, args);
	va_end(args);

	return retval;
}
