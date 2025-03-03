#include <lib/alg.h>
#include <driver/accdev.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <framework/common.h>
#include <framework/module.h>

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

static int start_compute(struct akchipher_alg *alg, struct rsa_ctx *ctx, const char *func, ...)
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

static int do_modexp(struct akchipher_alg *alg, struct rsa_ctx *ctx)
{
	/* Montgomery precomputation. */
	if (!start_compute(alg, ctx, "calc_r_inv",
			"%D0",  ctx->m,
			(char *)NULL))
		return -1;

	if (!start_compute(alg, ctx, "calc_mp", 
			(char *)NULL))
		return -1;

	if (!start_compute(alg, ctx, "calc_r_sqr", 
			(char *)NULL))
		return -1;

	/* Modular exponentiation. */
	if (!start_compute(alg, ctx, "modexp",
			"%A0",  ctx->x,
			"%D2",  ctx->e,
			"%D0",  ctx->m,
			"=%A0", ctx->y,
			(char *)NULL))
		return -1;

	return 0;
}

int rsa_set_priv_key(struct akchipher_alg *alg, struct akchipher_param *param)
{
	return 0;
}

int rsa_set_pub_key(struct akchipher_alg *alg, struct akchipher_param *param)
{
	return 0;
}

int rsa_sign(struct akchipher_alg *alg, struct akchipher_param *param)
{
	return 0;
}

int rsa_verify(struct akchipher_alg *alg, struct akchipher_param *param)
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
	struct acc_dev *rsa_drv;

	alg_unregister(alg);
	rsa_drv = alg->priv;
	rsa_drv->ops->release_dev(rsa_drv);
	free(alg);

}

struct akchipher_ops rsa_ops = {
	.set_priv_key = rsa_set_priv_key,
	.set_pub_key = rsa_set_pub_key,
	.sign = rsa_sign,
	.verify = rsa_verify,
	.release = rsa_release,
};

int rsa_alg_init(struct akchipher_alg *rsa)
{
	struct acc_dev *rsa_drv;

	/*init alg name*/
	strncpy(rsa->name, "rsa", 4);

	/*init base alg*/
	strncpy(rsa->base.name, "pka", 4);
	//rsa->base.priority = 3000;

	/*init ops*/
	rsa->ops = &rsa_ops;

	/*init priv*/
	rsa_drv = acc_find_by_name(rsa->base.name);
	if (!rsa_drv) {
		pr_err("failed to find %s driver\n", rsa->base.name);
		return -1;
	}
	rsa->priv = rsa_drv;
	return 0;
}

int rsa_init()
{
	struct akchipher_alg *rsa_alg;
	int ret;

	rsa_alg = alg_alloc();
	if (!rsa_alg) {
		pr_err("alg malloc failed\n");
		return -ENOMEM;
	}

	ret = rsa_alg_init(rsa_alg);
	if (ret) {
		pr_err("init failed\n");
		return -1;
	}
	
	ret = alg_register(rsa_alg);
	return 0;
}

late_init(rsa_init);