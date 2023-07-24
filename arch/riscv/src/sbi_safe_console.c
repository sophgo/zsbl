#include <spinlock.h>
#include <stdio.h>

#define va_start(v, l) __builtin_va_start((v), l)
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg
typedef __builtin_va_list va_list;

static spinlock_t console_out_lock	       = SPIN_LOCK_INITIALIZER;

int thread_safe_printf(const char *format, ...)
{
	va_list args;
	int retval;

	spin_lock(&console_out_lock);
	va_start(args, format);
	retval = vprintf(format, args);
	va_end(args);
	spin_unlock(&console_out_lock);

	return retval;
}
