ifeq ($(strip $(CONFIG_ROM_BOOT)), y)
extra-y += target-rom.lds
else
extra-y += target-ram.lds
endif

obj-$(CONFIG_SUPPORT_SMP) += smp.o
obj-y += vector.o
obj-y += crt.o
obj-y += timer.o
obj-y += atomic.o
obj-y += cache.o
obj-y += locks.o
obj-y += trap.o

obj-$(CONFIG_SUPPORT_BUILTIN_DTB) += dtb.o builtin_dtb.o
