#include <wchar.h>

int wcscmp(const wchar_t *s1, const wchar_t *s2)
{
	wchar_t c;

	while (*s1) {
		c = *s1 - *s2;
		if (c)
			return c;

		++s1;
		++s2;
	}

	return *s1 - *s2;
}
