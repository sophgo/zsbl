#ifndef __SM3_H__
#define __SM3_H__

#include <lib/hash/tomcrypt.h>

void do_sm3(unsigned char *msg, unsigned char *res, unsigned long len);
void sm3_init(hash_state * md);
void sm3_process(hash_state * md, const unsigned char *data, unsigned long data_len);
void sm3_done(hash_state * md, unsigned char *digest);

#endif
