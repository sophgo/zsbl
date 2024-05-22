obj-y += core.o main.o

core-$(CONFIG_MMIO) += mmio.o

core-y += stdio.o

ifeq ($(CONFIG_TEST), y)
core-y += test.o
else
core-y += firmware.o
endif
