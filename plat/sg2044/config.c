#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <lib/ini.h>
#include <driver/bootdev.h>
#include <framework/common.h>

#include <board.h>

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
	else
		return 0;

	return -1;
}

int parse_config_file(struct config *cfg)
{
	const char *header = "[sophgo-config]";
	const char *tail = "[eof]";
	char *eof;
	void *buf;
	long size;
	const char *file = "conf.ini";

	size = bdm_get_file_size(file);
	if (size < 0) {
		pr_debug("No config file found, using default configurations\n");
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

	if (ini_parse_string(buf, handler_img, cfg) < 0)
		return -EINVAL;

	free(buf);

	return 0;
}

