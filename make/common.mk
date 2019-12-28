ifneq ($(PLATFORM),)
# Save PLATFORM that is specified so we don't need to specify it every time
   $(shell echo $(PLATFORM) > $(TOPDIR)/.platform)
else
   ifneq ("$(wildcard $(TOPDIR)/.platform)","")
   # Use last PLATFORM specified
      PLATFORM := $(shell cat $(TOPDIR)/.platform)
   else
   # Use default
      PLATFORM := pano-g2
   endif
endif

ifeq ($(V),)
   Q=@
   TTY=/dev/null
else
   TTY=/dev/stdout
endif

# directory related
MAKE_DIR     := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
CORES_DIR    := $(TOPDIR)/cores
PANO_DIR     := $(TOPDIR)/pano
PANO_CORES_DIR := $(PANO_DIR)/cores
PANO_FW_DIR  := $(PANO_DIR)/fw
TOOLS_DIR    := $(PANO_DIR)/tools
CPU_CORE_DIR := $(CORES_DIR)/cpu/riscv
PREBUILT_DIR := $(TOPDIR)/prebuilt
PANO_RTL_DIR := $(TOPDIR)/fpga
RTL_INIT_MEM := $(PANO_RTL_DIR)/firmware.mem

# xc3sprog related
CABLE 	      ?= jtaghs2
XC3SPROG_OPTS ?= -c $(CABLE) -v
XC3SPROG      ?= xc3sprog

# run tool related
TARGET_BAUD ?= 1000000
TARGET_PORT ?= /dev/ttyUSB1
RUN_PREFIX  := $(TOOLS_DIR)/dbg_bridge/run.py -d $(TARGET_PORT) -b $(TARGET_BAUD) -f 


