#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libfdt.h>

#include <common/common.h>
#include <of.h>

int of_property_read_string(void *blob, char *path,
			    char *propname,
			    char **out_string)
{
	int node;
	int len;
	const void *value = NULL;

	node = fdt_path_offset(blob, path);
	if (node < 0) {
		pr_err("can not find %s\n", path);
		return -1;
	}

	value = fdt_getprop(blob, node, propname, &len);
	if (!value) {
		pr_err("can not get %s\n", propname);
		return -1;
	}

	*out_string = (char *)value;

	return len;
}

int of_property_read_u64(void *blob, char *path,
			 char *propname, uint64_t *out_value)
{
	int node;
	int len;
	const void *value = NULL;

	node = fdt_path_offset(blob, path);
	if (node < 0) {
		pr_err("can not find %s\n", path);
		return -1;
	}

	value = fdt_getprop(blob, node, propname, &len);
	if (!value) {
		pr_err("can not get %s\n", propname);
		return -1;
	}

	if (len != sizeof(uint64_t)) {
		pr_err("the size of uint64_t dont match\n");
		return -1;
	}
	*out_value = fdt64_ld((const fdt64_t *)value);

	return len;
}

int of_property_read_u32(void *blob, char *path,
			 char *propname, uint32_t *out_value)
{
	int node;
	int len;
	const void *value = NULL;

	node = fdt_path_offset(blob, path);
	if (node < 0) {
		pr_err("%s can not find %s\n", __func__, path);
		return -1;
	}

	value = fdt_getprop(blob, node, propname, &len);
	if (!value) {
		pr_err("%s can not get %s\n", __func__, propname);
		return -1;
	}

	if (len != sizeof(uint32_t)) {
		pr_err("the size of uint32_t dont match\n");
		return -1;
	}
	*out_value = fdt32_ld((const fdt32_t *)value);

	return len;
}

extern int32_t g_filelen;
int of_test(const void *fdt)
{
	int len;
	char *prop = NULL;
	uint32_t data32;
	uint64_t data64;

	len = of_property_read_string((void *)fdt, "/", "compatible", &prop);
	if (len > 0) {
		pr_info("read %d bytes, context is \"%s\"\n", len, prop);
	}

	len = of_property_read_u32((void *)fdt, "/", "intr", &data32);
	if (len > 0) {
		pr_info("read 32 data=0x%x\n", data32);
	}

	len = of_property_read_u64((void *)fdt, "/", "u64intr", &data64);
	if (len > 0) {
		pr_info("read 64 data=0x%llx\n", data64);
	}

	of_modify_prop((void *)fdt, g_filelen, "/", "compatible",
		       "newvaluetest_addresses", sizeof("newvaluetest_addresses"), PROP_TYPE_STR);

	len = of_property_read_string((void *)fdt, "/", "compatible", &prop);
	if (len > 0) {
		pr_info("after change read %d bytes, context is \"%s\"\n", len, prop);
	}

	return 0;
}
