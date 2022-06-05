###############################################################################
## Params
###############################################################################
XILINX_ISE ?= /opt/Xilinx/14.7/ISE_DS/ISE/bin/lin64
EXTRA_VFLAGS     ?= 
PROJECT_DIR      ?= build
PROJECT          ?= fpga
TOP_MODULE       ?= top
PROG_FPGA_OFFSET ?= 0

###############################################################################
# Checks
###############################################################################

ifeq ($(PLATFORM),pano-g2-c)
    PART_NAME    = xc6slx100
    PART_PACKAGE = fgg484
    PART_SPEED   = 3
    PANO_SERIES  = g2
else ifeq ($(PLATFORM),pano-g2)
    PART_NAME    = xc6slx150
    PART_PACKAGE = fgg484
    PART_SPEED   = 2
    PANO_SERIES  = g2
else ifeq ($(PLATFORM),pano_logic_g2)
# for compatibility with litex-buildenv
    PART_NAME    = xc6slx150
    PART_PACKAGE = fgg484
    PART_SPEED   = 2
    PANO_SERIES  = g2
else
   $(error Unknown PLATFORM $(PLATFORM))
endif

PLATFORM_BITFILE = $(PREBUILT_DIR)/$(PLATFORM).bit

TOP_MODULE   ?= top

ifeq ($(XILINX_ISE),)
$(error "XILINX_ISE not set - e.g. export XILINX_ISE=/opt/Xilinx/14.7/ISE_DS/ISE/bin/lin64")
endif
TOOL_PATH    := $(XILINX_ISE)

VERILOG_SRCS += $(filter-out $(EXCLUDE_SRC),$(abspath $(foreach _dir,$(SRC_DIR), $(wildcard $(_dir)/*.v)))$(EXTRA_SRC))
VHDL_SRCS := $(filter-out $(EXCLUDE_SRC),$(abspath $(foreach _dir,$(SRC_DIR), $(wildcard $(_dir)/*.vhd)))$(EXTRA_SRC))
SRC_FILES := $(VERILOG_SRCS) $(VHDL_SRCS)
BIT_FILE = $(PROJECT_DIR)/${PROJECT}_routed.bit
VPATH = $(SRC_DIR)

###############################################################################
# Rules:
###############################################################################

all: $(PROJECT_DIR) $(NGC_FILES) $(BIT_FILE) $(PLATFORM_BITFILE)

clean:
	rm -rf $(PROJECT_DIR)

$(PROJECT_DIR):
	@mkdir -p $@

###############################################################################
# PROJECT.ut
###############################################################################
$(PROJECT_DIR)/$(PROJECT).ut: | $(PROJECT_DIR)
	if [ -e $(PROJECT).ut ]; then cp $(PROJECT).ut $(PROJECT_DIR); else \
	cp $(MAKE_DIR)/default.ut $(PROJECT_DIR)/$(PROJECT).ut; fi

###############################################################################
# PROJECT.xst
###############################################################################
$(PROJECT_DIR)/$(PROJECT).xst: | $(PROJECT_DIR)
	if [ -e $(PROJECT).xst ]; then cp $(PROJECT).xst $(PROJECT_DIR); else \
	cp $(MAKE_DIR)/default.xst $(PROJECT_DIR)/$(PROJECT).xst; fi
	echo "-ifn $(PROJECT).prj" >> $@
	echo "-ofn $(PROJECT)" >> $@
	echo "-p $(PART_NAME)-$(PART_SPEED)-$(PART_PACKAGE)" >> $@
	echo "-top $(TOP_MODULE)" >> $@
ifneq ($(EXTRA_VFLAGS),)
	echo "-define {$(EXTRA_VFLAGS)}" >> $@
endif


###############################################################################
# PROJECT.prj
###############################################################################
$(PROJECT_DIR)/$(PROJECT).prj: $(PROJECT_DIR)/$(PROJECT).ut $(PROJECT_DIR)/$(PROJECT).xst $(firstword $(MAKEFILE_LIST))
	@if [ -e $@ ] ; then rm $@; fi
	@$(foreach _file,$(VERILOG_SRCS),echo "verilog work \"$(_file)\"" >> $@;)
	@$(foreach _file,$(VHDL_SRCS),echo "vhdl work \"$(_file)\"" >> $@;)
	@$(foreach _file,$(abspath $(wildcard $(PROJECT_DIR)/*.v)),echo "verilog work \"$(_file)\"" >> $@;)
	@$(foreach _file,$(abspath $(wildcard $(PROJECT_DIR)/*.vhd)),echo "vhdl work \"$(_file)\"" >> $@;)

###############################################################################
# PROJECT.ucf
###############################################################################
UCF_FILE  = $(PROJECT_DIR)/$(PROJECT).ucf
UCF_FILES += $(foreach _dir,$(SRC_DIR), $(wildcard $(_dir)/*_$(PANO_SERIES).ucf))
$(UCF_FILE): $(UCF_FILES)
	cat $^ > $@

XCO_FILES += $(foreach _dir,$(SRC_DIR), $(wildcard $(_dir)/*.xco))
NGC_FILES += $(foreach _file,$(XCO_FILES), $(PROJECT_DIR)/$(addsuffix .ngc,$(basename $(notdir $(_file)))))

###############################################################################
# Rule: Synth
###############################################################################
$(PROJECT_DIR)/$(PROJECT).ngc: $(NGC_FILES) $(PROJECT_DIR)/$(PROJECT).prj $(SRC_FILES) $(INIT_IMAGE)
	@echo "####################################################################"
	@echo "# ISE: Synth"
	@echo "####################################################################"
	@mkdir -p $(PROJECT_DIR)/xst/projnav.tmp/
	@cd $(PROJECT_DIR); $(TOOL_PATH)/xst -intstyle ise -ifn $(PROJECT).xst -ofn $(PROJECT).syr

###############################################################################
# Rule: Convert netlist
###############################################################################
$(PROJECT_DIR)/$(PROJECT).ngd: $(PROJECT_DIR)/$(PROJECT).ngc $(UCF_FILE)
	@echo "####################################################################"
	@echo "# ISE: Convert netlist"
	@echo "####################################################################"
	@cd $(PROJECT_DIR); $(TOOL_PATH)/ngdbuild -intstyle ise -dd _ngo -nt timestamp \
	-uc $(abspath $(UCF_FILE)) -p $(PART_NAME)-$(PART_PACKAGE)-$(PART_SPEED) $(PROJECT).ngc $(PROJECT).ngd -bm ../firmware.bmm

###############################################################################
# Rule: Map
###############################################################################
MAP_CMDS ?= -w -intstyle ise -detail -ir off -ignore_keep_hierarchy -pr b -timing -ol high -logic_opt on

$(PROJECT_DIR)/$(PROJECT).ncd: $(PROJECT_DIR)/$(PROJECT).ngd
	@echo "####################################################################"
	@echo "# ISE: Map"
	@echo "####################################################################"
	@cd $(PROJECT_DIR); $(TOOL_PATH)/map $(MAP_CMDS) \
	-p $(PART_NAME)-$(PART_PACKAGE)-$(PART_SPEED) \
	-o $(PROJECT).ncd $(PROJECT).ngd $(PROJECT).pcf 

###############################################################################
# Rule: Place and route
###############################################################################
PAR_CMDS ?= -w -intstyle ise -ol high 
$(PROJECT_DIR)/$(PROJECT)_routed.ncd: $(PROJECT_DIR)/$(PROJECT).ncd
	@echo "####################################################################"
	@echo "# ISE: Place and route"
	@echo "####################################################################"
	@cd $(PROJECT_DIR); $(TOOL_PATH)/par $(PAR_CMDS) $(PROJECT).ncd $(PROJECT)_routed.ncd $(PROJECT).pcf

###############################################################################
# Rule: Bitstream
###############################################################################
$(BIT_FILE): $(PROJECT_DIR)/$(PROJECT)_routed.ncd
	@echo "####################################################################"
	@echo "# ISE: Create bitstream"
	@echo "####################################################################"
	@cd $(PROJECT_DIR); $(TOOL_PATH)/bitgen -f $(PROJECT).ut $(PROJECT)_routed.ncd

$(PLATFORM_BITFILE) : $(BIT_FILE)
	@cp $(BIT_FILE) $(PLATFORM_BITFILE)

###############################################################################
# Rule: Bitstream -> binary
###############################################################################
$(PROJECT_DIR)/$(PROJECT).bin: $(BIT_FILE)
	@echo "####################################################################"
	@echo "# ISE: Convert bitstream"
	@echo "####################################################################"
	@cd $(PROJECT_DIR); $(TOOL_PATH)/promgen -u 0x0 $(PROJECT)_routed.bit -p bin -w -b -o $(PROJECT).bin

###############################################################################
# Rule: Load Bitstream using XC2PROG
###############################################################################
load:
	$(XC3SPROG) $(XC3SPROG_OPTS) $(PLATFORM_BITFILE)

###############################################################################
# Rule: Upate Bitstream with new firmware image
###############################################################################
update_ram:
	$(Q)data2mem -bm firmware_bd.bmm -bt $(BIT_FILE) -bd $(RTL_INIT_MEM) -o b $(BIT_FILE).new.bit
	$(Q)data2mem -bm firmware_bd.bmm -bt $(BIT_FILE).new.bit -d > $(BIT_FILE).new.bit.dump
	$(Q)mv $(BIT_FILE) $(BIT_FILE).orig
	$(Q)mv $(BIT_FILE).new.bit $(BIT_FILE)

###############################################################################
# Rule: Program Bitstream into SPI flash using XC2PROG
###############################################################################
BSCAN_SPI_BITFILE = $(BSCAN_SPI_DIR)/$(PART_NAME).bit
prog_fpga:
	$(XC3SPROG) $(XC3SPROG_OPTS) -I$(BSCAN_SPI_BITFILE) $(PLATFORM_BITFILE):w:$(PROG_FPGA_OFFSET)

###############################################################################
# Rule: build .ngc from .xco
###############################################################################
$(PROJECT_DIR)/$(PROJECT).cgp:
	if [ -e $(PROJECT).cgp ]; then cp $(PROJECT).cgp $(PROJECT_DIR); else \
	cp $(MAKE_DIR)/default.cgp $(PROJECT_DIR)/$(PROJECT).cgp; fi
	echo "SET workingdirectory = ./tmp/" >> $@
	echo "SET devicefamily = spartan6" >> $@
	echo "SET device = $(PART_NAME)" >> $@
	echo "SET package = $(PART_PACKAGE)" >> $@
	echo "SET speedgrade = -$(PART_SPEED)" >> $@

$(PROJECT_DIR)/%.ngc: %.xco $(PROJECT_DIR)/$(PROJECT).cgp
	(cd $(PROJECT_DIR); coregen -p $(PROJECT).cgp -b  $<)

include $(MAKE_DIR)/tools.mk

