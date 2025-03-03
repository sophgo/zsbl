#ifndef __PKA_H__
#define __PKA_H__

int rsa_verify(unsigned char *modulus, unsigned char *signature, unsigned char *e,
	       unsigned char *hmsg);

int sm2_verify(unsigned char *hmsg, unsigned char *pubx, unsigned char *puby,
	       unsigned char *r, unsigned char *s);

void calculate_za(unsigned char *pubx, unsigned char *puby, unsigned char *za);

void calculate_hmsg(unsigned char *za, unsigned char *msg, unsigned long msg_len, unsigned char *hmsg);
#endif
