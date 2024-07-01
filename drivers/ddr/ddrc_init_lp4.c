#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <lib/mmio.h>
#include <driver/ddr/bitwise_ops.h>
#include <driver/ddr/ddr.h>

void ddrc_init_lp4(uint64_t base_addr_ctrl, lpddr_attr *p_lpddr_attr)
{
	uint32_t rddata;

	//printf("----[%s]----\n", __func__);

	rddata = mmio_rd32(base_addr_ctrl + 0x10ff8); // DWC_ddrctl_map_REGB_DDRC_CH0.DDRCTL_VER_NUMBER
	rddata = mmio_rd32(base_addr_ctrl + 0x10ffc); // DWC_ddrctl_map_REGB_DDRC_CH0.DDRCTL_VER_TYPE
	mmio_wr32(base_addr_ctrl + 0x10b84, 0x1); // DWC_ddrctl_map_REGB_DDRC_CH0.OPCTRL1
#ifdef HALF_DATA_WIDTH
	if (p_lpddr_attr->num_rank == 2)
		mmio_wr32(base_addr_ctrl + 0x10000, 0x3081002); // DWC_ddrctl_map_REGB_DDRC_CH0.MSTR0
	else
		mmio_wr32(base_addr_ctrl + 0x10000, 0x1081002); // DWC_ddrctl_map_REGB_DDRC_CH0.MSTR0

#else
	if (p_lpddr_attr->num_rank == 2)
		mmio_wr32(base_addr_ctrl + 0x10000, 0x3080002); // DWC_ddrctl_map_REGB_DDRC_CH0.MSTR0
	else
		mmio_wr32(base_addr_ctrl + 0x10000, 0x1080002); // DWC_ddrctl_map_REGB_DDRC_CH0.MSTR0
#endif

	mmio_wr32(base_addr_ctrl + 0x10010, 0x0); // DWC_ddrctl_map_REGB_DDRC_CH0.MSTR4
	mmio_wr32(base_addr_ctrl + 0x10104, 0x1); // DWC_ddrctl_map_REGB_DDRC_CH0.DERATECTL1
	mmio_wr32(base_addr_ctrl + 0x10108, 0x1); // DWC_ddrctl_map_REGB_DDRC_CH0.DERATECTL2
	mmio_wr32(base_addr_ctrl + 0x10118, 0x1); // DWC_ddrctl_map_REGB_DDRC_CH0.DERATECTL6
	mmio_wr32(base_addr_ctrl + 0x10184, 0x2); // DWC_ddrctl_map_REGB_DDRC_CH0.HWLPCTL
	mmio_wr32(base_addr_ctrl + 0x1018c, 0x0); // DWC_ddrctl_map_REGB_DDRC_CH0.CLKGATECTL
	//mmio_wr32(base_addr_ctrl + 0x10208, 0x1); // DWC_ddrctl_map_REGB_DDRC_CH0.RFSHCTL0
	//if($test$plusargs("DISABLE_AUTO_REFRESH"))begin
	#ifdef DISABLE_AUTO_REFRESH
	mmio_wr32(base_addr_ctrl + 0x10208, 0x1); // DWC_ddrctl_map_REGB_DDRC_CH0.RFSHCTL0
	rddata = mmio_rd32(base_addr_ctrl + 0x10200);
	//rddata = {rddata[31:9], 1'b0, rddata[7:0] };
	rddata = modified_bits_by_value(rddata, 0, 8, 8);
	mmio_wr32(base_addr_ctrl + 0x10200, rddata);
	#else
	// DWC_ddrctl_map_REGB_DDRC_CH0.RFSHCTL0  rfshctl0.dis_auto_refresh = 0x0
	mmio_wr32(base_addr_ctrl + 0x10208, 0x0);
	rddata = mmio_rd32(base_addr_ctrl + 0x10200);
	//rddata = {rddata[31:9], 1'b0, rddata[7:0] };   // rfshmod0.per_bank_refresh = 0x0;
	rddata = modified_bits_by_value(rddata, 0, 8, 8);
	mmio_wr32(base_addr_ctrl + 0x10200, rddata);
	#endif

	mmio_wr32(base_addr_ctrl + 0x10220, 0x5001f00); // DWC_ddrctl_map_REGB_DDRC_CH0.RFMMOD0
	mmio_wr32(base_addr_ctrl + 0x10224, 0x11e); // DWC_ddrctl_map_REGB_DDRC_CH0.RFMMOD1
	mmio_wr32(base_addr_ctrl + 0x10280, 0x80000000); // DWC_ddrctl_map_REGB_DDRC_CH0.ZQCTL0
	mmio_wr32(base_addr_ctrl + 0x10288, 0x1); // DWC_ddrctl_map_REGB_DDRC_CH0.ZQCTL2
	mmio_wr32(base_addr_ctrl + 0x10300, 0x200020); // DWC_ddrctl_map_REGB_DDRC_CH0.DQSOSCRUNTIME
	mmio_wr32(base_addr_ctrl + 0x10308, 0x1); // DWC_ddrctl_map_REGB_DDRC_CH0.DQSOSCCFG0
	mmio_wr32(base_addr_ctrl + 0x10380, 0xa8013f14); // DWC_ddrctl_map_REGB_DDRC_CH0.SCHED0
  
	rddata = mmio_rd32(base_addr_ctrl + 0x10384);
	rddata = modified_bits_by_value(rddata, 0b001, 22, 20);
	mmio_wr32(base_addr_ctrl + 0x10384, rddata);// DWC_ddrctl_map_REGB_DDRC_CH0.SCHED1
	rddata = mmio_rd32(base_addr_ctrl + 0x1038c);
	rddata = modified_bits_by_value(rddata, 0x0000, 31, 16);
	mmio_wr32(base_addr_ctrl + 0x1038c, rddata);// DWC_ddrctl_map_REGB_DDRC_CH0.SCHED3

	mmio_wr32(base_addr_ctrl + 0x10394, 0x10000104); // DWC_ddrctl_map_REGB_DDRC_CH0.SCHED5
	mmio_wr32(base_addr_ctrl + 0x10500, 0x110111); // DWC_ddrctl_map_REGB_DDRC_CH0.DFILPCFG0
	mmio_wr32(base_addr_ctrl + 0x10508, 0xa0000000); // DWC_ddrctl_map_REGB_DDRC_CH0.DFIUPD0
	mmio_wr32(base_addr_ctrl + 0x10510, 0x5); // DWC_ddrctl_map_REGB_DDRC_CH0.DFIMISC
	mmio_wr32(base_addr_ctrl + 0x10518, 0xde000000); // DWC_ddrctl_map_REGB_DDRC_CH0.DFIPHYMSTR
	mmio_wr32(base_addr_ctrl + 0x10600, 0x23f7f00); // DWC_ddrctl_map_REGB_DDRC_CH0.ECCCFG0
	mmio_wr32(base_addr_ctrl + 0x10604, 0x7b2); // DWC_ddrctl_map_REGB_DDRC_CH0.ECCCFG1
	mmio_wr32(base_addr_ctrl + 0x10648, 0x1000278); // DWC_ddrctl_map_REGB_DDRC_CH0.ECCPOISONADDR0
	mmio_wr32(base_addr_ctrl + 0x1064c, 0x402209a); // DWC_ddrctl_map_REGB_DDRC_CH0.ECCPOISONADDR1
	mmio_wr32(base_addr_ctrl + 0x10658, 0xab2eb8fb); // DWC_ddrctl_map_REGB_DDRC_CH0.ECCPOISONPAT0
	mmio_wr32(base_addr_ctrl + 0x10660, 0x6a); // DWC_ddrctl_map_REGB_DDRC_CH0.ECCPOISONPAT2
	mmio_wr32(base_addr_ctrl + 0x10984, 0x77); // DWC_ddrctl_map_REGB_DDRC_CH0.LNKECCCTL1

	//mmio_wr32(base_addr_ctrl + 0x10d00, 0x40020002); // DWC_ddrctl_map_REGB_DDRC_CH0.INITTMG0
	mmio_wr32(base_addr_ctrl + 0x10d00, 0x00020002); // DWC_ddrctl_map_REGB_DDRC_CH0.INITTMG0

	mmio_wr32(base_addr_ctrl + 0x10f00, 0x800060c0); // DWC_ddrctl_map_REGB_DDRC_CH0.PPT2CTRL0
	//mmio_wr32(base_addr_ctrl + 0x0, 0x6440925a); // DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x0, 0x6440495a); // DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x4, 0x5101080); // DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG1
	mmio_wr32(base_addr_ctrl + 0x8, 0x12243031); // DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG2
	mmio_wr32(base_addr_ctrl + 0xc, 0x1e4d76); // DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG3
	mmio_wr32(base_addr_ctrl + 0x10, 0x27081027); // DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG4
	mmio_wr32(base_addr_ctrl + 0x14, 0x40b2020); // DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG5
	mmio_wr32(base_addr_ctrl + 0x18, 0x12); // DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG6
	mmio_wr32(base_addr_ctrl + 0x24, 0x40000); // DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG9
	mmio_wr32(base_addr_ctrl + 0x30, 0x40000); // DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG12
	mmio_wr32(base_addr_ctrl + 0x38, 0x560126); // DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG14
	mmio_wr32(base_addr_ctrl + 0x60, 0x0); // DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG24
	mmio_wr32(base_addr_ctrl + 0x64, 0x26610); // DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG25
	mmio_wr32(base_addr_ctrl + 0x78, 0x373e08); // DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG30
	mmio_wr32(base_addr_ctrl + 0x80, 0x0); // DWC_ddrctl_map_REGB_FREQ0_CH0.DRAMSET1TMG32
	mmio_wr32(base_addr_ctrl + 0x500, 0xfc003f); // DWC_ddrctl_map_REGB_FREQ0_CH0.INITMR0
	mmio_wr32(base_addr_ctrl + 0x504, 0x20000); // DWC_ddrctl_map_REGB_FREQ0_CH0.INITMR1
	mmio_wr32(base_addr_ctrl + 0x508, 0x53000e); // DWC_ddrctl_map_REGB_FREQ0_CH0.INITMR2
	mmio_wr32(base_addr_ctrl + 0x50c, 0x3); // DWC_ddrctl_map_REGB_FREQ0_CH0.INITMR3
	mmio_wr32(base_addr_ctrl + 0x580, 0x31f020e); // DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG0
	mmio_wr32(base_addr_ctrl + 0x584, 0xb0303); // DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG1
	mmio_wr32(base_addr_ctrl + 0x588, 0x1f0e); // DWC_ddrctl_map_REGB_FREQ0_CH0.DFITMG2
	mmio_wr32(base_addr_ctrl + 0x5a0, 0x20202); // DWC_ddrctl_map_REGB_FREQ0_CH0.DFILPTMG0
	mmio_wr32(base_addr_ctrl + 0x5a4, 0x201); // DWC_ddrctl_map_REGB_FREQ0_CH0.DFILPTMG1
	mmio_wr32(base_addr_ctrl + 0x5a8, 0x2160003); // DWC_ddrctl_map_REGB_FREQ0_CH0.DFIUPDTMG0
	mmio_wr32(base_addr_ctrl + 0x5ac, 0x500a2); // DWC_ddrctl_map_REGB_FREQ0_CH0.DFIUPDTMG1
	mmio_wr32(base_addr_ctrl + 0x5b0, 0x1e); // DWC_ddrctl_map_REGB_FREQ0_CH0.DFIMSGTMG0
	mmio_wr32(base_addr_ctrl + 0x5b4, 0x40000006); // DWC_ddrctl_map_REGB_FREQ0_CH0.DFIUPDTMG2
	mmio_wr32(base_addr_ctrl + 0x5b8, 0x1ce); // DWC_ddrctl_map_REGB_FREQ0_CH0.DFIUPDTMG3
	mmio_wr32(base_addr_ctrl + 0x600, 0xc2102084); // DWC_ddrctl_map_REGB_FREQ0_CH0.RFSHSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x604, 0x32b032b); // DWC_ddrctl_map_REGB_FREQ0_CH0.RFSHSET1TMG1
	mmio_wr32(base_addr_ctrl + 0x608, 0x800000); // DWC_ddrctl_map_REGB_FREQ0_CH0.RFSHSET1TMG2
	mmio_wr32(base_addr_ctrl + 0x60c, 0x1000000); // DWC_ddrctl_map_REGB_FREQ0_CH0.RFSHSET1TMG3
	mmio_wr32(base_addr_ctrl + 0x650, 0x16b); // DWC_ddrctl_map_REGB_FREQ0_CH0.RFMSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x800, 0x400855); // DWC_ddrctl_map_REGB_FREQ0_CH0.ZQSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x804, 0x6b00059); // DWC_ddrctl_map_REGB_FREQ0_CH0.ZQSET1TMG1
	mmio_wr32(base_addr_ctrl + 0xa80, 0x3fb0); // DWC_ddrctl_map_REGB_FREQ0_CH0.DQSOSCCTL0
	mmio_wr32(base_addr_ctrl + 0xb00, 0x41bfb427); // DWC_ddrctl_map_REGB_FREQ0_CH0.DERATEINT
	mmio_wr32(base_addr_ctrl + 0xb04, 0x2b5e2b14); // DWC_ddrctl_map_REGB_FREQ0_CH0.DERATEVAL0
	mmio_wr32(base_addr_ctrl + 0xb08, 0x588); // DWC_ddrctl_map_REGB_FREQ0_CH0.DERATEVAL1
	mmio_wr32(base_addr_ctrl + 0xb80, 0xe380000); // DWC_ddrctl_map_REGB_FREQ0_CH0.HWLPTMG0
	mmio_wr32(base_addr_ctrl + 0xd00, 0x1); // DWC_ddrctl_map_REGB_FREQ0_CH0.TMGCFG
	mmio_wr32(base_addr_ctrl + 0xd04, 0xd06); // DWC_ddrctl_map_REGB_FREQ0_CH0.RANKTMG0
	mmio_wr32(base_addr_ctrl + 0xd08, 0x3007); // DWC_ddrctl_map_REGB_FREQ0_CH0.RANKTMG1
	mmio_wr32(base_addr_ctrl + 0xd0c, 0x400005); // DWC_ddrctl_map_REGB_FREQ0_CH0.PWRTMG
	//mmio_wr32(base_addr_ctrl + 0x100000, 0x6440925a); // DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x100000, 0x6440495a); // DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x100004, 0x5101080); // DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG1
	mmio_wr32(base_addr_ctrl + 0x100008, 0x12243031); // DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG2
	mmio_wr32(base_addr_ctrl + 0x10000c, 0x1e4d76); // DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG3
	mmio_wr32(base_addr_ctrl + 0x100010, 0x27081027); // DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG4
	mmio_wr32(base_addr_ctrl + 0x100014, 0x40b2020); // DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG5
	mmio_wr32(base_addr_ctrl + 0x100018, 0x12); // DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG6
	mmio_wr32(base_addr_ctrl + 0x100024, 0x40000); // DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG9
	mmio_wr32(base_addr_ctrl + 0x100030, 0x40000); // DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG12
	mmio_wr32(base_addr_ctrl + 0x100038, 0x560126); // DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG14
	mmio_wr32(base_addr_ctrl + 0x100060, 0x0); // DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG24
	mmio_wr32(base_addr_ctrl + 0x100064, 0x26610); // DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG25
	mmio_wr32(base_addr_ctrl + 0x100078, 0x373e08); // DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG30
	mmio_wr32(base_addr_ctrl + 0x100080, 0x0); // DWC_ddrctl_map_REGB_FREQ1_CH0.DRAMSET1TMG32
	mmio_wr32(base_addr_ctrl + 0x100500, 0xfc003f); // DWC_ddrctl_map_REGB_FREQ1_CH0.INITMR0
	mmio_wr32(base_addr_ctrl + 0x100504, 0x20000); // DWC_ddrctl_map_REGB_FREQ1_CH0.INITMR1
	mmio_wr32(base_addr_ctrl + 0x100508, 0x30012); // DWC_ddrctl_map_REGB_FREQ1_CH0.INITMR2
	mmio_wr32(base_addr_ctrl + 0x10050c, 0x7); // DWC_ddrctl_map_REGB_FREQ1_CH0.INITMR3
	mmio_wr32(base_addr_ctrl + 0x100580, 0x31f020e); // DWC_ddrctl_map_REGB_FREQ1_CH0.DFITMG0
	mmio_wr32(base_addr_ctrl + 0x100584, 0xb0303); // DWC_ddrctl_map_REGB_FREQ1_CH0.DFITMG1
	mmio_wr32(base_addr_ctrl + 0x100588, 0x1f0e); // DWC_ddrctl_map_REGB_FREQ1_CH0.DFITMG2
	mmio_wr32(base_addr_ctrl + 0x1005ac, 0x3c00ae); // DWC_ddrctl_map_REGB_FREQ1_CH0.DFIUPDTMG1
	mmio_wr32(base_addr_ctrl + 0x1005b4, 0x6000000f); // DWC_ddrctl_map_REGB_FREQ1_CH0.DFIUPDTMG2
	mmio_wr32(base_addr_ctrl + 0x1005b8, 0x1ce); // DWC_ddrctl_map_REGB_FREQ1_CH0.DFIUPDTMG3
	mmio_wr32(base_addr_ctrl + 0x100600, 0xc2102084); // DWC_ddrctl_map_REGB_FREQ1_CH0.RFSHSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x100604, 0x1160116); // DWC_ddrctl_map_REGB_FREQ1_CH0.RFSHSET1TMG1
	mmio_wr32(base_addr_ctrl + 0x100608, 0x800000); // DWC_ddrctl_map_REGB_FREQ1_CH0.RFSHSET1TMG2
	mmio_wr32(base_addr_ctrl + 0x10060c, 0x0); // DWC_ddrctl_map_REGB_FREQ1_CH0.RFSHSET1TMG3
	mmio_wr32(base_addr_ctrl + 0x100650, 0x16b); // DWC_ddrctl_map_REGB_FREQ1_CH0.RFMSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x100800, 0x400855); // DWC_ddrctl_map_REGB_FREQ1_CH0.ZQSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x100804, 0x6b00059); // DWC_ddrctl_map_REGB_FREQ1_CH0.ZQSET1TMG1
	mmio_wr32(base_addr_ctrl + 0x100a80, 0x3e70); // DWC_ddrctl_map_REGB_FREQ1_CH0.DQSOSCCTL0
	mmio_wr32(base_addr_ctrl + 0x100b00, 0xe2a51c31); // DWC_ddrctl_map_REGB_FREQ1_CH0.DERATEINT
	mmio_wr32(base_addr_ctrl + 0x100b04, 0x2b5e2b14); // DWC_ddrctl_map_REGB_FREQ1_CH0.DERATEVAL0
	mmio_wr32(base_addr_ctrl + 0x100b08, 0x588); // DWC_ddrctl_map_REGB_FREQ1_CH0.DERATEVAL1
	mmio_wr32(base_addr_ctrl + 0x100b80, 0xdd70000); // DWC_ddrctl_map_REGB_FREQ1_CH0.HWLPTMG0
	mmio_wr32(base_addr_ctrl + 0x100d04, 0xd06); // DWC_ddrctl_map_REGB_FREQ1_CH0.RANKTMG0
	mmio_wr32(base_addr_ctrl + 0x100d08, 0x3007); // DWC_ddrctl_map_REGB_FREQ1_CH0.RANKTMG1
	mmio_wr32(base_addr_ctrl + 0x100d0c, 0x400005); // DWC_ddrctl_map_REGB_FREQ1_CH0.PWRTMG
	//mmio_wr32(base_addr_ctrl + 0x200000, 0x6440925a); // DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x200000, 0x6440495a); // DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x200004, 0x5101080); // DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG1
	mmio_wr32(base_addr_ctrl + 0x200008, 0x12243031); // DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG2
	mmio_wr32(base_addr_ctrl + 0x20000c, 0x1e4d76); // DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG3
	mmio_wr32(base_addr_ctrl + 0x200010, 0x27081027); // DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG4
	mmio_wr32(base_addr_ctrl + 0x200014, 0x40b2020); // DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG5
	mmio_wr32(base_addr_ctrl + 0x200018, 0x12); // DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG6
	mmio_wr32(base_addr_ctrl + 0x200024, 0x40000); // DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG9
	mmio_wr32(base_addr_ctrl + 0x200030, 0x40000); // DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG12
	mmio_wr32(base_addr_ctrl + 0x200038, 0x560126); // DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG14
	mmio_wr32(base_addr_ctrl + 0x200060, 0x0); // DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG24
	mmio_wr32(base_addr_ctrl + 0x200064, 0x26610); // DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG25
	mmio_wr32(base_addr_ctrl + 0x200078, 0x373e08); // DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG30
	mmio_wr32(base_addr_ctrl + 0x200080, 0x0); // DWC_ddrctl_map_REGB_FREQ2_CH0.DRAMSET1TMG32
	mmio_wr32(base_addr_ctrl + 0x200500, 0xfc003f); // DWC_ddrctl_map_REGB_FREQ2_CH0.INITMR0
	mmio_wr32(base_addr_ctrl + 0x200504, 0x20000); // DWC_ddrctl_map_REGB_FREQ2_CH0.INITMR1
	mmio_wr32(base_addr_ctrl + 0x200508, 0x660018); // DWC_ddrctl_map_REGB_FREQ2_CH0.INITMR2
	mmio_wr32(base_addr_ctrl + 0x20050c, 0x1b); // DWC_ddrctl_map_REGB_FREQ2_CH0.INITMR3
	mmio_wr32(base_addr_ctrl + 0x200580, 0x31f020e); // DWC_ddrctl_map_REGB_FREQ2_CH0.DFITMG0
	mmio_wr32(base_addr_ctrl + 0x200584, 0xb0303); // DWC_ddrctl_map_REGB_FREQ2_CH0.DFITMG1
	mmio_wr32(base_addr_ctrl + 0x200588, 0x1f0e); // DWC_ddrctl_map_REGB_FREQ2_CH0.DFITMG2
	mmio_wr32(base_addr_ctrl + 0x2005ac, 0xb000f); // DWC_ddrctl_map_REGB_FREQ2_CH0.DFIUPDTMG1
	mmio_wr32(base_addr_ctrl + 0x2005b4, 0xa0000002); // DWC_ddrctl_map_REGB_FREQ2_CH0.DFIUPDTMG2
	mmio_wr32(base_addr_ctrl + 0x2005b8, 0x1ce); // DWC_ddrctl_map_REGB_FREQ2_CH0.DFIUPDTMG3
	mmio_wr32(base_addr_ctrl + 0x200600, 0xc2102084); // DWC_ddrctl_map_REGB_FREQ2_CH0.RFSHSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x200604, 0x1160116); // DWC_ddrctl_map_REGB_FREQ2_CH0.RFSHSET1TMG1
	mmio_wr32(base_addr_ctrl + 0x200608, 0x800000); // DWC_ddrctl_map_REGB_FREQ2_CH0.RFSHSET1TMG2
	mmio_wr32(base_addr_ctrl + 0x20060c, 0x19000000); // DWC_ddrctl_map_REGB_FREQ2_CH0.RFSHSET1TMG3
	mmio_wr32(base_addr_ctrl + 0x200650, 0x16b); // DWC_ddrctl_map_REGB_FREQ2_CH0.RFMSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x200800, 0x400855); // DWC_ddrctl_map_REGB_FREQ2_CH0.ZQSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x200804, 0x6b00059); // DWC_ddrctl_map_REGB_FREQ2_CH0.ZQSET1TMG1
	mmio_wr32(base_addr_ctrl + 0x200a80, 0x25f4); // DWC_ddrctl_map_REGB_FREQ2_CH0.DQSOSCCTL0
	mmio_wr32(base_addr_ctrl + 0x200b00, 0x49eacbcc); // DWC_ddrctl_map_REGB_FREQ2_CH0.DERATEINT
	mmio_wr32(base_addr_ctrl + 0x200b04, 0x2b5e2b14); // DWC_ddrctl_map_REGB_FREQ2_CH0.DERATEVAL0
	mmio_wr32(base_addr_ctrl + 0x200b08, 0x588); // DWC_ddrctl_map_REGB_FREQ2_CH0.DERATEVAL1
	mmio_wr32(base_addr_ctrl + 0x200b80, 0xb420000); // DWC_ddrctl_map_REGB_FREQ2_CH0.HWLPTMG0
	mmio_wr32(base_addr_ctrl + 0x200d04, 0xd06); // DWC_ddrctl_map_REGB_FREQ2_CH0.RANKTMG0
	mmio_wr32(base_addr_ctrl + 0x200d08, 0x3007); // DWC_ddrctl_map_REGB_FREQ2_CH0.RANKTMG1
	mmio_wr32(base_addr_ctrl + 0x200d0c, 0x400005); // DWC_ddrctl_map_REGB_FREQ2_CH0.PWRTMG
	mmio_wr32(base_addr_ctrl + 0x300000, 0x6440925a); // DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x300004, 0x5101080); // DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG1
	mmio_wr32(base_addr_ctrl + 0x300008, 0x12243031); // DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG2
	mmio_wr32(base_addr_ctrl + 0x30000c, 0x1e4d76); // DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG3
	mmio_wr32(base_addr_ctrl + 0x300010, 0x27081027); // DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG4
	mmio_wr32(base_addr_ctrl + 0x300014, 0x40b2020); // DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG5
	mmio_wr32(base_addr_ctrl + 0x300018, 0x12); // DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG6
	mmio_wr32(base_addr_ctrl + 0x300024, 0x40000); // DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG9
	mmio_wr32(base_addr_ctrl + 0x300030, 0x40000); // DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG12
	mmio_wr32(base_addr_ctrl + 0x300038, 0x560126); // DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG14
	mmio_wr32(base_addr_ctrl + 0x300060, 0x0); // DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG24
	mmio_wr32(base_addr_ctrl + 0x300064, 0x26610); // DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG25
	mmio_wr32(base_addr_ctrl + 0x300078, 0x373e08); // DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG30
	mmio_wr32(base_addr_ctrl + 0x300080, 0x0); // DWC_ddrctl_map_REGB_FREQ3_CH0.DRAMSET1TMG32
	mmio_wr32(base_addr_ctrl + 0x300500, 0xfc003f); // DWC_ddrctl_map_REGB_FREQ3_CH0.INITMR0
	mmio_wr32(base_addr_ctrl + 0x300504, 0x20000); // DWC_ddrctl_map_REGB_FREQ3_CH0.INITMR1
	mmio_wr32(base_addr_ctrl + 0x300508, 0x52001f); // DWC_ddrctl_map_REGB_FREQ3_CH0.INITMR2
	mmio_wr32(base_addr_ctrl + 0x300580, 0x31f020e); // DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG0
	mmio_wr32(base_addr_ctrl + 0x300584, 0xb0303); // DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG1
	mmio_wr32(base_addr_ctrl + 0x300588, 0x1f0e); // DWC_ddrctl_map_REGB_FREQ3_CH0.DFITMG2
	mmio_wr32(base_addr_ctrl + 0x3005ac, 0x8100b2); // DWC_ddrctl_map_REGB_FREQ3_CH0.DFIUPDTMG1
	mmio_wr32(base_addr_ctrl + 0x3005b4, 0x6000000a); // DWC_ddrctl_map_REGB_FREQ3_CH0.DFIUPDTMG2
	mmio_wr32(base_addr_ctrl + 0x3005b8, 0x1ce); // DWC_ddrctl_map_REGB_FREQ3_CH0.DFIUPDTMG3
	mmio_wr32(base_addr_ctrl + 0x300600, 0xc2102084); // DWC_ddrctl_map_REGB_FREQ3_CH0.RFSHSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x300604, 0x1160116); // DWC_ddrctl_map_REGB_FREQ3_CH0.RFSHSET1TMG1
	mmio_wr32(base_addr_ctrl + 0x300608, 0x800000); // DWC_ddrctl_map_REGB_FREQ3_CH0.RFSHSET1TMG2
	mmio_wr32(base_addr_ctrl + 0x30060c, 0x4000000); // DWC_ddrctl_map_REGB_FREQ3_CH0.RFSHSET1TMG3
	mmio_wr32(base_addr_ctrl + 0x300650, 0x16b); // DWC_ddrctl_map_REGB_FREQ3_CH0.RFMSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x300800, 0x400855); // DWC_ddrctl_map_REGB_FREQ3_CH0.ZQSET1TMG0
	mmio_wr32(base_addr_ctrl + 0x300804, 0x6b00059); // DWC_ddrctl_map_REGB_FREQ3_CH0.ZQSET1TMG1
	mmio_wr32(base_addr_ctrl + 0x300a80, 0x2bf0); // DWC_ddrctl_map_REGB_FREQ3_CH0.DQSOSCCTL0
	mmio_wr32(base_addr_ctrl + 0x300b00, 0x4572ff73); // DWC_ddrctl_map_REGB_FREQ3_CH0.DERATEINT
	mmio_wr32(base_addr_ctrl + 0x300b04, 0x2b5e2b14); // DWC_ddrctl_map_REGB_FREQ3_CH0.DERATEVAL0
	mmio_wr32(base_addr_ctrl + 0x300b08, 0x588); // DWC_ddrctl_map_REGB_FREQ3_CH0.DERATEVAL1
	mmio_wr32(base_addr_ctrl + 0x300b80, 0x5330000); // DWC_ddrctl_map_REGB_FREQ3_CH0.HWLPTMG0
	mmio_wr32(base_addr_ctrl + 0x300d04, 0xd06); // DWC_ddrctl_map_REGB_FREQ3_CH0.RANKTMG0
	mmio_wr32(base_addr_ctrl + 0x300d08, 0x3007); // DWC_ddrctl_map_REGB_FREQ3_CH0.RANKTMG1
	mmio_wr32(base_addr_ctrl + 0x300d0c, 0x400005); // DWC_ddrctl_map_REGB_FREQ3_CH0.PWRTMG

	if (p_lpddr_attr->num_rank == 2)
		mmio_wr32(base_addr_ctrl + 0x30004, 0x18); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP1
	else
		mmio_wr32(base_addr_ctrl + 0x30004, 0x1f); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP1

	mmio_wr32(base_addr_ctrl + 0x3000c, 0x070707); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP3
	mmio_wr32(base_addr_ctrl + 0x30010, 0x3f3f); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP4
	mmio_wr32(base_addr_ctrl + 0x30014, 0x1f000000); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP5
	mmio_wr32(base_addr_ctrl + 0x30018, 0x00000000); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP6
	mmio_wr32(base_addr_ctrl + 0x3001c, 0x1f070707); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP7
	mmio_wr32(base_addr_ctrl + 0x30020, 0x07070707); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP8
	mmio_wr32(base_addr_ctrl + 0x30024, 0x07070707); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP9
	mmio_wr32(base_addr_ctrl + 0x30028, 0x07070707); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP10
	mmio_wr32(base_addr_ctrl + 0x3002c, 0x0707); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP11

	//`ifdef DUAL_RANK
	//mmio_wr32(base_addr_ctrl + 0x30004, 0x13); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP1
	//`else
	//mmio_wr32(base_addr_ctrl + 0x30004, 0x3f); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP1
	//`endif
	//mmio_wr32(base_addr_ctrl + 0x3000c, 0x160101); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP3
	//mmio_wr32(base_addr_ctrl + 0x30010, 0x3f3f); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP4
	//mmio_wr32(base_addr_ctrl + 0x30014, 0x1f000307); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP5
	//mmio_wr32(base_addr_ctrl + 0x30018, 0x7030300); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP6
	//mmio_wr32(base_addr_ctrl + 0x3001c, 0x1f1f1f1f); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP7
	//mmio_wr32(base_addr_ctrl + 0x30020, 0x1000008); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP8
	//mmio_wr32(base_addr_ctrl + 0x30024, 0x40c030b); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP9
	//mmio_wr32(base_addr_ctrl + 0x30028, 0x4020d0d); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP10
	//mmio_wr32(base_addr_ctrl + 0x3002c, 0x300); // DWC_ddrctl_map_REGB_ADDR_MAP0.ADDRMAP11


	mmio_wr32(base_addr_ctrl + 0x10b84, 0x0); // DWC_ddrctl_map_REGB_DDRC_CH0.OPCTRL1
	mmio_wr32(base_addr_ctrl + 0x20090, 0x1); // DWC_ddrctl_map_REGB_ARB_PORT0.PCTRL
	mmio_wr32(base_addr_ctrl + 0x21090, 0x1); // DWC_ddrctl_map_REGB_ARB_PORT1.PCTRL
	mmio_wr32(base_addr_ctrl + 0x22090, 0x1); // DWC_ddrctl_map_REGB_ARB_PORT2.PCTRL
	//mmio_wr32(base_addr_ctrl + 0x10208, 0x1); // DWC_ddrctl_map_REGB_DDRC_CH0.RFSHCTL0
	rddata = mmio_rd32(base_addr_ctrl + 0x20004); // PCFGGR
	//rddata = {rddata[31:24], 0x3, rddata[19:0] };
	rddata = modified_bits_by_value(rddata, 0x3, 23, 20);
	mmio_wr32(base_addr_ctrl + 0x20004, rddata);

	#ifdef SG_MDF
	rddata = mmio_rd32(base_addr_ctrl + 0x40000); // mdf_sel
	//rddata = {rddata[31:1], 1'b1};
	rddata = modified_bits_by_value(rddata, 1, 0, 0);
	mmio_wr32(base_addr_ctrl + 0x40000, rddata);
	#ifdef SG_MDF_ECC
	rddata = mmio_rd32(base_addr_ctrl + 0x40000); // mdf_sel
	//rddata = {rddata[31:6], 1'b1, rddata[4:0]};
	rddata = modified_bits_by_value(rddata, 1, 5, 5);
	mmio_wr32(base_addr_ctrl + 0x40000, rddata);
	#endif
	#ifdef SG_MDF_RD_CAM
	rddata = mmio_rd32(base_addr_ctrl + 0x40000); // mdf_sel
	//rddata = {rddata[31:7], 1'b1, rddata[5:0]};
	rddata = modified_bits_by_value(rddata, 1, 6, 6);
	mmio_wr32(base_addr_ctrl + 0x40000, rddata);
	#endif
	#endif
}
