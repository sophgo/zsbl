#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

/*
  Provides strcmp for the completeness of supporting string functions.
  it is not recommended to use strcmp() but use strncmp instead.
*/
int strcmp(const char *a, const char *b)
{
	/* search first diff or end of string */
	for (; *a == *b && *a != '\0'; a++, b++)
		;

	return *a - *b;
}

int strncmp(const char *a, const char *b, size_t count)
{
	/* search first diff or end of string */
	for (; count > 0 && *a == *b && *a != '\0'; a++, b++, count--)
		;

	/* No difference till the end */
	if (!count)
		return 0;

	return *a - *b;
}

size_t strlen(const char *str)
{
	unsigned long ret = 0;

	while (*str != '\0') {
		ret++;
		str++;
	}

	return ret;
}

size_t strnlen(const char *str, size_t count)
{
	unsigned long ret = 0;

	while (*str != '\0' && ret < count) {
		ret++;
		str++;
	}

	return ret;
}

char *strcpy(char *dest, const char *src)
{
	char *ret = dest;

	while (*src != '\0') {
		*dest++ = *src++;
	}

	return ret;
}

char *strncpy(char *dest, const char *src, size_t count)
{
	char *ret = dest;

	while (count-- && *src != '\0') {
		*dest++ = *src++;
	}

	return ret;
}

char *strchr(const char *s, int c)
{
	while (*s != '\0' && *s != (char)c)
		s++;

	if (*s == '\0')
		return NULL;
	else
		return (char *)s;
}

char *strrchr(const char *s, int c)
{
	const char *last = s + strlen(s);

	while (last > s && *last != (char)c)
		last--;

	if (*last != (char)c)
		return NULL;
	else
		return (char *)last;
}
void *memset(void *s, int c, size_t count)
{
	volatile char *temp = s;

	while (count > 0) {
		count--;
		*temp++ = c;
	}

	return s;
}

void *memcpy(void *dest, const void *src, size_t count)
{
	char *temp1	  = dest;
	const char *temp2 = src;

	while (count > 0) {
		*temp1++ = *temp2++;
		count--;
	}

	return dest;
}

void *memmove(void *dest, const void *src, size_t count)
{
	char *temp1	  = (char *)dest;
	const char *temp2 = (char *)src;

	if (src == dest)
		return dest;

	if (dest < src) {
		while (count > 0) {
			*temp1++ = *temp2++;
			count--;
		}
	} else {
		temp1 = (char *)dest + count - 1;
		temp2 = (char *)src + count - 1;

		while (count > 0) {
			*temp1-- = *temp2--;
			count--;
		}
	}

	return dest;
}

int memcmp(const void *s1, const void *s2, size_t count)
{
	const char *temp1 = s1;
	const char *temp2 = s2;

	for (; count > 0 && (*temp1 == *temp2); count--) {
		temp1++;
		temp2++;
	}

	if (count > 0)
		return *(unsigned char *)temp1 - *(unsigned char *)temp2;
	else
		return 0;
}

void *memchr(const void *s, int c, size_t count)
{
	const unsigned char *temp = s;

	while (count > 0) {
		if ((unsigned char)c == *temp++) {
			return (void *)(temp - 1);
		}
		count--;
	}

	return NULL;
}

unsigned long strtoul(const char *__restrict nptr,
		      char **__restrict endptr, int base)
{
	register const unsigned char *s = (const unsigned char *)nptr;
	register unsigned long acc;
	register int c;
	register unsigned long cutoff;
	register int neg = 0, any, cutlim;

	/*
	 * See strtol for comments as to the logic used.
	 */
	c = *s++;
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else if (c == '+')
		c = *s++;
	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;
	cutoff = (unsigned long)ULONG_MAX / (unsigned long)base;
	cutlim = (unsigned long)ULONG_MAX % (unsigned long)base;
	for (acc = 0, any = 0;; c = *s++) {
		if (c >= '0' && c <= '9')
			c -= '0';
		else if (c >= 'A' && c <= 'Z')
			c -= 'A' - 10;
		else if (c >= 'a' && c <= 'z')
			c -= 'a' - 10;
		else
			break;
		if (c >= base)
			break;
               if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = ULONG_MAX;
	} else if (neg)
		acc = -acc;
	if (endptr != 0)
		*endptr = (char *) (any ? (char *)s - 1 : nptr);
	return (acc);
}

char *strdup(const char   *str)
{
	size_t len = strlen (str) + 1;
	char *copy = malloc(len);
	if (copy)
	{
		memcpy (copy, str, len);
	}
	return copy;
}

char *strstr (const char *hs, const char *ne)
{
	size_t i;
	int c = ne[0];

	if (c == 0)
		return (char*)hs;

	for ( ; hs[0] != '\0'; hs++)
	{
		if (hs[0] != c)
			continue;
		for (i = 1; ne[i] != 0; i++)
		if (hs[i] != ne[i])
			break;
		if (ne[i] == '\0')
			return (char*)hs;
	}

	return NULL;
}

int insspace(char c) __attribute__((alias("my_isspace")));

int my_isspace(char c)
{
	if(c =='\t'|| c =='\n'|| c ==' ')
		return 1;
	else
		return 0;
}

int zsbl_isspace(int c)
{
	if(c =='\t'|| c =='\n'|| c ==' ')
		return 1;
	else
		return 0;
}

char *strcat (char *__restrict s1,
	const char *__restrict s2)
{
	char *s = s1;

	while (*s1)
		s1++;

	while (*s2)
		*s1++ = *s2++;

	return s;
}

