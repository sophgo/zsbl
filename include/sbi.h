#ifndef __SBI_H__
#define __SBI_H__

struct fw_dynamic_info {
	/** Info magic */
	unsigned long magic;
	/** Info version */
	unsigned long version;
	/** Next booting stage address */
	unsigned long next_addr;
	/** Next booting stage mode */
	unsigned long next_mode;
	/** Options for OpenSBI library */
	unsigned long options;
	/**
	 * Preferred boot HART id
	 *
	 * It is possible that the previous booting stage uses same link
	 * address as the FW_DYNAMIC firmware. In this case, the relocation
	 * lottery mechanism can potentially overwrite the previous booting
	 * stage while other HARTs are still running in the previous booting
	 * stage leading to boot-time crash. To avoid this boot-time crash,
	 * the previous booting stage can specify last HART that will jump
	 * to the FW_DYNAMIC firmware as the preferred boot HART.
	 *
	 * To avoid specifying a preferred boot HART, the previous booting
	 * stage can set it to -1UL which will force the FW_DYNAMIC firmware
	 * to use the relocation lottery mechanism.
	 */
	unsigned long boot_hart;
} __packed;

/** Expected value of info magic ('OSBI' ascii string in hex) */
#define FW_DYNAMIC_INFO_MAGIC_VALUE		0x4942534f

/** Maximum supported info version */
#define FW_DYNAMIC_INFO_VERSION_2		0x2

/** Possible next mode values */
#define FW_DYNAMIC_INFO_NEXT_MODE_U		0x0
#define FW_DYNAMIC_INFO_NEXT_MODE_S		0x1
#define FW_DYNAMIC_INFO_NEXT_MODE_M		0x3

#endif
