ifeq ($(strip $(CROSS_COMPILE)), )
    CROSS_COMPILE := aarch64-linux-gnu-
endif

core-y += arch/arm64/src/

ifeq ($(strip $(CONFIG_ROM_BOOT)), y)
KBUILD_LDS := arch/arm64/src/target-rom.lds
else
KBUILD_LDS := arch/arm64/src/target-ram.lds
endif

KBUILD_CFLAGS += -g -march=armv8-a+simd+nocrypto -DARCH_ARM64 -nostdinc
KBUILD_LDFLAGS += -nostdlib
