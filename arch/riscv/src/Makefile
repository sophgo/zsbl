ifeq ($(strip $(CONFIG_ROM_BOOT)), y)
extra-y += target-rom.lds
else
extra-y += target-ram.lds
endif

obj-$(CONFIG_SUPPORT_SMP) += smp.o
obj-y += vector.o
obj-y += crt.o
obj-y += timer.o
obj-y += riscv_atomic.o
obj-y += cache.o
obj-y += riscv_locks.o
obj-y += sbi_trap.o
obj-y += sbi_console.o
obj-y += riscv_asm.o