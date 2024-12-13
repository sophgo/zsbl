#ifndef __CTYPE_H__
#define __CTYPE_H__

static inline int isxdigit(int c)
{
	return ((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'));
}

#endif
