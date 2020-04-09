PLATFORM ?= pano-g2

ifeq ($(V),)
   Q=@
   TTY=/dev/null
else
   TTY=/dev/stdout
endif

# directory related
MAKE_DIR     := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
PATCHES_DIR  := $(abspath $(TOPDIR)/patches)
CORES_DIR    := $(abspath $(TOPDIR)/cores)
PANO_DIR     := $(abspath $(TOPDIR)/pano)
PANO_CORES_DIR := $(PANO_DIR)/cores
PANO_FW_DIR  := $(PANO_DIR)/fw
TOOLS_DIR    := $(PANO_DIR)/tools
CPU_CORE_DIR := $(CORES_DIR)/cpu/riscv
PREBUILT_DIR := $(abspath $(TOPDIR)/prebuilt)
PANO_RTL_DIR := $(abspath $(TOPDIR)/fpga)
RTL_INIT_MEM := $(PANO_RTL_DIR)/firmware.mem
BSCAN_SPI_DIR := $(PANO_CORES_DIR)/xc3sprog

# xc3sprog related
CABLE 	      ?= jtaghs2
XC3SPROG_OPTS ?= -c $(CABLE) -v
XC3SPROG      ?= xc3sprog

# run tool related
TARGET_BAUD ?= 1000000
TARGET_PORT ?= /dev/ttyUSB1
RUN_PREFIX  := $(TOOLS_DIR)/dbg_bridge/run.py -d $(TARGET_PORT) -b $(TARGET_BAUD) -f 
CONSOLE_PREFIX  := $(TOOLS_DIR)/dbg_bridge/console.py -d $(TARGET_PORT) -b $(TARGET_BAUD)

