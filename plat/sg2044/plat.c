#include <framework/common.h>
#include <platform.h>
#include <memmap.h>
#include <asm.h>
#include <lib/mmio.h>
#include <driver/bootdev.h>
#include <sbi.h>
#include <smp.h>

static void print_core_ctrlreg(void)
{
	pr_info("C920 control register information:\n");

#define P_REG(reg) \
	pr_info("\t %-12s - %016lx\n", #reg, csr_read(reg))

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

static void disable_mac_rxdelay(void)
{
	uint32_t misc_conf;

	misc_conf = mmio_read_32(REG_TOP_MISC_CONTROL_ADDR);
	misc_conf |= RGMII0_DISABLE_INTERNAL_DELAY;
	mmio_write_32(REG_TOP_MISC_CONTROL_ADDR, misc_conf);
}

#if 0
static int handler_img(void* user, const char* section, const char* name,
				   const char* value)
{
	config_ini *pconfig = (config_ini *)user;

	#define MATCH(s, n) (strcmp(section, s) == 0 && strcmp(name, n) == 0)

	if (MATCH("devicetree", "name"))
		pconfig->dtb_name = strdup(value);
	else if (MATCH("devicetree", "addr"))
		pconfig->dtb_addr = strtoul(value, NULL, 16);
	else if (MATCH("kernel", "name"))
		pconfig->kernel_name = strdup(value);
	else if (MATCH("kernel", "addr"))
		pconfig->kernel_addr = strtoul(value, NULL, 16);
	else if (MATCH("firmware", "name"))
		pconfig->fw_name = strdup(value);
	else if (MATCH("firmware", "addr"))
		pconfig->fw_addr = strtoul(value, NULL, 16);
	else if (MATCH("ramfs", "name")) {
		if (value && strlen(value))
			pconfig->ramfs_name = strdup(value);
	}
	else if (MATCH("ramfs", "addr"))
		pconfig->ramfs_addr = strtoul(value, NULL, 16);
	else
		return 0;

	return -1;
}

static int parse_config_file(void)
{
	FILINFO info;
	const char *header = "[sophgo-config]";
	const char *tail = "[eof]";
	char *eof;
	void *buf;
	long size;
	const char *file = "conf.ini";

	size = bdm_get_file_size(file);
	if (size < 0) {
		pr_info("No config file found, using default configurations\n");
		return -ENOENT;
	}

	buf = malloc(size);
	if (!size) {
		pr_err("Can not allocate memory for config file\n");
		return -ENOMEM;
	}

	bdm_load(file, buf);

	if (strncmp(header, buf, strlen(header))) {
		pr_err("Config file should start with \"%s\"\n", header);
		return -EINVAL;
	}

	eof = strstr(buf, tail);

	if (!eof) {
		pr_err("conf.ini should terminated by \"%s\"\n", tail);
		return -EINVAL;
	}

	*eof = 0;

	if (ini_parse_string(buf, handler_img, &(sg2042_board_info.config_ini)) < 0)
		return -EINVAL;

	free(buf);

	return 0;
}
#else

static int parse_config_file(void)
{
	pr_info("FIXME: parse config file\n");
	return 0;
}

#endif

struct boot_file {
	char *name;
	uintptr_t addr;
};

static struct boot_file sbi = {
	"fw_dynamic.bin", 0x80000000,
};

static struct boot_file kernel = {
	"SG2044.fd", 0x80200000,
};

static struct boot_file dtb = {
	"sg2044-evb.dtb", 0x88000000,
};

static struct boot_file ramfs = {
	"initrd", 0,
};

static long load(struct boot_file *file)
{
	if (!file->name || !file->addr)
		return 0;

	return bdm_load(file->name, (void *)file->addr);
}

static struct fw_dynamic_info dynamic_info;

static unsigned char secondary_core_stack[CONFIG_SMP_NUM][4096];

static void secondary_core_fun(void *priv)
{
	jump_to(sbi.addr, current_hartid(), dtb.addr, (long)&dynamic_info);
}

void boot_next_img(void)
{
	int i;
	unsigned int hartid = current_hartid();

	for (i = 0; i < CONFIG_SMP_NUM; i++) {
		if (i == hartid)
			continue;

		wake_up_other_core(i, secondary_core_fun, NULL,
				secondary_core_stack[i], sizeof(secondary_core_stack[i]));
	}

	jump_to(sbi.addr, hartid, dtb.addr, (long)&dynamic_info);
}

int plat_main(void)
{

	print_core_ctrlreg();
	disable_mac_rxdelay();

	parse_config_file();

	load(&sbi);
	load(&kernel);
	load(&dtb);
	load(&ramfs);


	dynamic_info.magic = FW_DYNAMIC_INFO_MAGIC_VALUE;
	dynamic_info.version = FW_DYNAMIC_INFO_VERSION_2;
	dynamic_info.next_addr = KERNEL_ADDR;
	dynamic_info.next_mode = FW_DYNAMIC_INFO_NEXT_MODE_S;

	boot_next_img();

	return 0;
}
