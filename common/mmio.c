#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <arch.h>
#include <common/common.h>
#include <common/mmio.h>

int is_normal_memory(const void *addr);

void *memcpy_to_io(void *dst, const void *src, size_t n)
{
	unsigned long s = (unsigned long)src;
	unsigned long d = (unsigned long)dst;

	while (n) {
		if ((d & (16 - 1)) == 0 && n >= 16) {
			*(volatile uint128_t *)d = *(volatile uint128_t *)s;
			d += 16;
			s += 16;
			n -= 16;
		} else if ((d & (8 - 1)) == 0 && n >= 8) {
			*(volatile uint64_t *)d = *(volatile uint64_t *)s;
			d += 8;
			s += 8;
			n -= 8;

		} else if ((d & (4 - 1)) == 0 && n >= 4) {
			*(volatile uint32_t *)d = *(volatile uint32_t *)s;
			d += 4;
			s += 4;
			n -= 4;
		} else {
			*(volatile uint8_t *)d = *(volatile uint8_t *)s;
			d += 1;
			s += 1;
			n -= 1;
		}
	}
	return dst;
}

void *memcpy_from_io(void *dst, const void *src, size_t n)
{
	unsigned long s = (unsigned long)src;
	unsigned long d = (unsigned long)dst;

	while (n) {
		if ((s & (16 - 1)) == 0 && n >= 16) {
			*(volatile uint128_t *)d = *(volatile uint128_t *)s;
			d += 16;
			s += 16;
			n -= 16;
		} else if ((s & (8 - 1)) == 0 && n >= 8) {
			*(volatile uint64_t *)d = *(volatile uint64_t *)s;
			d += 8;
			s += 8;
			n -= 8;

		} else if ((s & (4 - 1)) == 0 && n >= 4) {
			*(volatile uint32_t *)d = *(volatile uint32_t *)s;
			d += 4;
			s += 4;
			n -= 4;
		} else {
			*(volatile uint8_t *)d = *(volatile uint8_t *)s;
			d += 1;
			s += 1;
			n -= 1;
		}
	}
	return dst;
}

void *memcpy_between_io(void *dst, const void *src, size_t n)
{
	size_t i;

	for (i = 0; i < n; ++i)
		((volatile uint8_t *)dst)[i] = ((volatile uint8_t *)src)[i];

	return dst;
}

void *memmove_between_io(void *dst, const void *src, size_t n)
{
	unsigned long s = (unsigned long)src;
	unsigned long d = (unsigned long)dst;
	size_t i;

	if (s >= d || (s + n) < d) {
		for (i = 0; i < n; ++i)
			((volatile uint8_t *)dst)[i] = ((volatile uint8_t *)src)[i];
	}
	else {
		for (i = 0; i < n; ++i)
			((volatile uint8_t *)dst)[n - 1 - i] = ((volatile uint8_t *)src)[n - 1 - i];
	}

	return dst;
}

void *memset_io(void *b, int c, size_t n)
{
	unsigned long s = (unsigned long)b;
	uint128_t _tmp;
	unsigned long tmp;

	tmp = (unsigned long)&_tmp;
	memset_normal(&_tmp, c, sizeof(_tmp));

	while (n) {
		if ((s & (16 - 1)) == 0 && n >= 16) {
			*(volatile uint128_t *)s = _tmp;
			s += 16;
			n -= 16;
		} else if ((s & (8 - 1)) == 0 && n >= 8) {
			*(volatile uint64_t *)s = *((volatile uint64_t *)tmp);
			s += 8;
			n -= 8;

		} else if ((s & (4 - 1)) == 0 && n >= 4) {
			*(volatile uint32_t *)s = *((volatile uint32_t *)tmp);
			s += 4;
			n -= 4;
		} else {
			*(volatile uint8_t *)s = c;
			s += 1;
			n -= 1;
		}
	}
	return b;
}

/* overwrite libc functions */
void *memcpy(void *dst, const void *src, size_t n)
{
	if (is_normal_memory(src) && is_normal_memory(dst))
		memcpy_between_normal(dst, src, n);
	else if (is_normal_memory(src) && !is_normal_memory(dst))
		memcpy_to_io(dst, src, n);
	else if (!is_normal_memory(src) && is_normal_memory(dst))
		memcpy_from_io(dst, src, n);
	else
		memcpy_between_io(dst, src, n);

	return dst;
}

void *memmove(void *dst, const void *src, size_t n)
{
	if (is_normal_memory(src) && is_normal_memory(dst))
		return memmove_between_normal(dst, src, n);
	else if (is_normal_memory(src) && !is_normal_memory(dst))
		memcpy_to_io(dst, src, n);
	else if (!is_normal_memory(src) && is_normal_memory(dst))
		memcpy_from_io(dst, src, n);
	else
		memmove_between_io(dst, src, n);

	return dst;
}

void *memset(void *s, int c, size_t n)
{
	if (is_normal_memory(s))
		return memset_normal(s, c, n);
	else
		return memset_io(s, c, n);
}

