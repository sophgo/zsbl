#include <stdio.h>
#include <lib/mmio.h>
#include <sbi/riscv_asm.h>
#include <memmap.h>
#include <timer.h>
#include <platform.h>
#include <sifive_hwpf1.h>
#include <sifive_pl2cache0.h>

void sg2380_ssperi_phy_config(int mode)
{
	uint32_t value;

	value = mmio_read_32(SSPERI_PHY1_INTF_REG);
	value = (value & ~(SSPERI_MODE_MASK)) | mode;
	mmio_write_32(SSPERI_PHY1_INTF_REG, value);
}

void sg2380_eth_type_config(int type)
{
	uint32_t value;

	value = mmio_read_32(SSPERI_PHY1_INTF_REG);
	value = (value & ~(ETH_TYPE_MASK << ETH_TYPE_SHIFT)) | (type << ETH_TYPE_SHIFT);
	mmio_write_32(SSPERI_PHY1_INTF_REG, value);
}

void sg2380_eth_mul_channel_intr_enable(void)
{
	uint32_t val;

	val = 0xffffffff;
	mmio_write_32(SSPERI_SYS_TOP + 0x11c, val);

	val = mmio_read_32(SSPERI_SYS_TOP + 0x124);
	val &= ~(1 << 2 | 1 << 13);
	val |= 0xffff0000;
	mmio_write_32(SSPERI_SYS_TOP + 0x124, val);

	val = mmio_read_32(SSPERI_SYS_TOP + 0x12c);
	val &= ~(1 << 18);
	val |= 0xffff;
	mmio_write_32(SSPERI_SYS_TOP + 0x12c, val);

	val = 0xffffffff;
	mmio_write_32(SSPERI_SYS_TOP + 0x134, val);

	val = mmio_read_32(SSPERI_SYS_TOP + 0x13c);
	val &= ~(1 << 2 | 1 << 13);
	val |= 0xffff0000;
	mmio_write_32(SSPERI_SYS_TOP + 0x13c, val);

	val = mmio_read_32(SSPERI_SYS_TOP + 0x144);
	val &= ~(1 << 18);
	val |= 0xffff;
	mmio_write_32(SSPERI_SYS_TOP + 0x144, val);

	printf("eth: enable muli irq and disable mac_sbd_irq\n");
}

static void _reg_write_mask(uintptr_t addr, uint32_t mask, uint32_t data)
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

	printf("multimedia itlv init done...\n");
}

void sg2380_set_tpu_run(uint64_t addr)
{
	// release ec_sys reset
	mmio_write_32(0x5088103000, mmio_read_32(0x5088103000) | (1 << 18));
	printf("calling x280...\n");
	// set x280 rvba
	for (int i = 0; i < 4; i++) {
		mmio_write_32(0x50bf000000 + 0x168 + 0x8 * i, addr & 0xffffffff);
		mmio_write_32(0x50bf000000 + 0x16c + 0x8 * i, (addr >> 32) & 0xffffffff);
	}
	// reset x280
	mmio_write_32(0x50bf000124, mmio_read_32(0x50bf000124) & ~0x1f);
	mmio_write_32(0x50bf000124, mmio_read_32(0x50bf000124) | 0x1f);
}

void sg2380_top_reset(uint32_t index)
{
	uint32_t ret;

	ret = mmio_read_32(SOFT_RESET_REG2);
	ret &= ~(1 << index);
	mmio_write_32(SOFT_RESET_REG2, ret);

	timer_udelay(100);

	ret |= 1 << index;
	mmio_write_32(SOFT_RESET_REG2, ret);
}

void sg2380_platform_init(void)
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
