#define DEBUG
#include <stdio.h>
#include <timer.h>
#include <lib/mmio.h>
#include <string.h>
#include <framework/module.h>
#include <framework/common.h>
#include <lib/libc/errno.h>
#include <lib/utils_def.h>
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

// #define CONFIG_TPU_SYS

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

static inline uint32_t modified_bits_by_value(uint32_t orig, uint32_t value, uint32_t msb, uint32_t lsb)
{
	uint32_t bitmask = GENMASK_32(msb, lsb);

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

	mdelay(1);

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

void _reg_write_mask(uintptr_t addr, uint32_t mask, uint32_t data)
{
	uint32_t value;

	value = mmio_read_32(addr) & ~mask;
	value |= (data & mask);
	mmio_write_32(addr, value);
}

void sg2380_multimedia_itlvinit(void)
{
	_reg_write_mask(0x5082520000, 0x63, 0x3);
	_reg_write_mask(0x5082520004, 0x7fffffff, 0x7fffffff);
	_reg_write_mask(0x5082520008, 0x63, 0x3);
	_reg_write_mask(0x508252000c, 0x7fffffff, 0x7fffffff);
	_reg_write_mask(0x5082520010, 0x63, 0x3);
	_reg_write_mask(0x5082520014, 0x7fffffff, 0x7fffffff);
	_reg_write_mask(0x5082520018, 0x63, 0x3);
	_reg_write_mask(0x508252001c, 0x7fffffff, 0x7fffffff);
	_reg_write_mask(0x5082520020, 0x63, 0x3);
	_reg_write_mask(0x5082520024, 0x7fffffff, 0x7fffffff);
	_reg_write_mask(0x5082520028, 0x63, 0x3);
	_reg_write_mask(0x508252002c, 0x7fffffff, 0x7fffffff);
	_reg_write_mask(0x5082520030, 0x63, 0x3);
	_reg_write_mask(0x5082520034, 0x7fffffff, 0x7fffffff);
	_reg_write_mask(0x5082520038, 0x63, 0x3);
	_reg_write_mask(0x508252003c, 0x7fffffff, 0x7fffffff);
	printf("itlv init done...\n");
}

#ifdef CONFIG_TPU_SYS
void sg2380_set_tpu_run(void)
{
	// release ec_sys reset
	mmio_write_32(0x5088103000, mmio_read_32(0x5088103000) | (1 << 18));
	printf("calling x280...\n");
	// set x280 rvba
	for (int i = 0; i < 4; i++) {
		mmio_write_32(0x50bf000000 + 0x168 + 0x8 * i, i * 0x1800000);
		mmio_write_32(0x50bf000000 + 0x16c + 0x8 * i, 0x1);
	}
	// reset x280
	mmio_write_32(0x50bf000124, mmio_read_32(0x50bf000124) & ~0x1f);
	mmio_write_32(0x50bf000124, mmio_read_32(0x50bf000124) | 0x1f);
}
#endif

int boot(void)
{
#if defined(CONFIG_TARGET_PALLADIUM)
	printf("Sophgo SG2380 zsbl!\n");
	sifive_extensiblecache0_init();
	platform_init();
	ncore_direct_config();
	printf("ncore init done\n");
	sg2380_fakeddr_init();
	sg2380_multimedia_itlvinit();
	sg2380_ddr_init_asic();
#ifdef CONFIG_TPU_SYS
	sg2380_set_tpu_run();
#endif
	cli_loop(0);
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
