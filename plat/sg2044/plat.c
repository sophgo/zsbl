#include <common/common.h>
#include <platform.h>
#include <memmap.h>
#include <asm.h>
#include <lib/mmio.h>
#include <lib/mac.h>
#include <driver/bootdev.h>
#include <driver/platform.h>
#include <driver/blkdev.h>
#include <sbi.h>
#include <smp.h>
#include <libfdt.h>
#include <fdt_dump.h>
#include <of.h>
#include <lib/cli.h>
#include <lib/akcipher.h>

#include <driver/dtb.h>
#include <stdint.h>

#include "config.h"
#include "efuse.h"
#include "fdt_pcie.h"

#define RAM_BASE_MASK		(~(1UL * 1024 * 1024 * 1024 - 1))

#define OPENSBI_OFFSET		0x00000000
#define KERNEL_OFFSET		0x00200000
#define DEVICETREE_OFFSET	0x08000000
#define DEVICETREE_OVERLAY_OFFSET	0x08800000
#define CFG_FILE_OFFSET		0x09000000
#define RAMFS_OFFSET		0x0b000000

#define PUBKEY_DIG_SIZE		32
#define SIG_SIZE		256

static void command_show_csr(struct command *c, int argc, const char *argv[])
{
#define P_REG(reg)							\
	console_printf(command_get_console(c),				\
		       "%-12s %016lx\n",				\
		       #reg, csr_read(reg))

	P_REG(CSR_MCOR);
	P_REG(CSR_MHCR);
	P_REG(CSR_MCCR2);
	P_REG(CSR_MSMPR);
	P_REG(CSR_MENVCFG);
	P_REG(CSR_MHINT);
	P_REG(CSR_MHINT2);
	P_REG(CSR_MHINT4);
	P_REG(CSR_MXSTATUS);
}

cli_command(lscsr, command_show_csr);

static void disable_mac_rxdelay(void)
{
	uint32_t misc_conf;

	misc_conf = mmio_read_32(REG_TOP_MISC_CONTROL_ADDR);
	misc_conf |= RGMII0_DISABLE_INTERNAL_DELAY;
	mmio_write_32(REG_TOP_MISC_CONTROL_ADDR, misc_conf);
}

static int get_work_mode(void)
{
	uint32_t bootsel;

	bootsel = mmio_read_32(BOOT_SEL_ADDR);
	if (bootsel & BOOT_FROM_SRAM)
		return CHIP_WORK_MODE_PCIE;
	else
		return CHIP_WORK_MODE_CPU;
}

static long load(struct config *cfg, struct boot_file *file, int need_verify)
{
	long err;
	struct boot_file sig;
	char sig_name[64];
	uint8_t sig_data[SIG_SIZE];

	if (!file->name || !file->addr || !file->name[0])
		return 0;

	err = bdm_load(file->name, (void *)file->addr);

	if (err <= 0) {
		pr_err("Load %s failed, stop booting\n", file->name);
		while (true)
			;
	}

	file->size = err;

	if (!cfg->secure || !need_verify) {
		return err;
	}

	sprintf(sig_name, "%s.sig", file->name);
	sig.name = sig_name;
	sig.addr = (uintptr_t)sig_data;

	err = bdm_load(sig.name, (void *)sig.addr);
	if (err <= 0) {
		pr_err("Load %s failed, stop booting\n", file->name);
		while (true)
			;
	}
	sig.size = err;

	err = akcipher_verify(&cfg->akcipher_ctx,
			      (void *)file->addr, (void *)sig.addr,
			      file->size);
	if (err) {
		pr_err("%s verify failed\n", file->name);
		while (true)
			;
	}

	return err;
}

static struct fw_dynamic_info dynamic_info;

static unsigned char secondary_core_stack[CONFIG_SMP_NUM][4096];

static void secondary_core_fun(void *priv)
{
	struct config *cfg = priv;

	jump_to(cfg->sbi.addr, current_hartid(), cfg->dtb.addr, (long)&dynamic_info);
}

static void boot_next_img(struct config *cfg)
{
	int i;
	unsigned int hartid = current_hartid();

	for (i = 0; i < CONFIG_SMP_NUM; i++) {
		if (i == hartid)
			continue;

		wake_up_other_core(i, secondary_core_fun, cfg,
				secondary_core_stack[i], sizeof(secondary_core_stack[i]));
	}

	jump_to(cfg->sbi.addr, hartid, cfg->dtb.addr, (long)&dynamic_info);
}

int parse_config_file(struct config *cfg);
int parse_efi_variable(struct config *cfg);

static void load_dtbs(struct config *cfg)
{
	int err;
	void *dtbi;
	void *dtb = (void *)cfg->dtb.addr;
	void *dtbo = (void *)cfg->dtbo.addr;
	uint32_t dtb_size;
	uint32_t dtbo_size;

	if (cfg->dtb.name)
		dtbi = (void *)cfg->dtb.addr;
	else
		dtbi = dtb_get_base();

	dtb_size = fdt_totalsize(dtbi);
	dtbo_size = dtbo ? fdt_totalsize(dtbo) : 0;

	err = fdt_open_into(dtbi, dtb, ROUND_UP(dtb_size + dtbo_size + (16 * 1024), 8));
	if (err)
		pr_err("Failed to move core dtb to the destination\n");

}

static void merge_dtbs(struct config *cfg)
{
	int err;
	void *dtbo = (void *)cfg->dtbo.addr;
	void *dtb = (void *)cfg->dtb.addr;

	if (!dtbo || !cfg->dtbo.name || !cfg->dtbo.name[0])
		return;

	err = fdt_overlay_apply(dtb, dtbo);
	if (err)
		pr_err("Overlay core dtb and extended dtb failed with error code %d\n", err);
}

/* check if a file is a device tree overlay or not, by file extension */
static int file_is_dtbo(const char *name)
{
	int namelen, extlen;
	const char *ext = ".dtbo";

	if (!name)
		return false;

	namelen = strlen(name);
	extlen = strlen(ext);

	if (namelen < strlen(ext))
		return false;

	return strcmp(&name[namelen - extlen], ext) == 0;
}

static void load_images(struct config *cfg)
{
	uint8_t pubkey_dgst[PUBKEY_DIG_SIZE];
	uint8_t pubkey_der[512];
	struct boot_file pubkey = {0};
	int err;

	if (file_is_dtbo(cfg->dtb.name)) {
		/* check if dtbo is set */
		if (cfg->dtbo.name) {
			/* we cannot set two dtbo at the same time */
			pr_debug("We cannot set two dtbo at the same time\n");
			pr_debug("Ignore dtbo specified by devicetree-overlay command\n");
		}
		cfg->dtbo.name = cfg->dtb.name;
		cfg->dtb.name = NULL;
	}

	if (cfg->secure) {
		pubkey.name = "public_key.der";
		pubkey.addr = (uintptr_t)pubkey_der;
		load(cfg, &pubkey, false);
		/* verify and invoke public key */
		read_pubkey_hash(pubkey_dgst);
		err = akcipher_invoke_key(&cfg->akcipher_ctx,
					  (void *)pubkey.addr, pubkey.size,
					  pubkey_dgst, sizeof(pubkey_dgst));
		if (err) {
			pr_err("Invoke public key failed\n");
			/* must hang here */
			while (1)
				;
		}
	}

	load(cfg, &cfg->dtb, true);
	load(cfg, &cfg->dtbo, true);
	load(cfg, &cfg->sbi, true);
	load(cfg, &cfg->kernel, true);
	load(cfg, &cfg->ramfs, true);
}

static struct config cfg;

static void show_boot_file(const char *name, struct boot_file *p)
{
	pr_info("%-16s %-20s 0x%010lx\n", name, p->name ? p->name : "[null]", p->addr);
}

static const char *mode_names[] = {
	"CPU",
	"SoC",
	"PCIe"
};
static const char *mode2str(int mode)
{
	return mode_names[mode];
}

static const char *conner_names[] = {"TT", "FF", "FS", "SF", "SS"};
static const char *conner2str(int conner)
{
	return conner_names[conner];
}

static void show_config(struct config *cfg)
{
	/* include 0 terminator */
	int i;
	const struct op_point *op;

	op = cfg->op;

	pr_info("\n");
	pr_info("%-16s %s %s\n", "Chip Type",
		conner2str(cfg->conner), cfg->tpu_avl ? "CPU-TPU" : "CPU-ONLY");
	pr_info("%-16s %s | %s\n", "Working Mode",
		mode2str(cfg->mode), cfg->secure ? "Secure" : "Normal");
	pr_info("%-16s %lluMHz\n", "Max CPU Freq", op->cpu_freq / 1000 / 1000);
	pr_info("%-16s %lluMHz\n", "Max TPU Freq", op->tpu_freq / 1000 / 1000);
	pr_info("%-16s %lluMHz\n", "Max NoC Freq", op->noc_freq / 1000 / 1000);
	pr_info("%-16s %llumV\n", "Core Voltage", op->vddr);

	pr_info("%-16s %s %lluGiB %lluMT/s\n", "DRAM Chip",
			cfg->dram.vendor,
			cfg->dram.capacity / 1024 / 1024 / 1024,
			cfg->dram.data_rate / 1000 / 1000 );

	pr_info("%-16s [", "DRAM Map");

	for (i = 0; i < 8; ++i) {
		if ((cfg->dram.channel_map >> i) & 1)
			pr_info("O");
		else
			pr_info("X");

		if (i != 7)
			pr_info(" ");
	}

	pr_info("]\n");


	if (cfg->sn)
		pr_info("%-16s %s\n", "SN", cfg->sn);

	show_boot_file("SBI", &cfg->sbi);
	show_boot_file("Kernel", &cfg->kernel);
	show_boot_file("Device tree", &cfg->dtb);

	if (cfg->ramfs.addr)
		show_boot_file("Ramfs", &cfg->ramfs);

	pr_info("\n");
}

/* #define USE_LINUX_BOOT */

extern unsigned long __ld_program_start[0];

static void init_pcie_res(struct pcie_res *r, uint64_t base, uint64_t len)
{
	r->base = base;
	r->len = len;
}

static void init_pcie_win(struct pcie_win *w, uint64_t pci, uint64_t cpu, uint64_t len)
{
	w->pci = pci;
	w->cpu = cpu;
	w->len = len;
}

#define KHz(f)	(((uint64_t)f) * 1000)
#define MHz(f)	(KHz(f) * 1000)

/* dvfs.vol is 0 is the terminator */

const struct op_point op_point_list[] = {
	{
		/* on tt, ff, fs, sf conner chips, when tpu is disabled */
		"[CPU] [TT FF FS SF]",
		850, MHz(2600), 0, MHz(2000),
		{{1, 0}, {1, 1, 1, 1, 0}},
		{
			{0, MHz(2600) + 1, 850},
			{0, 0, 0},
		}
	},
	{
		/* on ss conner chips, when tpu is disabled */
		"[CPU] [SS]",
		900, MHz(2600), 0, MHz(2000),
		{{1, 0}, {0, 0, 0, 0, 1}},
		{
			{0, MHz(2600) + 1, 900},
			{0, 0, 0},
		}
	},
	{
		/* on all kinde of conner chips, when tpu is enabled */
		"[SOC]",
		800, MHz(2000), MHz(1000), MHz(2000),
		{{0, 1}, {1, 1, 1, 1, 1}},
		{
			{0, MHz(2000) + 1, 800},
			{0, 0, 0},
		}
	},
	/* the default one, always match, it should sets a safe operating point */
	{
		/* on all kinde of conner chips, when tpu is enabled */
		"[DEFAULT]",
		800, MHz(1000), 0, MHz(1000),
		{{1, 1}, {1, 1, 1, 1, 1}},
		{
			{0, MHz(1000) + 1, 800},
			{0, 0, 0},
		}
	},
};

static int condition_match(const struct condition *c, int conner, int mode)
{
	int index;

	index = (mode == CHIP_WORK_MODE_CPU) ? 0 : 1;

	if (!c->tpu[index])
		return false;

	/* tpu status condition matches, check conner */
	/* correct invalid parameter */
	index = ((conner < 0) || (conner >= CHIP_CONNER_MAX)) ? CHIP_CONNER_TT : conner;

	return c->conner[index];
}

static const struct op_point *get_op_point(int conner, int mode)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(op_point_list); ++i) {
		if (condition_match(&op_point_list[i].cond, conner, mode))
			return &op_point_list[i];
	}

	/* should not reach here, the last one(default operating point) should always match */
	return NULL;
}

static void config_init(struct config *cfg)
{
	unsigned long ram_base = (unsigned long)__ld_program_start & RAM_BASE_MASK;
	struct pcie_config *p = cfg->pcie;
	int i;

	pr_debug("ZSBL is loaded at 0x%010lx\n", (unsigned long)__ld_program_start);

	cfg->mode = get_work_mode();

	cfg->sbi.name = "fw_dynamic.bin";
	cfg->sbi.addr = ram_base + OPENSBI_OFFSET;
	cfg->dtb.name = NULL;
	cfg->dtb.addr = ram_base + DEVICETREE_OFFSET;
	cfg->dtbo.name = "sg2044-evb.dtbo";
	cfg->dtbo.addr = ram_base + DEVICETREE_OVERLAY_OFFSET;
#ifdef USE_LINUX_BOOT
	cfg->kernel.name = "riscv64_Image";
	cfg->ramfs.name = "initrd";
	cfg->ramfs.addr = ram_base + RAMFS_OFFSET;
#else
	cfg->kernel.name = "SG2044.fd";
	cfg->ramfs.name = NULL;
	cfg->ramfs.addr = 0;
#endif
	cfg->kernel.addr = ram_base + KERNEL_OFFSET;

	cfg->cfg.name = "conf.ini";
	cfg->cfg.addr = ram_base + CFG_FILE_OFFSET;

	cfg->conner = CHIP_CONNER_TT;
	cfg->tpu_avl = true;

	get_info_in_efuse(cfg);

	/* init pcie config */
	for (i = 0; i < PCIE_MAX; ++i, ++p) {
		p->compatible = "sophgo,sg2044-pcie-host";
		p->bus_start = 0;
		p->bus_end = 255;
		p->coherent = true;
		p->domain = i;
		init_pcie_win(&p->ib, 0x80000000, 0x80000000, 0x4000000000UL - 0x80000000);
		if ((i % 2)  == 1)
			p->enable = false;
		else
			p->enable = true;
	}

	/* pcie0 c2c0-w0-p0 */
	p = &cfg->pcie[0];
	init_pcie_res(&p->dbi, 0x6c00400000UL, 0x1000);
	init_pcie_res(&p->ctl, 0x6c00780000UL, 0x1000);
	init_pcie_res(&p->atu, 0x6c00700000UL, 0x4000);
	init_pcie_res(&p->cfg, 0x4000000000UL, 0x1000);

	init_pcie_win(&p->io, 0, 0x4010000000UL, 0x00200000);
	init_pcie_win(&p->mem32p, 0, 0, 0x04000000);
	init_pcie_win(&p->mem32, 0x04000000, 0x04000000, 0x04000000);
	init_pcie_win(&p->mem64p, 0x4400000000UL, 0x4400000000UL, 0x400000000UL);
	init_pcie_win(&p->mem64, 0x4200000000UL, 0x4200000000UL, 0x200000000UL);

	p->irq = 64;

	/* pcie1 c2c0-w0-p1 */
	p = &cfg->pcie[1];
	init_pcie_res(&p->dbi, 0x6c00800000UL, 0x1000);
	init_pcie_res(&p->ctl, 0x6c00b80000UL, 0x1000);
	init_pcie_res(&p->atu, 0x6c00b00000UL, 0x4000);
	init_pcie_res(&p->cfg, 0x4800000000UL, 0x1000);

	init_pcie_win(&p->io, 0, 0x4810000000UL, 0x00200000);
	init_pcie_win(&p->mem32p, 0x08000000, 0x08000000, 0x04000000);
	init_pcie_win(&p->mem32, 0x0c000000, 0x0c000000, 0x04000000);
	init_pcie_win(&p->mem64p, 0x4c00000000UL, 0x4c00000000UL, 0x400000000UL);
	init_pcie_win(&p->mem64, 0x4a00000000UL, 0x4a00000000UL, 0x200000000UL);

	p->irq = 66;

	/* pcie2 c2c0-w1-p0 */
	p = &cfg->pcie[2];
	init_pcie_res(&p->dbi, 0x6c00000000UL, 0x1000);
	init_pcie_res(&p->ctl, 0x6c000c0000UL, 0x1000);
	init_pcie_res(&p->atu, 0x6c00300000UL, 0x4000);
	init_pcie_res(&p->cfg, 0x5000000000UL, 0x1000);

	init_pcie_win(&p->io, 0, 0x5010000000UL, 0x00200000);
	init_pcie_win(&p->mem32p, 0x10000000, 0x10000000, 0x04000000);
	init_pcie_win(&p->mem32, 0x14000000, 0x14000000, 0x04000000);
	init_pcie_win(&p->mem64p, 0x5400000000UL, 0x5400000000UL, 0x400000000UL);
	init_pcie_win(&p->mem64, 0x5200000000UL, 0x5200000000UL, 0x200000000UL);

	p->irq = 65;

	/* pcie3 c2c0-w1-p1 */
	p = &cfg->pcie[3];
	init_pcie_res(&p->dbi, 0x6c00c00000UL, 0x1000);
	init_pcie_res(&p->ctl, 0x6c00f80000UL, 0x1000);
	init_pcie_res(&p->atu, 0x6c00f00000UL, 0x4000);
	init_pcie_res(&p->cfg, 0x5800000000UL, 0x1000);

	init_pcie_win(&p->io, 0, 0x5810000000UL, 0x00200000);
	init_pcie_win(&p->mem32p, 0x18000000, 0x18000000, 0x04000000);
	init_pcie_win(&p->mem32, 0x1c000000, 0x1c000000, 0x04000000);
	init_pcie_win(&p->mem64p, 0x5c00000000UL, 0x5c00000000UL, 0x400000000UL);
	init_pcie_win(&p->mem64, 0x5a00000000UL, 0x5a00000000UL, 0x200000000UL);

	p->irq = 67;

	/* pcie4 c2c1-w0-p0 */
	p = &cfg->pcie[4];
	init_pcie_res(&p->dbi, 0x6c04400000UL, 0x1000);
	init_pcie_res(&p->ctl, 0x6c04780000UL, 0x1000);
	init_pcie_res(&p->atu, 0x6c04700000UL, 0x4000);
	init_pcie_res(&p->cfg, 0x7800000000UL, 0x1000);

	init_pcie_win(&p->io, 0, 0x7810000000UL, 0x00200000);
	init_pcie_win(&p->mem32p, 0x20000000, 0x20000000, 0x04000000);
	init_pcie_win(&p->mem32, 0x24000000, 0x24000000, 0x04000000);
	init_pcie_win(&p->mem64p, 0x7900000000UL, 0x7900000000UL, 0x100000000UL);
	init_pcie_win(&p->mem64, 0x7880000000UL, 0x7880000000UL, 0x080000000UL);

	p->irq = 73;

	/* pcie5 c2c1-w0-p1 */
	p = &cfg->pcie[5];
	init_pcie_res(&p->dbi, 0x6c04800000UL, 0x1000);
	init_pcie_res(&p->ctl, 0x6c04b80000UL, 0x1000);
	init_pcie_res(&p->atu, 0x6c04b00000UL, 0x4000);
	init_pcie_res(&p->cfg, 0x7a00000000UL, 0x1000);

	init_pcie_win(&p->io, 0, 0x7a10000000UL, 0x00200000);
	init_pcie_win(&p->mem32p, 0x28000000, 0x28000000, 0x04000000);
	init_pcie_win(&p->mem32, 0x2c000000, 0x2c000000, 0x04000000);
	init_pcie_win(&p->mem64p, 0x7b00000000UL, 0x7b00000000UL, 0x100000000UL);
	init_pcie_win(&p->mem64, 0x7a80000000UL, 0x7a80000000UL, 0x080000000UL);

	p->irq = 75;

	/* pcie6 c2c1-w1-p0 */
	p = &cfg->pcie[6];
	init_pcie_res(&p->dbi, 0x6c04000000UL, 0x1000);
	init_pcie_res(&p->ctl, 0x6c040c0000UL, 0x1000);
	init_pcie_res(&p->atu, 0x6c04300000UL, 0x4000);
	init_pcie_res(&p->cfg, 0x7c00000000UL, 0x1000);

	init_pcie_win(&p->io, 0, 0x7c10000000UL, 0x00200000);
	init_pcie_win(&p->mem32p, 0x30000000, 0x30000000, 0x04000000);
	init_pcie_win(&p->mem32, 0x34000000, 0x34000000, 0x04000000);
	init_pcie_win(&p->mem64p, 0x7d00000000UL, 0x7d00000000UL, 0x100000000UL);
	init_pcie_win(&p->mem64, 0x7c80000000UL, 0x7c80000000UL, 0x080000000UL);

	p->irq = 74;

	/* pcie7 c2c1-w1-p1 */
	p = &cfg->pcie[7];
	init_pcie_res(&p->dbi, 0x6c04c00000UL, 0x1000);
	init_pcie_res(&p->ctl, 0x6c04f80000UL, 0x1000);
	init_pcie_res(&p->atu, 0x6c04f00000UL, 0x4000);
	init_pcie_res(&p->cfg, 0x7e00000000UL, 0x1000);

	init_pcie_win(&p->io, 0, 0x7e10000000UL, 0x00200000);
	init_pcie_win(&p->mem32p, 0x38000000, 0x38000000, 0x04000000);
	init_pcie_win(&p->mem32, 0x3c000000, 0x3c000000, 0x04000000);
	init_pcie_win(&p->mem64p, 0x7f00000000UL, 0x7f00000000UL, 0x100000000UL);
	init_pcie_win(&p->mem64, 0x7e80000000UL, 0x7e80000000UL, 0x080000000UL);

	p->irq = 76;

	/* pcie8 cxp-w0-p0 */
	p = &cfg->pcie[8];
	init_pcie_res(&p->dbi, 0x6c08400000UL, 0x1000);
	init_pcie_res(&p->ctl, 0x6c08780000UL, 0x1000);
	init_pcie_res(&p->atu, 0x6c08700000UL, 0x4000);
	init_pcie_res(&p->cfg, 0x6000000000UL, 0x1000);

	init_pcie_win(&p->io, 0, 0x6010000000UL, 0x00200000);
	init_pcie_win(&p->mem32p, 0x40000000, 0x40000000, 0x04000000);
	init_pcie_win(&p->mem32, 0x44000000, 0x44000000, 0x04000000);
	init_pcie_win(&p->mem64p, 0x6200000000UL, 0x6200000000UL, 0x200000000UL);
	init_pcie_win(&p->mem64, 0x6100000000UL, 0x6100000000UL, 0x100000000UL);

	p->irq = 125;

	/* pcie9 cxp-w0-p1 */
	p = &cfg->pcie[9];
	init_pcie_res(&p->dbi, 0x6c08800000UL, 0x1000);
	init_pcie_res(&p->ctl, 0x6c08b80000UL, 0x1000);
	init_pcie_res(&p->atu, 0x6c08b00000UL, 0x4000);
	init_pcie_res(&p->cfg, 0x6400000000UL, 0x1000);

	init_pcie_win(&p->io, 0, 0x6410000000UL, 0x00200000);
	init_pcie_win(&p->mem32p, 0x48000000, 0x48000000, 0x04000000);
	init_pcie_win(&p->mem32, 0x4c000000, 0x4c000000, 0x04000000);
	init_pcie_win(&p->mem64p, 0x6600000000UL, 0x6600000000UL, 0x200000000UL);
	init_pcie_win(&p->mem64, 0x6500000000UL, 0x6500000000UL, 0x100000000UL);

	p->irq = 126;

}

/* Resize fdt to modify some node, eg: bootargs */
int resize_dtb(struct config *cfg, int delta)
{
	void *fdt;
	int size;
	int ret = 0;

	fdt = (void *)cfg->dtb.addr;
	size = fdt_totalsize(fdt) + delta;

	ret = fdt_open_into(fdt, fdt, size);
	if (ret != 0)
		pr_err("fdt: resize failed, error[%d\n]", ret);

	return ret;
}

static void modify_eth_node(struct config *cfg)
{
	uint8_t byte[6];

	if (cfg->mac0)
		of_modify_prop((void *)cfg->dtb.addr, cfg->dtb.size,
				"/soc/ethernet@7030006000/", "local-mac-address",
				mac2byte(cfg->mac0, byte), 6, PROP_TYPE_U8);

	if (cfg->mac1)
		of_modify_prop((void *)cfg->dtb.addr, cfg->dtb.size,
				"/soc/dwcxlg@6c08000000/", "local-mac-address",
				mac2byte(cfg->mac1, byte), 6, PROP_TYPE_U8);
}

static void modify_memory_node(struct config *cfg)
{
	uint64_t dram_base, dram_size, system_memory_size;
	void *fdt;
	char memory_node_name[64] = "/memory";
	int root_node_offset, memory_node_offset;

	dram_base = 0x80000000;
	dram_size = cfg->dram.channel_number * cfg->dram.capacity;
	system_memory_size = dram_size - cfg->reserved_memory_size;

	fdt = (void *)cfg->dtb.addr;

	/* remove all memory nodes */
	do {
		memory_node_offset = fdt_path_offset_namelen(fdt, memory_node_name,
							     strlen(memory_node_name));

		if (memory_node_offset >= 0) {
			pr_debug("Remove memory node at offset %d\n", memory_node_offset);
			fdt_del_node(fdt, memory_node_offset);
		}

	} while (memory_node_offset >= 0);

	/* add memory node at root node */
	root_node_offset = fdt_path_offset(fdt, "/");

	snprintf(memory_node_name, sizeof(memory_node_name), "memory@%llx", dram_base);

	memory_node_offset = fdt_add_subnode(fdt, root_node_offset, memory_node_name);
	if (memory_node_offset < 0) {
		pr_err("Failed to add memory node %s\n", memory_node_name);
		return;
	}

	if (fdt_appendprop_addrrange(fdt, root_node_offset, memory_node_offset, "reg",
				     dram_base, system_memory_size)) {
		pr_err("Failed to add memory region in node %s\n", memory_node_name);
		return;
	}

	if (fdt_setprop_string(fdt, memory_node_offset, "device_type", "memory")) {
		pr_err("Failed to set memory node type for node %s\n", memory_node_name);
		return;
	}
}

static void add_cppc_node(struct config *cfg)
{
	const struct op_point *op;
	void *fdt;
	int off;
	struct dvfs_entry dvfs[DVFS_ENTRY_MAX];
	int i;

	op = cfg->op;
	fdt = (void *)cfg->dtb.addr;

	off = fdt_path_offset(fdt, "/soc");
	if (off < 0) {
		pr_err("Cannot find \"/soc\" node\n");
		return;
	}

	off = fdt_add_subnode(fdt, off, "cppc@0");
	fdt_setprop_string(fdt, off, "compatible", "sophgo,sg2044-cppc");
	fdt_setprop_u64(fdt, off, "min-frequency", MHz(1000));
	fdt_setprop_u64(fdt, off, "max-frequency", op->cpu_freq);
	fdt_setprop_u64(fdt, off, "step", MHz(25));
	fdt_setprop_string(fdt, off, "clock-names", "mpll1-clock");

	for (i = 0; op->dvfs[i].vol; ++i) {
		/* convert endian */
		dvfs[i].min = cpu_to_fdt64(op->dvfs[i].min);
		dvfs[i].max = cpu_to_fdt64(op->dvfs[i].max);
		dvfs[i].vol = cpu_to_fdt64(op->dvfs[i].vol);
	}

	if (i)
		fdt_setprop(fdt, off, "frequency-voltage-relationship",
			    dvfs, i * sizeof(struct dvfs_entry));
}

static int of_get_chosen(void *fdt)
{
	int node;

	node = fdt_path_offset(fdt, "/chosen");
	if (node < 0) {
		node = fdt_path_offset(fdt, "/");
		node = fdt_add_subnode(fdt, node, "chosen");
		if (node < 0) {
			pr_err("fdt: create /chosen failed, error[%d]\n", node);
			return -1;
		}
	}

	return node;
}

static int modify_initramfs(struct config *cfg)
{
	uint64_t addr, size;
	void *fdt;
	int node, ret;

	if (!cfg->ramfs.name)
		return 0;

	addr = (uint64_t)cfg->ramfs.addr;
	size = (uint64_t)cfg->ramfs.size;
	fdt = (void *)cfg->dtb.addr;

	node = of_get_chosen(fdt);
	if (node < 0)
		return node;

	ret = fdt_setprop_u64(fdt, node, "linux,initrd-start", addr);
	if (ret < 0) {
		pr_err("fdt: failed to set linux,initrd-start, error[%d]\n", ret);
		return -1;
	}

	ret = fdt_setprop_u64(fdt, node, "linux,initrd-end", addr + size);
	if (ret < 0) {
		pr_err("fdt: failed to set linux,initrd-end, error[%d]\n", ret);
		return -1;
	}

	return 0;
}

static int modify_bootargs(struct config *cfg)
{
	void *fdt;
	int node;
	int ret;

	if (!cfg->bootargs)
		return 0;

	fdt = (void *)cfg->dtb.addr;

	node = of_get_chosen(fdt);
	if (node < 0)
		return node;

	ret = fdt_setprop_string(fdt, node, "bootargs", cfg->bootargs);
	if (ret < 0) {
		pr_err("fdt: failed to set bootargs, error[%d]\n", ret);
		return -1;
	}

	return 0;
}

static void modify_dtb(struct config *cfg)
{
	load_dtbs(cfg);
	merge_dtbs(cfg);

	modify_eth_node(cfg);
	modify_memory_node(cfg);
	modify_pcie_node(cfg);
	add_cppc_node(cfg);
	modify_initramfs(cfg);
	modify_bootargs(cfg);
}

int plat_main(void)
{
	disable_mac_rxdelay();

	config_init(&cfg);

	if (cfg.mode != CHIP_WORK_MODE_PCIE) {
		parse_config_file(&cfg);
		parse_efi_variable(&cfg);
		cfg.op = get_op_point(cfg.conner, cfg.mode);
		show_config(&cfg);
		cli_loop(100000);
		load_images(&cfg);
		modify_dtb(&cfg);
	} else {
		show_config(&cfg);
	}

	dynamic_info.magic = FW_DYNAMIC_INFO_MAGIC_VALUE;
	dynamic_info.version = FW_DYNAMIC_INFO_VERSION_2;
	dynamic_info.next_addr = cfg.kernel.addr;
	dynamic_info.next_mode = FW_DYNAMIC_INFO_NEXT_MODE_S;
	dynamic_info.boot_hart = 0xffffffffffffffffUL;

	boot_next_img(&cfg);

	return 0;
}

