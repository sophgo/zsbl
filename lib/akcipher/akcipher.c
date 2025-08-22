#include <driver/pka/pka.h>
#include <lib/hash/sha256.h>
#include <lib/hash/sm3.h>
#include <lib/akcipher.h>
#include <common/common.h>
#include <string.h>
#include <stdint.h>
#include <driver/alg.h>

#define RSA_OID_SIZE 13
#define SM2_OID_SIZE 20

static const uint8_t rsa_oid[RSA_OID_SIZE] = {
	0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
	0x01, 0x01, 0x01, 0x05, 0x00
};

static const uint8_t sm2_oid[SM2_OID_SIZE] = {
	0x06, 0x08, 0x2a, 0x81, 0x1c, 0xcf, 0x55, 0x01,
	0x82, 0x2d, 0x06, 0x08, 0x2a, 0x81, 0x1c, 0xcf,
	0x55, 0x01, 0x82, 0x2d
};

static bool cmp_alg_oid(const uint8_t *data, const uint8_t *oid, uint32_t len)
{
	int i;

	for (i = 0; i < len; i++) {
		if(data[i] != oid[i])
			return 0;
	}

	return 1;
}

static uint32_t parse_seq_length(uint8_t *data, uint32_t *ctx_len)
{
	uint32_t offset = 0;
	int i;
	uint32_t length_bytes;

	if (data[offset] & 0x80) {
		length_bytes = data[offset++] & 0x7F;
		if (!ctx_len)
			return offset + length_bytes;
		for (i = 0; i < length_bytes; i++)
			*ctx_len = (*ctx_len << 8) | data[offset++];
	} else {
		if (!ctx_len)
			return offset + 1;
		*ctx_len = data[offset++];
	}
	return offset;
}

static int parse_alg_oid(uint8_t *data, uint32_t len, int *flag)
{
	switch (len) {
	case RSA_OID_SIZE:
		if (cmp_alg_oid(data, rsa_oid, len)) {
			*flag = RSA;
			break;
		}
		return -1;

	case SM2_OID_SIZE:
		if (cmp_alg_oid(data, sm2_oid, len)) {
			*flag = SM2;
			break;
		}
		return -1;

	default:
		return -1;
	}

	return 0;
}

static int extract_rsa_key(void *param, uint8_t *data, uint32_t *modulus_length)
{
	uint32_t exponent_length = 0;
	uint32_t offset = 0;
	int i;
	struct rsa_parameter *rsa = param;

	*modulus_length = 0;

	/* checkout sequence flag */
	if (data[offset++] != 0x30) {
		pr_err("failed: not a sequence!\n");
		return -1;
	}

	/* skip sequence length */
	offset += parse_seq_length(data + offset, NULL);

	/* parse m head */
	if (data[offset++] != 0x02) {
		pr_err("failed: not a valid module head!\n");
		return -1;
	}

	/* read m length */
	offset += parse_seq_length(data + offset, modulus_length);

	while (!data[offset]) {
		offset++;
		(*modulus_length)--;
	}

	/* save m */
	pr_debug("modules (n):\n");
	for (i = 0; i < (*modulus_length); i++) {
		rsa->modulus[i] = data[offset + i];
		pr_debug("%02X", data[offset + i]);
		pr_debug((i + 1) % 16 ? "" : "\n");
	}

	offset += *modulus_length;

	/* parse e head */
	if (data[offset++] != 0x02) {
		pr_err("failed: not a valid exponent!\n");
		return -1;
	}

	/* read e length */
	offset += parse_seq_length(data + offset, &exponent_length);

	/* save e */
	pr_debug("exponent:\n");
	for (i = 0; i < 256 - exponent_length; i++) {
		rsa->exponent[i] = 0;
		pr_debug("%02X", rsa->exponent[i]);
		pr_debug((i + 1) % 16 ? "" : "\n");
	}

	for (; i < 256 ; i++) {
		rsa->exponent[i] = data[offset + i + exponent_length - 256];
		pr_debug("%02X", rsa->exponent[i]);
		pr_debug((i + 1) % 16 ? "" : "\n");
	}

	return 0;
}

static int extract_sm2_key(void *param, uint8_t *data, uint32_t *modulus_length)
{
	uint32_t offset = 0;
	struct sm2_parameter *sm2;

	sm2 = param;

	if (data[offset++] != 0x04) {
		pr_err("failed: sm2 key is not uncompressed!\n");
		return -1;
	}

	/* save x */
	pr_debug("sm2 x:\n");
	for (int i = 0; i < 32; i++) {
		sm2->pubx[i] = data[offset++];
		pr_debug("%02X", sm2->pubx[i]);
		pr_debug((i + 1) % 16 ? "" : "\n");
	}
	pr_debug("\n");

	/* save y */
	pr_debug("sm2 y:\n");
	for (int i = 0; i < 32; i++) {
		sm2->puby[i] = data[offset++];
		pr_debug("%02X", sm2->puby[i]);
		pr_debug((i + 1) % 16 ? "" : "\n");
	}
	pr_debug("\n");

	*modulus_length = 32;

	return 0;
}

static int (*extract_keys[])(void *, uint8_t *, uint32_t *) = {
	[RSA] = extract_rsa_key,
	[SM2] = extract_sm2_key,
};

static int pubkey_verify(uint8_t *der_hash, int alg, void *der, unsigned long der_len)
{
	uint8_t hmsg[32];

	switch (alg) {
	case RSA:
		do_sha256(der, hmsg, der_len);

		return memcmp(der_hash, hmsg, 32);

	case SM2:
		do_sm3(der, hmsg, der_len);

		return memcmp(der_hash, hmsg, 32);
	default:
		return -1;
	}
}

int akcipher_invoke_key(struct akcipher_ctx *ctx,
			void *der, int der_len,
			void *der_hash, int hash_len)
{
	uint32_t oid_len = 0;
	uint32_t offset = 0;
	int ret = 0;
	int alg;
	uint32_t key_len;

	/* check the pubkey length */
	if (der_len < 15) {
		pr_err("failed: data len is too short!\n");
		return -1;
	}

	/* check if the sequence */
	if (((uint8_t *)der)[offset++] != 0x30) {
		pr_err("failed: data type is not sequence!\n");
		return -1;
	}

	/* skip the length byte */
	offset += parse_seq_length(der + offset, NULL);

	/* check if Algorithm Identifier */
	if (((uint8_t *)der)[offset++] != 0x30) {
		pr_err("failed: data type is not sequence!\n");
		return -1;
	}

	/* record Algorithm Identifier length */
	offset += parse_seq_length(der + offset, &oid_len);

	ret = parse_alg_oid(der + offset, oid_len, &alg);
	if (ret) {
		pr_err("failed: alg oid is not correct!\n");
		return -1;
	}

	ctx->alg = alg;

	offset += oid_len;

	/* skip fixed context */
	if (((uint8_t *)der)[offset++] != 0x03) {
		pr_err("failed: data type is not sequence!\n");
		return -1;
	}

	/* skip BIT STRING , which has 0 */
	offset += parse_seq_length(der + offset, NULL) + 1;

	/* select an alg */
	ret = extract_keys[alg](&ctx->key, der + offset, &key_len);
	ctx->key_len = key_len;

	/* verify public key */
	return pubkey_verify(der_hash, alg, der, der_len);
}

static int get_s_r(uint8_t *sig, uint8_t *s, uint8_t *r)
{
	uint32_t i, pos;
	uint32_t s_len = 0, r_len = 0, pad = 0;

	if (sig[0] != 0x30) {
		pr_err("read sig file error\n");
		return -1;
	}

	if (sig[0] == 0x30 && sig[2] == 0x02) {
		r_len = sig[3];

		if(r_len == 0x21)
			pad = 1;

		pr_debug("r:\n");
		for (i = 0; i < r_len - pad; i++) {
			r[i] = sig[4 + i + pad];
			pr_debug("%02X", r[i]);
			pr_debug((i + 1) % 16 ? "" : "\n");
		}
		pr_debug("\n");
	}

	pos = 4 + r_len;

	pad = 0;
	if (sig[pos] == 0x02) {
		s_len = sig[pos + 1];

		if (s_len == 0x21)
			pad = 1;
		pos += 2;

		pr_debug("s:\n");
		for (i = 0; i < s_len - pad; i++) {
			s[i] = sig[pos + i + pad];
			pr_debug("%02X", s[i]);
			pr_debug((i + 1) % 16 ? "" : "\n");
		}
		pr_debug("\n");
	}

	return  0;
}

static int rsa_verify(struct akcipher_ctx *ctx, union akcipher_param *param,
		      uint8_t *msg, uint8_t *sig, uint64_t msg_len)
{
	int ret = 0;
	uint8_t hmsg[32];
	struct akcipher_alg *alg;
	uint8_t y[256];

	alg = alg_find_by_name("rsa");
	if (!alg) {
		pr_err("RSA algrithm not found\n");
		return -1;
	}

	/* compute hash */
	do_sha256(msg, hmsg, msg_len);

	/* init param */
	param->rsa_ctx.size = ctx->key_len;
	param->rsa_ctx.x = sig;
	param->rsa_ctx.m = ctx->key.rsa.modulus;
	param->rsa_ctx.e = ctx->key.rsa.exponent;
	param->rsa_ctx.y = y;

	ret = alg_verify(alg, param);
	if (ret)
		return -1;

	ret = memcmp(hmsg, &y[256 - 32], 32);
	if (ret)
		return -1;

	return 0;
}

static int sm2_verify(struct akcipher_ctx *ctx, union akcipher_param *param,
		      uint8_t *msg, uint8_t *sig, uint64_t msg_len)
{
	int ret = 0;
	uint8_t hmsg[32], s[32], r[32], za[32];
	struct akcipher_alg *alg;
	struct sm2_parameter sm2;

	alg = alg_find_by_name("sm2");
	if (!alg) {
		pr_err("SM2 algrithm not found\n");
		return -1;
	}

	calculate_za(sm2.pubx, sm2.puby, za);
	calculate_hmsg(za, msg, msg_len, hmsg);

	if (get_s_r(sig, s, r))
		return -1;

	/* init param */
	param->sm2_ctx.size = ctx->key_len;
	param->sm2_ctx.e = hmsg;
	param->sm2_ctx.pubx = ctx->key.sm2.pubx;
	param->sm2_ctx.puby = ctx->key.sm2.puby;
	param->sm2_ctx.r = r;
	param->sm2_ctx.s = s;
	param->sm2_ctx.cofactor = 1;

	ret = alg_verify(alg, param);
	if (ret)
		return -1;

	return 0;
}

static int (*akcipher_algs[])(struct akcipher_ctx *, union akcipher_param *,
			      uint8_t *, uint8_t *, uint64_t) = {
	[RSA] = rsa_verify,
	[SM2] = sm2_verify,
};

int akcipher_verify(struct akcipher_ctx *ctx, void *msg, void *sig, unsigned long len)
{
	int ret = 0;
	union akcipher_param *param;

	param = malloc(sizeof(union akcipher_param));
	if (!param) {
		pr_err("param alloc failed\n");
		return -1;
	}

	ret = akcipher_algs[ctx->alg](ctx, param, msg, sig, len);

	free(param);
	return ret;
}
