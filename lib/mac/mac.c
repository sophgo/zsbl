#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <lib/mac.h>

uint64_t str2mac(const char *s)
{
	const char *p;
	uint64_t mac;

	if (!s)
		return 0;

	if (!strchr(s, ':'))
		return strtoul(s, NULL, 16);

	/* mac address with ':' seperated */

	/* skip leading spaces */
	for (p = s; *p; ++p)
		if (isxdigit(*p))
			break;

	/* check length */
	if (strlen(p) < 17)
		return 0;

	mac = 0;
	/* format should be xx:xx:xx:xx, space is not allowed */
	((uint8_t *)&mac)[5] = strtoul(p + 0, NULL, 16);
	((uint8_t *)&mac)[4] = strtoul(p + 3, NULL, 16);
	((uint8_t *)&mac)[3] = strtoul(p + 6, NULL, 16);
	((uint8_t *)&mac)[2] = strtoul(p + 9, NULL, 16);
	((uint8_t *)&mac)[1] = strtoul(p + 12, NULL, 16);
	((uint8_t *)&mac)[0] = strtoul(p + 15, NULL, 16);

	return mac;
}

char *mac2str(uint64_t mac, char *s)
{
	sprintf(s, "%02lx:%02lx:%02lx:%02lx:%02lx:%02lx",
			(mac >> 40) & 0xff,
			(mac >> 32) & 0xff,
			(mac >> 24) & 0xff,
			(mac >> 16) & 0xff,
			(mac >> 8) & 0xff,
			mac & 0xff);

	return s;
}

void *mac2byte(uint64_t mac, uint8_t *byte)
{
	byte[0] = (mac >> 40) & 0xff;
	byte[1] = (mac >> 32) & 0xff;
	byte[2] = (mac >> 24) & 0xff;
	byte[3] = (mac >> 16) & 0xff;
	byte[4] = (mac >> 8) & 0xff;
	byte[5] = mac & 0xff;

	return byte;
}
