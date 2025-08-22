#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <wchar.h>

#include <common/common.h>

#include <lib/efi.h>

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

void efi_show_fv_header(struct efi_firmware_volume_header *fv)
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

int efi_vz_check_fv(struct efi_variable_zone *vz)
{
	struct efi_firmware_volume_header *fv = vz->fv;
	struct efi_guid efi_system_nvdata_fv_guid = EFI_SYSTEM_NVDATA_FV_GUID;

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

void efi_show_variable_store_header(struct efi_variable_store_header *vs)
{
	pr_info("GUID: ");
	show_guid(&vs->guid);
	pr_info("Size: 0x%08x\n", vs->size);
	pr_info("Format: 0x%02x\n", vs->format);
	pr_info("Status: 0x%02x\n", vs->status);
}

int efi_vz_check_variable_store(struct efi_variable_zone *vz)
{
	struct efi_variable_store_header *vs = vz->vs;
	struct efi_firmware_volume_header *fv = vz->fv;
	struct efi_guid efi_variable_guid = EFI_VARIABLE_GUID;
	struct efi_guid efi_auth_variable_guid = EFI_AUTHENTICATED_VARIABLE_GUID;

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

void efi_show_variable_zone(struct efi_variable_zone *vz)
{
	pr_info("Firmware Volume Header: 0x%lx (0x%08lx)\n",
		(unsigned long)vz->fv, 0UL);
	pr_info("Variable Store Header: 0x%lx (0x%08lx)\n",
		(unsigned long)vz->vs, (unsigned long)vz->vs - (unsigned long)vz->fv);
	pr_info("Variable Store Start: 0x%lx (0x%08lx)\n",
		(unsigned long)vz->var, (unsigned long)vz->var - (unsigned long)vz->fv);
	pr_info("Variable Header Type: %s\n", vz->auth ? "Authenticated" : "Normal");
}

int efi_vz_init(struct efi_variable_zone *vz, void *fv)
{
	vz->fv = fv;

	if (efi_vz_check_fv(vz)) {
		pr_debug("Variable zone at 0x%lx is not a valid EFI firmware volume\n",
		       (unsigned long)vz);
		return -EINVAL;
	}

	vz->vs = fv + vz->fv->header_len;

	if (efi_vz_check_variable_store(vz)) {
		pr_err("Variable zone at 0x%lx is not a valid EFI variable store\n",
		       (unsigned long)vz);
		return -EINVAL;
	}

	vz->var = (void *)HEADER_ALIGN((void *)vz->vs + sizeof(struct efi_variable_store_header));
	vz->end = (void *)vz->vs + vz->vs->size;

	return 0;
}

static void wputs(wchar_t *ws, unsigned long n)
{
	int i;

	for (i = 0; ws[i] && i < n / sizeof(wchar_t); ++i) {
		if (ws[i] > 255)
			pr_info("[%04x]", ws[i]);
		else
			pr_info("%c", ws[i]);
	}
}

/* id */
uint16_t efi_variable_id(struct efi_variable *var)
{
	if (var->auth)
		return ((struct efi_authenticated_variable_header *)(var->header))->id;
	else
		return ((struct efi_variable_header *)(var->header))->id;
}

/* status */
uint8_t efi_variable_status(struct efi_variable *var)
{
	if (var->auth)
		return ((struct efi_authenticated_variable_header *)(var->header))->status;
	else
		return ((struct efi_variable_header *)(var->header))->status;
}

/* attr */
uint32_t efi_variable_attr(struct efi_variable *var)
{
	if (var->auth)
		return ((struct efi_authenticated_variable_header *)(var->header))->attr;
	else
		return ((struct efi_variable_header *)(var->header))->attr;
}

/* name_size */
uint32_t efi_variable_name_size(struct efi_variable *var)
{
	if (var->auth)
		return ((struct efi_authenticated_variable_header *)(var->header))->name_size;
	else
		return ((struct efi_variable_header *)(var->header))->name_size;
}

/* data_size */
uint32_t efi_variable_data_size(struct efi_variable *var)
{
	if (var->auth)
		return ((struct efi_authenticated_variable_header *)(var->header))->data_size;
	else
		return ((struct efi_variable_header *)(var->header))->data_size;
}

/* vendor_guid */
struct efi_guid *efi_variable_vendor_guid(struct efi_variable *var)
{
	if (var->auth)
		return &((struct efi_authenticated_variable_header *)(var->header))->vendor_guid;
	else
		return &((struct efi_variable_header *)(var->header))->vendor_guid;
}

/* name */
wchar_t *efi_variable_name(struct efi_variable *var)
{
	if (var->auth)
		return (void *)(var->header) + sizeof(struct efi_authenticated_variable_header);
	else
		return (void *)(var->header) + sizeof(struct efi_variable_header);
}

/* data */
void *efi_variable_data(struct efi_variable *var)
{
	return (void *)efi_variable_name(var) + efi_variable_name_size(var);
}

/* total size of a variable, including variable header */
uint32_t efi_variable_size(struct efi_variable *var)
{
	uint32_t size;

	if (var->auth)
		size = sizeof(struct efi_authenticated_variable_header);
	else
		size = sizeof(struct efi_variable_header);

	size += efi_variable_name_size(var);
	size += efi_variable_data_size(var);

	return size;
}

/*
 * return true if variable is valid
 */
int efi_variable_valid(struct efi_variable *var)
{
	return efi_variable_id(var) == VARIABLE_DATA;
}

int efi_variable_first(struct efi_variable_zone *vz, struct efi_variable *var)
{
	var->auth = vz->auth;
	var->header = vz->var;

	if (!efi_variable_valid(var))
		return -ENOENT;

	return 0;
}

int efi_variable_next(struct efi_variable_zone *vz, struct efi_variable *var)
{
	var->header = (void *)HEADER_ALIGN(var->header + efi_variable_size(var));

	if (!efi_variable_valid(var))
		return -ENOENT;

	if ((unsigned long)var->header >= (unsigned long)vz->end)
		return -ENOENT;

	return 0;
}

void efi_show_variable(struct efi_variable_zone *vz, struct efi_variable *var)
{
	pr_info("Offset Inner FV 0x%08lx, Name Length 0x%08x, Data Length 0x%08x\n",
		(unsigned long)var->header - (unsigned long)vz->fv,
		efi_variable_name_size(var), efi_variable_data_size(var));
	pr_info("Variable Vendor GUID: ");
	show_guid(efi_variable_vendor_guid(var));
	pr_info("Variable Name: ");
	wputs(efi_variable_name(var), efi_variable_name_size(var));
	pr_info("\n");
	pr_info("Variable Attributies: %08x\n", efi_variable_attr(var));
	pr_info("Variable Status: %02x\n", efi_variable_status(var));

}

void efi_list_variables(struct efi_variable_zone *vz)
{

	int i;
	int err;
	struct efi_variable var;

	for (err = efi_variable_first(vz, &var), i = 0; err == 0;
	     err = efi_variable_next(vz, &var), ++i) {

		if (!(efi_variable_status(&var) == VAR_ADDED ||
		      efi_variable_status(&var) == (VAR_IN_DELETED_TRANSITION & VAR_ADDED))) {
			continue;
		}

		pr_info("Show %dth Variable\n", i);
		efi_show_variable(vz, &var);
	}
}

int efi_vz_find_variable(struct efi_variable_zone *vz, struct efi_variable *var,
			 const struct efi_guid *vendor_guid, const wchar_t *name)
{
	int err;

	for (err = efi_variable_first(vz, var); err == 0; err = efi_variable_next(vz, var)) {

		if (!(efi_variable_status(var) == VAR_ADDED ||
		      efi_variable_status(var) == (VAR_IN_DELETED_TRANSITION & VAR_ADDED))) {
			continue;
		}

		if (compare_guid(efi_variable_vendor_guid(var), vendor_guid) == 0 &&
		    wcscmp(name, efi_variable_name(var)) == 0)
			return 0;
	}

	return -ENOENT;
}

