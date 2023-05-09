ifeq ($(strip $(CROSS_COMPILE)), )
    CROSS_COMPILE := riscv64-unknown-elf-
endif

core-y += arch/riscv/src/

ifeq ($(strip $(CONFIG_ROM_BOOT)), y)
KBUILD_LDS := arch/riscv/src/target-rom.lds
else
KBUILD_LDS := arch/riscv/src/target-ram.lds
endif

KBUILD_CFLAGS += -g -mcmodel=medany -DARCH_RISCV
KBUILD_LDFLAGS += -g
