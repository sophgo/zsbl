#ifndef __DDR_H__
#define __DDR_H__

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <lib/mmio.h>
#include <stdio.h>

//#define SG_MDF
//#define SG_MDF_ECC
//#define SG_MDF_RD_CAM

#define CTRL_OFFSET  0x400000
#define HALF_DATA_WIDTH

#define mmio_wr32	mmio_write_32
#define mmio_rd32	mmio_read_32

#define BRINGUP_DEBUG

//#ifdef BRINGUP_DEBUG
//#define DDR_DBG(fmt, ...)	printf(fmt, ##__VA_ARGS__)
//#else
//#define DDR_DBG(fmt, ...)
//#endif

typedef enum {
	DWC_PHY_TRAINING = 0,//<! if set to 0 firmware training is executed
	DWC_PHY_SKIP_TRAINING = 1,
	//<! if set to 1 training is skipped and registers are programmed to work with zero board delay.
	DWC_PHY_DEV_INIT = 2
	//<! if set to 2 training is used to skip training but execute the firmware to
} dwc_ddrctl_phy_training_e;

typedef enum DramTypes {
	LPDDR4,
	LPDDR4X,
	LPDDR5,
	LPDDR5X
} DramType_t;

/**
 * @brief This is the main structure for initializing the DDR Subsystem handler
 * This acts as a container for all the required structures.
 */
typedef struct SubsysHdlr_priv {
	uint8_t enable_non_jedec_tck; //!< Allow a out of range TCK value.
				      //The value spdData_m.tck_ps must be manually specified in this mode.
	uint8_t use_snps_vip_timing; //!< Adjust timings for Synopsys VIP
	uint8_t use_jedec_init; //!< Use JEDEC timings for initialization.
	uint8_t num_pstates; //!< Number of frequencies to setup
	uint8_t num_amap; //!< Number of address maps
	//uint8_t num_dch; //!< Number of data channels to setup
	uint8_t num_rank; //!< Number of ranks to setup
	uint8_t num_ctrl; //ctrl num per sys
	//uint8_t lut_entry[DWC_DDRCTL_MAX_LUT_ENTRIES]; // lut entry buffer, used in cs map lut configuration
	// Bug7444 Single element arrays are treated differently between C and simulator
	//ddrTimingParameters_t timingParams_m[UMCTL2_FREQUENCY_NUM + 1][DDRCTL_CINIT_MAX_DEV_NUM];
	//!< timing parameters
	//mctl_t memCtrlr_m; //!< Structures to configure umctl register field values.
	uint32_t mCtrlBaseAddr_m; //!< Memory controller base address
	// PHY options
	//phy_timing_params_t phy_timing_params; //!< PHY specific timing parameters.
	uint32_t num_anibs; //!< Number of PHY address nibbles
	uint32_t num_dbytes; //!< Number of PHY dbytes
	uint8_t dfi1_exists; //!< PHY exist dfi1
	uint8_t lpddr4_pop_support;
	// HdtCtrl
	// 0x05 = Detailed debug messages (e.g. Eye delays)
	// 0x0A = Coarse debug messages (e.g. rank information)
	// 0xC8 = Stage completion
	// 0xC9 = Assertion messages
	// 0xFF = Firmware completion messages only
	uint8_t HdtCtrl; //!< Used in setting up PUB message block (Hardware Debug Trace Control)

	uint32_t phyBaseAddr_m; //!< PHY utility block (PUB) base address
	uint32_t mr7_by_phy;
	uint32_t mr0_pdx_time;
	uint32_t phy_training; //!< 0 - full training, 1 - skip training, 2 - dev_inti
	// Some control on behaviour of CINIT library
	uint8_t is_constraint_assert_set; //!< enable inline checking of controller register values.
	uint8_t PhyMstrCtrlMode;
	//!< When this bit is 1, a PHY Master transaction is initiated only by a dfi_ctrlmsg transaction.
	uint8_t PhyMstrTrainInterval; //!< Time between the end of one training and the start of the next.
	uint8_t disable_fsp_op; //!Use to control DisableFspOP in LPDDR54 PHYINIT
	uint8_t enable_retention; //!Use to control RetEn in LPDDR54 PHYINIT
	void *phy_config; //!< A pointer to PHYINT meta structure that holds other PHYINT structures
	uint8_t wr_crc_retry_window_internal_delay_extra;
	//!< Randomize the tinternal_delay_extra calculated for wr_crc_retry_window
	uint8_t capar_retry_window_internal_delay_extra;
	//!< Randomize the tinternal_delay_extra calculated for capar_retry_window

	// === Global Struct Defines === //
	//runtime_config_t runtimeConfig;		///< Instance of runtime objects
	//user_input_basic_t userInputBasic;	///< Instance of useInputBasic
	//user_input_advanced_t userInputAdvanced;
	// === Firmware Message Block Structs === //
	//PMU_SMB_LPDDR5X_1D_t mb_LPDDR5X_1D[DWC_DDRPHY_PHYINIT_MAX_NUM_PSTATE];	///< 1D message block instance
	//PMU_SMB_LPDDR4X_1D_t mb_LPDDR4X_1D[DWC_DDRPHY_PHYINIT_MAX_NUM_PSTATE];	///< 1D message block instance

	uint8_t lpddr_type;
	bool remap0_2G;
	bool ddr_no_strip;
	uint8_t ddr_sys_num;
	uint8_t ddr_sys_type; // 0 full 1 location0 2 location1
	uint16_t *ddr_sys_list;
	uint8_t ddr_intlv; // 0 256B 1 64B
	bool lp5_linkecc_en;
	bool lp5_inlineecc_en;
	bool dbi_en;
	bool dm_en;
	bool pll_bypass;
	bool otsd_latency;
	bool dfs_pll_bypass;
	uint8_t ecc_region;
	uint8_t addr_map;
} lpddr_attr; // fwd SubsysHdlr_t

void sg2380_ddr_init_asic();
void ddrc_init_lp4(uint64_t base_addr_ctrl, lpddr_attr *p_lpddr_attr);
void ddrc_init_lp5(uint64_t base_addr_ctrl, lpddr_attr *p_lpddr_attr);

#endif