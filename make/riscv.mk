###############################################################################
# RISC-V Flags
###############################################################################
# GCC_PREFIX     = arch-toolchain-
GCC_PREFIX=riscv32-unknown-elf-
# ARCH_CFLAGS
ARCH_CFLAGS=
# ARCH_LFLAGS
ARCH_LFLAGS=
# FPIC           = yes | no
FPIC = no
# SHARED_LIB     = yes | no
SHARED_LIB = no
# COMPILER       = g++ | gcc
COMPILER = gcc

###############################################################################
# Platform
###############################################################################
PLATFORM    ?= machine-fpga
PLATFORM_DIR ?= $(PANO_FW_DIR)/arch/riscv

BASE_ADDRESS ?= 0x80000000
MEM_SIZE     ?= 65535

ARCH_CFLAGS += -DMACHINE_FPGA
ARCH_LFLAGS += -nostartfiles -nodefaultlibs -lm

ARCH_CFLAGS += -DCONFIG_USE_LOCAL_STRING_H
ARCH_LFLAGS += -Wl,--defsym=BASE_ADDRESS=$(BASE_ADDRESS)
ARCH_LFLAGS += -Wl,--defsym=MEM_SIZE=$(MEM_SIZE)

BOOT_SRC    := $(PLATFORM_DIR)/boot.S

###############################################################################
# Link type (RAM or ROM)
###############################################################################
LINKER_SCRIPT ?= linker_script.ld
EXTRA_CFLAGS += -DLINK_TYPE_RAM

###############################################################################
# Board Settings
###############################################################################
LIBSTD_DIR  ?= $(TOPDIR)/src_c/lib/libstd

ifeq ($(NOLIBSTD),)
  SRC_DIR     += $(LIBSTD_DIR)
endif

SRC_DIR     += $(PLATFORM_DIR)
ARCH_LFLAGS += -T$(PLATFORM_DIR)/$(LINKER_SCRIPT)

###############################################################################
# Run
###############################################################################
ifeq ($(PLATFORM),machine-fpga)
  BRIDGE_TYPE ?= uart
  BAUD_RATE   ?= 1000000
  TARGET_PORT ?= /dev/ttyUSB2
  TOOLS_ARGS  := -t $(BRIDGE_TYPE) -d $(TARGET_PORT) -b $(BAUD_RATE)
  RUN_PREFIX  ?= ../run.py $(TOOLS_ARGS) -f 
endif


