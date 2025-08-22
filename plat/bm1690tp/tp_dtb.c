#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <common/common.h>
#include <lib/libc/errno.h>
#include <platform.h>
#include <memmap.h>
#include <ff.h>
#include <of.h>
#include <libfdt.h>

#include "config.h"

static int core_id;

#define TP_SYS_STRIDE   0x10000000

static int modify_node_reg(void *fdt, char *node_path, uint64_t offset)
{
	int nodeoffset;
	const void *nodep;
	char node_name[32];
	char *node;
	uint64_t reg[2] = {0};
	uint64_t addr;
	int len, ret;

	nodeoffset = fdt_path_offset(fdt, node_path);
	if (nodeoffset < 0) {
		pr_err("Unable to find node: %s\n", node_path);
		return -1;
	}

	nodep = fdt_getprop(fdt, nodeoffset, "reg", &len);
	if (nodep == NULL) {
		pr_err("Unable to find property: reg\n");
		return -1;
	}

	pr_info("%s offset is %d, prop reg len is %d\n", node_path, nodeoffset, len);
	pr_info("%s reg start: 0x%llx, len: 0x%llx\n", node_path,
		fdt64_to_cpu(*(fdt64_t *)nodep), fdt64_to_cpu(*((fdt64_t *)nodep + 1)));
	pr_info("%s name is %s\n", node_path, fdt_get_name(fdt, nodeoffset, NULL));

	addr = fdt64_to_cpu(*(fdt64_t *)nodep) + (core_id * offset);
	reg[0] = cpu_to_fdt64(addr);
	reg[1] = *((fdt64_t *)nodep + 1);

	ret = fdt_setprop(fdt, nodeoffset, "reg", reg, len);
	if (ret != 0) {
		pr_err("set property: reg failed\n");
		return -1;
	}

	node = strrchr(node_path, '/');
	if (node == NULL) {
		pr_err("not legal node path: %s\n", node_path);
		return -1;
	}
	node += 1;
	pr_info("first part of node name is %s\n", node);

	sprintf(node_name, "%s@%llx", node, addr);
	ret = fdt_set_name(fdt, nodeoffset, node_name);
	if (ret) {
		pr_err("Unable to set name: %s\n", node_name);
		return -1;
	}
	nodep = fdt_getprop(fdt, nodeoffset, "reg", &len);
	pr_info("%s name is %s\n", node_path, fdt_get_name(fdt, nodeoffset, NULL));

	return 0;
}

static int delete_node_serial(void *fdt)
{
	int nodeoffset;
	int endoffset = 0;
	int ret = 0;

	nodeoffset = fdt_path_offset(fdt, "/soc/serial");
	if (nodeoffset < 0) {
		pr_err("Unable to find node: /soc/serial\n");
		return -1;
	}

	endoffset = fdt_del_node(fdt, nodeoffset);
	if (endoffset < 0) {
		pr_err("Unable to find node: /soc/serial\n");
		return -1;
	}

	nodeoffset = fdt_path_offset(fdt, "/aliases");
	if (nodeoffset < 0) {
		pr_err("Unable to find node: /aliases\n");
		return -1;
	}

	ret = fdt_delprop(fdt, nodeoffset, "serial0");
	if (ret) {
		pr_err("fdt_delprop serial0 ret:%d\n", ret);
	}

	return ret;
}

static int modify_node_vtty(void *fdt)
{
	int nodeoffset;
	const void *nodep;
	char node_name[32];
	uint64_t reg[4] = {0};
	uint32_t irq_reg[3] = {0};
	int len, ret;

	nodeoffset = fdt_path_offset(fdt, "/soc/vtty");
	if (nodeoffset < 0) {
		pr_err("Unable to find node: /soc/vtty\n");
		return -1;
	}

	nodep = fdt_getprop(fdt, nodeoffset, "reg", &len);
	if (nodep == NULL) {
		pr_err("Unable to find property: reg\n");
		return -1;
	}

	pr_info("/soc/vtty offset is %d, prop reg len is %d\n", nodeoffset, len);
	pr_info("/soc/vtty reg start: 0x%llx, len: 0x%llx\n",
		fdt64_to_cpu(*(fdt64_t *)nodep), fdt64_to_cpu(*((fdt64_t *)nodep + 1)));
	pr_info("/soc/vtty name is %s\n", fdt_get_name(fdt, nodeoffset, NULL));

	reg[0] = fdt64_to_cpu(*((fdt64_t *)nodep));
	reg[1] = *((fdt64_t *)nodep + 1);
	reg[0] = cpu_to_fdt64(reg[0] + 2 * core_id * fdt64_to_cpu(reg[1]));
	reg[2] = fdt64_to_cpu(*((fdt64_t *)nodep + 2));
	reg[3] = *((fdt64_t *)nodep + 3);
	reg[2] = cpu_to_fdt64(reg[2] + 2 * core_id * fdt64_to_cpu(reg[1]));

	ret = fdt_setprop(fdt, nodeoffset, "reg", reg, len);
	if (ret != 0)
	{
		pr_err("set property: reg failed\n");
		return -1;
	}

	pr_info("modified tx reg start: 0x%llx, len: 0x%llx\n",
		fdt64_to_cpu(*(fdt64_t *)nodep), fdt64_to_cpu(*((fdt64_t *)nodep + 1)));
	pr_info("modified rx reg start: 0x%llx, len: 0x%llx\n",
		fdt64_to_cpu(*((fdt64_t *)nodep + 2)), fdt64_to_cpu(*((fdt64_t *)nodep + 3)));

	nodep = fdt_getprop(fdt, nodeoffset, "virtaul-msi", &len);
	if (nodep == NULL) {
		pr_err("Unable to find property: virtaul-msi\n");
		return -1;
	}

	irq_reg[0] = *((fdt32_t *)nodep);
	irq_reg[1] = cpu_to_fdt32(fdt32_to_cpu(*((fdt32_t *)nodep + 1)) + core_id * 4);
	irq_reg[2] = *((fdt32_t *)nodep + 2);

	ret = fdt_setprop(fdt, nodeoffset, "virtaul-msi", &irq_reg, len);
	if (ret != 0) {
		pr_err("set property: virtaul-msi failed\n");
		return -1;
	}

	pr_info("modified virtual-msi reg: 0x%llx\n", fdt64_to_cpu(*(fdt64_t *)nodep));

	nodep = fdt_getprop(fdt, nodeoffset, "clr-irq", &len);
	if (nodep == NULL) {
		pr_err("Unable to find property: clr-irq\n");
		return -1;
	}

	irq_reg[0] = *((fdt32_t *)nodep);
	irq_reg[1] = cpu_to_fdt32(fdt32_to_cpu(*((fdt32_t *)nodep + 1)) + core_id * TP_SYS_STRIDE);
	irq_reg[2] = *((fdt32_t *)nodep + 2);

	ret = fdt_setprop(fdt, nodeoffset, "clr-irq", &irq_reg, len);
	if (ret != 0) {
		pr_err("set property: clr-irq failed\n");
		return -1;
	}

	pr_info("modified clr-irq reg: 0x%llx\n", fdt64_to_cpu(*(fdt64_t *)nodep));

	sprintf(node_name, "vtty@%llx", fdt64_to_cpu(reg[0]));
	ret = fdt_set_name(fdt, nodeoffset, node_name);
	if (ret) {
		pr_err("Unable to set name: %s\n", node_name);
		return -1;
	}

	pr_info("/soc/vtty name is %s\n", fdt_get_name(fdt, nodeoffset, NULL));

	nodeoffset = fdt_path_offset(fdt, "/aliases");
	if (nodeoffset < 0) {
		pr_err("Unable to find node: /aliases\n");
		return -1;
	}

	nodep = fdt_getprop(fdt, nodeoffset, "vtty0", &len);
	if (nodep == NULL) {
		pr_err("Unable to find property: vtty0\n");
		return -1;
	}
	pr_info("/aliases offset is %d, prop vtty0 len is %d\n", nodeoffset, len);
	pr_info("old_vtty0: %s\n", (char *)nodep);

	sprintf(node_name, "/soc/vtty@%llx", fdt64_to_cpu(reg[0]));
	ret = fdt_setprop_string(fdt, nodeoffset, "vtty0", node_name);
	if (ret != 0) {
		pr_err("set aliases: vtty0 failed\n");
		return -1;
	}
	pr_info("new_vtty0: %s\n", (char *)fdt_getprop(fdt, nodeoffset, "vtty0", &len));

	return 0;
}

static int modify_node_chosen(struct config *cfg, void *fdt, bool modify_stdout)
{
	int nodeoffset;
	const void *bootargs;
	char *new_bootargs;
	char *substr_start;
	char ramfs_addr_str[32];
	char *end_ptr;
	uint64_t ramfs_addr;
	int len, ret;

	nodeoffset = fdt_path_offset(fdt, "/chosen");
	if (nodeoffset < 0) {
		pr_err("Unable to find node: /chosen\n");
		return -1;
	}

	bootargs = fdt_getprop(fdt, nodeoffset, "bootargs", &len);
	if (bootargs == NULL) {
		pr_err("Unable to find property: bootargs\n");
		return -1;
	}
	pr_info("old_bootargs: %s\n", (char *)bootargs);

	substr_start = strstr(bootargs, "initrd=") + strlen("initrd=");
	ramfs_addr = strtoul(substr_start, &end_ptr, 16);
	if (ramfs_addr == 0 && end_ptr == substr_start) {
		pr_err("Unable to parse initrd\n");
		return -1;
	}

	ramfs_addr += cfg->ram_size * core_id;
	sprintf(ramfs_addr_str, "0x%llx", ramfs_addr);
	new_bootargs = (char *)malloc(strlen(bootargs) + 1 - (end_ptr - substr_start) + strlen(ramfs_addr_str));
	memset(new_bootargs, 0, strlen(bootargs) + 1 - (end_ptr - substr_start) + strlen(ramfs_addr_str));

	strncpy(new_bootargs, bootargs, (const char *)substr_start - (const char *)bootargs);
#if 0
	strncpy(new_bootargs + ((const char *)substr_start - (const char *)bootargs), ramfs_addr_str, strlen(ramfs_addr_str));
#else
	strcpy(new_bootargs + ((const char *)substr_start - (const char *)bootargs), ramfs_addr_str);
#endif

	strcat(new_bootargs, end_ptr);
	pr_info("new_bootargs: %s\n", new_bootargs);

	ret = fdt_setprop_string(fdt, nodeoffset, "bootargs", new_bootargs);
	if (ret) {
		pr_err("Unable to set property: bootargs\n");
		free(new_bootargs);
		return -1;
	}
	free(new_bootargs);

	if (modify_stdout) {
		ret = fdt_setprop_string(fdt, nodeoffset, "stdout-path", "vtty0");
		if (ret)
		{
			pr_err("Unable to set property: stdout-path\n");
			return -1;
		}
	}

	return 0;
}

int modify_tpu_dtb(struct config *cfg)
{
	int ret;

	void *fdt = (void *)cfg->dtb.addr;
	core_id = cfg->core_id;

	ret = fdt_open_into(fdt, fdt, fdt_totalsize(fdt) + 32);
	if (ret < 0) {
		printf("add space to fdt buff fail\n");
		return -1;
	}

	pr_info("modifying tpu dtb, core_id = %d, dtb_addr is 0x%llx\n",
		core_id, (uint64_t)fdt);
	pr_info("fdt_totalsize is %d\n", fdt_totalsize(fdt));
	pr_info("fdt_header_size is %ld\n", fdt_header_size(fdt));
	pr_info("fdt_version is %d\n", fdt_version(fdt));
	pr_info("fdt_magic is %#x\n", fdt_magic(fdt));

	ret = fdt_check_header(fdt);
	if (ret != 0) {
		pr_err("check dtb header failed, ret = %d\n", ret);
		return -1;
	}
	pr_info("check dtb header success\n");
	// TP_SYS_STRIDE unused while modifying memory node
	ret = modify_node_reg(fdt, "/memory", cfg->ram_size);
	if (ret != 0) {
		pr_err("modify memory node failed, ret = %d\n", ret);
		return -1;
	}
	ret = modify_node_reg(fdt, "/soc/interrupt-controller", TP_SYS_STRIDE);
	if (ret != 0) {
		pr_err("modify plic node failed, ret = %d\n", ret);
		return -1;
	}

	if (core_id != 0) {
		delete_node_serial(fdt);
	}

	ret = modify_node_vtty(fdt);
	if (ret != 0) {
		pr_err("modify vtty node failed, ret = %d\n", ret);
		return -1;
	}

	ret = modify_node_chosen(cfg, fdt, true);
	if (ret != 0) {
		pr_err("modify chosen node failed, ret = %d\n", ret);
		return -1;
	}

	return 0;
}
