#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <lib/ini.h>
#include <lib/mac.h>
#include <lib/efi.h>
#include <driver/bootdev.h>
#include <driver/mtd.h>
#include <common/common.h>

#include "config.h"

static int handler_img(void* user, const char* section, const char* name,
				   const char* value)
{
	struct config *pconfig = user;
	uint32_t ctrl_id = 0;
	static uint32_t pcie_use_ini;
	const char *pcie_section_prefix = "pcie";

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
	else if (MATCH("mac-address", "mac0"))
		pconfig->mac0 = str2mac(value);
	else if (MATCH("mac-address", "mac1"))
		pconfig->mac1 = str2mac(value);
	else if (MATCH("boot", "cmdline"))
		pconfig->bootargs = strdup(value);
	else if (strncmp(section, pcie_section_prefix, strlen(pcie_section_prefix)) == 0) {
		if (pcie_use_ini != 0x12345678) {
			pcie_use_ini = 0x12345678;
			for (ctrl_id = 0; ctrl_id < PCIE_MAX; ctrl_id++)
				pconfig->pcie[ctrl_id].enable = false;
		}
		ctrl_id = strtoul((section + strlen(pcie_section_prefix)), NULL, 0);
		if (strcmp(name, "width") == 0) {
			pconfig->pcie[ctrl_id].enable = true;
			pconfig->pcie[ctrl_id].domain = ctrl_id;
		} else if (strcmp(name, "io-addr") == 0) {
			pconfig->pcie[ctrl_id].io.cpu = strtoul(value, NULL, 0);
		} else if (strcmp(name, "io-transl") == 0) {
			pconfig->pcie[ctrl_id].io.pci = strtoul(value, NULL, 0);
		} else if (strcmp(name, "io-length") == 0) {
			pconfig->pcie[ctrl_id].io.len = strtoul(value, NULL, 0);
		} else if (strcmp(name, "pmem64-addr") == 0) {
			pconfig->pcie[ctrl_id].mem64p.cpu = strtoul(value, NULL, 0);
		} else if (strcmp(name, "pmem64-transl") == 0) {
			pconfig->pcie[ctrl_id].mem64p.pci = strtoul(value, NULL, 0);
		} else if (strcmp(name, "pmem64-length") == 0) {
			pconfig->pcie[ctrl_id].mem64p.len = strtoul(value, NULL, 0);
		} else if (strcmp(name, "mem64-addr") == 0) {
			pconfig->pcie[ctrl_id].mem64.cpu = strtoul(value, NULL, 0);
		} else if (strcmp(name, "mem64-transl") == 0) {
			pconfig->pcie[ctrl_id].mem64.pci = strtoul(value, NULL, 0);
		} else if (strcmp(name, "mem64-length") == 0) {
			pconfig->pcie[ctrl_id].mem64.len = strtoul(value, NULL, 0);
		} else if (strcmp(name, "pmem32-addr") == 0) {
			pconfig->pcie[ctrl_id].mem32p.cpu = strtoul(value, NULL, 0);
		} else if (strcmp(name, "pmem32-transl") == 0) {
			pconfig->pcie[ctrl_id].mem32p.pci = strtoul(value, NULL, 0);
		} else if (strcmp(name, "pmem32-length") == 0) {
			pconfig->pcie[ctrl_id].mem32p.len = strtoul(value, NULL, 0);
		} else if (strcmp(name, "mem32-addr") == 0) {
			pconfig->pcie[ctrl_id].mem32.cpu = strtoul(value, NULL, 0);
		} else if (strcmp(name, "mem32-transl") == 0) {
			pconfig->pcie[ctrl_id].mem32.pci = strtoul(value, NULL, 0);
		} else if (strcmp(name, "mem32-length") == 0) {
			pconfig->pcie[ctrl_id].mem32.len = strtoul(value, NULL, 0);
		}
	} else
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

