#ifndef __STDDEF_H__
#define __STDDEF_H__

#include <stdint.h>

#define NULL	((void *)0)

typedef long int ssize_t;
typedef unsigned long size_t ;

typedef size_t uintptr_t;
typedef uint16_t wchar_t;

typedef signed long intptr_t;
typedef intptr_t ptrdiff_t;

#endif
