#include <efi.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <wchar.h>

#include <framework/common.h>

static void show_guid(struct efi_guid *guid)
{
	int i;

	/* always show in byte order */
	for (i = 0; i < sizeof(guid->d0); ++i)
		pr_info("%02x", ((uint8_t *)&guid->d0)[i]);

	pr_info("-");
	for (i = 0; i < sizeof(guid->d1); ++i)
		pr_info("%02x", ((uint8_t *)&guid->d1)[i]);

	pr_info("-");
	for (i = 0; i < sizeof(guid->d2); ++i)
		pr_info("%02x", ((uint8_t *)&guid->d2)[i]);

	pr_info("-");
	for (i = 0; i < sizeof(guid->d3); ++i)
		pr_info("%02x", ((uint8_t *)&guid->d3)[i]);

	pr_info("\n");
}

static int compare_guid(const struct efi_guid *g0, const struct efi_guid *g1)
{
	return memcmp((void *)g0, (void *)g1, sizeof(struct efi_guid));
}

static void show_sig(uint32_t sig)
{
	int i;

	pr_info("0x%08x (", sig);

	for (i = 0; i < sizeof(sig); ++i)
		pr_info("%c", ((uint8_t *)&sig)[i]);

	pr_info(")\n");
}

uint16_t checksum(void *data, unsigned long size)
{
	int i;
	uint16_t sum = 0;

	for (i = 0; i < size / 2; ++i)
		sum += ((uint16_t *)data)[i];

	return sum;
}

static void show_fv(struct efi_firmware_volume_header *fv)
{
	pr_info("GUID: ");
	show_guid(&fv->guid);
	pr_info("Volume Length: 0x%08llx\n", fv->volume_len);
	pr_info("Signature: ");
	show_sig(fv->sig);
	pr_info("Attributes: 0x%08x\n", fv->attr);
	pr_info("Header Length: 0x%08x\n", fv->header_len);
	pr_info("Checksum: 0x%04x\n", fv->checksum);
	pr_info("External Header Offset: 0x%04x\n", fv->ext_hdr_off);
	pr_info("Version: %d\n", fv->version);
	pr_info("Calculated Checksum: %d\n", checksum(fv, fv->header_len));
}

int efi_vz_check_fv(struct variable_zone *vz)
{
	struct efi_firmware_volume_header *fv = vz->fv;
	struct efi_guid efi_system_nvdata_fv_guid = EFI_SYSTEM_NVDATA_FV_GUID;

	show_fv(fv);

	if (compare_guid(&fv->guid, &efi_system_nvdata_fv_guid)) {
		pr_debug("Not a non-volatile firmware volume\n");
		return -EINVAL;
	}

	if (fv->volume_len == 0 || fv->volume_len == 0xffffffffffffffffUL) {
		pr_debug("Invalid volume length\n");
		return -EINVAL;
	}

	if (fv->sig != EFI_FVH_SIGNATURE) {
		pr_debug("Invalid signature\n");
		return -EINVAL;
	}

	if (fv->header_len < sizeof(*fv)) {
		pr_debug("Invalid header length\n");
		return -EINVAL;
	}
	if (checksum(fv, fv->header_len)) {
		pr_debug("Invalid checksum\n");
		return -EINVAL;
	}

	return 0;
}

static void show_variable_store_header(struct efi_variable_store_header *vs)
{
	pr_info("GUID: ");
	show_guid(&vs->guid);
	pr_info("Size: 0x%08x\n", vs->size);
	pr_info("Format: 0x%02x\n", vs->format);
	pr_info("Status: 0x%02x\n", vs->status);
}

int efi_vz_check_variable_store(struct variable_zone *vz)
{
	struct efi_variable_store_header *vs = vz->vs;
	struct efi_firmware_volume_header *fv = vz->fv;
	struct efi_guid efi_variable_guid = EFI_VARIABLE_GUID;
	struct efi_guid efi_auth_variable_guid = EFI_AUTHENTICATED_VARIABLE_GUID;

	show_variable_store_header(vs);

	if (compare_guid(&vs->guid, &efi_variable_guid) == 0) {
		vz->auth = false;
	} else if (compare_guid(&vs->guid, &efi_auth_variable_guid) == 0) {
		vz->auth = true;
	} else {
		pr_debug("Not a variable store region\n");
		return -EINVAL;
	}

	if (vs->size > fv->volume_len - fv->header_len) {
		pr_debug("Invalid variable store size\n");
		return -EINVAL;
	}

	if (vs->format != VARIABLE_STORE_FORMATTED) {
		pr_debug("Invalid format status\n");
		return -EINVAL;
	}

	if (vs->status != VARIABLE_STORE_HEALTHY) {
		pr_debug("Invalid status\n");
		return -EINVAL;
	}

	return 0;
}

static void efi_vz_show(struct variable_zone *vz)
{
	pr_info("Firmware Volume Header: 0x%lx (0x%08lx)\n",
		(unsigned long)vz->fv, 0UL);
	pr_info("Variable Store Header: 0x%lx (0x%08lx)\n",
		(unsigned long)vz->vs, (unsigned long)vz->vs - (unsigned long)vz->fv);
	pr_info("Variable Store Start: 0x%lx (0x%08lx)\n",
		(unsigned long)vz->var, (unsigned long)vz->var - (unsigned long)vz->fv);
	pr_info("Variable Header Type: %s\n", vz->auth ? "Authenticated" : "Normal");
}

int efi_vz_init(struct variable_zone *vz, void *fv)
{
	vz->fv = fv;

	if (efi_vz_check_fv(vz)) {
		pr_err("Variable zone at 0x%lx is not a valid EFI firmware volume\n",
		       (unsigned long)vz);
		return -EINVAL;
	}

	pr_info("-----------------------------\n");

	vz->vs = fv + vz->fv->header_len;

	if (efi_vz_check_variable_store(vz)) {
		pr_err("Variable zone at 0x%lx is not a valid EFI variable store\n",
		       (unsigned long)vz);
		return -EINVAL;
	}

	pr_info("-----------------------------\n");
	vz->var = (void *)HEADER_ALIGN((void *)vz->vs + sizeof(struct efi_variable_store_header));
	efi_vz_show(vz);

	vz->end = (void *)vz->vs + vz->vs->size;

	return 0;
}

struct efi_variable_header *
efi_vz_find_variable(struct variable_zone *vz,
		     const struct efi_guid *vendor_guid, const wchar_t *name)
{
	struct efi_variable_header *var;
	wchar_t *var_name;

	for (var = vz->var;
	     var < vz->end && var->id == VARIABLE_DATA;
	     var = (void *)var + var->name_size + var->data_size) {
		var_name = (void *)var + sizeof(struct efi_variable_header);
		if (compare_guid(&var->vendor_guid, vendor_guid) == 0
		    && wcscmp(name, var_name) == 0)
			return var;
	}

	return NULL;
}

wchar_t *efi_var_get_name(struct efi_variable_header *var)
{
	return (void *)var + sizeof(struct efi_variable_header);
}

void *efi_var_get_data(struct efi_variable_header *var)
{
	wchar_t *var_name;
	void *var_data;

	var_name = (void *)var + sizeof(struct efi_variable_header);
	var_data = (void *)var_name + var->name_size;

	return var_data;
}

