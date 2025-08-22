#include <spinlock.h>
#include <common/common.h>
#include <string.h>
#include <stdbool.h>

#define CONSOLE_TBUF_MAX 256

static char console_tbuf[CONSOLE_TBUF_MAX];
static size_t console_tbuf_len;
static spinlock_t console_out_lock	       = SPIN_LOCK_INITIALIZER;

static void panic(const char *format, ...);

bool isprintable(char c)
{
	if (((31 < c) && (c < 127)) || (c == '\f') || (c == '\r') ||
	    (c == '\n') || (c == '\t')) {
		return true;
	}
	return false;
}

int putchar(int c)
{
	stdio_output(c);
	return c;
}

static unsigned long nputs(const char *str, unsigned long len)
{
	unsigned long i;

	for (i = 0; i < len; i++)
		putchar(str[i]);

	return len;
}

static void nputs_all(const char *str, unsigned long len)
{
	unsigned long p = 0;

	while (p < len)
		p += nputs(&str[p], len - p);
}

int puts(const char *str)
{
	unsigned long len = strlen(str);

	spin_lock(&console_out_lock);
	nputs_all(str, len);
	stdio_output('\n');
	spin_unlock(&console_out_lock);

	return len;
}

void gets(char *s, int maxwidth, char endchar)
{
	int ch;
	char *retval = s;

	while ((ch = stdio_input()) != endchar && ch >= 0 && maxwidth > 1) {
		*retval = (char)ch;
		retval++;
		maxwidth--;
	}
	*retval = '\0';
}

unsigned long ngets(char *str, unsigned long len)
{
	int ch;
	unsigned long i;

	for (i = 0; i < len; i++) {
		ch = stdio_input();
		if (ch < 0)
			break;
		str[i] = ch;
	}

	return i;
}

#define PAD_RIGHT 1
#define PAD_ZERO 2
#define PAD_ALTERNATE 4
#define PRINT_BUF_LEN 64

static void printc(char **out, size_t *out_len, char ch)
{
	if (!out) {
		stdio_output(ch);
		return;
	}

	/*
	 * The *printf entry point functions have enforced that (*out) can
	 * only be null when out_len is non-null and its value is zero.
	 */
	if (!out_len || *out_len > 1) {
		*(*out)++ = ch;
		**out = '\0';
	}

	if (out_len && *out_len > 0)
		--(*out_len);
}

static int prints(char **out, size_t *out_len, const char *string, int width,
		  int flags)
{
	int pc	     = 0;
	char padchar = ' ';

	if (width > 0) {
		int len = 0;
		const char *ptr;
		for (ptr = string; *ptr; ++ptr)
			++len;
		if (len >= width)
			width = 0;
		else
			width -= len;
		if (flags & PAD_ZERO)
			padchar = '0';
	}
	if (!(flags & PAD_RIGHT)) {
		for (; width > 0; --width) {
			printc(out, out_len, padchar);
			++pc;
		}
	}
	for (; *string; ++string) {
		printc(out, out_len, *string);
		++pc;
	}
	for (; width > 0; --width) {
		printc(out, out_len, padchar);
		++pc;
	}

	return pc;
}

static int printi(char **out, size_t *out_len, long long i, int b, int sg,
		  int width, int flags, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	char *s;
	int neg = 0, pc = 0;
	uint64_t t;
	unsigned long long u = i;

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u   = -i;
	}

	s  = print_buf + PRINT_BUF_LEN - 1;
	*s = '\0';

	if (!u) {
		*--s = '0';
	} else {
		while (u) {
			t = u % b;
			u = u / b;
			if (t >= 10)
				t += letbase - '0' - 10;
			*--s = t + '0';
		}
	}

	if (flags & PAD_ALTERNATE) {
		if ((b == 16) && (letbase == 'A')) {
			*--s = 'X';
		} else if ((b == 16) && (letbase == 'a')) {
			*--s = 'x';
		}
		*--s = '0';
	}

	if (neg) {
		if (width && (flags & PAD_ZERO)) {
			printc(out, out_len, '-');
			++pc;
			--width;
		} else {
			*--s = '-';
		}
	}

	return pc + prints(out, out_len, s, width, flags);
}

static int print(char **out, size_t *out_len, const char *format, va_list args)
{
	int width, flags, pc = 0;
	char scr[2], *tout;
	bool use_tbuf = (!out) ? true : false;
	unsigned long long tmp;

	/*
	 * The console_tbuf is protected by console_out_lock and
	 * print() is always called with console_out_lock held
	 * when out == NULL.
	 */
	if (use_tbuf) {
		console_tbuf_len = CONSOLE_TBUF_MAX;
		tout = console_tbuf;
		out = &tout;
		out_len = &console_tbuf_len;
	}

	for (; *format != 0; ++format) {
		if (use_tbuf && !console_tbuf_len) {
			nputs_all(console_tbuf, CONSOLE_TBUF_MAX);
			console_tbuf_len = CONSOLE_TBUF_MAX;
			tout = console_tbuf;
		}

		if (*format == '%') {
			++format;
			width = flags = 0;
			if (*format == '\0')
				break;
			if (*format == '%')
				goto literal;
			/* Get flags */
			if (*format == '-') {
				++format;
				flags = PAD_RIGHT;
			}
			if (*format == '#') {
				++format;
				flags |= PAD_ALTERNATE;
			}
			if (*format == '*') {
				++format;
				width = va_arg(args, int);
			}
			while (*format == '0') {
				++format;
				flags |= PAD_ZERO;
			}
			/* Get width */
			for (; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if (*format == 's') {
				char *s = va_arg(args, char *);
				pc += prints(out, out_len, s ? s : "(null)",
					     width, flags);
				continue;
			}
			if ((*format == 'd') || (*format == 'i')) {
				pc += printi(out, out_len, va_arg(args, int),
					     10, 1, width, flags, '0');
				continue;
			}
			if (*format == 'x') {
				pc += printi(out, out_len,
					     va_arg(args, unsigned int), 16, 0,
					     width, flags, 'a');
				continue;
			}
			if (*format == 'X') {
				pc += printi(out, out_len,
					     va_arg(args, unsigned int), 16, 0,
					     width, flags, 'A');
				continue;
			}
			if (*format == 'u') {
				pc += printi(out, out_len,
					     va_arg(args, unsigned int), 10, 0,
					     width, flags, 'a');
				continue;
			}
			if (*format == 'p') {
				pc += printi(out, out_len,
					     va_arg(args, unsigned long), 16, 0,
					     width, flags, 'a');
				continue;
			}
			if (*format == 'P') {
				pc += printi(out, out_len,
					     va_arg(args, unsigned long), 16, 0,
					     width, flags, 'A');
				continue;
			}
			if (*format == 'l' && *(format + 1) == 'l') {
				tmp = va_arg(args, unsigned long long);
				if (*(format + 2) == 'u') {
					format += 2;
					pc += printi(out, out_len, tmp, 10, 0,
						     width, flags, 'a');
				} else if (*(format + 2) == 'x') {
					format += 2;
					pc += printi(out, out_len, tmp, 16, 0,
						     width, flags, 'a');
				} else if (*(format + 2) == 'X') {
					format += 2;
					pc += printi(out, out_len, tmp, 16, 0,
						     width, flags, 'A');
				} else if (*(format + 2) == 'd') {
					format += 2;
					pc += printi(out, out_len, tmp, 10, 1,
						     width, flags, '0');
				} else {
					format += 1;
					pc += printi(out, out_len, tmp, 10, 1,
						     width, flags, '0');
				}
				continue;
			} else if (*format == 'l') {
				if (*(format + 1) == 'u') {
					format += 1;
					pc += printi(
						out, out_len,
						va_arg(args, unsigned long), 10,
						0, width, flags, 'a');
				} else if (*(format + 1) == 'x') {
					format += 1;
					pc += printi(
						out, out_len,
						va_arg(args, unsigned long), 16,
						0, width, flags, 'a');
				} else if (*(format + 1) == 'X') {
					format += 1;
					pc += printi(
						out, out_len,
						va_arg(args, unsigned long), 16,
						0, width, flags, 'A');
				} else if (*(format + 1) == 'd') {
					format += 1;
					pc += printi(out, out_len,
						     va_arg(args, long), 10, 1,
						     width, flags, '0');
				} else {
					pc += printi(out, out_len,
						     va_arg(args, long), 10, 1,
						     width, flags, '0');
				}
			}
			if (*format == 'c') {
				/* char are converted to int then pushed on the stack */
				scr[0] = va_arg(args, int);
				scr[1] = '\0';
				pc += prints(out, out_len, scr, width, flags);
				continue;
			}
		} else {
literal:
			printc(out, out_len, *format);
			++pc;
		}
	}

	if (use_tbuf && console_tbuf_len < CONSOLE_TBUF_MAX)
		nputs_all(console_tbuf, CONSOLE_TBUF_MAX - console_tbuf_len);

	return pc;
}

int sprintf(char *out, const char *format, ...)
{
	va_list args;
	int retval;

	if (!out)
		panic("sprintf called with NULL output string\n");

	va_start(args, format);
	retval = print(&out, NULL, format, args);
	va_end(args);

	return retval;
}

int snprintf(char *out, size_t out_sz, const char *format, ...)
{
	va_list args;
	int retval;

	if (!out && out_sz != 0)
		panic("snprintf called with NULL output string and "
			  "output size is not zero\n");

	va_start(args, format);
	retval = print(&out, &out_sz, format, args);
	va_end(args);

	return retval;
}

int printf(const char *format, ...)
{
	va_list args;
	int retval;

	spin_lock(&console_out_lock);
	va_start(args, format);
	retval = print(NULL, NULL, format, args);
	va_end(args);
	spin_unlock(&console_out_lock);

	return retval;
}

static void panic(const char *format, ...)
{
	va_list args;

	spin_lock(&console_out_lock);
	va_start(args, format);
	print(NULL, NULL, format, args);
	va_end(args);
	spin_unlock(&console_out_lock);

	while (1)
		;
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	return print(&str, &size, format, ap);
}

FILE _stdin, _stdout, _stderr;
FILE *stdin = &_stdin;
FILE *stdout = &_stdout;
FILE *stderr = &_stderr;

int fputs(const char *s, FILE *stream)
{
	return puts(s);
}

int fputc(int c, FILE *stream)
{
	return putchar(c);
}

int putc(int c, FILE *stream)
{
	stdio_output(c);
	return c;
}

