#ifndef __MMIO_H__
#define __MMIO_H__

#include <stddef.h>

void *memcpy_to_io(void *dst, const void *src, size_t n);
void *memcpy_from_io(void *dst, const void *src, size_t n);
void *memcpy_between_io(void *dst, const void *src, size_t n);
void *memcpy_between_normal(void *dst, const void *src, size_t n);
void *memmove_between_io(void *dst, const void *src, size_t n);
void *memmove_between_normal(void *dest, const void *src, size_t n);
void *memset_normal(void *s, int c, size_t n);
void *memset_io(void *s, int c, size_t n);

#endif
