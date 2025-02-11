// ------------------------------------------------------------------------
//
//                (C) COPYRIGHT 2011 - 2015 SYNOPSYS, INC.
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

#ifndef ELPPKA_H_
#define ELPPKA_H_

#include "elppdu.h"

struct pka_state {
   uint32_t *regbase;

   struct pka_config {
      unsigned alu_size, rsa_size, ecc_size;
      unsigned fw_ram_size, fw_rom_size;
      unsigned ram_offset, rom_offset;
   } cfg;
};

struct pka_fw {
   unsigned long ram_size, rom_size;
   const char *errmsg;

   struct pka_fw_tag {
      unsigned long origin, tag_length, timestamp, md5_coverage;
      unsigned char md5[16];
   } ram_tag, rom_tag;

   /* For internal use */
   struct elppka_fw_priv *priv;
};

enum {
   PKA_OPERAND_A,
   PKA_OPERAND_B,
   PKA_OPERAND_C,
   PKA_OPERAND_D,
   PKA_OPERAND_MAX
};

int elppka_setup(struct pka_state *pka, uint32_t *regbase);

int elppka_start(struct pka_state *pka, uint32_t entry, uint32_t flags,
                                                        unsigned size);
void elppka_abort(struct pka_state *pka);
int elppka_get_status(struct pka_state *pka, unsigned *code);

int elppka_load_operand(struct pka_state *pka, unsigned bank, unsigned index,
                                         unsigned size, const uint8_t *data);
int elppka_unload_operand(struct pka_state *pka, unsigned bank, unsigned index,
                                                 unsigned size, uint8_t *data);

void elppka_set_byteswap(struct pka_state *pka, int swap);

/* Firmware image handling */
int elppka_fw_parse(struct pka_fw *fw, const unsigned char *data,
                                       unsigned long len);
void elppka_fw_free(struct pka_fw *fw);

int elppka_fw_lookup_entry(struct pka_fw *fw, const char *entry);

int elppka_fw_load(struct pka_state *pka, struct pka_fw *fw);

/* The firmware timestamp epoch (2009-11-11 11:00:00Z) as a UNIX timestamp. */
#define PKA_FW_TS_EPOCH 1257937200ull

/* Resolution of the timestamp, in seconds. */
#define PKA_FW_TS_RESOLUTION 20

#endif
