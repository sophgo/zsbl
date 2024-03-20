#define DEBUG
#include <stdio.h>
#include <timer.h>
#include <string.h>
#include <framework/module.h>
#include <framework/common.h>
#include <lib/libc/errno.h>
#include <smp.h>
#include <sifive_extensiblecache0.h>
#include <sifive_buserror0.h>
#include <sifive_pl2cache0.h>
#include <sifive_hwpf1.h>
#include <platform.h>
#include <sbi/riscv_asm.h>
#include <driver/ddr/ddr.h>
#include "sbi.h"

#define STACK_SIZE 4096

extern int boot_from_storage(void);

struct fw_dynamic_info pld_dynamic_info = {
	.magic = FW_DYNAMIC_INFO_MAGIC_VALUE,
	.version = 0,
	.next_addr = KERNEL_ADDR,
	.next_mode = FW_DYNAMIC_INFO_NEXT_MODE_S,
	.boot_hart = 0xffffffffffffffff,
};

typedef struct {
	uint8_t stack[STACK_SIZE];
} core_stack;

static core_stack secondary_core_stack[CONFIG_SMP_NUM];

void platform_init(void)
{
	struct metal_cpu *cpu;
	cpu = metal_cpu_get(csr_read(CSR_MHARTID));

	struct metal_buserror *beu = metal_cpu_get_buserror(cpu);
	if (beu != NULL) {
		metal_buserror_init(beu);
	}
	metal_buserror_init(); 
	sifive_pl2cache0_init();
	sifive_hwpf1_init();
}

static void secondary_core_fun(void *priv)
{
	platform_init();
	__asm__ __volatile__ ("fence.i"::);

	jump_to(OPENSBI_ADDR, current_hartid(),
			DEVICETREE_ADDR, (long)&pld_dynamic_info);
}

int boot_next_img(void)
{
	unsigned int hartid = current_hartid();

	for (int i = 0; i < CONFIG_SMP_NUM; i++) {
		if (i == hartid)
			continue;
		wake_up_other_core(i, secondary_core_fun, NULL,
					&secondary_core_stack[i], 4096);
	}

	return 0;
}

int boot(void)
{
#if defined(CONFIG_TARGET_PALLADIUM)
	printf("SOPHGO SG2380 ZSBL\n");
	sifive_extensiblecache0_init();
	platform_init();
	sg2380_ddr_init_asic();
	if (boot_next_img())
		pr_err("boot next img failed\n");

	__asm__ __volatile__ ("fence.i"::);
	jump_to(OPENSBI_ADDR, current_hartid(),
		DEVICETREE_ADDR, (unsigned long)&pld_dynamic_info);
#else
	boot_from_storage();
#endif
	return 0;
}
plat_init(boot);
