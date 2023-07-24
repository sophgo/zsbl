#ifndef __ASSERT_H__
#define __ASSERT_H__

# define assert(__e) ((__e) ? (void)0 : printf("%s:%s assert\n", __FUNCTION__, __LINE__))

#endif