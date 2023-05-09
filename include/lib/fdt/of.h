#ifndef __OF_H__
#define __OF_H__

int of_property_read_string(void *blob, char *path,
			    char *propname,
			    char **out_string);
int of_property_read_u64(void *blob, char *path,
			 char *propname, uint64_t *out_value);
int of_property_read_u32(void *blob, char *path,
			 char *propname, uint32_t *out_value);
int of_test(const void *fdt);
int of_modify_prop(void *original_fdt, int len, char *node_name,
		   char *prop_name, char *value, uint32_t prop_size,
		   uint8_t prop_type);

#endif