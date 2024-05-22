#ifndef __ASSERT_H__
#define __ASSERT_H__

#include <stdio.h>

# define assert(__e) ((__e) ? (void)0 : printf("%s:%d assert\n", __FUNCTION__, __LINE__))

#endif
