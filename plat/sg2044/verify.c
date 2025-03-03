#include <driver/pka/pka.h>
#include <lib/hash/sha256.h>
#include <lib/hash/sm3.h>
#include <framework/common.h>
#include <string.h>
#include <stdint.h>
#include <verify.h>
#include <driver/nvmem.h>
#include <lib/alg.h>

#define RSA_OID_SIZE 13
#define SM2_OID_SIZE 20

#define SCS_CONFIG 0x804
#define PUBKEY_HASH 0x830

struct sm2_parameter {
	unsigned char pubx[32];
	unsigned char puby[32];
} sm2;

struct rsa_parameter {
	unsigned char modulus[256];
	unsigned char exponent[256];
} rsa;

static uint8_t rsa_oid[RSA_OID_SIZE] = {0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00};
static uint8_t sm2_oid[SM2_OID_SIZE] = {0x06, 0x08, 0x2a, 0x81, 0x1c, 0xcf, 0x55, 0x01, 0x82, 0x2d, 0x06, 0x08, 0x2a, 0x81, 0x1c, 0xcf, 0x55, 0x01, 0x82, 0x2d};
static unsigned char y[256];

int secure_boot()
{
	struct nvmem *nvmem;
	unsigned int secure_boot_enable;

	nvmem = nvmem_find_by_name("efuse1");

	nvmem_read(nvmem, SCS_CONFIG, sizeof(secure_boot_enable), &secure_boot_enable);
	secure_boot_enable = (secure_boot_enable & 0xc) >> 2;

	/*test secure boot enable set 1*/
	//secure_boot_enable = 1;
	if (secure_boot_enable) {
		pr_info("secure boot start!\n");
		return 1;
	}	
	return 0;
}

void read_pubkey_hash(uint32_t *pubkey_dig)
{
	int i;
	struct nvmem *nvmem;
	nvmem = nvmem_find_by_name("efuse1");

	for (i = 0; i < 8; i++)
		nvmem_read(nvmem, PUBKEY_HASH + i * 4, sizeof(uint32_t), &pubkey_dig[i]);
}

static int cmp_alg_oid(uint8_t *data, uint8_t *oid, uint32_t len)
{
	int i;

	for (i = 0; i < len; i++) {
		if(data[i] != oid[i]) return 0;
	}

	return 1;
}

static uint32_t parse_seq_length(uint8_t *data, uint32_t *ctx_len)
{
	uint32_t offset = 0;

	if (data[offset] & 0x80) {
		uint32_t length_bytes = data[offset++] & 0x7F;
		if (!ctx_len) return offset + length_bytes;
		for (int i = 0; i < length_bytes; i++) {
			*ctx_len = (*ctx_len << 8) | data[offset++];
		}
	} else {
		if (!ctx_len) return offset + 1;
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

	case SM2_OID_SIZE:
		if (cmp_alg_oid(data, sm2_oid, len)) {
			*flag = SM2;
			break;
		}

	default:
		return -1;
	}

	return 0;
}

static int extract_rsa_key(uint8_t *data, uint32_t *modulus_length)
{
	uint32_t exponent_length = 0;
	uint32_t offset = 0;
	int i;

	/*checkout sequence flag*/
	if (data[offset++] != 0x30) {
		pr_err("failed: data type is not sequence!\n");
		return -1;
	}

	/*skip sequence length */
	offset += parse_seq_length(data+offset, NULL);

	/*parse m head*/
	if (data[offset++] != 0x02) {
		pr_err("failed: not a valid module head!\n");
		return -1;
	}

	/*read m length*/
	offset += parse_seq_length(data+offset, modulus_length);

	while (!data[offset]) {
		offset++;
		(*modulus_length)--;
	}

	/*save m*/
	pr_debug("modules (n):\n");
	for (i = 0; i < (*modulus_length); i++) {
		rsa.modulus[i] = data[offset + i];
		pr_debug("%02X", data[offset + i]);
		pr_debug((i+1)%16 ? "" : "\n");
	}

	offset += *modulus_length;

	/*parse e head*/
	if (data[offset++] != 0x02) {
		pr_err("failed: not a valid exponent!\n");
		return -1;
	}

	/*read e length*/
	offset += parse_seq_length(data+offset, &exponent_length);

	/*save e*/
	pr_debug("exponent:\n");
	for (i = 0; i < 256 - exponent_length; i++) {
		rsa.exponent[i] = 0;
		pr_debug("%02X", rsa.exponent[i]);
		pr_debug((i+1)%16 ? "" : "\n");
	}

	for (; i < 256 ; i++) {
		rsa.exponent[i] = data[offset + i + exponent_length - 256];
		pr_debug("%02X", rsa.exponent[i]);
		pr_debug((i+1)%16 ? "" : "\n");
	}
	
	return 0;
}

static int extract_sm2_key(uint8_t *data, uint32_t *modulus_length)
{
	uint32_t offset = 0;

	if (data[offset++] != 0x04) {
		pr_err("failed: sm2 key is not uncompressed!\n");
		return -1;
	}

	/*save x*/
	pr_debug("sm2 x:\n");
	for (int i = 0; i < 32; i++) {
		sm2.pubx[i] = data[offset++];
		pr_debug("%02X", sm2.pubx[i]);
		pr_debug((i+1)%16 ? "" : "\n");
	}
	pr_debug("\n");

	/*save y*/
	pr_debug("sm2 y:\n");
	for (int i = 0; i < 32; i++) {
		sm2.puby[i] = data[offset++];
		pr_debug("%02X", sm2.puby[i]);
		pr_debug((i+1)%16 ? "" : "\n");
	}
	pr_debug("\n");

	*modulus_length = 32;

	return 0;
}

int parse_public_key(uint8_t *data, uint32_t len, uint32_t *m_len, int *flag) 
{
	uint32_t oid_len = 0;
	uint32_t offset = 0;
	int ret =0;

	/*check the pubkey length*/
	if (len < 15) {
		pr_err("failed: data len is too short!\n");
		return -1;
	}

	/*check if the sequence*/
	if (data[offset++] != 0x30) {
		pr_err("failed: data type is not sequence!\n");
		return -1;
	}

	/*skip the length byte*/
	offset += parse_seq_length(data+offset, NULL);

	/*check if Algorithm Identifier*/
	if (data[offset++] != 0x30) {
		pr_err("failed: data type is not sequence!\n");
		return -1;
	}

	/*record Algorithm Identifier length*/
	offset += parse_seq_length(data+offset, &oid_len);

	ret = parse_alg_oid(data+offset, oid_len, flag);
	if (ret) {
		pr_err("failed: alg oid is not correct!\n");
		return -1;
	}

	offset += oid_len; 

	/*skip fixed context*/
	if (data[offset++] != 0x03) {
		pr_err("failed: data type is not sequence!\n");
		return -1;
	}

	/*skip BIT STRING , which has 0*/
	offset += parse_seq_length(data+offset, NULL) + 1;

	/*select an alg*/
	switch (*flag) {
	case RSA:
		ret = extract_rsa_key(data+offset, m_len);
		if (ret) return -1;
		break;
	case SM2:
		ret = extract_sm2_key(data+offset, m_len);
		if (ret) return -1;
		break;
	default:
		return -1;
	}		

	return 0;
}

static int get_s_r(unsigned char *sig, unsigned char *s, unsigned char *r)
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
			pr_debug((i+1)%16 ? "" : "\n");
		}
		pr_debug("\n");
	}

	pos = 4 + r_len;

	pad = 0;
	if (sig[pos] == 0x02) {
		s_len = sig[pos + 1];

		if(s_len == 0x21)
			pad = 1;
		pos += 2;

		pr_debug("s:\n");
		for (i = 0; i < s_len - pad; i++) {
			s[i] = sig[pos + i + pad];
			pr_debug("%02X", s[i]);
			pr_debug((i+1)%16 ? "" : "\n");
		}
		pr_debug("\n");
	}

	return  0;
}

int pubkey_verify(unsigned char *pubkey_hash, DER_INFO *der_info, unsigned long len)
{
	unsigned char hmsg[32];

	switch (der_info->alg) {
	case RSA:
		do_sha256(der_info->pubkey, hmsg, len);

		return memcmp(pubkey_hash, hmsg, 32);

	case SM2:
		do_sm3(der_info->pubkey, hmsg, len);

		return memcmp(pubkey_hash, hmsg, 32);
	default:
		return -1;
	}
}

int akchipher_verify(DER_INFO *der_info, unsigned char *msg, unsigned char *sig,
	        unsigned long msg_len)
{
	int ret = 0;
	unsigned char hmsg[32], s[32], r[32], za[32];
	struct akchipher_alg *alg;
	struct akchipher_param *param;

	param = malloc(sizeof(struct akchipher_param));
	if (!param) {
		pr_err("param alloc failed\n");
	}

	switch (der_info->alg) {
	case RSA:
		/*find alg*/
		alg = alg_find_by_name("rsa");
		if (!alg) {
			pr_err("no found rsa alg\n");
			goto failed;
		} 

		/*compute hash*/
		do_sha256(msg, hmsg, msg_len);

		/*init param*/
		param->rsa_ctx.size = der_info->m_len;
		param->rsa_ctx.x = sig;
		param->rsa_ctx.m = rsa.modulus;
		param->rsa_ctx.e = rsa.exponent;
		param->rsa_ctx.y = y;
		
		ret = alg_verify(alg, param);
		if (ret) 
			goto failed;

		pr_debug("compute hash:\n");
		for(int i = 0; i < 32; i++) {
			r[i] = param->rsa_ctx.y[224 + i];
			pr_debug("0x%X", r[i]);
		}
		pr_debug("\n");

		pr_debug("origin hash:\n");
		for(int i = 0; i < 32; i++) {
			pr_debug("0x%X", hmsg[i]);
		}
		pr_debug("\n");
		 
		ret = memcmp(hmsg, r, 32);
		if (ret) 
			goto failed;
		break;

	case SM2:
		alg = alg_find_by_name("sm2");
		if (!alg) {
			pr_err("no find SM2 alg\n");
			goto failed;
		} 

		calculate_za(sm2.pubx, sm2.puby, za);
		calculate_hmsg(za, msg, msg_len, hmsg);

		if (get_s_r(sig, s, r))
			return -1;

		/*init param*/
		param->sm2_ctx.size = der_info->m_len;
		param->sm2_ctx.e = hmsg;
		param->sm2_ctx.pubx = sm2.pubx;
		param->sm2_ctx.puby = sm2.puby;
		param->sm2_ctx.r = r;
		param->sm2_ctx.s = s;
		param->sm2_ctx.cofactor = 1;

		ret = alg_verify(alg, param);
		if (ret)
			goto failed;

		break;
	default:
		pr_err("no found alg\n");
		goto failed;
	}
failed:
	free(param);
	return ret;
}
