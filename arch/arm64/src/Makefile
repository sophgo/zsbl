ifeq ($(strip $(CONFIG_ROM_BOOT)), y)
extra-y += target-rom.lds
else
extra-y += target-ram.lds
endif

obj-y += crt.o
obj-y += mm.o
obj-y += timer.o

obj-y += vector.o
obj-y += cache.o
obj-y += memcpy.o
obj-y += memmove.o
obj-y += memset.o
obj-$(CONFIG_SUPPORT_SMP) += smp.o

