#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <lib/mmio.h>
#include <driver/ddr/bitwise_ops.h>
#include <driver/ddr/ddr.h>

void ddrc_init_lp5(uint64_t base_addr_ctrl, lpddr_attr *p_lpddr_attr)
{
	uint32_t rddata;

	//printf("----[%s]----\n", __func__);

	//rddata = mmio_rd32(base_addr_ctrl + 0x10ff8, rddata);// DWC_ddrctl_map_REGB_DDRC_CH0.DDRCTL_VER_NUMBER
	//rddata = mmio_rd32(base_addr_ctrl + 0x10ffc, rddata);// DWC_ddrctl_map_REGB_DDRC_CH0.DDRCTL_VER_TYPE
	mmio_wr32(base_addr_ctrl + 0x10b84, 0x1);// DWC_ddrctl_map_REGB_DDRC_CH0.OPCTRL1

	//if ($test$plusargs("ENABLE_RW_LINK_ECC"))begin
	//mmio_wr32(base_addr_ctrl + 0xd80, 0x3);// DWC_ddrctl_map_REGB_FREQ0_CH0.LNKECCCTL0
	//end else if ($test$plusargs("ENABLE_RD_LINK_ECC"))begin
	//mmio_wr32(base_addr_ctrl + 0xd80, 0x2);// DWC_ddrctl_map_REGB_FREQ0_CH0.LNKECCCTL0
	//end else if ($test$plusargs("ENABLE_WR_LINK_ECC"))begin
	//mmio_wr32(base_addr_ctrl + 0xd80, 0x1);// DWC_ddrctl_map_REGB_FREQ0_CH0.LNKECCCTL0
	//end

	if (p_lpddr_attr->lp5_linkecc_en == true)
		mmio_wr32(base_addr_ctrl + 0xd80, 0x3);// DWC_ddrctl_map_REGB_FREQ0_CH0.LNKECCCTL0

	#ifdef HALF_DATA_WIDTH
		if (p_lpddr_attr->num_rank == 2)
			mmio_wr32(base_addr_ctrl + 0x10000, 0x3081008);// DWC_ddrctl_map_REGB_DDRC_CH0.MSTR0
		else
			mmio_wr32(base_addr_ctrl + 0x10000, 0x1081008);// DWC_ddrctl_map_REGB_DDRC_CH0.MSTR0
	#else
		if (p_lpddr_attr->num_rank == 2)
			mmio_wr32(base_addr_ctrl + 0x10000, 0x3080008);// DWC_ddrctl_map_REGB_DDRC_CH0.MSTR0
		else
			mmio_wr32(base_addr_ctrl + 0x10000, 0x1080008);// DWC_ddrctl_map_REGB_DDRC_CH0.MSTR0
	#endif

	mmio_wr32(base_addr_ctrl + 0x10010, 0x111);// DWC_ddrctl_map_REGB_DDRC_CH0.MSTR4
	mmio_wr32(base_addr_ctrl + 0x10100, 0x0);// DWC_ddrctl_map_REGB_DDRC_CH0.DERATECTL0
	mmio_wr32(base_addr_ctrl + 0x10104, 0x1);// DWC_ddrctl_map_REGB_DDRC_CH0.DERATECTL1
	mmio_wr32(base_addr_ctrl + 0x10108, 0x1);// DWC_ddrctl_map_REGB_DDRC_CH0.DERATECTL2
	mmio_wr32(base_addr_ctrl + 0x10118, 0x1);// DWC_ddrctl_map_REGB_DDRC_CH0.DERATECTL6
	mmio_wr32(base_addr_ctrl + 0x10184, 0x2);// DWC_ddrctl_map_REGB_DDRC_CH0.HWLPCTL
	mmio_wr32(base_addr_ctrl + 0x1018c, 0x0);// DWC_ddrctl_map_REGB_DDRC_CH0.CLKGATECTL

	//rfshctl0.dis_auto_refresh = 0x1 before dfi_init_start,
	//rfshctl0.dis_auto_refresh = 0x0 after config mode register
	mmio_wr32(base_addr_ctrl + 0x10208, 0x1);// DWC_ddrctl_map_REGB_DDRC_CH0.RFSHCTL0

	//if (!$test$plusargs("MASK_TRAFFIC"))begin
	rddata = mmio_rd32(base_addr_ctrl + 0x10200);//
	//rddata = {rddata[31:9], 1'b1, rddata[7:6], 6'b101111};// rfshmod0.per_bank_refresh = 0x1;
	rddata = modified_bits_by_value(rddata, 1, 8, 8);
	rddata = modified_bits_by_value(rddata, 0b101111, 5, 0);
	mmio_wr32(base_addr_ctrl + 0x10200, rddata);//
	//end

	//?
	//mmio_wr32(base_addr_ctrl + 0x10220, 0x1f000100);// DWC_ddrctl_map_REGB_DDRC_CH0.RFMMOD0
	//mmio_wr32(base_addr_ctrl + 0x10224, 0xd);// DWC_ddrctl_map_REGB_DDRC_CH0.RFMMOD1
	mmio_wr32(base_addr_ctrl + 0x10280, 0x80000000);// DWC_ddrctl_map_REGB_DDRC_CH0.ZQCTL0
	mmio_wr32(base_addr_ctrl + 0x10288, 0x3);// DWC_ddrctl_map_REGB_DDRC_CH0.ZQCTL2
	mmio_wr32(base_addr_ctrl + 0x10300, 0xc000c0);// DWC_ddrctl_map_REGB_DDRC_CH0.DQSOSCRUNTIME
	mmio_wr32(base_addr_ctrl + 0x10308, 0x1);// DWC_ddrctl_map_REGB_DDRC_CH0.DQSOSCCFG0

	//?
	if (p_lpddr_attr->otsd_latency == true)
		mmio_wr32(base_addr_ctrl + 0x10380, 0xa8013f10);// DWC_ddrctl_map_REGB_DDRC_CH0.SCHED0
	else
		mmio_wr32(base_addr_ctrl + 0x10380, 0xa8013f14);// DWC_ddrctl_map_REGB_DDRC_CH0.SCHED0

	rddata = mmio_rd32(base_addr_ctrl + 0x10384);// DWC_ddrctl_map_REGB_DDRC_CH0.SCHED1
	//rddata = {rddata[31:23], 3'b001, rddata[19:0]};
	rddata = modified_bits_by_value(rddata, 0b001, 22, 20);
	mmio_wr32(base_addr_ctrl + 0x10384, rddata);
	rddata = mmio_rd32(base_addr_ctrl + 0x1038c);// DWC_ddrctl_map_REGB_DDRC_CH0.SCHED3
	//rddata = {0x0000, rddata[15:0]};
	rddata = modified_bits_by_value(rddata, 0x0000, 31, 16);
	mmio_wr32(base_addr_ctrl + 0x1038c, rddata);

	mmio_wr32(base_addr_ctrl + 0x10394, 0x10000104);// DWC_ddrctl_map_REGB_DDRC_CH0.SCHED5
	mmio_wr32(base_addr_ctrl + 0x10400, 0x2000010);// DWC_ddrctl_map_REGB_DDRC_CH0.HWFFCCTL
	mmio_wr32(base_addr_ctrl + 0x10500, 0x110111);// DWC_ddrctl_map_REGB_DDRC_CH0.DFILPCFG0
	mmio_wr32(base_addr_ctrl + 0x10508, 0x80000000);// DWC_ddrctl_map_REGB_DDRC_CH0.DFIUPD0
	mmio_wr32(base_addr_ctrl + 0x10510, 0x10005);// DWC_ddrctl_map_REGB_DDRC_CH0.DFIMISC
	mmio_wr32(base_addr_ctrl + 0x10518, 0xf4000000);// DWC_ddrctl_map_REGB_DDRC_CH0.DFIPHYMSTR
	mmio_wr32(base_addr_ctrl + 0x10600, 0x43f7f00);// DWC_ddrctl_map_REGB_DDRC_CH0.ECCCFG0
	mmio_wr32(base_addr_ctrl + 0x10648, 0x1000210);// DWC_ddrctl_map_REGB_DDRC_CH0.ECCPOISONADDR0
	mmio_wr32(base_addr_ctrl + 0x1064c, 0x403d488);// DWC_ddrctl_map_REGB_DDRC_CH0.ECCPOISONADDR1
	mmio_wr32(base_addr_ctrl + 0x10658, 0x28dde5eb);// DWC_ddrctl_map_REGB_DDRC_CH0.ECCPOISONPAT0
	mmio_wr32(base_addr_ctrl + 0x10660, 0xe6);// DWC_ddrctl_map_REGB_DDRC_CH0.ECCPOISONPAT2

	//?
	if (p_lpddr_attr->phy_training == DWC_PHY_TRAINING) {
		rddata = mmio_rd32(base_addr_ctrl + 0x10180);
		//rddata = {rddata[31:12], 1'b1, rddata[10:0]};
		rddata = modified_bits_by_value(rddata, 1, 11, 11);
		mmio_wr32(base_addr_ctrl + 0x10180, rddata);// DWC_ddrctl_map_REGB_DDRC_CH0.PWRCTL.selfref_sw=1
		mmio_wr32(base_addr_ctrl + 0x10d00, 0xc0020002);// DWC_ddrctl_map_REGB_DDRC_CH0.INITTMG0
	} else {
		mmio_wr32(base_addr_ctrl + 0x10d00, 0x40020002);// DWC_ddrctl_map_REGB_DDRC_CH0.INITTMG0
	}
	mmio_wr32(base_addr_ctrl + 0x10f00, 0x800060c0);// DWC_ddrctl_map_REGB_DDRC_CH0.PPT2CTRL0
	if (p_lpddr_attr->lp5_linkecc_en == true) {
		mmio_wr32(base_addr_ctrl + 0x0, 0x2c101b22);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG0
		mmio_wr32(base_addr_ctrl + 0x4, 0x7060630);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG1
		mmio_wr32(base_addr_ctrl + 0x8, 0x913121a);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG2
		mmio_wr32(base_addr_ctrl + 0xc, 0xc2332);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG3
		mmio_wr32(base_addr_ctrl + 0x24, 0x20416);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG9
		mmio_wr32(base_addr_ctrl + 0x60, 0x10180e);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG24
		mmio_wr32(base_addr_ctrl + 0x64, 0x2b06);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG25
		mmio_wr32(base_addr_ctrl + 0x78, 0x1b141a);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG30
		mmio_wr32(base_addr_ctrl + 0x50c, 0x5000);// DWC_ddrctl_map_REGB_FREQ0_CH0.INITMR3
		mmio_wr32(base_addr_ctrl + 0x10050c, 0x5000);// DWC_ddrctl_map_REGB_FREQ1_CH0.INITMR3
		mmio_wr32(base_addr_ctrl + 0x20050c, 0x5000);// DWC_ddrctl_map_REGB_FREQ1_CH0.INITMR3
		mmio_wr32(base_addr_ctrl + 0x30050c, 0x5000);// DWC_ddrctl_map_REGB_FREQ1_CH0.INITMR3
		mmio_wr32(base_addr_ctrl + 0x580, 0x347021f);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG0
		mmio_wr32(base_addr_ctrl + 0x100580, 0x347021f);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFITMG0
		mmio_wr32(base_addr_ctrl + 0x200580, 0x347021f);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFITMG0
		mmio_wr32(base_addr_ctrl + 0x300580, 0x347021f);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG0
		mmio_wr32(base_addr_ctrl + 0x588, 0x183f1f);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG2
		mmio_wr32(base_addr_ctrl + 0x100588, 0x183f1f);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFITMG2
		mmio_wr32(base_addr_ctrl + 0x200588, 0x183f1f);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFITMG2
		mmio_wr32(base_addr_ctrl + 0x300588, 0x183f1f);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG2
		mmio_wr32(base_addr_ctrl + 0x590, 0x200c0403);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG4
		mmio_wr32(base_addr_ctrl + 0x100590, 0x200c0403);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFITMG4
		mmio_wr32(base_addr_ctrl + 0x200590, 0x200c0403);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFITMG4
		mmio_wr32(base_addr_ctrl + 0x300590, 0x200c0403);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG4
		mmio_wr32(base_addr_ctrl + 0x5b8, 0x147);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFIUPDTMG3
		mmio_wr32(base_addr_ctrl + 0x1005b8, 0x147);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFIUPDTMG3
		mmio_wr32(base_addr_ctrl + 0x2005b8, 0x147);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFIUPDTMG3
		mmio_wr32(base_addr_ctrl + 0x3005b8, 0x147);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFIUPDTMG3
		mmio_wr32(base_addr_ctrl + 0x600, 0xc21e0c34);// DWC_ddrctl_map_REGB_FREQ0_CH0.RFSHSET1TMG0
		mmio_wr32(base_addr_ctrl + 0x100600, 0xc21e0c34);// DWC_ddrctl_map_REGB_FREQ1_CH0.RFSHSET1TMG0
		mmio_wr32(base_addr_ctrl + 0x200600, 0xc21e0c34);// DWC_ddrctl_map_REGB_FREQ2_CH0.RFSHSET1TMG0
		mmio_wr32(base_addr_ctrl + 0x300600, 0xc21e0c34);// DWC_ddrctl_map_REGB_FREQ3_CH0.RFSHSET1TMG0
		mmio_wr32(base_addr_ctrl + 0xd08, 0x1703);// DWC_ddrctl_map_REGB_FREQ0_CH0.RANKTMG1
		mmio_wr32(base_addr_ctrl + 0x100d08, 0x1703);// DWC_ddrctl_map_REGB_FREQ1_CH0.RANKTMG1
		mmio_wr32(base_addr_ctrl + 0x200d08, 0x1703);// DWC_ddrctl_map_REGB_FREQ2_CH0.RANKTMG1
		mmio_wr32(base_addr_ctrl + 0x300d08, 0x1703);// DWC_ddrctl_map_REGB_FREQ3_CH0.RANKTMG1
	} else if (p_lpddr_attr->pll_bypass == true) {
		mmio_wr32(base_addr_ctrl + 0x0, 0x1306090c);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG0
		mmio_wr32(base_addr_ctrl + 0x4, 0x60410);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG1
		mmio_wr32(base_addr_ctrl + 0x8, 0x4080d0c);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG2
		mmio_wr32(base_addr_ctrl + 0xc, 0xd1116);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG3
		mmio_wr32(base_addr_ctrl + 0x24, 0x4020c);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG9
		mmio_wr32(base_addr_ctrl + 0x60, 0x20d0f0b);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG24
		mmio_wr32(base_addr_ctrl + 0x64, 0x1304);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG25
		mmio_wr32(base_addr_ctrl + 0x78, 0x110f11);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG30
		mmio_wr32(base_addr_ctrl + 0x50c, 0x0);// DWC_ddrctl_map_REGB_FREQ0_CH0.INITMR3
		mmio_wr32(base_addr_ctrl + 0x580, 0x50f0207);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG0
		mmio_wr32(base_addr_ctrl + 0x588, 0x140f07);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG2
		mmio_wr32(base_addr_ctrl + 0x590, 0x7);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG4
		mmio_wr32(base_addr_ctrl + 0x5b8, 0xca);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFIUPDTMG3
		mmio_wr32(base_addr_ctrl + 0x600, 0xc2100411);// DWC_ddrctl_map_REGB_FREQ0_CH0.RFSHSET1TMG0
		mmio_wr32(base_addr_ctrl + 0xd08, 0x1006);// DWC_ddrctl_map_REGB_FREQ0_CH0.RANKTMG1

	} else {
		mmio_wr32(base_addr_ctrl + 0x0, 0x280c1b22);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG0
		mmio_wr32(base_addr_ctrl + 0x4, 0x60630);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG1
		mmio_wr32(base_addr_ctrl + 0x8, 0x9111117);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG2
		mmio_wr32(base_addr_ctrl + 0xc, 0xc212f);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG3
		mmio_wr32(base_addr_ctrl + 0x24, 0x20412);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG9
		mmio_wr32(base_addr_ctrl + 0x60, 0xf160e);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG24
		mmio_wr32(base_addr_ctrl + 0x64, 0x2806);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG25
		mmio_wr32(base_addr_ctrl + 0x78, 0x191318);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG30
		mmio_wr32(base_addr_ctrl + 0x50c, 0x0);// DWC_ddrctl_map_REGB_FREQ0_CH0.INITMR3
		mmio_wr32(base_addr_ctrl + 0x10050c, 0x0);// DWC_ddrctl_map_REGB_FREQ1_CH0.INITMR3
		mmio_wr32(base_addr_ctrl + 0x10050c, 0x0);// DWC_ddrctl_map_REGB_FREQ1_CH0.INITMR3
		mmio_wr32(base_addr_ctrl + 0x20050c, 0x0);// DWC_ddrctl_map_REGB_FREQ1_CH0.INITMR3
		mmio_wr32(base_addr_ctrl + 0x30050c, 0x0);// DWC_ddrctl_map_REGB_FREQ1_CH0.INITMR3
		mmio_wr32(base_addr_ctrl + 0x580, 0x33f021f);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG0
		mmio_wr32(base_addr_ctrl + 0x100580, 0x337021f);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFITMG0
		mmio_wr32(base_addr_ctrl + 0x200580, 0x337021f);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFITMG0
		mmio_wr32(base_addr_ctrl + 0x300580, 0x337021f);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG0
		mmio_wr32(base_addr_ctrl + 0x588, 0x18371f);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG2
		mmio_wr32(base_addr_ctrl + 0x100588, 0x18371f);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFITMG2
		mmio_wr32(base_addr_ctrl + 0x200588, 0x18371f);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFITMG2
		mmio_wr32(base_addr_ctrl + 0x300588, 0x18371f);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG2
		mmio_wr32(base_addr_ctrl + 0x590, 0x180c0411);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG4
		mmio_wr32(base_addr_ctrl + 0x100590, 0x180c0411);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFITMG4
		mmio_wr32(base_addr_ctrl + 0x200590, 0x180c0411);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFITMG4
		mmio_wr32(base_addr_ctrl + 0x300590, 0x180c0411);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG4
		mmio_wr32(base_addr_ctrl + 0x5b8, 0x146);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFIUPDTMG3
		mmio_wr32(base_addr_ctrl + 0x1005b8, 0x146);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFIUPDTMG3
		mmio_wr32(base_addr_ctrl + 0x2005b8, 0x146);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFIUPDTMG3
		mmio_wr32(base_addr_ctrl + 0x3005b8, 0x146);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFIUPDTMG3
		mmio_wr32(base_addr_ctrl + 0x600, 0x82300186);// DWC_ddrctl_map_REGB_FREQ0_CH0.RFSHSET1TMG0
		mmio_wr32(base_addr_ctrl + 0x100600, 0xc2180c32);// DWC_ddrctl_map_REGB_FREQ1_CH0.RFSHSET1TMG0
		mmio_wr32(base_addr_ctrl + 0x200600, 0xc2180c32);// DWC_ddrctl_map_REGB_FREQ2_CH0.RFSHSET1TMG0
		mmio_wr32(base_addr_ctrl + 0x300600, 0xc2180c32);// DWC_ddrctl_map_REGB_FREQ3_CH0.RFSHSET1TMG0
		mmio_wr32(base_addr_ctrl + 0xd08, 0x1302);// DWC_ddrctl_map_REGB_FREQ0_CH0.RANKTMG1
		mmio_wr32(base_addr_ctrl + 0x100d08, 0x1302);// DWC_ddrctl_map_REGB_FREQ1_CH0.RANKTMG1
		mmio_wr32(base_addr_ctrl + 0x200d08, 0x1302);// DWC_ddrctl_map_REGB_FREQ2_CH0.RANKTMG1
		mmio_wr32(base_addr_ctrl + 0x300d08, 0x1302);// DWC_ddrctl_map_REGB_FREQ3_CH0.RANKTMG1
	}
	if (p_lpddr_attr->pll_bypass == true) {
		mmio_wr32(base_addr_ctrl + 0x10, 0x5040205);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG4
		mmio_wr32(base_addr_ctrl + 0x14, 0x2030404);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG5
		mmio_wr32(base_addr_ctrl + 0x18, 0x8);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG6
		mmio_wr32(base_addr_ctrl + 0x1c, 0x1);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG7
		mmio_wr32(base_addr_ctrl + 0x30, 0x30000);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG12
		mmio_wr32(base_addr_ctrl + 0x34, 0x9100002);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG13
		mmio_wr32(base_addr_ctrl + 0x38, 0xb0068);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG14
		mmio_wr32(base_addr_ctrl + 0x44, 0x28001b);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG17
		mmio_wr32(base_addr_ctrl + 0x5c, 0x350004);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG23
		mmio_wr32(base_addr_ctrl + 0x80, 0x30404);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG32
		mmio_wr32(base_addr_ctrl + 0x500, 0x510);// DWC_ddrctl_map_REGB_FREQ0_CH0.INITMR0

		mmio_wr32(base_addr_ctrl + 0x504, 0x0);// DWC_ddrctl_map_REGB_FREQ0_CH0.INITMR1
		mmio_wr32(base_addr_ctrl + 0x508, 0x0);// DWC_ddrctl_map_REGB_FREQ0_CH0.INITMR2
		mmio_wr32(base_addr_ctrl + 0x584, 0xa0505);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG1
		mmio_wr32(base_addr_ctrl + 0x594, 0x40007);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG5
		mmio_wr32(base_addr_ctrl + 0x5a0, 0x20202);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFILPTMG0
		mmio_wr32(base_addr_ctrl + 0x5a4, 0x201);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFILPTMG1
		mmio_wr32(base_addr_ctrl + 0x5a8, 0x1c0003);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFIUPDTMG0
		mmio_wr32(base_addr_ctrl + 0x5ac, 0xa00a0);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFIUPDTMG1
		mmio_wr32(base_addr_ctrl + 0x5b4, 0xc0000002);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFIUPDTMG2

		mmio_wr32(base_addr_ctrl + 0x604, 0x660066);// DWC_ddrctl_map_REGB_FREQ0_CH0.RFSHSET1TMG1
		mmio_wr32(base_addr_ctrl + 0x608, 0x2180000);// DWC_ddrctl_map_REGB_FREQ0_CH0.RFSHSET1TMG2
		mmio_wr32(base_addr_ctrl + 0x60c, 0x11000000);// DWC_ddrctl_map_REGB_FREQ0_CH0.RFSHSET1TMG3
		mmio_wr32(base_addr_ctrl + 0x800, 0x801b6);// DWC_ddrctl_map_REGB_FREQ0_CH0.ZQSET1TMG0
		mmio_wr32(base_addr_ctrl + 0x804, 0xe0003f);// DWC_ddrctl_map_REGB_FREQ0_CH0.ZQSET1TMG1
		mmio_wr32(base_addr_ctrl + 0xa80, 0xed5);// DWC_ddrctl_map_REGB_FREQ0_CH0.DQSOSCCTL0
		mmio_wr32(base_addr_ctrl + 0xb00, 0x803b518d);// DWC_ddrctl_map_REGB_FREQ0_CH0.DERATEINT
		mmio_wr32(base_addr_ctrl + 0xb04, 0x60c0604);// DWC_ddrctl_map_REGB_FREQ0_CH0.DERATEVAL0
		mmio_wr32(base_addr_ctrl + 0xb08, 0x11);// DWC_ddrctl_map_REGB_FREQ0_CH0.DERATEVAL1
		mmio_wr32(base_addr_ctrl + 0xb80, 0xcd00000);// DWC_ddrctl_map_REGB_FREQ0_CH0.HWLPTMG0
		mmio_wr32(base_addr_ctrl + 0xd04, 0xc0c);// DWC_ddrctl_map_REGB_FREQ0_CH0.RANKTMG0
		mmio_wr32(base_addr_ctrl + 0xd0c, 0x400005);// DWC_ddrctl_map_REGB_FREQ0_CH0.PWRTMG
	} else {
		mmio_wr32(base_addr_ctrl + 0x10, 0xf04040f);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG4
		mmio_wr32(base_addr_ctrl + 0x14, 0x2040c09);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG5
		mmio_wr32(base_addr_ctrl + 0x18, 0x8);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG6
		mmio_wr32(base_addr_ctrl + 0x1c, 0x3);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG7
		mmio_wr32(base_addr_ctrl + 0x30, 0x30000);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG12
		mmio_wr32(base_addr_ctrl + 0x34, 0xc100002);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG13
		mmio_wr32(base_addr_ctrl + 0x38, 0x200136);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG14
		mmio_wr32(base_addr_ctrl + 0x44, 0x780050);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG17
		mmio_wr32(base_addr_ctrl + 0x5c, 0x9d0009);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG23

		mmio_wr32(base_addr_ctrl + 0x500, 0x510);// DWC_ddrctl_map_REGB_FREQ0_CH0.INITMR0
		mmio_wr32(base_addr_ctrl + 0x504, 0x0);// DWC_ddrctl_map_REGB_FREQ0_CH0.INITMR1
		mmio_wr32(base_addr_ctrl + 0x508, 0x0);// DWC_ddrctl_map_REGB_FREQ0_CH0.INITMR2

		mmio_wr32(base_addr_ctrl + 0x584, 0x80303);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG1
		mmio_wr32(base_addr_ctrl + 0x594, 0x410000f);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG5
		//mmio_wr32(base_addr_ctrl + 0x598, 0x117);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG6
		mmio_wr32(base_addr_ctrl + 0x5a0, 0x30303);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFILPTMG0
		mmio_wr32(base_addr_ctrl + 0x5a4, 0x302);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFILPTMG1
		mmio_wr32(base_addr_ctrl + 0x5a8, 0x1900004);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFIUPDTMG0
		mmio_wr32(base_addr_ctrl + 0x5ac, 0x9d009e);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFIUPDTMG1
		mmio_wr32(base_addr_ctrl + 0x5b4, 0x40000007);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFIUPDTMG2

		mmio_wr32(base_addr_ctrl + 0x604, 0x1300098);// DWC_ddrctl_map_REGB_FREQ0_CH0.RFSHSET1TMG1
		mmio_wr32(base_addr_ctrl + 0x608, 0x6480000);// DWC_ddrctl_map_REGB_FREQ0_CH0.RFSHSET1TMG2
		mmio_wr32(base_addr_ctrl + 0x60c, 0xc000000);// DWC_ddrctl_map_REGB_FREQ0_CH0.RFSHSET1TMG3
		//mmio_wr32(base_addr_ctrl + 0x650, 0xd0);// DWC_ddrctl_map_REGB_FREQ0_CH0.RFMSET1TMG0
		mmio_wr32(base_addr_ctrl + 0x800, 0x1804d7);// DWC_ddrctl_map_REGB_FREQ0_CH0.ZQSET1TMG0
		mmio_wr32(base_addr_ctrl + 0x804, 0x280004b);// DWC_ddrctl_map_REGB_FREQ0_CH0.ZQSET1TMG1
		mmio_wr32(base_addr_ctrl + 0xa80, 0x3bb1);// DWC_ddrctl_map_REGB_FREQ0_CH0.DQSOSCCTL0
		mmio_wr32(base_addr_ctrl + 0xb00, 0x9bb4bf0f);// DWC_ddrctl_map_REGB_FREQ0_CH0.DERATEINT
		mmio_wr32(base_addr_ctrl + 0xb04, 0x1024100a);// DWC_ddrctl_map_REGB_FREQ0_CH0.DERATEVAL0
		mmio_wr32(base_addr_ctrl + 0xb08, 0x533);// DWC_ddrctl_map_REGB_FREQ0_CH0.DERATEVAL1
		mmio_wr32(base_addr_ctrl + 0xb80, 0xf0000);// DWC_ddrctl_map_REGB_FREQ0_CH0.HWLPTMG0
		rddata = mmio_rd32(base_addr_ctrl + 0xc84);
		//rddata = {rddata[31:16], 0x10a};
		rddata = modified_bits_by_value(rddata, 0x10a, 15, 0);
		mmio_wr32(base_addr_ctrl + 0xc84, rddata);// modify for lp5 6400 performance
		rddata = mmio_rd32(base_addr_ctrl + 0xc88);
		//rddata = {rddata[31:16], 0x214};
		rddata = modified_bits_by_value(rddata, 0x214, 15, 0);
		mmio_wr32(base_addr_ctrl + 0xc88, rddata);// modify for lp5 6400 performance
		mmio_wr32(base_addr_ctrl + 0xd00, 0x1);// DWC_ddrctl_map_REGB_FREQ0_CH0.TMGCFG
		mmio_wr32(base_addr_ctrl + 0xd04, 0xb07);// DWC_ddrctl_map_REGB_FREQ0_CH0.RANKTMG0
		mmio_wr32(base_addr_ctrl + 0xd0c, 0x400005);// DWC_ddrctl_map_REGB_FREQ0_CH0.PWRTMG
	}

	//for freq1 4800mhz(1:4)
	mmio_wr32(base_addr_ctrl + 0x100000, 0x1f0c141a);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x100004, 0xc0524);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG1
	mmio_wr32(base_addr_ctrl + 0x100008, 0x70d0f13);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG2
	mmio_wr32(base_addr_ctrl + 0x10000c, 0xa1a25);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG3
	mmio_wr32(base_addr_ctrl + 0x100010, 0xb04030b);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG4
	mmio_wr32(base_addr_ctrl + 0x100014, 0x2030907);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG5
	mmio_wr32(base_addr_ctrl + 0x100018, 0xe);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG6
	mmio_wr32(base_addr_ctrl + 0x10001c, 0x2);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG7
	mmio_wr32(base_addr_ctrl + 0x100024, 0x2030f);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG9
	mmio_wr32(base_addr_ctrl + 0x100030, 0x30000);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG12
	mmio_wr32(base_addr_ctrl + 0x100034, 0xa100002);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG13
	mmio_wr32(base_addr_ctrl + 0x100038, 0x1800e9);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG14
	mmio_wr32(base_addr_ctrl + 0x100044, 0x5a003c);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG17
	mmio_wr32(base_addr_ctrl + 0x10005c, 0x760007);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG23
	mmio_wr32(base_addr_ctrl + 0x100060, 0xd120c);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG24
	mmio_wr32(base_addr_ctrl + 0x100064, 0x1f05);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG25
	mmio_wr32(base_addr_ctrl + 0x100078, 0x141114);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG30
	mmio_wr32(base_addr_ctrl + 0x100080, 0x30407);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG32, added
	mmio_wr32(base_addr_ctrl + 0x100500, 0x510);// DWC_ddrctl_map_REGB_FREQ1_CH0.INITMR0
	mmio_wr32(base_addr_ctrl + 0x100504, 0x0);// DWC_ddrctl_map_REGB_FREQ1_CH0.INITMR1
	mmio_wr32(base_addr_ctrl + 0x100508, 0x0);// DWC_ddrctl_map_REGB_FREQ1_CH0.INITMR2
	mmio_wr32(base_addr_ctrl + 0x10050c, 0x0);// DWC_ddrctl_map_REGB_FREQ1_CH0.INITMR3, added
	mmio_wr32(base_addr_ctrl + 0x100580, 0x32f0217);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFITMG0, added
	mmio_wr32(base_addr_ctrl + 0x100584, 0x80303);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFITMG1
	mmio_wr32(base_addr_ctrl + 0x100588, 0x182f17);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFITMG2, added
	mmio_wr32(base_addr_ctrl + 0x100590, 0x1008040c);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFITMG4, added
	mmio_wr32(base_addr_ctrl + 0x100594, 0x40c000f);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFITMG5
	//mmio_wr32(base_addr_ctrl + 0x100598, 0x117);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFITMG6
	mmio_wr32(base_addr_ctrl + 0x1005ac, 0x490098);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFIUPDTMG1
	mmio_wr32(base_addr_ctrl + 0x1005b4, 0x4000000f);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFIUPDTMG2
	mmio_wr32(base_addr_ctrl + 0x1005b8, 0xe7);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFIUPDTMG3, added
	mmio_wr32(base_addr_ctrl + 0x100600, 0xc2140124);//DWC_ddrctl_map_REGB_FREQ1_CH0.RFSHSET1TMG0, added
	mmio_wr32(base_addr_ctrl + 0x100604, 0xe40072);// DWC_ddrctl_map_REGB_FREQ1_CH0.RFSHSET1TMG1
	mmio_wr32(base_addr_ctrl + 0x100608, 0x5360000);// DWC_ddrctl_map_REGB_FREQ1_CH0.RFSHSET1TMG2
	mmio_wr32(base_addr_ctrl + 0x10060c, 0x0);// DWC_ddrctl_map_REGB_FREQ1_CH0.RFSHSET1TMG3
	//mmio_wr32(base_addr_ctrl + 0x100650, 0xd0);// DWC_ddrctl_map_REGB_FREQ1_CH0.RFMSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x100800, 0x1203aa);// DWC_ddrctl_map_REGB_FREQ1_CH0.ZQSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x100804, 0x1e00060);// DWC_ddrctl_map_REGB_FREQ1_CH0.ZQSET1TMG1
	mmio_wr32(base_addr_ctrl + 0x100a80, 0x2371);// DWC_ddrctl_map_REGB_FREQ1_CH0.DQSOSCCTL0
	mmio_wr32(base_addr_ctrl + 0x100b00, 0xe296eaf3);// DWC_ddrctl_map_REGB_FREQ1_CH0.DERATEINT
	mmio_wr32(base_addr_ctrl + 0x100b04, 0xc1b0c08);// DWC_ddrctl_map_REGB_FREQ1_CH0.DERATEVAL0
	mmio_wr32(base_addr_ctrl + 0x100b08, 0x27);// DWC_ddrctl_map_REGB_FREQ1_CH0.DERATEVAL1
	mmio_wr32(base_addr_ctrl + 0x100b80, 0xf0000);// DWC_ddrctl_map_REGB_FREQ1_CH0.HWLPTMG0
	rddata = mmio_rd32(base_addr_ctrl + 0x100c84);//
	//rddata = {rddata[31:16], 0x0c8};
	rddata = modified_bits_by_value(rddata, 0x0c8, 15, 0);
	mmio_wr32(base_addr_ctrl + 0x100c84, rddata);// modify for lp5 4800 performance
	rddata = mmio_rd32(base_addr_ctrl + 0x100c88);
	//rddata = {rddata[31:16], 0x190};
	rddata = modified_bits_by_value(rddata, 0x190, 15, 0);
	mmio_wr32(base_addr_ctrl + 0x100c88, rddata);// modify for lp5 4800 performance
	mmio_wr32(base_addr_ctrl + 0x100d00, 0x1);// DWC_ddrctl_map_REGB_FREQ1_CH0.TMGCFG
	mmio_wr32(base_addr_ctrl + 0x100d04, 0xb07);// DWC_ddrctl_map_REGB_FREQ1_CH0.RANKTMG0
	mmio_wr32(base_addr_ctrl + 0x100d08, 0x1103);//DWC_ddrctl_map_REGB_FREQ1_CH0.RANKTMG1 added
	mmio_wr32(base_addr_ctrl + 0x100d0c, 0x400005);// DWC_ddrctl_map_REGB_FREQ1_CH0.PWRTMG

	//for freq2 3200mhz(1:2)
	mmio_wr32(base_addr_ctrl + 0x200000, 0x2b101b22);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x200004, 0x100630);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG1
	mmio_wr32(base_addr_ctrl + 0x200008, 0xa121318);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG2
	mmio_wr32(base_addr_ctrl + 0x20000c, 0x102030);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG3
	mmio_wr32(base_addr_ctrl + 0x200010, 0xf04040f);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG4
	mmio_wr32(base_addr_ctrl + 0x200014, 0x2040c09);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG5
	mmio_wr32(base_addr_ctrl + 0x200018, 0x12);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG6
	mmio_wr32(base_addr_ctrl + 0x20001c, 0x3);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG7
	mmio_wr32(base_addr_ctrl + 0x200024, 0x40413);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG9
	mmio_wr32(base_addr_ctrl + 0x200030, 0x30000);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG12
	mmio_wr32(base_addr_ctrl + 0x200034, 0xf100002);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG13
	mmio_wr32(base_addr_ctrl + 0x200038, 0x200136);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG14
	mmio_wr32(base_addr_ctrl + 0x200044, 0x780050);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG17
	mmio_wr32(base_addr_ctrl + 0x20005c, 0x9d0009);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG23
	mmio_wr32(base_addr_ctrl + 0x200060, 0x2131911);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG24
	mmio_wr32(base_addr_ctrl + 0x200064, 0x2b06);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG25
	mmio_wr32(base_addr_ctrl + 0x200078, 0x1b151b);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG30
	mmio_wr32(base_addr_ctrl + 0x200500, 0x510);// DWC_ddrctl_map_REGB_FREQ2_CH0.INITMR0
	mmio_wr32(base_addr_ctrl + 0x200504, 0x0);// DWC_ddrctl_map_REGB_FREQ2_CH0.INITMR1
	mmio_wr32(base_addr_ctrl + 0x200508, 0x0);// DWC_ddrctl_map_REGB_FREQ2_CH0.INITMR2
	mmio_wr32(base_addr_ctrl + 0x20050c, 0x0);// DWC_ddrctl_map_REGB_FREQ2_CH0.INITMR3, added
	mmio_wr32(base_addr_ctrl + 0x200580, 0x31f020f);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFITMG0, added
	mmio_wr32(base_addr_ctrl + 0x200584, 0x80303);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFITMG1
	mmio_wr32(base_addr_ctrl + 0x200588, 0x101f0f);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFITMG2, added
	mmio_wr32(base_addr_ctrl + 0x200590, 0x6020007);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFITMG4, added
	mmio_wr32(base_addr_ctrl + 0x200594, 0x80007);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFITMG5
	//mmio_wr32(base_addr_ctrl + 0x200598, 0x117);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFITMG6
	mmio_wr32(base_addr_ctrl + 0x2005ac, 0x490098);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFIUPDTMG1
	mmio_wr32(base_addr_ctrl + 0x2005b4, 0x4000000f);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFIUPDTMG2
	mmio_wr32(base_addr_ctrl + 0x2005b8, 0x13d);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFIUPDTMG3, added
	mmio_wr32(base_addr_ctrl + 0x200600, 0xc2190186);//DWC_ddrctl_map_REGB_FREQ2_CH0.RFSHSET1TMG0, added
	mmio_wr32(base_addr_ctrl + 0x200604, 0x1300098);// DWC_ddrctl_map_REGB_FREQ2_CH0.RFSHSET1TMG1
	mmio_wr32(base_addr_ctrl + 0x200608, 0x6480000);// DWC_ddrctl_map_REGB_FREQ2_CH0.RFSHSET1TMG2
	mmio_wr32(base_addr_ctrl + 0x20060c, 0x0);// DWC_ddrctl_map_REGB_FREQ2_CH0.RFSHSET1TMG3
	//mmio_wr32(base_addr_ctrl + 0x200650, 0xd0);// DWC_ddrctl_map_REGB_FREQ2_CH0.RFMSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x200800, 0x1804d6);// DWC_ddrctl_map_REGB_FREQ2_CH0.ZQSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x200804, 0x2800060);// DWC_ddrctl_map_REGB_FREQ2_CH0.ZQSET1TMG1
	mmio_wr32(base_addr_ctrl + 0x200a80, 0x2371);// DWC_ddrctl_map_REGB_FREQ2_CH0.DQSOSCCTL0
	mmio_wr32(base_addr_ctrl + 0x200b00, 0xe296eaf3);// DWC_ddrctl_map_REGB_FREQ2_CH0.DERATEINT
	mmio_wr32(base_addr_ctrl + 0x200b04, 0x1024100a);// DWC_ddrctl_map_REGB_FREQ2_CH0.DERATEVAL0
	mmio_wr32(base_addr_ctrl + 0x200b08, 0x33);// DWC_ddrctl_map_REGB_FREQ2_CH0.DERATEVAL1
	mmio_wr32(base_addr_ctrl + 0x200b80, 0xf0000);// DWC_ddrctl_map_REGB_FREQ2_CH0.HWLPTMG0
	rddata = mmio_rd32(base_addr_ctrl + 0x200c84);//
	//rddata = {rddata[31:16], 160x10a};
	rddata = modified_bits_by_value(rddata, 0x10a, 15, 0);
	mmio_wr32(base_addr_ctrl + 0x200c84, rddata);// modify for lp5 3200 performance
	rddata = mmio_rd32(base_addr_ctrl + 0x200c88);//
	//rddata = {rddata[31:16], 0x214};
	rddata = modified_bits_by_value(rddata, 0x214, 15, 0);
	mmio_wr32(base_addr_ctrl + 0x200c88, rddata);// modify for lp5 3200 performance
	mmio_wr32(base_addr_ctrl + 0x200d00, 0x0);// DWC_ddrctl_map_REGB_FREQ2_CH0.TMGCFG
	mmio_wr32(base_addr_ctrl + 0x200d04, 0x100c);// DWC_ddrctl_map_REGB_FREQ2_CH0.RANKTMG0
	mmio_wr32(base_addr_ctrl + 0x200d08, 0x1906);//DWC_ddrctl_map_REGB_FREQ2_CH0.RANKTMG1 added
	mmio_wr32(base_addr_ctrl + 0x200d0c, 0x400005);// DWC_ddrctl_map_REGB_FREQ2_CH0.PWRTMG

	if (p_lpddr_attr->dfs_pll_bypass == false) {
		//for freq3 2400mhz(1:2)
		mmio_wr32(base_addr_ctrl + 0x300000, 0x220c141a);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG0
		mmio_wr32(base_addr_ctrl + 0x300004, 0xe0524);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG1
		mmio_wr32(base_addr_ctrl + 0x300008, 0x8101214);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG2
		mmio_wr32(base_addr_ctrl + 0x30000c, 0xf1e2a);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG3
		mmio_wr32(base_addr_ctrl + 0x300010, 0xd04040d);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG4
		mmio_wr32(base_addr_ctrl + 0x300014, 0x2040b08);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG5
		mmio_wr32(base_addr_ctrl + 0x300018, 0x10);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG6
		mmio_wr32(base_addr_ctrl + 0x30001c, 0x3);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG7
		mmio_wr32(base_addr_ctrl + 0x300024, 0x40411);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG9
		mmio_wr32(base_addr_ctrl + 0x300030, 0x30000);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG12
		mmio_wr32(base_addr_ctrl + 0x300034, 0xd100002);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG13
		mmio_wr32(base_addr_ctrl + 0x300038, 0x1c010b);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG14
		mmio_wr32(base_addr_ctrl + 0x300044, 0x680045);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG17
		mmio_wr32(base_addr_ctrl + 0x30005c, 0x870008);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG23
		mmio_wr32(base_addr_ctrl + 0x300060, 0x212170f);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG24
		mmio_wr32(base_addr_ctrl + 0x300064, 0x2506);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG25
		mmio_wr32(base_addr_ctrl + 0x300078, 0x191419);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG30
		mmio_wr32(base_addr_ctrl + 0x300500, 0x510);// DWC_ddrctl_map_REGB_FREQ3_CH0.INITMR0
		mmio_wr32(base_addr_ctrl + 0x300504, 0x0);// DWC_ddrctl_map_REGB_FREQ3_CH0.INITMR1
		mmio_wr32(base_addr_ctrl + 0x300508, 0x0);// DWC_ddrctl_map_REGB_FREQ3_CH0.INITMR2
		mmio_wr32(base_addr_ctrl + 0x30050c, 0x0);// DWC_ddrctl_map_REGB_FREQ3_CH0.INITMR3, added
		mmio_wr32(base_addr_ctrl + 0x300580, 0x41d020d);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG0, added
		mmio_wr32(base_addr_ctrl + 0x300584, 0x90404);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG1
		mmio_wr32(base_addr_ctrl + 0x300588, 0x121d0d);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG2, added
		mmio_wr32(base_addr_ctrl + 0x300590, 0x4000008);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG4, added
		mmio_wr32(base_addr_ctrl + 0x300594, 0x80007);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG5
		//mmio_wr32(base_addr_ctrl + 0x300598, 0x117);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG6
		mmio_wr32(base_addr_ctrl + 0x3005ac, 0x490098);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFIUPDTMG1
		mmio_wr32(base_addr_ctrl + 0x3005b4, 0x4000000f);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFIUPDTMG2
		mmio_wr32(base_addr_ctrl + 0x3005b8, 0x13c);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFIUPDTMG3, added
		mmio_wr32(base_addr_ctrl + 0x300600, 0xc2160124);// DWC_ddrctl_map_REGB_FREQ3_CH0.RFSHSET1TMG0, added
		mmio_wr32(base_addr_ctrl + 0x300604, 0x1060072);// DWC_ddrctl_map_REGB_FREQ3_CH0.RFSHSET1TMG1
		mmio_wr32(base_addr_ctrl + 0x300608, 0x63e0000);// DWC_ddrctl_map_REGB_FREQ3_CH0.RFSHSET1TMG2
		mmio_wr32(base_addr_ctrl + 0x30060c, 0x0);// DWC_ddrctl_map_REGB_FREQ3_CH0.RFSHSET1TMG3
		//mmio_wr32(base_addr_ctrl + 0x300650, 0xd0);// DWC_ddrctl_map_REGB_FREQ3_CH0.RFMSET1TMG0
		mmio_wr32(base_addr_ctrl + 0x300800, 0x15042e);// DWC_ddrctl_map_REGB_FREQ3_CH0.ZQSET1TMG0
		mmio_wr32(base_addr_ctrl + 0x300804, 0x2300060);// DWC_ddrctl_map_REGB_FREQ3_CH0.ZQSET1TMG1
		mmio_wr32(base_addr_ctrl + 0x300a80, 0x2371);// DWC_ddrctl_map_REGB_FREQ3_CH0.DQSOSCCTL0
		mmio_wr32(base_addr_ctrl + 0x300b00, 0xe296eaf3);// DWC_ddrctl_map_REGB_FREQ3_CH0.DERATEINT
		mmio_wr32(base_addr_ctrl + 0x300b04, 0xe1f0e09);// DWC_ddrctl_map_REGB_FREQ3_CH0.DERATEVAL0
		mmio_wr32(base_addr_ctrl + 0x300b08, 0x2c);// DWC_ddrctl_map_REGB_FREQ3_CH0.DERATEVAL1
		mmio_wr32(base_addr_ctrl + 0x300b80, 0xf0000);// DWC_ddrctl_map_REGB_FREQ3_CH0.HWLPTMG0
		rddata = mmio_rd32(base_addr_ctrl + 0x300c84);//
		//rddata = {rddata[31:16], 0x0c8};
		rddata = modified_bits_by_value(rddata, 0x0c8, 15, 0);
		mmio_wr32(base_addr_ctrl + 0x300c84, rddata);// modify for lp5 2400 performance
		rddata = mmio_rd32(base_addr_ctrl + 0x300c88);//
		//rddata = {rddata[31:16], 0x190};
		rddata = modified_bits_by_value(rddata, 0x190, 15, 0);
		mmio_wr32(base_addr_ctrl + 0x300c88, rddata);// modify for lp5 2400 performance
		mmio_wr32(base_addr_ctrl + 0x300d00, 0x0);// DWC_ddrctl_map_REGB_FREQ3_CH0.TMGCFG
		mmio_wr32(base_addr_ctrl + 0x300d04, 0xf0c);// DWC_ddrctl_map_REGB_FREQ3_CH0.RANKTMG0
		mmio_wr32(base_addr_ctrl + 0x300d08, 0x1806);// DWC_ddrctl_map_REGB_FREQ3_CH0.RANKTMG1 added
		mmio_wr32(base_addr_ctrl + 0x300d0c, 0x400005);// DWC_ddrctl_map_REGB_FREQ3_CH0.PWRTMG
	} else {
		//for freq3 600mhz(1:2)
		mmio_wr32(base_addr_ctrl + 0x300000, 0x1306090c);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG0
		mmio_wr32(base_addr_ctrl + 0x300004, 0x60410);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG1
		mmio_wr32(base_addr_ctrl + 0x300008, 0x4080d0c);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG2
		mmio_wr32(base_addr_ctrl + 0x30000c, 0xd1116);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG3
		mmio_wr32(base_addr_ctrl + 0x300010, 0x5040205);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG4
		mmio_wr32(base_addr_ctrl + 0x300014, 0x2030404);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG5
		mmio_wr32(base_addr_ctrl + 0x300018, 0x8);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG6
		mmio_wr32(base_addr_ctrl + 0x30001c, 0x1);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG7
		mmio_wr32(base_addr_ctrl + 0x300024, 0x4020c);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG9
		mmio_wr32(base_addr_ctrl + 0x300030, 0x30000);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG12
		mmio_wr32(base_addr_ctrl + 0x300034, 0x9100002);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG13
		mmio_wr32(base_addr_ctrl + 0x300038, 0xb0068);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG14
		mmio_wr32(base_addr_ctrl + 0x300044, 0x28001b);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG17
		mmio_wr32(base_addr_ctrl + 0x30005c, 0x350004);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG23
		mmio_wr32(base_addr_ctrl + 0x300060, 0x20d0f0b);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG24
		mmio_wr32(base_addr_ctrl + 0x300064, 0x1304);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG25
		mmio_wr32(base_addr_ctrl + 0x300078, 0x110f11);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG30
		mmio_wr32(base_addr_ctrl + 0x300080, 0x30404);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG32
		mmio_wr32(base_addr_ctrl + 0x300500, 0x510);// DWC_ddrctl_map_REGB_FREQ3_CH0.INITMR0
		mmio_wr32(base_addr_ctrl + 0x300504, 0x0);// DWC_ddrctl_map_REGB_FREQ3_CH0.INITMR1
		mmio_wr32(base_addr_ctrl + 0x300508, 0x0);// DWC_ddrctl_map_REGB_FREQ3_CH0.INITMR2
		mmio_wr32(base_addr_ctrl + 0x30050c, 0x0);// DWC_ddrctl_map_REGB_FREQ3_CH0.INITMR3, added
		mmio_wr32(base_addr_ctrl + 0x300580, 0x50f0207);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG0, added
		mmio_wr32(base_addr_ctrl + 0x300584, 0xa0505);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG1
		mmio_wr32(base_addr_ctrl + 0x300588, 0x140f07);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG2, added
		mmio_wr32(base_addr_ctrl + 0x300590, 0x7);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG4, added
		mmio_wr32(base_addr_ctrl + 0x300594, 0x40007);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG5
		mmio_wr32(base_addr_ctrl + 0x3005ac, 0xa00a0);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFIUPDTMG1
		mmio_wr32(base_addr_ctrl + 0x3005b4, 0xc0000002);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFIUPDTMG2
		mmio_wr32(base_addr_ctrl + 0x3005b8, 0xca);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFIUPDTMG3, added
		mmio_wr32(base_addr_ctrl + 0x300600, 0xc2100411);// DWC_ddrctl_map_REGB_FREQ3_CH0.RFSHSET1TMG0, added
		mmio_wr32(base_addr_ctrl + 0x300604, 0x660066);// DWC_ddrctl_map_REGB_FREQ3_CH0.RFSHSET1TMG1
		mmio_wr32(base_addr_ctrl + 0x300608, 0x2180000);// DWC_ddrctl_map_REGB_FREQ3_CH0.RFSHSET1TMG2
		mmio_wr32(base_addr_ctrl + 0x30060c, 0x11000000);// DWC_ddrctl_map_REGB_FREQ3_CH0.RFSHSET1TMG3
		mmio_wr32(base_addr_ctrl + 0x300800, 0x801b6);// DWC_ddrctl_map_REGB_FREQ3_CH0.ZQSET1TMG0
		mmio_wr32(base_addr_ctrl + 0x300804, 0xe0003f);// DWC_ddrctl_map_REGB_FREQ3_CH0.ZQSET1TMG1
		mmio_wr32(base_addr_ctrl + 0x300a80, 0xed5);// DWC_ddrctl_map_REGB_FREQ3_CH0.DQSOSCCTL0
		mmio_wr32(base_addr_ctrl + 0x300b00, 0x803b518d);// DWC_ddrctl_map_REGB_FREQ3_CH0.DERATEINT
		mmio_wr32(base_addr_ctrl + 0x300b04, 0x60c0604);// DWC_ddrctl_map_REGB_FREQ3_CH0.DERATEVAL0
		mmio_wr32(base_addr_ctrl + 0x300b08, 0x11);// DWC_ddrctl_map_REGB_FREQ3_CH0.DERATEVAL1
		mmio_wr32(base_addr_ctrl + 0x300b80, 0xcd00000);// DWC_ddrctl_map_REGB_FREQ3_CH0.HWLPTMG0
		mmio_wr32(base_addr_ctrl + 0x300d04, 0xc0c);// DWC_ddrctl_map_REGB_FREQ3_CH0.RANKTMG0
		mmio_wr32(base_addr_ctrl + 0x300d08, 0x1006);// DWC_ddrctl_map_REGB_FREQ3_CH0.RANKTMG1 added
		mmio_wr32(base_addr_ctrl + 0x300d0c, 0x1006);// DWC_ddrctl_map_REGB_FREQ3_CH0.PWRTMG
	}

	//SNPS ADDRESS MAP
	#ifndef SG_MDF_ECC //SNPS CODE
	if (p_lpddr_attr->lp5_inlineecc_en == true) {
		#ifdef SG_MDF_RD_CAM
		if (p_lpddr_attr->num_rank == 2)
			mmio_wr32(base_addr_ctrl + 0x30004, 0x3f16);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP1
		else
			mmio_wr32(base_addr_ctrl + 0x30004, 0x3f3f);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP1

		mmio_wr32(base_addr_ctrl + 0x3000c, 0x3f0606);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP3
		mmio_wr32(base_addr_ctrl + 0x30010, 0x0404);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP4
		if (p_lpddr_attr->num_rank == 2)
			mmio_wr32(base_addr_ctrl + 0x30014, 0x1f161616);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP5
		else
			mmio_wr32(base_addr_ctrl + 0x30014, 0x1f151515);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP5

		mmio_wr32(base_addr_ctrl + 0x30018, 0x00000000);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP6
		mmio_wr32(base_addr_ctrl + 0x3001c, 0x1f050505);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP7
		mmio_wr32(base_addr_ctrl + 0x30020, 0x05050505);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP8
		mmio_wr32(base_addr_ctrl + 0x30024, 0x05050505);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP9
		mmio_wr32(base_addr_ctrl + 0x30028, 0x05050505);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP10
		mmio_wr32(base_addr_ctrl + 0x3002c, 0x00000505);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP11
		#else
		if (p_lpddr_attr->num_rank == 2)
			mmio_wr32(base_addr_ctrl + 0x30004, 0x3f16);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP1
		else
			mmio_wr32(base_addr_ctrl + 0x30004, 0x3f3f);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP1

		mmio_wr32(base_addr_ctrl + 0x3000c, 0x3f0606);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP3
		mmio_wr32(base_addr_ctrl + 0x30010, 0x0401);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP4
		if (p_lpddr_attr->num_rank == 2)
			mmio_wr32(base_addr_ctrl + 0x30014, 0x1f161616);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP5
		else
			mmio_wr32(base_addr_ctrl + 0x30014, 0x1f151515);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP5

		mmio_wr32(base_addr_ctrl + 0x30018, 0x01010100);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP6
		mmio_wr32(base_addr_ctrl + 0x3001c, 0x1f050505);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP7
		mmio_wr32(base_addr_ctrl + 0x30020, 0x05050505);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP8
		mmio_wr32(base_addr_ctrl + 0x30024, 0x05050505);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP9
		mmio_wr32(base_addr_ctrl + 0x30028, 0x05050505);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP10
		mmio_wr32(base_addr_ctrl + 0x3002c, 0x00000505);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP11
		#endif
		} else {
			if (p_lpddr_attr->num_rank == 2)
				mmio_wr32(base_addr_ctrl + 0x30004, 0x3f19);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP1
			else
				mmio_wr32(base_addr_ctrl + 0x30004, 0x3f3f);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP1

			mmio_wr32(base_addr_ctrl + 0x3000c, 0x3f0909);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP3
			mmio_wr32(base_addr_ctrl + 0x30010, 0x704);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP4
			mmio_wr32(base_addr_ctrl + 0x30014, 0x1f010101);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP5
			mmio_wr32(base_addr_ctrl + 0x30018, 0x0);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP6
			mmio_wr32(base_addr_ctrl + 0x3001c, 0x1f080808);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP7
			mmio_wr32(base_addr_ctrl + 0x30020, 0x08080808);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP8
			mmio_wr32(base_addr_ctrl + 0x30024, 0x08080808);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP9
			mmio_wr32(base_addr_ctrl + 0x30028, 0x08080808);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP10
			mmio_wr32(base_addr_ctrl + 0x3002c, 0x808);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP11
		}
	#else
		if (p_lpddr_attr->num_rank == 2)
			//mmio_wr32(base_addr_ctrl + 0x30004, 0x3f15);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP1
			mmio_wr32(base_addr_ctrl + 0x30004, 0x3f19);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP1
		else
			mmio_wr32(base_addr_ctrl + 0x30004, 0x3f3f);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP1

		mmio_wr32(base_addr_ctrl + 0x3000c, 0x3f0909);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP3
		mmio_wr32(base_addr_ctrl + 0x30010, 0x704);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP4
		mmio_wr32(base_addr_ctrl + 0x30014, 0x1f010101);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP5
		mmio_wr32(base_addr_ctrl + 0x30018, 0x0);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP6
		#ifdef HALF_CAPACITY_MODE
			mmio_wr32(base_addr_ctrl + 0x3001c, 0x1f1f0808);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP7
		#else
			mmio_wr32(base_addr_ctrl + 0x3001c, 0x1f080808);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP7
		#endif
		mmio_wr32(base_addr_ctrl + 0x30020, 0x08080808);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP8
		mmio_wr32(base_addr_ctrl + 0x30024, 0x08080808);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP9
		mmio_wr32(base_addr_ctrl + 0x30028, 0x08080808);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP10
		mmio_wr32(base_addr_ctrl + 0x3002c, 0x808);// DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP11
	#endif

	mmio_wr32(base_addr_ctrl + 0x10b84, 0x0);// DWC_ddrctl_map_REGB_DDRC_CH0.OPCTRL1
	mmio_wr32(base_addr_ctrl + 0x20090, 0x1);// DWC_ddrctl_map_REGB_ARB_PORT0.PCTRL
	mmio_wr32(base_addr_ctrl + 0x21090, 0x1);// DWC_ddrctl_map_REGB_ARB_PORT1.PCTRL
	//mmio_wr32(base_addr_ctrl + 0x10208, 0x1);// DWC_ddrctl_map_REGB_DDRC_CH0.RFSHCTL0

	//if ($test$plusargs("RRB_LOCK_THRESHOLD"))begin  //performance related
	rddata = mmio_rd32(base_addr_ctrl + 0x20004);// PCFGGR
	//rddata = {rddata[31:24], 0x8, rddata[19:0] };//
	rddata = modified_bits_by_value(rddata, 0x8, 23, 20);
	mmio_wr32(base_addr_ctrl + 0x20004, rddata);
	//end

	//if ($test$plusargs("DISABLE_AGING"))begin  //performance related
	//rddata = mmio_rd32(base_addr_ctrl + 0x20004, rddata);// PCFGGR
	//rddata = {rddata[31:13], 10x0, rddata[11:0] };//
	//mmio_wr32(base_addr_ctrl + 0x20004, rddata);

	//rddata = mmio_rd32(base_addr_ctrl + 0x20008, rddata);// PCFGGW
	//rddata = {rddata[31:13], 10x0, rddata[11:0] };//
	//mmio_wr32(base_addr_ctrl + 0x20008, rddata);
	//end

	//modify aging_en & pagematch_limit
	rddata = mmio_rd32(base_addr_ctrl + 0x20000);// PCCFG
	//rddata = {rddata[31:5], 0x1, rddata[3:0] };
	rddata = modified_bits_by_value(rddata, 1, 4, 4);
	mmio_wr32(base_addr_ctrl + 0x20000, rddata);

	rddata = mmio_rd32(base_addr_ctrl + 0x20004);// PCFGR
	//rddata = {rddata[31:13], 0x1, rddata[11:10], 0x20 };
	rddata = modified_bits_by_value(rddata, 1, 12, 12);
	rddata = modified_bits_by_value(rddata, 0x20, 9, 0);
	mmio_wr32(base_addr_ctrl + 0x20004, rddata);

	rddata = mmio_rd32(base_addr_ctrl + 0x20008);// PCFGW
	//rddata = {rddata[31:13], 0x0, rddata[11:0] };
	rddata = modified_bits_by_value(rddata, 0, 12, 12);
	mmio_wr32(base_addr_ctrl + 0x20008, rddata);

	rddata = mmio_rd32(base_addr_ctrl + 0x21004);// PCFGR
	//rddata = {rddata[31:13], 0x0, rddata[11:0] };
	rddata = modified_bits_by_value(rddata, 0, 12, 12);
	mmio_wr32(base_addr_ctrl + 0x21004, rddata);

	rddata = mmio_rd32(base_addr_ctrl + 0x21008);// PCFGW
	//rddata = {rddata[31:13], 0x0, rddata[11:0] };
	rddata = modified_bits_by_value(rddata, 0, 12, 12);
	mmio_wr32(base_addr_ctrl + 0x21008, rddata);
	//modify qos REG
	mmio_wr32(base_addr_ctrl + 0x20094, 0x1000e0d);//PCFGQOS0
	mmio_wr32(base_addr_ctrl + 0x2009c, 0xe0d);//PCFGWQOS0

	//if ($test$plusargs("STARVE"))begin  //performance related
	//rddata = mmio_rd32(base_addr_ctrl + 0xc80, rddata);// PCFGGR
	//rddata = {rddata[31:16], 160x200};//
	//mmio_wr32(base_addr_ctrl + 0xc80, rddata);

	//rddata = mmio_rd32(base_addr_ctrl + 0xc84, rddata);// PCFGGR
	//rddata = {rddata[31:16], 160x200};//
	//mmio_wr32(base_addr_ctrl + 0xc84, rddata);

	//rddata = mmio_rd32(base_addr_ctrl + 0xc88, rddata);// PCFGGR
	//rddata = {rddata[31:16], 160x200};//
	//mmio_wr32(base_addr_ctrl + 0xc88, rddata);
	//end

	if (p_lpddr_attr->dbi_en == true) {
		rddata = mmio_rd32(base_addr_ctrl + 0x10c94);// DWC_ddrctl_map_REGB_DDRC_CH0.DBICTL
		//rddata = {rddata[31:3], 1'b1, 1'b1, 1'b0};
		rddata = modified_bits_by_value(rddata, 0b110, 2, 0);
		mmio_wr32(base_addr_ctrl + 0x10c94, rddata);// DWC_ddrctl_map_REGB_DDRC_CH0.ECCCFG0

		//DRAMSET1TMG2.wr2rd([7:0])= 0x17, DRAMSET1TMG2.rd2wr([15:8])= 0x11,
		//DRAMSET1TMG2.read_latency([22:16]) = 0x11->0x13
		mmio_wr32(base_addr_ctrl + 0x8, 0x9131117);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG2
		mmio_wr32(base_addr_ctrl + 0x100008, 0x9131117);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG2
		mmio_wr32(base_addr_ctrl + 0x200008, 0x9131117);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG2
		mmio_wr32(base_addr_ctrl + 0x300008, 0x9131117);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG2

		//DRAMSET1TMG3.wr2mr([7:0]) = 0x2f , DRAMSET1TMG3.rd2mr([15:8]) = 0x21 -> 0x23
		mmio_wr32(base_addr_ctrl + 0xc, 0xc232f);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG3
		mmio_wr32(base_addr_ctrl + 0x10000c, 0xc232f);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG3
		mmio_wr32(base_addr_ctrl + 0x20000c, 0xc232f);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG3
		mmio_wr32(base_addr_ctrl + 0x30000c, 0xc232f);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG3

		//DRAMSET1TMG24.max_rd_sync([15:8]) = 0x16 -> 0x18 DRAMSET1TMG24.rd2wr_s([23:16])= 0xf -> 0x11
		mmio_wr32(base_addr_ctrl + 0x60, 0x11180e);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG24
		mmio_wr32(base_addr_ctrl + 0x100060, 0x11180e);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG24
		mmio_wr32(base_addr_ctrl + 0x200060, 0x11180e);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG24
		mmio_wr32(base_addr_ctrl + 0x300060, 0x11180e);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG24

		//DRAMSET1TMG30.mrr2rd([7:0]) = 0x18 -> 0x1a, DRAMSET1TMG30.mrr2wr([15:8]) = 0x13 -> 0x15,
		//DRAMSET1TMG30.mrr2mrw([23:16]) = 0x19 -> 0x1b
		mmio_wr32(base_addr_ctrl + 0x78, 0x1b151a);// DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG30
		mmio_wr32(base_addr_ctrl + 0x100078, 0x1b151a);// DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG30
		mmio_wr32(base_addr_ctrl + 0x200078, 0x1b151a);// DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG30
		mmio_wr32(base_addr_ctrl + 0x300078, 0x1b151a);// DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG30

		//DFITMG0.dfi_t_rddata_en([22:16])= 0x3f -> 0x43
		mmio_wr32(base_addr_ctrl + 0x580, 0x343021f);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG0
		mmio_wr32(base_addr_ctrl + 0x100580, 0x343021f);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFITMG0
		mmio_wr32(base_addr_ctrl + 0x200580, 0x343021f);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFITMG0
		mmio_wr32(base_addr_ctrl + 0x300580, 0x343021f);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG0

		//DFITMG2.dfi_tphy_rdcslat([14:8])  = 0x37 -> 3f
		mmio_wr32(base_addr_ctrl + 0x588, 0x183f1f);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG2
		mmio_wr32(base_addr_ctrl + 0x100588, 0x183f1f);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFITMG2
		mmio_wr32(base_addr_ctrl + 0x200588, 0x183f1f);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFITMG2
		mmio_wr32(base_addr_ctrl + 0x300588, 0x183f1f);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG2

		//DFITMG4.dfi_twck_en_rd([31:24]) = 0x18
		mmio_wr32(base_addr_ctrl + 0x590, 0x180c0411);// DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG4
		mmio_wr32(base_addr_ctrl + 0x100590, 0x180c0411);// DWC_ddrctl_map_REGB_FREQ1_CH0.DFITMG4
		mmio_wr32(base_addr_ctrl + 0x200590, 0x180c0411);// DWC_ddrctl_map_REGB_FREQ2_CH0.DFITMG4
		mmio_wr32(base_addr_ctrl + 0x300590, 0x180c0411);// DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG4

		//RANKTMG1.rd2wr_dr([15:8])= 0x17
		mmio_wr32(base_addr_ctrl + 0xd08, 0x1702);// DWC_ddrctl_map_REGB_FREQ0_CH0.RANKTMG1
		mmio_wr32(base_addr_ctrl + 0x100d08, 0x1702);// DWC_ddrctl_map_REGB_FREQ1_CH0.RANKTMG1
		mmio_wr32(base_addr_ctrl + 0x200d08, 0x1702);// DWC_ddrctl_map_REGB_FREQ2_CH0.RANKTMG1
		mmio_wr32(base_addr_ctrl + 0x300d08, 0x1702);// DWC_ddrctl_map_REGB_FREQ3_CH0.RANKTMG1
	}
	if (p_lpddr_attr->dm_en == true) {
		rddata = mmio_rd32(base_addr_ctrl + 0x10c94);// DWC_ddrctl_map_REGB_DDRC_CH0.DBICTL
		//rddata = {rddata[31:1], 1'b1};
		rddata = modified_bits_by_value(rddata, 1, 0, 0);
		mmio_wr32(base_addr_ctrl + 0x10c94, rddata);
	}

	#ifdef SG_MDF
		rddata = mmio_rd32(base_addr_ctrl + 0x40000);// mdf_sel
		//rddata = {rddata[31:1], 1'b1};
		rddata = modified_bits_by_value(rddata, 1, 0, 0);
		mmio_wr32(base_addr_ctrl + 0x40000, rddata);
		#ifdef SG_MDF_ECC
			rddata = mmio_rd32(base_addr_ctrl + 0x40000);// mdf_sel
			//rddata = {rddata[31:6], 1'b1, rddata[4:0]};
			rddata = modified_bits_by_value(rddata, 1, 5, 5);
			mmio_wr32(base_addr_ctrl + 0x40000, rddata);
		#endif
		#ifdef SG_MDF_RD_CAM
			rddata = mmio_rd32(base_addr_ctrl + 0x40000);// mdf_sel
			//rddata = {rddata[31:7], 1'b1, rddata[5:0]};
			rddata = modified_bits_by_value(rddata, 1, 6, 6);
			mmio_wr32(base_addr_ctrl + 0x40000, rddata);
		#endif
	#endif
}