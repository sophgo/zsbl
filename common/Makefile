obj-y += core.o

core-y += main.o stdio.o

core-$(CONFIG_MMIO) += mmio.o

ifeq ($(CONFIG_TEST), y)
core-y += test.o
else
core-y += firmware.o
endif
