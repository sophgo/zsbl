#include <framework/common.h>

void dump_hex(void *addr, int len)
{
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char *)addr;

	/* Process every byte in the data. */
	for (i = 0; i < len; i++) {
		/* Multiple of 16 means new line (with line offset). */
		if ((i % 16) == 0) {
			/* Just don't print ASCII for the zeroth line. */
			if (i != 0)
				pr_info("  %s\n", buff);

			/* Output the offset. */
			pr_info("%04x ", i);
		}

		/* Now the hex code for the specific character. */
		pr_info(" %02x", pc[i]);

		/* And store a printable ASCII character for later. */
		if ((pc[i] < 0x20) || (pc[i] > 0x7e))
			buff[i % 16] = '.';
		else
			buff[i % 16] = pc[i];
		buff[(i % 16) + 1] = '\0';
	}

	/* Pad out last line if not exactly 16 characters. */
	while ((i % 16) != 0) {
		pr_info("   ");
		i++;
	}

	/* And print the final ASCII bit. */
	pr_info("  %s\n", buff);
}
