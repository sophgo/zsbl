ifeq ($(strip $(CONFIG_ROM_BOOT)), y)
extra-y += target-rom.lds
else
extra-y += target-ram.lds
endif

obj-y += smp.o
obj-y += crt.o
obj-y += timer.o
obj-y += riscv_atomic.o
obj-$(CONFIG_THEAD_CMO) += thead_cache.o
obj-$(CONFIG_RISCV_CMO) += riscv_cache.o
obj-y += riscv_locks.o
obj-y += sbi_trap.o
obj-n += sbi_safe_console.o
obj-y += riscv_asm.o

obj-$(CONFIG_THEAD) += t-head/
obj-$(CONFIG_SIFIVE) += sifive/