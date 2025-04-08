#ifndef __VERIFY_H__
#define __VERIFY_H__

#define PUBKEY_SIZE	300

#include <string.h>
#include <stdint.h>

struct der_info {
	uint8_t pubkey[PUBKEY_SIZE];
	int alg;
	uint32_t key_size;
	uint32_t m_len;
};

enum ALG {
	RSA = 0,
	SM2
};

int secure_boot();
void read_pubkey_hash(uint32_t *pubkey_dig);
int parse_public_key(uint8_t *data, uint32_t len, uint32_t *m_len, int *flag) ;
int akcipher_verify(struct der_info *der_info, unsigned char *msg,
		    unsigned char *sig, unsigned long len);
int pubkey_verify(unsigned char *pubkey_hash, struct der_info *der_info, unsigned long len);

#endif
