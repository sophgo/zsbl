ifeq ($(strip $(CROSS_COMPILE)), )
    CROSS_COMPILE := riscv64-unknown-linux-gnu-
endif

core-y += arch/riscv/src/

ifeq ($(strip $(CONFIG_ROM_BOOT)), y)
KBUILD_LDS := arch/riscv/src/target-rom.lds
else
KBUILD_LDS := arch/riscv/src/target-ram.lds
endif


KBUILD_CFLAGS += -march=rv64imafdc_zifencei -mcmodel=medany -DARCH_RISCV

KBUILD_CFLAGS += -nostdinc -fstack-protector -fshort-wchar

KBUILD_LDFLAGS += -nostdlib
KBUILD_CFLAGS += -march=rv64imafdc_zicbom -misa-spec=2.2
KBUILD_LDFLAGS += -Wl,--no-dynamic-linker -Wl,--no-warn-rwx-segments
KBUILD_AFLAGS += -march=rv64imafdc_zicbom -misa-spec=2.2 -fstack-protector

ifeq ($(strip $(CONFIG_SUPPORT_PIC)), y)
KBUILD_CFLAGS += -fpie
KBUILD_LDFLAGS += -Wl,-pie
else
KBUILD_CFLAGS += -fno-pie
KBUILD_LDFLAGS += -Wl,-no-pie
endif
