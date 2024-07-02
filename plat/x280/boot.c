#define DEBUG
#include <stdio.h>
#include <timer.h>
#include <string.h>
#include <framework/module.h>
#include <framework/common.h>
#include <lib/libc/errno.h>
#include <smp.h>
#include <sifive_metal.h>
#include <sifive_platform.h>
#include <sifive_buserror0.h>
#include <sifive_pl2cache0.h>
#include <sifive_interrupt.h>
#include <sifive_l2pf2.h>
#include <sifive_io.h>
#include <sifive_cpu.h>
#include <riscv_cpu.h>
#include <platform.h>
#include <sbi/riscv_asm.h>
#include <driver/ddr/ddr.h>
#include "sbi.h"
#include <stdlib.h>

extern uintptr_t __metal_init_hart;

struct fw_dynamic_info pld_dynamic_info = {
	.magic = FW_DYNAMIC_INFO_MAGIC_VALUE,
	.version = 0,
	.next_addr = KERNEL_ADDR,
	.next_mode = FW_DYNAMIC_INFO_NEXT_MODE_S,
	.boot_hart = 0xffffffffffffffff,
};

#define METAL_REG(base, offset) (((unsigned long)(base) + (offset)))
#define METAL_REGW(base, offset) (__METAL_ACCESS_ONCE((__metal_io_u32 *)METAL_REG((base), (offset))))
#define METAL_MSIP(base, hart) (METAL_REGW((base), 4 * (hart)))

/*
 * _synchronize_harts() is called by init.c to cause harts > 0 to wait for
 * hart 0 to finish copying the data section, zeroing the BSS, and running
 * the libc contstructors.
 */
static void __metal_wake_harts(void) {
	int hart;
	__asm__ volatile("csrr %0, mhartid" : "=r"(hart)::);

	uintptr_t msip_base = 0;

	/* Get the base address of the MSIP registers */

	msip_base = __metal_driver_sifive_clint0_control_base(__METAL_DT_RISCV_CLINT0_HANDLE);
	msip_base += METAL_RISCV_CLINT0_MSIP_BASE;

	/* Disable machine interrupts as a precaution */
	__asm__ volatile("csrc mstatus, %0" ::"r"(METAL_MSTATUS_MIE));

	/* Current Hart (__metal_init_hart) set MSIP bit for all other harts to
	* wake them from their WFI loop */
	for (int i = 0; i < __METAL_DT_MAX_HARTS; i++) {
		if (i != hart) {
			METAL_MSIP(msip_base, i) = 1;
		}
	}
}

extern __inline__ void metal_interrupt_init(struct metal_interrupt *controller);
extern __inline__ struct metal_interrupt *
	metal_cpu_timer_interrupt_controller(struct metal_cpu *cpu);

void metal_init(void) {
	/* Make sure the initialization is only run once */
	static int init_done = 0;
	if (init_done) {
		return;
	}
	init_done = 1;
printf("%s: %d\n", __func__, __LINE__);
	/* METAL_SIFIVE_CCACHE1 */
	sifive_ccache1_init();

	/* Prepare interrupt structure for all cores. Should only be done by a
	 * single hart because there is one structure by hart but it is
	 * accessible from all harts. */
	struct metal_cpu *cpu;
	struct metal_interrupt *cpu_intr;
	struct metal_interrupt *tmr_intr;
printf("%s: %d\n", __func__, __LINE__);
	cpu = metal_cpu_get(metal_cpu_get_current_hartid());
	if (cpu == NULL) {
		return ;
	}
	cpu_intr = metal_cpu_interrupt_controller(cpu);
	if (cpu_intr == NULL) {
		return ;
	}
printf("%s: %d\n", __func__, __LINE__);
	metal_interrupt_init(cpu_intr);
printf("%s: %d\n", __func__, __LINE__);
	tmr_intr = metal_cpu_timer_interrupt_controller(cpu);
	if (tmr_intr == NULL) {
		return ;
	}
printf("%s: %d\n", __func__, __LINE__);
	metal_interrupt_init(tmr_intr);
}

void metal_secondary_init(void) {
// TODO: buserror0 init success
#if 0
	struct metal_cpu *cpu;
	cpu = metal_cpu_get(metal_cpu_get_current_hartid());
printf("%s: %d\n", __func__, __LINE__);
	struct metal_buserror *beu = metal_cpu_get_buserror(cpu);
	if (beu != NULL) {
		metal_buserror_init(beu);
	}
#endif
	sifive_pl2cache0_init();
	/* Do L2 Stride Prefetcher initialization. */
	sifive_l2pf2_init();
}

void metal_init_run(void) {
	metal_init();
}

void metal_secondary_init_run(void) {
	metal_secondary_init();
}

void platform_init(void)
{
	unsigned int hartid = current_hartid();

	if (hartid == __metal_init_hart) {
		printf("%s: %d\n", __func__, __LINE__);
		/* Run primary initialization (shared peripherals) */
		metal_init_run();
		printf("%s: %d\n", __func__, __LINE__);
		/* Wake up other harts (if any) that were waiting in a WFI loop */
		__metal_wake_harts();
		printf("%s: %d\n", __func__, __LINE__);
	}

	printf("%s: %d %d\n", __func__, hartid, __LINE__);
	/* Run secondary initialization (per-hart peripherals) */
	metal_secondary_init_run();
	printf("%s: %d %d\n", __func__, hartid, __LINE__);
}

int boot(void)
{
	printf("SOPHGO SG2380 X280 ZSBL: %s %s\n", __DATE__, __TIME__);
	platform_init();

	printf("%s: %d\n", __func__, __LINE__);
	__asm__ __volatile__ ("fence.i"::);
	jump_to(OPENSBI_ADDR, current_hartid(),
		DEVICETREE_ADDR, (unsigned long)&pld_dynamic_info);

	return 0;
}
plat_init(boot);
