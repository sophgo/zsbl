#ifndef _EC_H_
#define _EC_H_

#define SM2_SIZE 32
#define USRID_SIZE 16

extern unsigned char prime[SM2_SIZE];
extern unsigned char sm2p256v1_A[SM2_SIZE];
extern unsigned char sm2p256v1_B[SM2_SIZE];
extern unsigned char sm2p256v1_order[SM2_SIZE];
extern unsigned char sm2p256v1_Gx[SM2_SIZE];
extern unsigned char sm2p256v1_Gy[SM2_SIZE];
extern unsigned char gm_r_inv[SM2_SIZE];
extern unsigned char gm_mp[SM2_SIZE];
extern unsigned char gm_r_sqr[SM2_SIZE];
extern unsigned char user_id[USRID_SIZE];

#endif

