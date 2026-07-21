#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <lib/ini.h>
#include <lib/efi.h>
#include <driver/bootdev.h>
#include <driver/mtd.h>
#include <common/common.h>

#include "config.h"

static int handler_img(void* user, const char* section, const char* name,
				   const char* value)
{
	struct config *pconfig = user;

	#define MATCH(s, n) (strcmp(section, s) == 0 && strcmp(name, n) == 0)

	if (MATCH("devicetree", "name"))
		pconfig->dtb.name = strdup(value);
	else if (MATCH("devicetree", "addr"))
		pconfig->dtb.addr = strtoul(value, NULL, 16);
	else if (MATCH("devicetree-overlay", "name"))
		pconfig->dtbo.name = strdup(value);
	else if (MATCH("devicetree-overlay", "addr"))
		pconfig->dtbo.addr = strtoul(value, NULL, 16);
	else if (MATCH("kernel", "name"))
		pconfig->kernel.name = strdup(value);
	else if (MATCH("kernel", "addr"))
		pconfig->kernel.addr = strtoul(value, NULL, 16);
	else if (MATCH("firmware", "name"))
		pconfig->sbi.name = strdup(value);
	else if (MATCH("firmware", "addr"))
		pconfig->sbi.addr = strtoul(value, NULL, 16);
	else if (MATCH("ramfs", "name"))
		pconfig->ramfs.name = strdup(value);
	else if (MATCH("ramfs", "addr"))
		pconfig->ramfs.addr = strtoul(value, NULL, 16);
	else if (MATCH("boot", "cmdline"))
		pconfig->bootargs = strdup(value);
	else
		return 0;

	return -1;
}

int parse_config_file(struct config *cfg)
{
	const char *header = "[sophgo-config]";
	const char *tail = "[eof]";
	char *eof;
	long size;

	size = bdm_get_file_size(cfg->cfg.name);
	if (size <= 0) {
		pr_debug("No config file found, using default configurations\n");
		return -ENOENT;
	}

	bdm_load(cfg->cfg.name, (void *)cfg->cfg.addr);

	if (strncmp(header, (void *)cfg->cfg.addr, strlen(header))) {
		pr_err("Config file should start with \"%s\"\n", header);
		return -EINVAL;
	}

	eof = strstr((void *)cfg->cfg.addr, tail);

	if (!eof) {
		pr_err("conf.ini should terminated by \"%s\"\n", tail);
		return -EINVAL;
	}

	eof += strlen(tail);
	*eof = 0;

	if (ini_parse_string((void *)cfg->cfg.addr, handler_img, cfg) < 0)
		return -EINVAL;

	return 0;
}

#define EFI_VARIABLE_FIRMWARE_VOLUME_OFFSET	(0x600000 + 0x800000)
#define EFI_VARIABLE_FIRMWARE_VOLUME_SIZE	(0x10000)

int parse_efi_variable(struct config *cfg)
{
	struct mtd *mtd;
	void *fv;
	struct efi_variable_zone vz;
	struct efi_variable var;
	int err;
	uint32_t rms_in_gb;

	const struct efi_guid sophgo_variable_guid = {
		0x570CF83D, 0x5A8D, 0x4F79,
		{ 0x91, 0xA9, 0xBA, 0x82, 0x8F, 0x05, 0x79, 0xF6 }
	};
	const wchar_t *rms_var_name = L"ReservedMemorySize";

	mtd = mtd_find_by_name("flash0");

	if (!mtd) {
		pr_debug("Device not found\n");
		return 0;
	}

	fv = malloc(EFI_VARIABLE_FIRMWARE_VOLUME_SIZE);

	if (!fv) {
		pr_err("Allocate EFI variable buffer failed\n");
		return -ENOMEM;
	}

	if (mtd_read(mtd, EFI_VARIABLE_FIRMWARE_VOLUME_OFFSET,
		     EFI_VARIABLE_FIRMWARE_VOLUME_SIZE, fv) < 0) {
		pr_err("Read EFI variable firmware volume failed\n");
		return -EIO;
	}

	err = efi_vz_init(&vz, fv);
	if (err)
		return err;

	err = efi_vz_find_variable(&vz, &var, &sophgo_variable_guid, rms_var_name);
	if (err) {
		pr_debug("Variable %s not found\n", "ReservedMemorySize");
		return 0;
	}

	/* little endian */
	memcpy(&rms_in_gb, efi_variable_data(&var), sizeof(rms_in_gb));

	/* little endian */
	cfg->reserved_memory_size = (uint64_t)rms_in_gb * 1024 * 1024 * 1024;

	if (cfg->reserved_memory_size)
	    cfg->mode = CHIP_WORK_MODE_SOC;

	pr_debug("Reserved memory size 0x%lx\n", cfg->reserved_memory_size);

	return 0;
}

