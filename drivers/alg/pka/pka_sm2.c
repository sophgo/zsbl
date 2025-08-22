#include <driver/alg.h>
#include <driver/pka/pka.h>
#include <stdio.h>
#include <string.h>
#include <common/common.h>
#include <common/module.h>
#include <lib/hash/ec.h>

static void param_last_init(union akcipher_param *param)
{
	param->sm2_ctx.A = sm2p256v1_A;
	param->sm2_ctx.B = sm2p256v1_B;
	param->sm2_ctx.order = sm2p256v1_order;
	param->sm2_ctx.Gx = sm2p256v1_Gx;
	param->sm2_ctx.Gy = sm2p256v1_Gy;
	param->sm2_ctx.prime = prime;
	param->sm2_ctx.r_inv = gm_r_inv;
	param->sm2_ctx.mp = gm_mp;
	param->sm2_ctx.r_sqr = gm_r_sqr;

	return ;
}

static int do_pmult(struct akcipher_alg *alg, struct ecc_ctx *ctx)
{
	unsigned char r1[32];
	unsigned char r2[32];
	unsigned char r3[32];
	unsigned char temp[32];

	struct pka_dev *pka_dev = alg->priv;

	/* start verify */
	if (!start_compute(pka_dev, ctx->size, "modadd",
			   "%A0", ctx->r,
			   "%B0", ctx->s,
			   "%D0", ctx->order,
			   "=%A0", r1,
			   (char *)NULL))
		return -1;

	if (!start_compute(pka_dev, ctx->size, "pmult",
			   "%A2", ctx->pubx,
			   "%B2", ctx->puby,
			   "%A6", ctx->A,
			   "%D7", r1,
			   "%D0", ctx->prime,
			   "%D1", ctx->mp,
			   "%D3", ctx->r_sqr,
			   "=%A2", r1,
			   "=%B2", r2,
			   (char *)NULL))
		return -1;

	if (!start_compute(pka_dev, ctx->size, "pmult",
			   "%A2", ctx->Gx,
			   "%B2", ctx->Gy,
			   "%A6", ctx->A,
			   "%D7", ctx->s,
			   "%D0", ctx->prime,
			   "%D1", ctx->mp,
			   "%D3", ctx->r_sqr,
			   "=%A2", r3,
			   "=%B2", temp,
			   (char *)NULL))
		return -1;

	if (!start_compute(pka_dev, ctx->size, "padd",
			   "%A2", r1,
			   "%B2", r2,
			   "%A3", r3,
			   "%B3", temp,
			   "%A6", ctx->A,
			   "%D0", ctx->prime,
			   "%D1", ctx->mp,
			   "%D3", ctx->r_sqr,
			   "=%A2", r1,
			   "=%B2", r2,
			   (char *)NULL))
		return -1;

	if (!start_compute(pka_dev, ctx->size, "modadd",
			   "%A0", ctx->e,
			   "%B0", r1,
			   "%D0", ctx->order,
			   "=%A0", ctx->out,
			   (char *)NULL))
		return -1;

	return 0;
}

int sm2_verify(struct akcipher_alg *alg, union akcipher_param *param)
{
	int ret;

	param_last_init(param);

	param->sm2_ctx.out = (unsigned char *)malloc(param->sm2_ctx.size);
	if (!param->sm2_ctx.out) {
		pr_err("out malloc failed\n");
		return -1;
	}

	ret = do_pmult(alg, &param->sm2_ctx);
	if (ret) {
		pr_err("compute failed\n");
		goto failed;
	}

	unsigned char *op __attribute__((unused));
	op = (unsigned char*)param->sm2_ctx.r;

	pr_debug("origin r:\n");
	for(int i = 0; i < 32; i++) {
		pr_debug("0x%X", op[i]);
	}
	pr_debug("\n");

	pr_debug("compute r:\n");
	for(int i = 0; i < 32; i++) {
		pr_debug("0x%X", param->sm2_ctx.out[i]);
	}
	pr_debug("\n");

	if (memcmp(param->sm2_ctx.out, param->sm2_ctx.r, param->sm2_ctx.size))
		ret = -1;

failed:
	free(param->sm2_ctx.out);
	return ret;
}

void sm2_release(struct akcipher_alg *alg)
{
	alg_unregister(alg);
	free(alg);
}

struct akcipher_ops sm2_ops = {
	.verify = sm2_verify,
	.release = sm2_release,
};

static int sm2_alg_init(struct akcipher_alg *sm2, struct pka_dev *dev)
{
	/* init alg name */
	strncpy(sm2->name, "sm2", 4);
	/* init driver name */
	strncpy(sm2->base.name, "pka", 4);
	/* init ops */
	sm2->ops = &sm2_ops;
	/* init priv */
	sm2->priv = dev;

	return 0;
}

int sm2_register(struct pka_dev *dev)
{
	struct akcipher_alg *sm2_alg;
	int ret;

	sm2_alg = alg_alloc();
	if (!sm2_alg) {
		pr_err("alg malloc failed\n");
		return -ENOMEM;
	}

	ret = sm2_alg_init(sm2_alg, dev);
	if (ret) {
		pr_err("init failed\n");
		return -1;
	}

	ret = alg_register(sm2_alg);

	return 0;
}
