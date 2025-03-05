#ifndef __PKA_H__
#define __PKA_H__

#include <driver/pka/elppka.h>
#include <driver/pka/pka_port.h>

//HardWare-OPerations
typedef void (*hwop)(void);

struct pka_param {
	unsigned int size;
	const char *func;
	char *name;
	unsigned char *value;
};

struct pka_dev {
	struct pka_state pka;
	struct pka_fw fw;
	int opened;
	int fwloaded;	//firmware has been loaded
	hwop up;
	hwop down;
	char *name;
	unsigned int work_flags;
	unsigned int saved_flags;
	unsigned int *regs;
	void *fw_begin;
	void *fw_end;
	unsigned int irq_num;
	struct pka_sem sem;
};

int start_compute(struct pka_dev *pka_dev, unsigned int size, const char *func, ...);

int rsa_register(struct pka_dev *dev);
int sm2_register(struct pka_dev *dev);

void calculate_za(unsigned char *pubx, unsigned char *puby, unsigned char *za);
void calculate_hmsg(unsigned char *za, unsigned char *msg, unsigned long msg_len, unsigned char *hmsg);
#endif
