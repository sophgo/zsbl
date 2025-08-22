#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include <common/common.h>
#include <common/module.h>
#include <lib/libc/errno.h>
#include <driver/platform.h>

#include <driver/pka/pka.h>
#include <driver/pka/pkafw.h>
#include <driver/pka/elppka_hw.h>
#include <driver/pka/pkadev.h>

#define MAX_NAME	(8)
#define FUNC_MAX	(32)

#define dev_err(...)		printf(__VA_ARGS__)
#define dev_warning(...)	printf(__VA_ARGS__)
#define dev_info(...)		printf(__VA_ARGS__)
#define dev_dbg(...)		printf(__VA_ARGS__)

#define is_digit(c)      ((c) >= '0' && (c) <= '9')
#define PKA_DEV_NUM	(1)

#define TOP_CLOCK_ENABLE_1	(*(unsigned int *)0x7050002004)
#define TOP_SOFT_RESET_1	(*(unsigned int *)0x7050003014)

static void hw0_down(void)
{
#if 1
	TOP_CLOCK_ENABLE_1 &= ~(1 << 16);	//disable clock
	TOP_SOFT_RESET_1 &= ~(1 << 29);		//deactive reset
#endif 
}
static void hw0_up(void)
{
#if 1
	TOP_CLOCK_ENABLE_1 |= 1 << 16;		//enable clock
	TOP_SOFT_RESET_1 |= 1 << 29;		//release reset
#endif
}

static struct pka_dev *pka_alloc(void)
{
	struct pka_dev *pkadev;

	pkadev = malloc(sizeof(struct pka_dev));
	if (pkadev)
		memset(pkadev, 0, sizeof(struct pka_dev));

	return pkadev;
}

static int dev_init(struct pka_dev *dev, struct platform_device *pdev) 
{
	int rc;

	/* pka dev alloc */
	if (!dev) {
		pr_err("pka_dev alloc failed\n");
		return -ENOMEM;
	}

	/* name init */
	dev->name = "/dev/pka0";
	
	/* regbase init */
	dev->regs = (uint32_t *)pdev->reg_base;
	if (!dev->regs) {
		free(dev);
		pr_err("dev regs alloc failed\n");
		return -ENOMEM;
	}
	//dev_dbg("pka_dev regbase is %p\n", dev->regs);

	/* fw init */
	dev->fw_begin = (void *)pka0_fw_begin;
	dev->fw_end = (void *)pka0_fw_end;
	dev->fwloaded = 0;

	/* reset function init */
	dev->down = hw0_down;
	dev->up = hw0_up;

	/* call reset */
	dev->up();
	rc = elppka_setup(&dev->pka, dev->regs);
	if (rc < 0) {
		dev->down();
		goto failed;
	}

	/* Set a super-huge-looking (but reasonable) watchdog value. */
	pdu_io_write32(&dev->regs[PKA_WATCHDOG], 100000000);

	if (!dev->fwloaded) {

		rc = elppka_fw_parse(&dev->fw, dev->fw_begin,
		(unsigned long)dev->fw_end - (unsigned long)dev->fw_begin);

		if (rc < 0) {
			if (rc == CRYPTO_INVALID_FIRMWARE)
				dev_err("invalid firmware image: %s\n",
					dev->fw.errmsg);
			dev->down();
			goto failed;
		}
		//TODO:describe firmware

		rc = elppka_fw_load(&dev->pka, &dev->fw);
		if (rc < 0) {
			if (rc == CRYPTO_INVALID_FIRMWARE)
				dev_err("cannot load firmware: %s\n",
					dev->fw.errmsg);
			dev->down();
			goto failed;
		}
		dev->fwloaded = 1;
		//TODO:verify firmware
	}

	//pdu_io_write32(&dev->regs[PKA_IRQ_EN], 1 << PKA_IRQ_EN_STAT);
	pdu_io_write32(&dev->regs[PKA_IRQ_EN], 0);

	pka_sem_init(&dev->sem);
	pka_sem_post(&dev->sem);//set to 1

	dev->opened = 1;
	return 0;

failed:
	free(dev->regs);
	free(dev);
	return pdu_error_code(rc);
}

static int pka_lookup_param(
			const char *name,
			unsigned int *bank,
			unsigned int *index)
{
	/* Absolute operand references. */
	switch (name[0]) {
	case 'A': *bank = PKA_OPERAND_A; break;
	case 'B': *bank = PKA_OPERAND_B; break;
	case 'C': *bank = PKA_OPERAND_C; break;
	case 'D': *bank = PKA_OPERAND_D; break;
	default:
		  return -ENOENT;
	}

	if (!is_digit(name[1]))
		return -ENOENT;
	*index = name[1] - '0';
	/* Require null termination for forward compatibility. */
	if (name[2] != 0) {
		pr_info("name 2 is 0x%x", name[2]);
	}
	return 0;
}

static int set_param(struct pka_dev *dev, struct pka_param *param)
{
	int rc;
	unsigned int bank, index;

	if (!dev) {
		pr_err("acc dev priv is null\n");
	}
	rc = pka_lookup_param(param->name, &bank, &index);
	if (rc < 0)
		return rc;

	rc = elppka_load_operand(&dev->pka, bank, index, param->size, param->value);

	return pdu_error_code(rc);
}

static int get_param(struct pka_dev *dev, struct pka_param *param)
{
	int rc;
	unsigned int bank, index;

	rc = pka_lookup_param(param->name, &bank, &index);

	if (rc < 0)
		return rc;

	rc = elppka_unload_operand(&dev->pka, bank, index, param->size, param->value);

	return pdu_error_code(rc);
}

static long pka_call(struct pka_dev *dev, struct pka_param *param)
{
	int rc;
	pka_sem_wait(&dev->sem);

	/*
	 * The function name in the parameter struct is not necessarily
	 * 0-terminated; copy it into a local array that is.
	 */
	rc = elppka_fw_lookup_entry(&dev->fw, param->func);
	if (rc < 0) {
		if (rc == CRYPTO_INVALID_FIRMWARE)
			dev_err("invalid firmware image: %s\n",
				dev->fw.errmsg);
		rc = pdu_error_code(rc);
		goto err;
	}

	rc = elppka_start(&dev->pka, rc, dev->work_flags, param->size);
	if (rc < 0) {
		rc = pdu_error_code(rc);
		goto err;
	}

	return 0;
err:
	return rc;
}

static unsigned int pka_wait(struct pka_dev *dev)
{
	int rc;
	unsigned int code;

	pka_sem_wait(&dev->sem);
	rc = elppka_get_status(&dev->pka, &code);
	pka_sem_post(&dev->sem);
	if (rc != 0)
		return -1;
	return code;
}

static int wait_pka_done(void *priv)
{
	struct pka_dev *dev = (struct pka_dev *)priv;
	uint32_t status;
	
	do {
		status = pdu_io_read32(&dev->regs[PKA_STATUS]);
	} while(!(status & (1 << PKA_STAT_IRQ)));

	pdu_io_write32(&dev->regs[PKA_STATUS], 1 << PKA_STAT_IRQ);
	dev->saved_flags = pdu_io_read32(&dev->regs[PKA_FLAGS]);
	dev->work_flags = 0;

	pka_sem_post(&dev->sem);


	return 0;
}

/*
 * Start a PKA operation and block waiting for the result.  Returns the
 * (non-negative) PKA status on success, or -1 on failure.
 */
static int do_call(struct pka_dev *dev, struct pka_param *param)//const char *func, unsigned size)
{
	if (pka_call(dev, param) < 0)
		return -1;

	wait_pka_done(dev);
	return pka_wait(dev);
}

static void *read_va_param(int output, char *ret_name, va_list *ap)
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
			strncpy(ret_name, name + 1, MAX_NAME - 1);
		else
			//TODO:error
			;

		return data;
	}
}

static int load_inputs(struct pka_dev *dev, struct pka_param *param, va_list *ap)
{
	int rc;
	
	memset(param->name, 0, MAX_NAME);
	while ((param->value = (unsigned char *)read_va_param(0, param->name, ap)) != NULL) {
		rc = set_param(dev, param);
		if (rc)
			return -1;
	}

	return 0;
}

static int unload_outputs(struct pka_dev *dev, struct pka_param *param, va_list *ap)
{
	int rc;

	memset(param->name, 0, MAX_NAME);
	while ((param->value = (unsigned char *)read_va_param(1, param->name, ap)) != NULL) {
		rc = get_param(dev, param);
		if (rc == -1)
			return -1;
	}

	return 0;
}

static int parse_and_compute(struct pka_dev* pka_dev, const char *func,
		unsigned int size, va_list ap)
{
	va_list ap2;
	int rc;
	struct pka_param *param;

	param = malloc(sizeof(struct pka_param));
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
	rc = load_inputs(pka_dev, param, &ap2);
	va_end(ap2);
	if (rc != 0)
		goto failed;

	rc = do_call(pka_dev, param);
	if (rc != 0)
		goto failed;

	va_copy(ap2, ap);
	rc = unload_outputs(pka_dev, param, &ap2);
	va_end(ap2);
	if (rc != 0) 
		goto failed;
failed:
	free(param->name);
	free(param);
	return rc;
}

int start_compute(struct pka_dev *pka_dev, unsigned int size, const char *func, ...)
{
	va_list ap;
	int rc;

	va_start(ap, func);
	rc = parse_and_compute(pka_dev, func, size, ap);
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

static int probe(struct platform_device *pdev)
{
	struct pka_dev *pka_dev;
	int ret;

	pka_dev = pka_alloc();
	if (!pka_dev) {
		pr_err("pka dev alloc failed\n");
		return -1;
	}

	ret = dev_init(pka_dev, pdev);//need release pkadev pkadev->regs
	if (ret) {
		pr_err("dev_init failed\n");
		return -1;
	}

	ret = rsa_register(pka_dev);
	if (ret) {
		pr_err("rsa register failed\n");
		return -1;
	}
	
	ret = sm2_register(pka_dev);
	if (ret) {
		pr_err("sm2 register failed\n");
		return -1;
	}

	return 0;
}

static struct of_device_id match_table[] = {
	{
		.compatible = "snps,designware-pka",
		.data = (void *)NULL,
	},
	{},
};

static struct platform_driver driver = {
	.driver.name = "pka",
	.probe = probe,
	.of_match_table = match_table,
};

static int pka_init(void)
{
	platform_driver_register(&driver);
	return 0;
}

module_init(pka_init);