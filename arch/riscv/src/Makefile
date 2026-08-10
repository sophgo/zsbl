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
obj-y += arch_spinlock.o
obj-y += trap.o
obj-y += arch.o
obj-$(CONFIG_RISCV_CMO) += cache.o
obj-$(CONFIG_MULTI_THREAD) += arch_thread.o

obj-$(CONFIG_SUPPORT_BUILTIN_DTB) += dtb.o builtin_dtb.o

# builtin_dtb.S uses `.incbin CONFIG_BUILTIN_DTB` to embed the compiled dtb.
# The assembler's .incbin dependency is invisible to gcc's -MD auto-dep
# generation, so a rebuilt .dtb never triggers re-assembly of builtin_dtb.o.
# Add an explicit dependency so editing the .dts (which rebuilds the .dtb)
# forces builtin_dtb.o to be regenerated.
builtin-dtb := $(objtree)/arch/riscv/boot/dts/$(patsubst "%",%,$(CONFIG_BUILTIN_DTB))
$(obj)/builtin_dtb.o: $(builtin-dtb)
