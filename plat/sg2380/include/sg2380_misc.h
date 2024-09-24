#ifndef __SG2380_MISC_H__
#define __SG2380_MISC_H__

void sg2380_ssperi_phy_config(int mode);
void sg2380_eth_type_config(int type);
void sg2380_eth_mul_channel_intr_enable(void);
void sg2380_multimedia_itlvinit(void);
void sg2380_set_tpu_run(uint64_t addr);
void sg2380_platform_init(void);
void sg2380_top_reset(uint32_t index);

#endif
