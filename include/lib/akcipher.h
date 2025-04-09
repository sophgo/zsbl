#ifndef __VERIFY_H__
#define __VERIFY_H__

#define PUBKEY_SIZE	300

#include <string.h>
#include <stdint.h>

enum ALG {
	RSA = 0,
	SM2
};

struct sm2_parameter {
	unsigned char pubx[32];
	unsigned char puby[32];
};

struct rsa_parameter {
	unsigned char modulus[256];
	unsigned char exponent[256];
};

struct akcipher_ctx {
	int alg;
	int key_len;
	/* key */
	union {
		struct sm2_parameter sm2;
		struct rsa_parameter rsa;
	} key;
};

int akcipher_invoke_key(struct akcipher_ctx *ctx,
			void *der, int der_len, void *der_hash, int hash_len);
int akcipher_verify(struct akcipher_ctx *ctx, void *msg, void *sig, unsigned long len);

#endif
