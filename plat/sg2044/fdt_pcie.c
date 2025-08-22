#include <libfdt.h>
#include <fdt_dump.h>

#include <common/common.h>

#include "config.h"

static void pcie_win_to_dt_prop(void *p, uint32_t type, struct pcie_win *w)
{
	((uint32_t *)p)[0] = cpu_to_fdt32(type);
	p += 4;
	((uint64_t *)p)[0] = cpu_to_fdt64(w->pci);
	((uint64_t *)p)[1] = cpu_to_fdt64(w->cpu);
	((uint64_t *)p)[2] = cpu_to_fdt64(w->len);
}

static void pcie_res_to_dt_prop(void *p, struct pcie_res *r)
{
	((uint64_t *)p)[0] = cpu_to_fdt64(r->base);
	((uint64_t *)p)[1] = cpu_to_fdt64(r->len);
}

static void pcie_gen_int_map_n(void *p, int n,
			       uint32_t b0, uint32_t b1, uint32_t b2,
			       uint32_t b3, uint32_t b4, uint32_t b5)
{
	p += n * 6 * 4;
	((uint32_t *)p)[0] = cpu_to_fdt32(b0);
	((uint32_t *)p)[1] = cpu_to_fdt32(b1);
	((uint32_t *)p)[2] = cpu_to_fdt32(b2);
	((uint32_t *)p)[3] = cpu_to_fdt32(b3);
	((uint32_t *)p)[4] = cpu_to_fdt32(b4);
	((uint32_t *)p)[5] = cpu_to_fdt32(b5);
}

static void pcie_gen_int_map_dt(void *p, int phandle)
{
	pcie_gen_int_map_n(p, 0, 0, 0, 0, 1, phandle, 0);
	pcie_gen_int_map_n(p, 1, 0, 0, 0, 2, phandle, 1);
	pcie_gen_int_map_n(p, 2, 0, 0, 0, 3, phandle, 2);
	pcie_gen_int_map_n(p, 3, 0, 0, 0, 4, phandle, 3);
}

static int pcie_reg_names_to_dt_prop(void *p, int n, ...)
{
	va_list ap;
	char *reg_name;
	int reg_name_len, prop_len;
	int i;

	((char *)p)[0] = 0;

	va_start(ap, n);

	prop_len = 0;
	for (i = 0; i < n; ++i) {
		reg_name = va_arg(ap, char *);
		reg_name_len = strlen(reg_name);
		memcpy(p, reg_name, reg_name_len + 1);
		prop_len += reg_name_len + 1;
		p += reg_name_len + 1;
	}

	return prop_len;
}

static int add_pcie_node(void *fdt, struct pcie_config *pcfg)
{
	int soc_node_offset;
	int pcie_node_offset;

	int msic_node_offset;
	int intc_node_offset;

	uint32_t msic_phandle;
	uint32_t intc_phandle;

	const struct fdt_property *msic_phandle_prop;
	const struct fdt_property *intc_phandle_prop;

	int prop_len;

	char __attribute__((aligned(8))) tmp[256];

	soc_node_offset = fdt_path_offset(fdt, "/soc");

	if (soc_node_offset < 0) {
		pr_err("No /soc node found\n");
		return soc_node_offset;
	}

	sprintf(tmp, "pcie@%llx", pcfg->dbi.base);

	pcie_node_offset = fdt_add_subnode(fdt, soc_node_offset, tmp);

	*(uint32_t *)tmp = cpu_to_fdt32(3);
	fdt_setprop(fdt, pcie_node_offset, "#address-cells", tmp, 4);
	*(uint32_t *)tmp = cpu_to_fdt32(2);
	fdt_setprop(fdt, pcie_node_offset, "#size-cells", tmp, 4);

	fdt_setprop_string(fdt, pcie_node_offset, "compatible", pcfg->compatible);

	fdt_setprop_string(fdt, pcie_node_offset, "device_type", "pci");

	((uint32_t *)tmp)[0] = cpu_to_fdt32(pcfg->bus_start);
	((uint32_t *)tmp)[1] = cpu_to_fdt32(pcfg->bus_end);
	fdt_setprop(fdt, pcie_node_offset, "bus-range", tmp, 8);

	*(uint32_t *)tmp = cpu_to_fdt32(pcfg->domain);
	fdt_setprop(fdt, pcie_node_offset, "linux,pci-domain", tmp, 4);

	msic_node_offset = fdt_path_offset_namelen(fdt, "/soc/msi-controller",
						  strlen("/soc/msi-controller"));
	/* find a phandle in msi controller */
	msic_phandle_prop = fdt_get_property(fdt, msic_node_offset, "phandle", NULL);
	if (msic_phandle_prop) {
		msic_phandle = fdt32_to_cpu(*(uint32_t *)(msic_phandle_prop->data));
	} else {
		/* add a phandle in it */
		if (fdt_find_max_phandle(fdt, &msic_phandle) < 0)
			msic_phandle = -1;

		/* unique phandle int the whole dtb */
		++msic_phandle;
		*(uint32_t *)tmp = cpu_to_fdt32(msic_phandle);
		fdt_setprop(fdt, msic_node_offset, "phandle", tmp, 4);
	}

	*(uint32_t *)tmp = cpu_to_fdt32(msic_phandle);
	fdt_setprop(fdt, pcie_node_offset, "msi-parent", tmp, 4);

	pcie_res_to_dt_prop(tmp + 0 * 16, &pcfg->dbi);
	pcie_res_to_dt_prop(tmp + 1 * 16, &pcfg->ctl);
	pcie_res_to_dt_prop(tmp + 2 * 16, &pcfg->atu);
	pcie_res_to_dt_prop(tmp + 3 * 16, &pcfg->cfg);

	fdt_setprop(fdt, pcie_node_offset, "reg", tmp, 4 * 16);

	prop_len = pcie_reg_names_to_dt_prop(tmp, 4, "dbi", "ctrl_base", "atu", "config");

	fdt_setprop(fdt, pcie_node_offset, "reg-names", tmp, prop_len);

	if (pcfg->coherent)
		fdt_setprop(fdt, pcie_node_offset, "dma-coherent", NULL, 0);
	else
		fdt_setprop(fdt, pcie_node_offset, "dma-noncoherent", NULL, 0);

	pcie_win_to_dt_prop(tmp + (4 + 8 * 3) * 0, 0x01000000, &pcfg->io);
	pcie_win_to_dt_prop(tmp + (4 + 8 * 3) * 1, 0x42000000, &pcfg->mem32p);
	pcie_win_to_dt_prop(tmp + (4 + 8 * 3) * 2, 0x02000000, &pcfg->mem32);
	pcie_win_to_dt_prop(tmp + (4 + 8 * 3) * 3, 0x43000000, &pcfg->mem64p);
	pcie_win_to_dt_prop(tmp + (4 + 8 * 3) * 4, 0x03000000, &pcfg->mem64);

	fdt_setprop(fdt, pcie_node_offset, "ranges", tmp, (4 + 8 * 3) * 5);

	pcie_win_to_dt_prop(tmp + (4 + 8 * 3) * 0, 0x03000000, &pcfg->ib);
	fdt_setprop(fdt, pcie_node_offset, "dma-ranges", tmp, (4 + 8 * 3) * 1);

	intc_node_offset = fdt_path_offset(fdt, "/soc/interrupt-controller@6d40000000");
	/* find a phandle in msi controller */
	intc_phandle_prop = fdt_get_property(fdt, intc_node_offset, "phandle", NULL);
	if (intc_phandle_prop) {
		intc_phandle = fdt32_to_cpu(*(uint32_t *)(intc_phandle_prop->data));
	} else {
		/* add a phandle in it */
		if (fdt_find_max_phandle(fdt, &intc_phandle) < 0)
			intc_phandle = -1;

		/* unique phandle int the whole dtb */
		++intc_phandle;
		*(uint32_t *)tmp = cpu_to_fdt32(intc_phandle);
		fdt_setprop(fdt, intc_node_offset, "phandle", tmp, 4);
	}

	((uint32_t *)tmp)[0] = cpu_to_fdt32(intc_phandle);
	((uint32_t *)tmp)[1] = cpu_to_fdt32(pcfg->irq);
	((uint32_t *)tmp)[2] = cpu_to_fdt32(4); /* IRQ_TYPE_LEVEL_HIGH */

	fdt_setprop(fdt, pcie_node_offset, "interrupts-extended", tmp, 4 * 3);
	fdt_setprop_string(fdt, pcie_node_offset, "interrupt-names", "pcie_irq");
	*(uint32_t *)tmp = cpu_to_fdt32(1);
	fdt_setprop(fdt, pcie_node_offset, "#interrupt-cells", tmp, 4);

	((uint32_t *)tmp)[0] = cpu_to_fdt32(0);
	((uint32_t *)tmp)[1] = cpu_to_fdt32(0);
	((uint32_t *)tmp)[2] = cpu_to_fdt32(0);
	((uint32_t *)tmp)[3] = cpu_to_fdt32(7);
	fdt_setprop(fdt, pcie_node_offset, "interrupt-map-mask", tmp, 4 * 4);

	intc_node_offset = fdt_add_subnode(fdt, pcie_node_offset, "interrupt-controller");
	/* add interrupt-controller first, we need phandle of this node */
	if (fdt_find_max_phandle(fdt, &intc_phandle) < 0)
		intc_phandle = -1;

	/* unique phandle in the whole dtb */
	++intc_phandle;

	*(uint32_t *)tmp = cpu_to_fdt32(0);
	fdt_setprop(fdt, intc_node_offset, "#address-cells", tmp, 4);
	*(uint32_t *)tmp = cpu_to_fdt32(1);
	fdt_setprop(fdt, intc_node_offset, "#interrupt-cells", tmp, 4);
	fdt_setprop(fdt, intc_node_offset, "interrupt-controller", NULL, 0);
	*(uint32_t *)tmp = cpu_to_fdt32(intc_phandle);
	fdt_setprop(fdt, intc_node_offset, "phandle", tmp, 4);

	pcie_gen_int_map_dt(tmp, intc_phandle);

	fdt_setprop(fdt, pcie_node_offset, "interrupt-map", tmp, 4 * 6 * 4);

	fdt_setprop_string(fdt, pcie_node_offset, "status", pcfg->enable ? "okay" : "disabled");

	return pcie_node_offset;
}

void modify_pcie_node(struct config *cfg)
{
	const char *pcie_node_name = "/soc/pcie";
	int pcie_node_offset;
	void *fdt = (void *)cfg->dtb.addr;
	int i;

	/* remove all pcie node first */
	do {
		pcie_node_offset = fdt_path_offset_namelen(fdt, pcie_node_name,
							   strlen(pcie_node_name));

		if (pcie_node_offset >= 0) {
			fdt_del_node(fdt, pcie_node_offset);
		}
	} while (pcie_node_offset >= 0);

	for (i = 0; i < ARRAY_SIZE(cfg->pcie); ++i)
		if (cfg->pcie[i].enable)
			add_pcie_node(fdt, &cfg->pcie[i]);
}
