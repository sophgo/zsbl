// ------------------------------------------------------------------------
//
//                (C) COPYRIGHT 2012 - 2015 SYNOPSYS, INC.
//                          ALL RIGHTS RESERVED
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  version 2 as published by the Free Software Foundation.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <https://gnu.org/licenses/>.
//
// ------------------------------------------------------------------------

#include <driver/pka/elppdu.h>
#include <common/common.h>


/* Platform Specific I/O */
static int debug_on=0;

/* write a 32-bit word to a given address */
void pdu_io_write32 (void *addr, unsigned long val)
{
	if (addr == NULL) {
		debug_on ^= 1;
		return;
	}
	if (debug_on)
		ELPHW_PRINT("PDU: write %.8lx -> %p\n", val, addr);
	*((uint32_t *)addr) = val;
}

/* read a 32-bit word from a given address */
unsigned long pdu_io_read32 (void *addr)
{
	unsigned long foo;
	foo = *((uint32_t *)addr);
	if (debug_on)
		ELPHW_PRINT("PDU: read  %.8lx <- %p\n", foo, addr);
	return foo;
}

/* Platform specific DDT routines */

#define PDU_HEAP_SIZE	(128*1024)
#define PDU_HEAP_ALIGN	(16)
#define PDU_ROUNDUP(x,a)	((x+a-1)/a*a)
static unsigned char __attribute__((aligned(16))) pdu_heap_base[PDU_HEAP_SIZE];
static unsigned long pdu_heap_ptr;

// initialize memory for DDT routines
int pdu_mem_init(void *device)
{
   	return 0; // does nothing, here is where you could initialize your heap/etc
}

// cleanup memory used by DDT routines
void pdu_mem_deinit(void *device)
{
}


/* Platform specific memory allocation */
void *pdu_malloc (unsigned long n)
{
	unsigned long na=PDU_ROUNDUP(n,PDU_HEAP_ALIGN);
	unsigned long base = pdu_heap_ptr;

	pdu_heap_ptr += na;

	if (pdu_heap_ptr >= PDU_HEAP_SIZE)
		return NULL;

	return (void *)(pdu_heap_base+base);
}

void pdu_free (void *p)
{
}

/**** PORTABLE PDU CODE ****/
void pdu_io_cached_write32 (void *addr, unsigned long val, uint32_t *cache)
{
	if (*cache == val)
		return;
	*cache = val;
	pdu_io_write32 (addr, val);
}

void pdu_to_dev32(void *addr, uint32_t *src, unsigned long nword)
{
	while (nword--) {
		pdu_io_write32(addr, *src++);
		addr += 4;
	}
}

void pdu_from_dev32(uint32_t *dst, void *addr, unsigned long nword)
{
	while (nword--) {
		*dst++ = pdu_io_read32(addr);
		addr += 4;
	}
}

void pdu_to_dev32_big(void *addr, const unsigned char *src, unsigned long nword)
{
   	unsigned long v;

	while (nword--) {
		v = 0;
		v = (v << 8) | ((unsigned long)*src++);
		v = (v << 8) | ((unsigned long)*src++);
		v = (v << 8) | ((unsigned long)*src++);
		v = (v << 8) | ((unsigned long)*src++);
		pdu_io_write32(addr, v);
		addr += 4;
	}
}

void pdu_from_dev32_big(unsigned char *dst, void *addr, unsigned long nword)
{
	unsigned long v;

	while (nword--) {
		v = pdu_io_read32(addr);
		addr += 4;
		*dst++ = (v >> 24) & 0xFF; v <<= 8;
		*dst++ = (v >> 24) & 0xFF; v <<= 8;
		*dst++ = (v >> 24) & 0xFF; v <<= 8;
		*dst++ = (v >> 24) & 0xFF; v <<= 8;
	}
}

void pdu_to_dev32_little(void *addr, const unsigned char *src, unsigned long nword)
{
   	unsigned long v;

	while (nword--) {
		v = 0;
		v = (v >> 8) | ((unsigned long)*src++ << 24UL);
		v = (v >> 8) | ((unsigned long)*src++ << 24UL);
		v = (v >> 8) | ((unsigned long)*src++ << 24UL);
		v = (v >> 8) | ((unsigned long)*src++ << 24UL);
		pdu_io_write32(addr, v);
		addr += 4;
	}
}

void pdu_from_dev32_little(unsigned char *dst, void *addr, unsigned long nword)
{
	unsigned long v;

	while (nword--) {
		v = pdu_io_read32(addr);
		addr += 4;
		*dst++ = v & 0xFF; v >>= 8;
		*dst++ = v & 0xFF; v >>= 8;
		*dst++ = v & 0xFF; v >>= 8;
		*dst++ = v & 0xFF; v >>= 8;
	}
}

void pdu_to_dev32_s(void *addr, const unsigned char *src, unsigned long nword, int endian)
{
	if (endian)
		pdu_to_dev32_big(addr, src, nword);
	else
		pdu_to_dev32_little(addr, src, nword);
}

void pdu_from_dev32_s(unsigned char *dst, void *addr, unsigned long nword, int endian)
{
	if (endian)
		pdu_from_dev32_big(dst, addr, nword);
	else
		pdu_from_dev32_little(dst, addr, nword);

}

/* Convert SDK error codes to corresponding kernel error codes. */
int pdu_error_code(int code)
{
   	return code;
}

