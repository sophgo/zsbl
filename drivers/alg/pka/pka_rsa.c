#include <driver/alg.h>
#include <stdio.h>
#include <string.h>
#include <driver/pka/pka.h>
#include <framework/common.h>
#include <framework/module.h>

static int do_modexp(struct akchipher_alg *alg, struct rsa_ctx *ctx)
{
	struct pka_dev *pka_dev = alg->priv;

	/* Montgomery precomputation. */
	if (!start_compute(pka_dev, ctx->size, "calc_r_inv",
			"%D0",  ctx->m	,
			(char *)NULL))
		return -1;

	if (!start_compute(pka_dev, ctx->size, "calc_mp", 
			(char *)NULL))
		return -1;

	if (!start_compute(pka_dev, ctx->size, "calc_r_sqr", 
			(char *)NULL))
		return -1;

	/* Modular exponentiation. */
	if (!start_compute(pka_dev, ctx->size, "modexp",
			"%A0",  ctx->x,
			"%D2",  ctx->e,
			"%D0",  ctx->m,
			"=%A0", ctx->y,
			(char *)NULL))
		return -1;

	return 0;
}

int rsa_verify(struct akchipher_alg *alg, union akchipher_param *param)
{
	int ret;
	memset(param->rsa_ctx.y, 0x00, param->rsa_ctx.size);

	ret = do_modexp(alg, &param->rsa_ctx);
	if (ret) {
		pr_err("compute failed\n");
		return -1;
	}

	return 0;
}

void rsa_release(struct akchipher_alg *alg)
{
	alg_unregister(alg);
	free(alg);
}

struct akchipher_ops rsa_ops = {
	.verify = rsa_verify,
	.release = rsa_release,
};

static int rsa_alg_init(struct akchipher_alg *rsa, struct pka_dev *dev)
{
	/* init alg name */
	strncpy(rsa->name, "rsa", 4);
	/* init driver name */
	strncpy(rsa->base.name, "pka", 4);
	/* init ops */
	rsa->ops = &rsa_ops;
	/* init priv */
	rsa->priv = dev;

	return 0;
}

int rsa_register(struct pka_dev *dev)
{
	struct akchipher_alg *rsa_alg;
	int ret;

	rsa_alg = alg_alloc();
	if (!rsa_alg) {
		pr_err("alg malloc failed\n");
		return -ENOMEM;
	}

	ret = rsa_alg_init(rsa_alg, dev);
	if (ret) {
		pr_err("init failed\n");
		return -1;
	}
	
	ret = alg_register(rsa_alg);
	
	return 0;
}