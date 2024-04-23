#define DEBUG
#include <stdio.h>
#include <timer.h>
#include <lib/mmio.h>
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
#include <memmap.h>
#include <ncore_boot.h>
#include <sbi/riscv_asm.h>
#include <driver/ddr/ddr.h>
#include "sbi.h"
#include "cli.h"

#define STACK_SIZE 4096
#define DDR_CFG_BASEADDR 0X05000000000

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

#define GENMASK(h, l) \
	(((~0UL) << (l)) & (~0UL >> (32 - 1 - (h)))) 

static inline uint32_t modified_bits_by_value(uint32_t orig, uint32_t value, uint32_t msb, uint32_t lsb)
{
	uint32_t bitmask = GENMASK(msb, lsb);

	orig &= ~bitmask;
	return (orig | ((value << lsb) & bitmask));
}

static void dwc_ddrctl_cinit_seq_pwr_on_rst(uint64_t base_ddr_subsys_reg)
{
	uint32_t rddata;

	// step1: gate aclk core_ddrc_clk
	rddata = mmio_read_32(base_ddr_subsys_reg + 0x0); //bit 0-5 -> gate clk
	rddata = modified_bits_by_value(rddata, 0, 6, 0);
	mmio_write_32(base_ddr_subsys_reg + 0x0, rddata);

	// assert core_ddrc_rstn, areset_n
	rddata = mmio_read_32(base_ddr_subsys_reg + 0x4);
	rddata = modified_bits_by_value(rddata, 0, 5, 0);
	mmio_write_32(base_ddr_subsys_reg + 0x4, rddata);

	// assert preset
	rddata = mmio_read_32(base_ddr_subsys_reg + 0x4);
	rddata = modified_bits_by_value(rddata, 0, 7, 6);
	mmio_write_32(base_ddr_subsys_reg + 0x4, rddata);

	//start clk
	rddata = mmio_read_32(base_ddr_subsys_reg + 0x0);
	rddata = modified_bits_by_value(rddata, 0x7f, 6, 0);
	mmio_write_32(base_ddr_subsys_reg + 0x0, rddata);

	//de-assert preset
	rddata = mmio_read_32(base_ddr_subsys_reg + 0x4);
	rddata = modified_bits_by_value(rddata, 0b11, 7, 6);
	mmio_write_32(base_ddr_subsys_reg + 0x4, rddata);

	// mdelay(1);
	printf("mdelay in fake ddr init\n");

	rddata = mmio_read_32(base_ddr_subsys_reg + 0x4); //bit 5 -> soft-reset
	rddata = modified_bits_by_value(rddata, 0x3f, 5, 0);
	mmio_write_32(base_ddr_subsys_reg + 0x4, rddata);

}

void sg2380_fakeddr_init(void)
{
	for (int i = 0; i < 8; i++) {
		dwc_ddrctl_cinit_seq_pwr_on_rst(DDR_CFG_BASEADDR + i * 0x4000000 + 0x02800000);
		mmio_write_32(DDR_CFG_BASEADDR + i * 0x4000000 + 0x028000d0, 0x0fffffe8);
		mmio_write_32(DDR_CFG_BASEADDR + i * 0x4000000 + 0x028000d4, 0x0fffffe8);
	}

}

void switch_clk_to_pll(void)
{
	mmio_write_32(REG_CLK_BYP_H1A8, 0);
	mmio_write_32(REG_CLK_BYP_H1AC, 0);
	mmio_write_32(REG_CLK_BYP_H1B0, 0);
	mmio_write_32(REG_CLK_BYP_H1B4, 0);
	mmio_write_32(REG_CLK_BYP_H1B8, 0);
}

int boot(void)
{
#if defined(CONFIG_TARGET_PALLADIUM)
	printf("Sophgo SG2380 zsbl!\n");
	switch_clk_to_pll();
	sifive_extensiblecache0_init();
	platform_init();
	ncore_direct_config();
	printf("ncore init done\n");
	sg2380_fakeddr_init();
	cli_loop(0);
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
