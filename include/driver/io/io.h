#ifndef __IO_H__
#define __IO_H__

#include <ff.h>

typedef enum {
	IO_DEVICE_SD = 0,
	IO_DEVICE_SPIFLASH,
	IO_DEVICE_SPINAND,
	IO_DEVICE_EMMC,
	IO_DEVICE_MAX,
} IO_DEVICE;
extern const char* IO_DEVICE_NAMES[];


struct __attribute__((aligned(512))) part_info {
		/* disk partition table magic number */
		uint32_t magic;
		char name[32];
		uint32_t offset;
		uint32_t size;
		char reserve[4];
		/* load memory address*/
		uint64_t lma;
};


typedef struct boot_file {
	uint32_t id;
	char *name[4];
	//*name[4] --> *name 01
	// 改回去了
	uint64_t addr;
	int len;
	uint32_t offset;
} BOOT_FILE;

typedef struct {
	//int (*init)(void);
	int (*open)(char *name, uint8_t flags);
	int (*read)(BOOT_FILE *boot_file, int id, uint64_t len);
	int (*close)(void);
	int (*get_file_info)(BOOT_FILE *boot_file, int id, FILINFO *info,
			    int dev_num, uint64_t table_addr);
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
const char* get_io_device_name(IO_DEVICE dev);
int get_current_io_device(IO_DEV **io_dev, uint32_t dev_num);
int device_get_file_info(BOOT_FILE *boot_file, int id, FILINFO *f_info,
				  int dev_num, uint64_t table_addr);



#endif
