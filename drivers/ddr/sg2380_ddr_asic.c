#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <lib/mmio.h>
#include <timer.h>
#include <framework/common.h>
#include <platform.h>
#include <driver/ddr/bitwise_ops.h>
#include <driver/ddr/ddr.h>

lpddr_attr g_lpddr_attr;

void top_itlv_reg_config(void)
{
	//0x00
	//2~0:itlv_size			0x3
	//4~3:itlv_ratio		0x0
	//5:bypass_ch			0x0
	//6:bypass				0x0

	//0x04
	//14~0:addr_itlv		0x7fff
	//30~15:addr_ddr1		0xffff

	//printf("----[%s]----\n", __func__);

	//Left nodeA itlv setting
	mmio_wr32(0x5082520000, 0x00000003);
	mmio_wr32(0x5082520004, 0x7fffffff);
	//printf("----[%s]----1\n", __func__);
	//Left nodeB itlv setting
	mmio_wr32(0x5082520008, 0x00000003);
	mmio_wr32(0x508252000c, 0x7fffffff);
	//printf("----[%s]----2\n", __func__);
	//Left nodeC itlv setting
	mmio_wr32(0x5082520010, 0x00000003);
	mmio_wr32(0x5082520014, 0x7fffffff);
	//printf("----[%s]----3\n", __func__);
	//Left nodeD itlv setting
	mmio_wr32(0x5082520018, 0x00000003);
	mmio_wr32(0x508252001c, 0x7fffffff);
	//printf("----[%s]----4\n", __func__);
	//Right nodeA itlv setting
	mmio_wr32(0x5082520020, 0x00000003);
	mmio_wr32(0x5082520024, 0x7fffffff);
	//printf("----[%s]----5\n", __func__);
	//Right nodeB itlv setting
	mmio_wr32(0x5082520028, 0x00000003);
	mmio_wr32(0x508252002c, 0x7fffffff);
	//printf("----[%s]----6\n", __func__);
	//Right nodeC itlv setting
	mmio_wr32(0x5082520030, 0x00000003);
	mmio_wr32(0x5082520034, 0x7fffffff);
	//printf("----[%s]----7\n", __func__);
	//Right nodeD itlv setting
	mmio_wr32(0x5082520038, 0x00000003);
	mmio_wr32(0x508252003c, 0x7fffffff);
}

void axi_itlv_reg_config(uint64_t base_addr_ddr_subsys_reg)
{
	uint32_t rddata;
	uint8_t  itlv_size;      //3bit
	uint16_t addr_itlv;      //11bit
	uint16_t addr_ddr1;		 //12bit
	uint8_t  itlv_bypass;    //1bit
	uint8_t  itlv_bypass_ch; //1bit

	#ifdef HALF_DATA_WIDTH
	#ifdef ONLY_ONE_DDRC_USED
	//itlv bypass to only one ddrc---x16
	itlv_size = 2;
	addr_itlv = 0x7ff;
	//itlv_m0
	rddata = mmio_rd32(base_addr_ddr_subsys_reg + 0xd0);
	itlv_bypass = 0b1;
	itlv_bypass_ch = 0b0;
	//rddata = {rddata[31:16], addr_itlv, itlv_size, itlv_bypass_ch, itlv_bypass};
	rddata = modified_bits_by_value(rddata, itlv_bypass, 0, 0);
	rddata = modified_bits_by_value(rddata, itlv_bypass_ch, 1, 1);
	rddata = modified_bits_by_value(rddata, itlv_size, 4, 2);
	rddata = modified_bits_by_value(rddata, addr_itlv, 15, 5);
	//rddata = {rddata[31:16], addr_itlv, itlv_size, rddata[1:0]};
	mmio_wr32(base_addr_ddr_subsys_reg + 0xd0, rddata);

	//1to2
	rddata = mmio_rd32(base_addr_ddr_subsys_reg + 0xd4);
	itlv_bypass = 0b1;
	itlv_bypass_ch = 0b1;
	//rddata = {rddata[31:2], itlv_bypass_ch, itlv_bypass};
	//rddata = {rddata[31:16], addr_itlv, itlv_size, itlv_bypass_ch, itlv_bypass};
	rddata = modified_bits_by_value(rddata, itlv_bypass, 0, 0);
	rddata = modified_bits_by_value(rddata, itlv_bypass_ch, 1, 1);
	rddata = modified_bits_by_value(rddata, itlv_size, 4, 2);
	rddata = modified_bits_by_value(rddata, addr_itlv, 15, 5);
	mmio_wr32(base_addr_ddr_subsys_reg + 0xd4, rddata);
	#else
	itlv_size = 2;
	addr_itlv = 0x7ff;
	addr_ddr1 = 0xfff;
	itlv_bypass = 0b0;
	itlv_bypass_ch = 0b0;
	//itlv_m0
	rddata = mmio_rd32(base_addr_ddr_subsys_reg + 0xd0);
	//rddata = {rddata[31:16], addr_itlv, itlv_size, itlv_bypass_ch, itlv_bypass};
	rddata = modified_bits_by_value(rddata, itlv_bypass, 0, 0);
	rddata = modified_bits_by_value(rddata, itlv_bypass_ch, 1, 1);
	rddata = modified_bits_by_value(rddata, itlv_size, 4, 2);
	rddata = modified_bits_by_value(rddata, addr_itlv, 15, 5);
	rddata = modified_bits_by_value(rddata, addr_ddr1, 27, 16);
	//rddata = {rddata[31:16], addr_itlv, itlv_size, rddata[1:0]};
	mmio_wr32(base_addr_ddr_subsys_reg + 0xd0, rddata);
	//printf("axi_itlv  bypass: %d, bypass_ch: %d, itlv_size: %d, addr_itlv: 0x%x, addr_ddr1: 0x%x\n",
	//		itlv_bypass, itlv_bypass_ch, itlv_size, addr_itlv, addr_ddr1);

	//1to2
	rddata = mmio_rd32(base_addr_ddr_subsys_reg + 0xd4);
	rddata = modified_bits_by_value(rddata, itlv_bypass, 0, 0);
	rddata = modified_bits_by_value(rddata, itlv_bypass_ch, 1, 1);
	rddata = modified_bits_by_value(rddata, itlv_size, 4, 2);
	rddata = modified_bits_by_value(rddata, addr_itlv, 15, 5);
	rddata = modified_bits_by_value(rddata, addr_ddr1, 27, 16);
	mmio_wr32(base_addr_ddr_subsys_reg + 0xd4, rddata);
	//printf("axi_1to2  bypass: %d, bypass_ch: %d, itlv_size: %d, addr_itlv: 0x%x, addr_ddr1: 0x%x\n",
	//		itlv_bypass, itlv_bypass_ch, itlv_size, addr_itlv, addr_ddr1);
	#endif
	#else
	//itlv_m0 --bypass to m00
	rddata = mmio_rd32(base_addr_ddr_subsys_reg + 0xd0);
	itlv_bypass = 0b1;
	itlv_bypass_ch = 0b0;
	//rddata = {rddata[31:2], itlv_bypass_ch, itlv_bypass};
	rddata = modified_bits_by_value(rddata, itlv_bypass, 0, 0);
	rddata = modified_bits_by_value(rddata, itlv_bypass_ch, 1, 1);
	mmio_wr32(base_addr_ddr_subsys_reg + 0xd0, rddata);

	//1to2 --bypass to m10
	rddata = mmio_rd32(base_addr_ddr_subsys_reg + 0xd4);
	itlv_bypass = 0b1;
	itlv_bypass_ch = 0b0;
	//rddata = {rddata[31:2], itlv_bypass_ch, itlv_bypass};
	rddata = modified_bits_by_value(rddata, itlv_bypass, 0, 0);
	rddata = modified_bits_by_value(rddata, itlv_bypass_ch, 1, 1);
	mmio_wr32(base_addr_ddr_subsys_reg + 0xd4, rddata);
	#endif
}

static void sg2380_ddr_setup_init(lpddr_attr *p_lpddr_attr)
{
	//printf("----[%s]----\n", __func__);

	p_lpddr_attr->lpddr_type = LPDDR5;
	p_lpddr_attr->ddr_sys_num = 8;
	p_lpddr_attr->num_rank = 2;
#ifdef HALF_DATA_WIDTH
	p_lpddr_attr->num_ctrl = 2;
#else
	p_lpddr_attr->num_ctrl = 1;
#endif
	p_lpddr_attr->lp5_linkecc_en = false;
	p_lpddr_attr->lp5_inlineecc_en = false;
	p_lpddr_attr->dbi_en = false;
	p_lpddr_attr->phy_training = DWC_PHY_SKIP_TRAINING;//Skip Training

	//printf("ddrtype: %d\n", p_lpddr_attr->lpddr_type);
	//printf("ddr_sys_num: %d\n", p_lpddr_attr->ddr_sys_num);
	//printf("rank_num: %d\n", p_lpddr_attr->num_rank);
	//printf("ctrl_num: %d\n", p_lpddr_attr->num_ctrl);
	//printf("linkecc_en: %d\n", p_lpddr_attr->lp5_linkecc_en);
	//printf("inlineecc_en: %d\n", p_lpddr_attr->lp5_inlineecc_en);
	//printf("dbi_en: %d\n", p_lpddr_attr->dbi_en);
	//printf("phy_training: %d\n", p_lpddr_attr->phy_training);
}

static void dwc_ddrctl_cinit_seq_pwr_on_rst(uint64_t base_ddr_subsys_reg, lpddr_attr *p_lpddr_attr)
{
	uint32_t rddata;

	//printf("----[%s]----\n", __func__);

	// step1: gate aclk core_ddrc_clk
	rddata = mmio_rd32(base_ddr_subsys_reg + 0x0); //bit 0-5 -> gate clk
	//rddata = {rddata[31:7], 0x0};
	rddata = modified_bits_by_value(rddata, 0, 6, 0);
	mmio_wr32(base_ddr_subsys_reg + 0x0, rddata);

	// assert core_ddrc_rstn, areset_n
	rddata = mmio_rd32(base_ddr_subsys_reg + 0x4);
	rddata = modified_bits_by_value(rddata, 0, 5, 0);
	mmio_wr32(base_ddr_subsys_reg + 0x4, rddata);

	// assert preset
	rddata = mmio_rd32(base_ddr_subsys_reg + 0x4);
	rddata = modified_bits_by_value(rddata, 0, 7, 6);
	mmio_wr32(base_ddr_subsys_reg + 0x4, rddata);

	//start clk
	if (p_lpddr_attr->num_ctrl == 2) {
		rddata = mmio_rd32(base_ddr_subsys_reg + 0x0);
		rddata = modified_bits_by_value(rddata, 0x7f, 6, 0);
		mmio_wr32(base_ddr_subsys_reg + 0x0, rddata);
	} else {
		rddata = mmio_rd32(base_ddr_subsys_reg + 0x0);
		rddata = modified_bits_by_value(rddata, 0x3f, 5, 0);
		mmio_wr32(base_ddr_subsys_reg + 0x0, rddata);
	}

	//de-assert preset
	rddata = mmio_rd32(base_ddr_subsys_reg + 0x4);
	//rddata = {rddata[31:8], 0b11, rddata[5:0]};
	rddata = modified_bits_by_value(rddata, 0b11, 7, 6);
	mmio_wr32(base_ddr_subsys_reg + 0x4, rddata);

	//#1us;
	//mdelay(1);//pld
}

static void dwc_ddrctl_cinit_seq_ctrl_init(uint64_t base_addr_ctrl, lpddr_attr *p_lpddr_attr)
{
	if (p_lpddr_attr->lpddr_type == LPDDR4)
		ddrc_init_lp4(base_addr_ctrl, p_lpddr_attr);

	if (p_lpddr_attr->lpddr_type == LPDDR5)
		ddrc_init_lp5(base_addr_ctrl, p_lpddr_attr);
}

static void dwc_ddrctl_cinit_seq_initialize_ctrl_regs(uint64_t base_addr_ctrl, lpddr_attr *p_lpddr_attr)
{
	for (int i = 0; i < p_lpddr_attr->num_ctrl; i++)
		dwc_ddrctl_cinit_seq_ctrl_init(base_addr_ctrl + CTRL_OFFSET * i, p_lpddr_attr);
}

void dwc_ddrctl_cinit_mrw(uint64_t base_addr_ctrl, uint32_t mr_addr, uint32_t mrw_data, uint32_t rank)
{
	uint32_t rddata;

	//printf("R%d MR%d: 0x%x\n", rank, mr_addr, mrw_data);

	rddata = mmio_rd32(base_addr_ctrl + 0x10080);
	rddata = modified_bits_by_value(rddata, rank, 7, 4);
	mmio_wr32(base_addr_ctrl + 0x10080, rddata);

	rddata = mmio_rd32(base_addr_ctrl + 0x10084);
	rddata = modified_bits_by_value(rddata, mrw_data, 7, 0);
	rddata = modified_bits_by_value(rddata, mr_addr, 15, 8);
	mmio_wr32(base_addr_ctrl + 0x10084, rddata);

	rddata = mmio_rd32(base_addr_ctrl + 0x10080);
	rddata = modified_bits_by_value(rddata, 1, 31, 31);
	mmio_wr32(base_addr_ctrl + 0x10080, rddata);

	//mmio_wr32(base_addr_ctrl + 0x10080, 0x20); // DWC_ddrctl_map_REGB_DDRC_CH0.MRCTRL0
	//mmio_wr32(base_addr_ctrl + 0x10084, 0x23f); // DWC_ddrctl_map_REGB_DDRC_CH0.MRCTRL1
	//mmio_wr32(base_addr_ctrl + 0x10080, 0x80000020); // DWC_ddrctl_map_REGB_DDRC_CH0.MRCTRL0
	//mmio_rd32(base_addr_ctrl + 0x10090); // DWC_ddrctl_map_REGB_DDRC_CH0.MRSTAT
	//mmio_rd32(base_addr_ctrl + 0x10090); // DWC_ddrctl_map_REGB_DDRC_CH0.MRSTAT
	while (1) {
		rddata = mmio_rd32(base_addr_ctrl + 0x10090);
		if (get_bits_from_value(rddata, 0, 0) == 0)
			break;
	}
}

void disable_phy_sram_ecc(uint64_t base_addr_phy)
{
	mmio_wr32(base_addr_phy + (0xc0086 << 2), 0x1f);//ArcPmuEccCtl[4:0]
}

void dwc_ddrctl_cinit_set_dfi_init_start(uint64_t base_addr_ctrl)
{
	//printf("----[%s]----\n", __func__);

	uint32_t rddata;

	mmio_wr32(base_addr_ctrl + 0x10c80, 0x0); // DWC_ddrctl_map_REGB_DDRC_CH0.SWCTL
	mmio_wr32(base_addr_ctrl + 0x10510, 0xc034); // DWC_ddrctl_map_REGB_DDRC_CH0.DFIMISC
	mmio_wr32(base_addr_ctrl + 0x10c80, 0x1); // DWC_ddrctl_map_REGB_DDRC_CH0.SWCTL
	while (1) {
		rddata = mmio_rd32(base_addr_ctrl + 0x10c84);
		if (get_bits_from_value(rddata, 0, 0) == 1)
			break;
	}
}

void dwc_ddrctl_cinit_polling_dfi_init_complete(uint64_t base_addr_ctrl)
{
	uint32_t rddata;

	//printf("----[%s]----\n", __func__);

	//mmio_rd32(base_addr_ctrl + 0x10514); // DWC_ddrctl_map_REGB_DDRC_CH0.DFISTAT
	while (1) {
		rddata = mmio_rd32(base_addr_ctrl + 0x10514);
		if (get_bits_from_value(rddata, 0, 0) == 1) //[0]:dfi_init_complete
			break;
	}
}

void dwc_ddrctl_cinit_set_dfi_param(uint64_t base_addr_ctrl, lpddr_attr *p_lpddr_attr)
{
	uint32_t rddata;

	//printf("----[%s]----\n", __func__);

	mmio_wr32(base_addr_ctrl + 0x10c80, 0x0); // DWC_ddrctl_map_REGB_DDRC_CH0.SWCTL
	mmio_wr32(base_addr_ctrl + 0x10510, 0x14); // DWC_ddrctl_map_REGB_DDRC_CH0.DFIMISC
	mmio_wr32(base_addr_ctrl + 0x10c80, 0x1); // DWC_ddrctl_map_REGB_DDRC_CH0.SWCTL
	while (1) {
		rddata = mmio_rd32(base_addr_ctrl + 0x10c84);
		if (get_bits_from_value(rddata, 0, 0) == 1)
			break;
	}

	mmio_wr32(base_addr_ctrl + 0x10c80, 0x0); // DWC_ddrctl_map_REGB_DDRC_CH0.SWCTL
	mmio_wr32(base_addr_ctrl + 0x10510, 0x15); // DWC_ddrctl_map_REGB_DDRC_CH0.DFIMISC
	mmio_wr32(base_addr_ctrl + 0x10c80, 0x1); // DWC_ddrctl_map_REGB_DDRC_CH0.SWCTL
	while (1) {
		rddata = mmio_rd32(base_addr_ctrl + 0x10c84);
		if (get_bits_from_value(rddata, 0, 0) == 1)
			break;
	}

	if (p_lpddr_attr->phy_training == DWC_PHY_SKIP_TRAINING) {
		mmio_wr32(base_addr_ctrl + 0x10180, 0x0); // DWC_ddrctl_map_REGB_DDRC_CH0.PWRCTL
		while (1) {
			rddata = mmio_rd32(base_addr_ctrl + 0x10014);
			if (get_bits_from_value(rddata, 2, 0) == 1)
				break;
		}
	}

	//set dfi_channel_mode
	mmio_wr32(base_addr_ctrl + 0x10c80, 0x0); // DWC_ddrctl_map_REGB_DDRC_CH0.SWCTL
	rddata = mmio_rd32(base_addr_ctrl + 0x10510);
	//rddata = {rddata[31:18], 2'b01, rddata[15:0]};
	rddata = modified_bits_by_value(rddata, 1, 17, 16);
	mmio_wr32(base_addr_ctrl + 0x10510, rddata); // DWC_ddrctl_map_REGB_DDRC_CH0.DFIMISC
	mmio_wr32(base_addr_ctrl + 0x10c80, 0x1); // DWC_ddrctl_map_REGB_DDRC_CH0.SWCTL

	while (1) {
		rddata = mmio_rd32(base_addr_ctrl + 0x10c84);
		if (get_bits_from_value(rddata, 0, 0) == 1)
			break;
	}
}

void dwc_ddrctl_cinit_polling_normal_mode(uint64_t base_addr_ctrl, lpddr_attr *p_lpddr_attr)
{
	uint32_t rddata;

	//printf("----[%s]----\n", __func__);

	if (p_lpddr_attr->phy_training == DWC_PHY_TRAINING) {
		rddata = mmio_rd32(base_addr_ctrl + 0x10180);
		//rddata[11] = 0;
		rddata = modified_bits_by_value(rddata, 0, 11, 11);
		mmio_wr32(base_addr_ctrl + 0x10180, rddata); // DWC_ddrctl_map_REGB_DDRC_CH0.PWRCTL.selref_sw=0
	}

	while (1) {
		rddata = mmio_rd32(base_addr_ctrl + 0x10014);
		// 000---init  001--normal  010--power_down  011 self refresh/self refresh power-down
		if (get_bits_from_value(rddata, 2, 0) == 1)
			break;
	}
}

void set_dram_param(uint64_t base_addr_ddr_subsys, uint64_t base_addr_ctrl, lpddr_attr *p_lpddr_attr)
{
	uint32_t rddata;

	//printf("----[%s]----\n", __func__);

	if (p_lpddr_attr->lpddr_type == LPDDR4) {
		for (int rank_id = 1; rank_id <= p_lpddr_attr->num_rank; rank_id++) {
			//MR13
			dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0xd, 0x88, rank_id);
			//MR23
			dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x17, 0x40, rank_id);
			//MR1
			dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x1, 0xfc, rank_id);
			//MR2
			dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x2, 0x3f, rank_id);
			//MR3
			if (p_lpddr_attr->dbi_en == true)
				dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x3, 0xc3, rank_id);
			else
				dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x3, 0x03, rank_id);
			//MR13
			dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0xd, 0x48, rank_id);
			dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0xd, 0x40, rank_id);
		}

		mmio_wr32(base_addr_ctrl + 0x10208, 0x0); // DWC_ddrctl_map_REGB_DDRC_CH0.RFSHCTL0
		mmio_wr32(base_addr_ctrl + 0x10180, 0x0); // DWC_ddrctl_map_REGB_DDRC_CH0.PWRCTL
		mmio_wr32(base_addr_ctrl + 0x10100, 0x20); // DWC_ddrctl_map_REGB_DDRC_CH0.DERATECTL0
		mmio_wr32(base_addr_ctrl + 0xa80, 0x3fb0); // DWC_ddrctl_map_REGB_FREQ0_CH0.DQSOSCCTL0
		mmio_wr32(base_addr_ctrl + 0x10180, 0x0); // DWC_ddrctl_map_REGB_DDRC_CH0.PWRCTL
		mmio_wr32(base_addr_ctrl + 0x10184, 0x2); // DWC_ddrctl_map_REGB_DDRC_CH0.HWLPCTL

		//mmio_rd32(base_addr_ctrl + 0x10090); // DWC_ddrctl_map_REGB_DDRC_CH0.MRSTAT
		while (1) {
			rddata = mmio_rd32(base_addr_ctrl + 0x10090);
			if (get_bits_from_value(rddata, 0, 0) == 0)
				break;
		}
		rddata = mmio_rd32(base_addr_ctrl + 0x10b88); // DWC_ddrctl_map_REGB_DDRC_CH0.OPCTRLCAM
	} else if (p_lpddr_attr->lpddr_type == LPDDR5) {
		for (int rank_id = 1; rank_id <= p_lpddr_attr->num_rank; rank_id++) {
			//MR16
			dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x10, 0x44, rank_id);
			//MR3
			if (p_lpddr_attr->dbi_en == true)
				dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x3, 0xc6, rank_id);
			else
				dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x3, 0x06, rank_id);

			//MR14
			dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0xe, 0x27, rank_id);

			//MR15
			dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0xf, 0x27, rank_id);
			//MR20
			dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x14, 0x2, rank_id);
			//MR19
			dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x13, 0x0, rank_id);
			//MR18
			dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x12, 0x18, rank_id);
			//MR2
			if (p_lpddr_attr->lp5_linkecc_en == true)
				dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x02, 0xbb, rank_id);
			else
				dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x02, 0x0b, rank_id);
			//MR1
			dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x01, 0xb0, rank_id);
			//MR10
			dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x0a, 0x54, rank_id);
			//MR22
			//if (ENABLE_RW_LINK_ECC) {
			//	dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x16, 0x50, 1);
			//} else if (ENABLE_RD_LINK_ECC) {
			//	dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x16, 0x40, 1);
			//} else if (ENABLE_WR_LINK_ECC) {
			//	dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x16, 0x10, 1);
			//} else {
			//	dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x16, 0x00, 1);
			//}
			//MR22
			if (p_lpddr_attr->lp5_linkecc_en == true) {
				dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x16, 0x50, rank_id);
			} else {
				dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x16, 0x00, rank_id);
			}
			//MR16
			dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x10, 0x41, rank_id);
			//mdelay(1);//pld
			dwc_ddrctl_cinit_mrw(base_addr_ctrl, 0x10, 0x01, rank_id);
		}
		mmio_wr32(base_addr_ctrl + 0x10208, 0x0); // DWC_ddrctl_map_REGB_DDRC_CH0.RFSHCTL0
		mmio_wr32(base_addr_ctrl + 0x10180, 0x0); // DWC_ddrctl_map_REGB_DDRC_CH0.PWRCTL
		mmio_wr32(base_addr_ctrl + 0x10100, 0x20); // DWC_ddrctl_map_REGB_DDRC_CH0.DERATECTL0
		mmio_wr32(base_addr_ctrl + 0xa80, 0x3781); // DWC_ddrctl_map_REGB_FREQ0_CH0.DQSOSCCTL0
		mmio_wr32(base_addr_ctrl + 0x10180, 0x0); // DWC_ddrctl_map_REGB_DDRC_CH0.PWRCTL
		mmio_wr32(base_addr_ctrl + 0x10184, 0x2); // DWC_ddrctl_map_REGB_DDRC_CH0.HWLPCTL
		while (1) {
			rddata = mmio_rd32(base_addr_ctrl + 0x10090);
			if (get_bits_from_value(rddata, 0, 0) == 0)
				break;
		}
		rddata = mmio_rd32(base_addr_ctrl + 0x10b88);
	} else {

	}
}

static void dwc_ddrctl_cinit_addr_cfg(uint64_t *base_addr_ctrl, uint64_t *base_addr_phy,
							uint64_t *base_ddr_subsys_reg, uint8_t ddr_sys_index)
{
	switch (ddr_sys_index) {
	case 0:
		*base_addr_ctrl = 0X05000000000 + 0x02000000;
		*base_addr_phy = 0X05000000000 + 0x00000000;
		*base_ddr_subsys_reg = 0X05000000000 + 0x02800000;
		break;
	case 1:
		*base_addr_ctrl = 0X05000000000 + 0x06000000;
		*base_addr_phy = 0X05000000000 + 0x04000000;
		*base_ddr_subsys_reg = 0X05000000000 + 0x06800000;
		break;
	case 2:
		*base_addr_ctrl = 0X05000000000 + 0x0A000000;
		*base_addr_phy = 0X05000000000 + 0x08000000;
		*base_ddr_subsys_reg = 0X05000000000 + 0x0A800000;
		break;
	case 3:
		*base_addr_ctrl = 0X05000000000 + 0x0E000000;
		*base_addr_phy = 0X05000000000 + 0x0C000000;
		*base_ddr_subsys_reg = 0X05000000000 + 0x0E800000;
		break;
	case 4:
		*base_addr_ctrl = 0X05000000000 + 0x12000000;
		*base_addr_phy = 0X05000000000 + 0x10000000;
		*base_ddr_subsys_reg = 0X05000000000 + 0x12800000;
		break;
	case 5:
		*base_addr_ctrl = 0X05000000000 + 0x16000000;
		*base_addr_phy = 0X05000000000 + 0x14000000;
		*base_ddr_subsys_reg = 0X05000000000 + 0x16800000;
		break;
	case 6:
		*base_addr_ctrl = 0X05000000000 + 0x1A000000;
		*base_addr_phy = 0X05000000000 + 0x18000000;
		*base_ddr_subsys_reg = 0X05000000000 + 0x1A800000;
		break;
	case 7:
		*base_addr_ctrl = 0X05000000000 + 0x1E000000;
		*base_addr_phy = 0X05000000000 + 0x1C000000;
		*base_ddr_subsys_reg = 0X05000000000 + 0x1E800000;
		break;
	default:
		*base_addr_ctrl = 0;
		*base_addr_phy = 0;
		*base_ddr_subsys_reg = 0;
		printf("invalid sys index!!!\n");
		break;
	}
}

void ddr_init(lpddr_attr *p_lpddr_attr, uint8_t ddr_sys_index)
{
	uint32_t rddata;
	uint64_t base_addr_ctrl, base_addr_phy, base_ddr_subsys_reg;

	//printf("sys%d: %s\n", ddr_sys_index, __func__);

	dwc_ddrctl_cinit_addr_cfg(&base_addr_ctrl, &base_addr_phy, &base_ddr_subsys_reg, ddr_sys_index);

	//printf("base_addr_ctrl = 0x%lx, base_addr_phy = 0x%lx, base_ddr_subsys_reg = 0x%lx\n",
	//		base_addr_ctrl, base_addr_phy, base_ddr_subsys_reg);

	//axi itlv setting
	axi_itlv_reg_config(base_ddr_subsys_reg);

	//reset setting
	dwc_ddrctl_cinit_seq_pwr_on_rst(base_ddr_subsys_reg, p_lpddr_attr);

	//ctrl0
	dwc_ddrctl_cinit_seq_initialize_ctrl_regs(base_addr_ctrl, p_lpddr_attr);
	// make sure the write is done
	// in ddrc_env, it just wait 10 clock to make sure write is done,
	// and then release core_ddrc_rstn
	//#1us;
	//mdelay(1);//
	//
	// the DDR initialization if done by here ////
	//
	// Dessert the resets: axi reset and core_ddrc_rstn
	// assert core_ddrc_rstn, areset_n
	rddata = mmio_rd32(base_ddr_subsys_reg + 0x4); //bit 5 -> soft-reset
	//rddata = {rddata[31:6], 0x3f};
	rddata = modified_bits_by_value(rddata, 0x3f, 5, 0);
	mmio_wr32(base_ddr_subsys_reg + 0x4, rddata);

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// PHY Inialization, without training
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//#if 0
	//disable_phy_sram_ecc(base_addr_phy);

	//`ifdef MEM_LPDDR4_ON
	//	pinswap(base_addr_phy, LP4X);
	//`elsif MEM_LPDDR5_ON
	//	`ifdef LPDDR5X
	//	pinswap(base_addr_phy, LP5X);
	//	`endif
	//`endif

	//if (p_lpddr_attr->phy_training == DWC_PHY_TRAINING)
	//	phy_init_trainig(base_addr_phy, p_lpddr_attr);
	//else
	//	phy_init(base_addr_phy, p_lpddr_attr);
	//#endif

	for (int i = 0; i < p_lpddr_attr->num_ctrl; i++)
		dwc_ddrctl_cinit_set_dfi_init_start(base_addr_ctrl + CTRL_OFFSET * i);

	for (int i = 0; i < p_lpddr_attr->num_ctrl; i++)
		dwc_ddrctl_cinit_polling_dfi_init_complete(base_addr_ctrl + CTRL_OFFSET * i);

	//assert dfi_reset_n
	for (int i = 0; i < p_lpddr_attr->num_ctrl; i++)
		dwc_ddrctl_cinit_set_dfi_param(base_addr_ctrl + CTRL_OFFSET * i, p_lpddr_attr);

	//#if 0
	////Isolate the APB access from the internal CSRs by setting the MicroContMuxSel CSR to 1
	////io_write16_sv(0xd0000, 1);
	////reg16_write(base_addr_phy + (0xd0000 << 1), 0x1);
	//mmio_wr32(base_addr_phy + (0xd0000 << 2), 0x1);
	//#endif

	if (p_lpddr_attr->phy_training == DWC_PHY_SKIP_TRAINING) {
		for (int i = 0; i < p_lpddr_attr->num_ctrl; i++)
			set_dram_param(base_ddr_subsys_reg, base_addr_ctrl + CTRL_OFFSET * i, p_lpddr_attr);
	}

	// polling normal mode
	for (int i = 0; i < p_lpddr_attr->num_ctrl; i++)
		dwc_ddrctl_cinit_polling_normal_mode(base_addr_ctrl + CTRL_OFFSET * i, p_lpddr_attr);

	//#1us;
	//mdelay(1);//pld
	mmio_wr32(base_addr_ctrl + 0x10208, 0x0); // DWC_ddrctl_map_REGB_DDRC_CH0.RFSHCTL0
}

void sg2380_ddr_init_asic()
{
	printf("-[%s]-\n", __func__);

	sg2380_ddr_setup_init(&g_lpddr_attr);

	top_itlv_reg_config();

	for (uint8_t i = 0; i < g_lpddr_attr.ddr_sys_num; i++)
		ddr_init(&g_lpddr_attr, i);

	printf("*************sg2380_ddr_init_complete*************\n");
}
