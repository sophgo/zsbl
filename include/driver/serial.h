#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <driver/device.h>
#include <lib/list.h>

#include <stdint.h>

struct serial;

struct serial_ops {
	int (*input)(struct serial *serial);
	int (*output)(struct serial *serial, uint8_t c);
};

struct serial {
	struct device device;
	struct serial_ops *ops;

	unsigned int baudrate;

	int stdio;

	void *data;
};

struct serial *serial_alloc(void);
int serial_register(struct serial *serial);
int serial_list_devices(void);
struct serial *serial_first(void);
struct serial *serial_next(struct serial *current);

int serial_input(struct serial *serial);
int serial_output(struct serial *serial, uint8_t c);

void serial_free(struct serial *serial);

#endif
