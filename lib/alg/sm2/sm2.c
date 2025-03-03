#include <lib/alg.h>
#include <driver/accdev.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <framework/common.h>
#include <framework/module.h>
#include <lib/hash/ec.h>

static void param_last_init(struct akchipher_param *param)
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

static void *read_va_param(
			   int output,
			   char *ret_name,
			   va_list *ap)
{
	while (1) {
		const char *name;
		void *data;

		name = va_arg(*ap, char *);
		if (!name)
			return NULL;
		data = va_arg(*ap, void *);

		/* Skip parameters we're not interested in. */
		if ((output && name[0] != '=') || (!output && name[0] == '='))
			continue;

		if (name[0] == '=')
			name++;

		if (name[0] == '%')/* Absolute register */
			strncpy(ret_name, name+1, MAX_NAME);
		else
			//TODO:error
			;

		return data;
	}
}

static int load_inputs(struct acc_dev *dev, struct acc_param *param, va_list *ap)
{
	int rc;
	
	memset(param->name, 0, MAX_NAME);
	while ((param->value = (unsigned char *)read_va_param(0, param->name, ap)) != NULL) {
		rc = acc_set_param(dev, param);
		if (rc)
			return -1;
	}

	return 0;
}

static int unload_outputs(struct acc_dev *dev, struct acc_param *param, va_list *ap)
{
	int rc;

	memset(param->name, 0, MAX_NAME);
	while ((param->value = (unsigned char *)read_va_param(1, param->name, ap)) != NULL) {
		rc = acc_get_param(dev, param);
		if (rc == -1)
			return -1;
	}

	return 0;
}

static int parse_and_compute(
		struct acc_dev* acc_dev,
		const char *func,
		unsigned int size,
		va_list ap)
{
	va_list ap2;
	int rc;
	struct acc_param *param;

	param = malloc(sizeof(struct acc_param));
	if (!param) {
		pr_err("param alloc failed\n");
		return -1;
	}

	param->name = malloc(MAX_NAME);
	if (!param->name) {
		pr_err("param name alloc failed\n");
		return -1;
	}
	param->size = size;
	param->func = func;

	va_copy(ap2, ap);
	rc = load_inputs(acc_dev, param, &ap2);
	va_end(ap2);
	if (rc != 0)
		goto failed;

	rc = acc_do_call(acc_dev, param);
	if (rc != 0)
		goto failed;

	va_copy(ap2, ap);
	rc = unload_outputs(acc_dev, param, &ap2);
	va_end(ap2);
	if (rc != 0) 
		goto failed;
failed:
	free(param->name);
	free(param);
	return rc;
}

static int start_compute(struct akchipher_alg *alg, struct ecc_ctx *ctx, const char *func, ...)
{
	va_list ap;
	int rc;
	struct acc_dev *acc_dev;
	acc_dev = alg->priv;

	va_start(ap, func);
	rc = parse_and_compute(acc_dev, func, ctx->size, ap);
	va_end(ap);

	if (rc == -1) {
		pr_err(func);
		return 0;
	} else if (rc > 0) {
		pr_err("%s func returned error: %d\n", func, rc);
		return 0;
	}

	return 1;
}

static int do_pmult(struct akchipher_alg *alg, struct ecc_ctx *ctx)
{
	unsigned char r1[32];
	unsigned char r2[32];
	unsigned char r3[32];
	unsigned char temp[32];

	/* start verify */
	if (!start_compute(alg, ctx, "modadd",
		     	"%A0", ctx->r,
			"%B0", ctx->s,
			"%D0", ctx->order,
			"=%A0", r1,
			(char *)NULL)) 
		return -1;
	
	if (!start_compute(alg, ctx, "pmult", 
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

	if (!start_compute(alg, ctx, "pmult",
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

	if (!start_compute(alg, ctx, "padd",
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

	if (!start_compute(alg, ctx, "modadd", 
			"%A0", ctx->e,
			"%B0", r1,
			"%D0", ctx->order,
			"=%A0", ctx->out,
			(char *)NULL))
		return -1;

	return 0;
}

int sm2_set_priv_key(struct akchipher_alg *alg, struct akchipher_param *param)
{
	return 0;
}

int sm2_set_pub_key(struct akchipher_alg *alg, struct akchipher_param *param)
{
	return 0;
}

int sm2_sign(struct akchipher_alg *alg, struct akchipher_param *param)
{
	return 0;
}

int sm2_verify(struct akchipher_alg *alg, struct akchipher_param *param)
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
		ret =-1;

failed:
	free(param->sm2_ctx.out);
	return ret;
}

void sm2_release(struct akchipher_alg *alg)
{
	struct acc_dev *sm2_drv;

	alg_unregister(alg);
	sm2_drv = alg->priv;
	sm2_drv->ops->release_dev(sm2_drv);
	free(alg);

}

struct akchipher_ops sm2_ops = {
	.set_priv_key = sm2_set_priv_key,
	.set_pub_key = sm2_set_pub_key,
	.sign = sm2_sign,
	.verify = sm2_verify,
	.release = sm2_release,
};

int sm2_alg_init(struct akchipher_alg *sm2)
{
	struct acc_dev *sm2_drv;

	/*init alg name*/
	strncpy(sm2->name, "sm2", 4);

	/*init base alg*/
	strncpy(sm2->base.name, "pka", 4);
	//sm2->base.priority = 3000;

	/*init ops*/
	sm2->ops = &sm2_ops;

	/*init priv*/
	sm2_drv = acc_find_by_name(sm2->base.name);
	if (!sm2_drv) {
		pr_err("failed to find %s driver\n", sm2->base.name);
		return -1;
	}
	sm2->priv = sm2_drv;
	return 0;
}

int sm2_init()
{
	struct akchipher_alg *sm2_alg;
	int ret;

	sm2_alg = alg_alloc();
	if (!sm2_alg) {
		pr_err("alg malloc failed\n");
		return -ENOMEM;
	}

	ret = sm2_alg_init(sm2_alg);
	if (ret) {
		pr_err("init failed\n");
		return -1;
	}
	
	ret = alg_register(sm2_alg);
	return 0;
}

late_init(sm2_init);