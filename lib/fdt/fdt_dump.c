// SPDX-License-Identifier: GPL-2.0-or-later

#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include <libfdt.h>
#include <libfdt_env.h>
#include <fdt.h>

#define ALIGN(x, a)	(((x) + ((a) - 1)) & ~((a) - 1))
#define PALIGN(p, a)	((void *)(ALIGN((uintptr_t)(p), (a))))
#define GET_CELL(p)	(p += 4, *((const fdt32_t *)(p-4)))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))
#endif

static bool util_is_printable_string(const void *data, int len)
{
	const char *s = data;
	const char *ss, *se;

	/* zero length is not */
	if (len == 0)
		return 0;

	/* must terminate with zero */
	if (s[len - 1] != '\0')
		return 0;

	se = s + len;

	while (s < se) {
		ss = s;
		while (s < se && *s && isprint((unsigned char)*s))
			s++;

		/* not zero, or not done yet */
		if (*s != '\0' || s == ss)
			return 0;

		s++;
	}

	return 1;
}

static void utilfdt_print_data(const char *data, int len)
{
	int i;
	const char *s;

	/* no data, don't print */
	if (len == 0)
		return;

	if (util_is_printable_string(data, len)) {
		printf(" = ");

		s = data;
		do {
			printf("\"%s\"", s);
			s += strlen(s) + 1;
			if (s < data + len)
				printf(", ");
		} while (s < data + len);

	} else if ((len % 4) == 0) {
		const fdt32_t *cell = (const fdt32_t *)data;

		printf(" = <");
		for (i = 0, len /= 4; i < len; i++)
			printf("0x%08" PRIx32 "%s", fdt32_to_cpu(cell[i]),
			       i < (len - 1) ? " " : "");
		printf(">");
	} else {
		const unsigned char *p = (const unsigned char *)data;
		printf(" = [");
		for (i = 0; i < len; i++)
			printf("%02x%s", *p++, i < len - 1 ? " " : "");
		printf("]");
	}
}

static const char *tagname(uint32_t tag)
{
	static const char * const names[] = {
#define TN(t) [t] = #t
		TN(FDT_BEGIN_NODE),
		TN(FDT_END_NODE),
		TN(FDT_PROP),
		TN(FDT_NOP),
		TN(FDT_END),
#undef TN
	};
	if (tag < ARRAY_SIZE(names))
		if (names[tag])
			return names[tag];
	return "FDT_???";
}

#define dumpf(fmt, args...) \
	do { if (debug) printf("// " fmt, ## args); } while (0)

void fdt_dump_blob(void *blob, bool debug)
{
	uintptr_t blob_off = (uintptr_t)blob;
	struct fdt_header *bph = blob;
	uint32_t off_mem_rsvmap = fdt32_to_cpu(bph->off_mem_rsvmap);
	uint32_t off_dt = fdt32_to_cpu(bph->off_dt_struct);
	uint32_t off_str = fdt32_to_cpu(bph->off_dt_strings);
	struct fdt_reserve_entry *p_rsvmap =
		(struct fdt_reserve_entry *)((char *)blob + off_mem_rsvmap);
	const char *p_struct = (const char *)blob + off_dt;
	const char *p_strings = (const char *)blob + off_str;
	uint32_t version = fdt32_to_cpu(bph->version);
	uint32_t totalsize = fdt32_to_cpu(bph->totalsize);
	uint32_t tag;
	const char *p, *s, *t;
	int depth, sz, shift;
	int i;
	uint64_t addr, size;

	depth = 0;
	shift = 4;

	printf("/dts-v1/;\n");
	printf("// magic:\t\t0x%"PRIx32"\n", fdt32_to_cpu(bph->magic));
	printf("// totalsize:\t\t0x%"PRIx32" (%"PRIu32")\n",
	       totalsize, totalsize);
	printf("// off_dt_struct:\t0x%"PRIx32"\n", off_dt);
	printf("// off_dt_strings:\t0x%"PRIx32"\n", off_str);
	printf("// off_mem_rsvmap:\t0x%"PRIx32"\n", off_mem_rsvmap);
	printf("// version:\t\t%"PRIu32"\n", version);
	printf("// last_comp_version:\t%"PRIu32"\n",
	       fdt32_to_cpu(bph->last_comp_version));
	if (version >= 2)
		printf("// boot_cpuid_phys:\t0x%"PRIx32"\n",
		       fdt32_to_cpu(bph->boot_cpuid_phys));

	if (version >= 3)
		printf("// size_dt_strings:\t0x%"PRIx32"\n",
		       fdt32_to_cpu(bph->size_dt_strings));
	if (version >= 17)
		printf("// size_dt_struct:\t0x%"PRIx32"\n",
		       fdt32_to_cpu(bph->size_dt_struct));
	printf("\n");

	for (i = 0; ; i++) {
		addr = fdt64_to_cpu(p_rsvmap[i].address);
		size = fdt64_to_cpu(p_rsvmap[i].size);
		if (addr == 0 && size == 0)
			break;

		printf("/memreserve/ %#"PRIx64" %#"PRIx64";\n",
		       addr, size);
	}

	p = p_struct;
	while ((tag = fdt32_to_cpu(GET_CELL(p))) != FDT_END) {

		dumpf("%04"PRIxPTR": tag: 0x%08"PRIx32" (%s)\n",
		        (uintptr_t)p - blob_off - 4, tag, tagname(tag));

		if (tag == FDT_BEGIN_NODE) {
			s = p;
			p = PALIGN(p + strlen(s) + 1, 4);

			if (*s == '\0')
				s = "/";

			printf("%*s%s {\n", depth * shift, "", s);

			depth++;
			continue;
		}

		if (tag == FDT_END_NODE) {
			depth--;

			printf("%*s};\n", depth * shift, "");
			continue;
		}

		if (tag == FDT_NOP) {
			printf("%*s// [NOP]\n", depth * shift, "");
			continue;
		}

		if (tag != FDT_PROP) {
			printf("%*s ** Unknown tag 0x%08"PRIx32"\n", depth * shift, "", tag);
			break;
		}
		sz = fdt32_to_cpu(GET_CELL(p));
		s = p_strings + fdt32_to_cpu(GET_CELL(p));
		if (version < 16 && sz >= 8)
			p = PALIGN(p, 8);
		t = p;

		p = PALIGN(p + sz, 4);

		dumpf("%04"PRIxPTR": string: %s\n", (uintptr_t)s - blob_off, s);
		dumpf("%04"PRIxPTR": value\n", (uintptr_t)t - blob_off);
		printf("%*s%s", depth * shift, "", s);
		utilfdt_print_data(t, sz);
		printf(";\n");
	}
}
