#ifndef _ALG_H_
#define _ALG_H_

#include <lib/list.h>
#define ALG_NAME_MAX 16

struct akcipher_alg;

struct base_alg {
	struct list_head list_head;
	char name[ALG_NAME_MAX];
};

struct rsa_ctx {
	unsigned int size;
	unsigned char *m, *e, *x, *y;
	unsigned char *r_inv,*mp,*r_sqr;
};

struct ecc_ctx {
	unsigned int size;
	unsigned char *prime, *e, *out;
	unsigned char *A, *B, *Gx, *Gy, *order;

	void * key, *randomkey;
	void * pubx, *puby, *r, *s, *msg;

	unsigned char *r_inv, *mp, *r_sqr; 
	unsigned long cofactor;

	//unsigned long msglen;
	//unsigned char *encmsg;
	//unsigned long encmsglen;
};

union akcipher_param {
	struct rsa_ctx rsa_ctx;
	struct ecc_ctx sm2_ctx;
};

struct akcipher_ops {
	int (*verify)(struct akcipher_alg *alg, union akcipher_param *param);
	void (*release)(struct akcipher_alg *alg);
};

struct akcipher_alg {
	char name[ALG_NAME_MAX];
	struct base_alg base;
	struct akcipher_ops *ops;
	void *priv;
};

int alg_verify(struct akcipher_alg *alg, union akcipher_param *param);
void alg_release(struct akcipher_alg *alg);
struct akcipher_alg *alg_find_by_name(const char *name);

int alg_register(struct akcipher_alg *alg);
void alg_unregister(struct akcipher_alg *alg);
struct akcipher_alg *alg_alloc(void);

#endif
