#ifndef __MAC_H__
#define __MAC_H__

#include <stdint.h>

uint64_t str2mac(const char *s);
char *mac2str(uint64_t mac, char *s);
void *mac2byte(uint64_t mac, uint8_t *byte);

#endif
