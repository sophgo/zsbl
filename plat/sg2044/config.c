#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <lib/ini.h>
#include <lib/mac.h>
#include <driver/bootdev.h>
#include <framework/common.h>

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

