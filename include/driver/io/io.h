#ifndef __IO_H__
#define __IO_H__

#include <ff.h>

enum {
		IO_DEVICE_SD = 0,
		IO_DEVICE_SPIFLASH,
		IO_DEVICE_MAX,
};

typedef struct boot_file {
		uint32_t id;
		char *name;
		uint64_t addr;
		int len;
		uint32_t offset;
} BOOT_FILE;

typedef struct {
	int (*init)(void);
	int (*open)(char *name, uint8_t flags);
	int (*read)(BOOT_FILE *boot_file, int id, uint64_t len);
	int (*close)(void);
	int (*get_file_info)(BOOT_FILE *boot_file, int id, FILINFO *info);
	int (*destroy)(void);

}io_funcs;

typedef struct {
	uint32_t type;
	io_funcs func;

	FATFS SDC_FS;
	FIL fp;

}IO_DEV;

int io_device_register(uint32_t dev_num, IO_DEV *dev);
IO_DEV *set_current_io_device(uint32_t dev_num);

#endif