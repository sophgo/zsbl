ifeq ($(strip $(CROSS_COMPILE)), )
	CROSS_COMPILE := riscv64-unknown-elf-
endif

MARCH = rv64imafdc_zifencei

ifeq ($(strip $(CONFIG_RISCV_CMO)), y)
MARCH := $(MARCH)_zicbom
endif

core-y += arch/riscv/src/

KBUILD_LDS := arch/riscv/src/target-ram.lds

KBUILD_CFLAGS += -g -mcmodel=medany -DARCH_RISCV

KBUILD_CFLAGS += -nostdlib -O1

KBUILD_LDFLAGS += -nostdlib
KBUILD_CFLAGS += -fno-pie -no-pie -fPIE -pie -include libc.h -march=$(MARCH) -misa-spec=2.2

ifeq ($(strip $(CONFIG_PLAT_SG2042)), y)
	KBUILD_LDFLAGS += -g -Wl,--no-dynamic-linker -Wl,-pie -Wl,--no-warn-rwx-segments
else
	KBUILD_LDFLAGS += -g -Wl,--no-dynamic-linker -Wl,--no-warn-rwx-segments
endif
KBUILD_AFLAGS += -march=$(MARCH) -misa-spec=2.2
