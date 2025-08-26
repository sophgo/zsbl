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
KBUILD_CFLAGS += -fno-pie -no-pie -fPIE -pie -march=rv64imafdc_zicbom -misa-spec=2.2
KBUILD_LDFLAGS += -Wl,--no-dynamic-linker -Wl,-pie -Wl,--no-warn-rwx-segments
KBUILD_AFLAGS += -march=rv64imafdc_zicbom -misa-spec=2.2 -fstack-protector
