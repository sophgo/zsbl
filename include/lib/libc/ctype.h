#ifndef __CTYPE_H__
#define __CTYPE_H__

#include <stdbool.h>

static inline int isxdigit(int c)
{
	return ((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'));
}

static inline int isprint(int c)
{
	if (((31 < c) && (c < 127)) || (c == '\f') || (c == '\r') ||
	    (c == '\n') || (c == '\t')) {
		return true;
	}
	return false;
}

#endif
