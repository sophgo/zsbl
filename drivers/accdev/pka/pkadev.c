#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include <driver/pka/pka_port.h>
#include <driver/pka/pkafw.h>
#include <driver/pka/elppka_hw.h>
#include <driver/pka/elppka.h>
#include <driver/pka/pkadev.h>

#include <framework/common.h>
#include <framework/module.h>
#include <lib/libc/errno.h>
#include <driver/accdev.h>
#include <driver/platform.h>

#define MAX_NAME	(8)
#define FUNC_MAX	(32)

#define dev_err(...)		printf(__VA_ARGS__)
#define dev_warning(...)	printf(__VA_ARGS__)
#define dev_info(...)		printf(__VA_ARGS__)
#define dev_dbg(...)		printf(__VA_ARGS__)

#define is_digit(c)      ((c) >= '0' && (c) <= '9')
#define PKA_DEV_NUM	(1)

//HardWare-OPerations
typedef void (*hwop)(void);

struct pka_dev {
	struct pka_state pka;
	struct pka_fw fw;
	int opened;
	int fwloaded;	//firmware has been loaded
	hwop up;
	hwop down;
	char *name;
	uint32_t work_flags;
	uint32_t saved_flags;
	uint32_t *regs;
	void *fw_begin;
	void *fw_end;
	unsigned int irq_num;
	struct pka_sem sem;
};

#define TOP_CLOCK_ENABLE_1	(*(unsigned int *)0x7050002004)
#define TOP_SOFT_RESET_1	(*(unsigned int *)0x7050003014)

static void hw0_down(void)
{
#if 1
	TOP_CLOCK_ENABLE_1 &= ~(1 << 15);	//disable clock
	TOP_SOFT_RESET_1 &= ~(1 << 29);		//deactive reset
#endif 
}
static void hw0_up(void)
{
#if 1
	TOP_CLOCK_ENABLE_1 |= 1 << 15;		//enable clock
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

	/*pka dev alloc*/
	if (!dev) {
		pr_err("pka_dev alloc failed\n");
		return -ENOMEM;
	}

	/*name init*/
	dev->name = "/dev/pka0";
	
	/*regbase init*/
	dev->regs = (uint32_t *)pdev->reg_base;
	if (!dev->regs) {
		free(dev);
		pr_err("dev regs alloc failed\n");
		return -ENOMEM;
	}
	//dev_dbg("pka_dev regbase is %p\n", dev->regs);

	/*fw init*/
	dev->fw_begin = (void *)pka0_fw_begin;
	dev->fw_end = (void *)pka0_fw_end;
	dev->fwloaded = 0;

	/*reset function init*/
	dev->down = hw0_down;
	dev->up = hw0_up;

	/*call reset*/
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

static int dev_destroy(struct pka_dev *dev)
{
	if (!dev->opened)
		return 0;
	dev->down();
	pka_sem_destroy(&dev->sem);
	dev->opened = 0;
	return 0;
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

static int set_param(struct acc_dev *acc_dev, struct acc_param *param)
{
	int rc;
	unsigned int bank, index;
	struct pka_dev *dev;

	dev = acc_dev->priv;
	if (!dev) {
		pr_err("acc dev priv is null\n");
	}
	rc = pka_lookup_param(param->name, &bank, &index);
	if (rc < 0)
		return rc;

	rc = elppka_load_operand(&dev->pka, bank, index, param->size, param->value);

	return pdu_error_code(rc);
}

static int get_param(struct acc_dev *acc_dev, struct acc_param *param)
{
	int rc;
	unsigned int bank, index;
	struct pka_dev *dev;

	dev = acc_dev->priv;
	rc = pka_lookup_param(param->name, &bank, &index);

	if (rc < 0)
		return rc;

	rc = elppka_unload_operand(&dev->pka, bank, index, param->size, param->value);

	return pdu_error_code(rc);
}

static int pka_lookup_flag(const char *name)
{
	int flagbit;

	/* Absolute flag references. */
	if (name[0] == 'F') {
		/* User flags */
		if (!is_digit(name[1]) || name[1] > '3')
			return -ENOENT;
		flagbit = PKA_FLAG_F0 + (name[1] - '0');

		if (name[2] != 0)
			return -ENOENT;

		return flagbit;
	}

	switch (name[0]) {
	case 'Z': flagbit = PKA_FLAG_ZERO;   break;
	case 'M': flagbit = PKA_FLAG_MEMBIT; break;
	case 'B': flagbit = PKA_FLAG_BORROW; break;
	case 'C': flagbit = PKA_FLAG_CARRY;  break;
	default:
		  return -ENOENT;
	}

	if (name[1] != 0)
		return -ENOENT;

	return flagbit;
}

static long pka_call(struct pka_dev *dev, struct acc_param *param)
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

	//unsigned long item_sz;

	//item_sz = pdu_io_read32(&dev->regs[PKA_CYCLES_SINCE_GO]);
	//dev_info("pka compute cycles since go is %ld\n", item_sz);

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
static int do_call(struct acc_dev *acc_dev, struct acc_param *param)//const char *func, unsigned size)
{
	struct pka_dev *dev;

	dev = acc_dev->priv;
	if (pka_call(dev, param) < 0)
		return -1;

	wait_pka_done(dev);
	return pka_wait(dev);
}


static int test_flag(struct acc_dev *acc_dev, struct acc_param *param)
{
	struct pka_dev *dev;
	u32 saved_flags;
	int flag_shift;

	dev = acc_dev->priv;
	if (!dev->opened)
		return -1;

	flag_shift = pka_lookup_flag(dev->name);
	if (flag_shift < 0)
		return flag_shift;

	saved_flags = dev->saved_flags;

	return (saved_flags & (1ul << flag_shift)) != 0;
}

static int set_flag(struct acc_dev *acc_dev, struct acc_param *param)//const char *func, const char *name)
{
	struct pka_dev *dev;
	u32 mask, prev_flags;
	int rc;

	dev = acc_dev->priv;
	if (!dev->opened)
		return -1;

	rc = pka_lookup_flag(dev->name);
	if (rc < 0)
		return rc;

	mask = 1ul << rc;
	prev_flags = dev->work_flags;

	dev->work_flags |= mask;

	return !!(prev_flags & mask);
}

static int open_dev(struct acc_dev *dev)
{
	return 0;
}

static void release_dev(struct acc_dev *dev)
{
	struct pka_dev *pkadev; 
	
	acc_unregister(dev);
	pkadev = dev->priv;
	dev_destroy(pkadev);

	free(pkadev->regs);
	free(pkadev);
	free(dev);
}

struct acc_ops pka_dev_ops = {
	.open_dev = open_dev,
	.release_dev = release_dev,
	.set_param = set_param,
	.get_param = get_param,
	.do_call = do_call,
	.set_flag = set_flag,
	.test_flag = test_flag,

};

static int probe(struct platform_device *pdev)
{
	struct acc_dev *acc_dev;
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

	acc_dev = acc_alloc();
	if (!acc_dev) {
		pr_err("acc_dev alloc failed\n");
		return -ENOMEM;
	}

	acc_dev->priv = pka_dev;
	acc_dev->ops = &pka_dev_ops;
	snprintf(acc_dev->name, sizeof(acc_dev->name), "pka");

	device_set_child_name(&acc_dev->device, &pdev->device, "accelerator");

	return acc_register(acc_dev);
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