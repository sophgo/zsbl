ifeq ($(strip $(CROSS_COMPILE)), )
    CROSS_COMPILE := riscv64-unknown-linux-gnu-
endif

core-y += arch/riscv/src/

ifeq ($(strip $(CONFIG_ROM_BOOT)), y)
KBUILD_LDS := arch/riscv/src/target-rom.lds
else
KBUILD_LDS := arch/riscv/src/target-ram.lds
endif

RISCV_CAFLAGS = -march=rv64imac_zicbom_zifencei_zicsr \
		-mabi=lp64 \
		-mcmodel=medany \
		-DARCH_RISCV \
		-fstack-protector \
		-fshort-wchar \
		-nostdinc \
		-mno-relax

KBUILD_CFLAGS += $(RISCV_CAFLAGS)
KBUILD_AFLAGS += $(RISCV_CAFLAGS)

KBUILD_CFLAGS += -nostdlib
KBUILD_LDFLAGS += -nostdlib
KBUILD_LDFLAGS += -Wl,--no-dynamic-linker -Wl,--no-warn-rwx-segments

ifeq ($(strip $(CONFIG_SUPPORT_PIC)), y)
KBUILD_CFLAGS += -fpie
KBUILD_LDFLAGS += -Wl,-pie
else
KBUILD_CFLAGS += -fno-pie
KBUILD_LDFLAGS += -Wl,-no-pie
endif
