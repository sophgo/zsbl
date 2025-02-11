/* LibTomCrypt, modular cryptographic library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

#ifndef __TOMCRYPT_HASH_H__
#define __TOMCRYPT_HASH_H__

#include <stdint.h>

/* ---- HASH FUNCTIONS ---- */
enum hash_descriptor_id {
    NID_HASH_SHA256 = 0,
    NID_HASH_TIGER = 1,
    NID_HASH_SHA1 = 2,
    NID_HASH_MD5 =3,
    NID_HASH_SHA384 = 4,
    NID_HASH_SHA512 = 5,
    NID_HASH_MD4 = 6,
    NID_HASH_MD2 = 7,
    NID_HASH_RMD128 = 8,
    NID_HASH_RMD160 = 9,
    NID_HASH_SHA224 = 10,
    NID_HASH_WHIRLPOOL = 11,
    NID_HASH_CHC = 12,
    NID_HASH_RMD256 = 13,
    NID_HASH_RMD320 = 14,
    NID_HASH_SHA512_224 = 15,
    NID_HASH_SHA512_256 = 16,
    NID_HASH_SHA3_224 = 17,
    NID_HASH_SHA3_256 = 18,
    NID_HASH_SHA3_384 = 19,
    NID_HASH_SHA3_512 = 20,
    NID_HASH_BLAKE2S_128 = 21,
    NID_HASH_BLAKE2S_160 = 22,
    NID_HASH_BLAKE2S_224 = 23,
    NID_HASH_BLAKE2S_256 = 24,
    NID_HASH_BLAKE2B_160 = 25,
    NID_HASH_BLAKE2B_256 = 26,
    NID_HASH_BLAKE2B_384 = 27,
    NID_HASH_BLAKE2B_512 = 28,
    NID_HASH_KECCAK224 = 29,
    NID_HASH_KECCAK256 = 30,
    NID_HASH_KECCAK384 = 31,
    NID_HASH_KECCAK512 = 32,
    NID_HASH_SM3 = 33,
};

#define HASH_MAX_MD_SIZE                 64  /* longest known is SHA512 */
#define SHA256
#define SM3

struct sha256_state {
    uint64_t length;
    uint32_t state[8], curlen;
    unsigned char buf[64];
};

#define SM3_DIGEST_LENGTH	32
#define SM3_BLOCK_SIZE		64

struct sm3_state {
	uint32_t digest[8];
	uint64_t nblocks;
	unsigned char block[64];
	int num;
};

typedef struct sm3_state sm3_ctx_t;

typedef union Hash_state {
    char dummy[1];
#ifdef SHA256
    struct sha256_state sha256;
#endif
#ifdef SM3
    struct sm3_state sm3;
#endif
    void *data;
} hash_state;

/** hash descriptor */
extern  struct ltc_hash_descriptor {
    /** name of hash */
    const char *name;
    /** internal ID */
    unsigned char ID;
    /** Size of digest in octets */
    unsigned long hashsize;
    /** Input block size in octets */
    unsigned long blocksize;
    /** ASN.1 OID */
    unsigned long OID[16];
    /** Length of DER encoding */
    unsigned long OIDlen;

    /** Init a hash state
      @param hash   The hash to initialize
      @return CRYPT_OK if successful
    */
    int (*init)(hash_state *hash);
    /** Process a block of data
      @param hash   The hash state
      @param in     The data to hash
      @param inlen  The length of the data (octets)
      @return CRYPT_OK if successful
    */
    int (*process)(hash_state *hash, const unsigned char *in, unsigned long inlen);
    /** Produce the digest and store it
      @param hash   The hash state
      @param out    [out] The destination of the digest
      @return CRYPT_OK if successful
    */
    int (*done)(hash_state *hash, unsigned char *out);
    /** Self-test
      @return CRYPT_OK if successful, CRYPT_NOP if self-tests have been disabled
    */
    int (*test)(void);

    /* accelerated hmac callback: if you need to-do multiple packets just use the generic hmac_memory and provide a hash callback */
    int  (*hmac_block)(const unsigned char *key, unsigned long  keylen,
                       const unsigned char *in,  unsigned long  inlen,
                             unsigned char *out, unsigned long *outlen);

} hash_descriptor[];

#endif
