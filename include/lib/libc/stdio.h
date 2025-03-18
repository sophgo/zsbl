#ifndef __STDIO_H__
#define __STDIO_H__

#include <stdint.h>
#include <stddef.h>

#define EOF	-1

typedef struct {
	void *__unused;
} FILE;

extern FILE *stdin, *stdout, *stderr;

#define va_start(v, l) __builtin_va_start((v), l)
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg
#define va_copy(d, s)	__builtin_va_copy(d, s)
typedef __builtin_va_list va_list;

int putchar(int c);

int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);

int vprintf(const char *format, va_list ap);
int vfprintf(FILE *stream, const char *format, va_list ap);
int vsprintf(char *str, const char *format, va_list ap);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);

int sscanf(const char *str, const char *format, ...);
char *fgets(char *s, int size, FILE *stream);
FILE *fopen(const char *pathname, const char *mode);
int fclose(FILE *stream);

int fputs(const char *s, FILE *stream);
int fputc(int c, FILE *stream);
int putc(int c, FILE *stream);

#endif
