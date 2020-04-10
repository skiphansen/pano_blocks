###############################################################################
# Inputs
###############################################################################
# SRC_DIR
SRC_DIR ?= .
# INCLUDE_PATH
# TARGET
TARGET ?= target
# EXTRA_CFLAGS
# EXTRA_LIBS
# EXTRA_LIBDIRS
# GCC_PREFIX     = arch-toolchain-
# OPT            = [0-2]
OPT ?= 0
# FPIC           = yes | no
# RUN_PREFIX
# RUN_ARGS
# ARCH
# ARCH_CFLAGS
# ARCH_LFLAGS
# COMPILER       = g++ | gcc

###############################################################################
# Checks
###############################################################################

###############################################################################
# Arch options
###############################################################################
ifneq ($(ARCH),)
  include $(MAKE_DIR)/$(ARCH).mk
endif

###############################################################################
# Variables
###############################################################################
ifneq ($(ARCH),)
  ARCH_TGT_DIR=$(ARCH)
else
  ARCH_TGT_DIR=linux
endif

BUILD_DIR      ?= build

###############################################################################
# Variables: GCC
###############################################################################
GCC_PREFIX   ?= 
COMPILER     ?= g++

GCC          = $(GCC_PREFIX)$(COMPILER)
OBJCOPY      = $(GCC_PREFIX)objcopy
OBJDUMP      = $(GCC_PREFIX)objdump
AR	     = $(GCC_PREFIX)ar
CP	     = cp   
LN	     = ln
DATA2MEM     = data2mem   

###############################################################################
# Variables: Compilation flags
###############################################################################

# Additional include directories
INCLUDE_PATH += $(SRC_DIR)

# Flags
ifeq ($(PLATFORM),pano-g2-c)
    CFLAGS = -DPANO_G2_C
else ifeq ($(PLATFORM),pano-g2)
    CFLAGS = -DPANO_G2
else
   $(error Unknown PLATFORM $(PLATFORM))
endif
CFLAGS       += $(ARCH_CFLAGS) -O$(OPT)
ifeq ($(FPIC), yes)
CFLAGS       += -fpic
endif
CFLAGS       += $(patsubst %,-I%,$(INCLUDE_PATH))
CFLAGS       += $(EXTRA_CFLAGS)

LFLAGS       += $(ARCH_LFLAGS)
LFLAGS       += $(patsubst %,-L%,$(EXTRA_LIBDIRS))
LFLAGS       += $(EXTRA_LIBS)
LFLAGS 	     += -Wl,-Map=$(BUILD_DIR)/$(TARGET).map

###############################################################################
# Variables: Lists of objects, source and deps
###############################################################################
# SRC / Object list
src2obj       = $(BUILD_DIR)/$(patsubst %$(suffix $(1)),%.o,$(notdir $(1)))
src2dep       = $(BUILD_DIR)/$(patsubst %,%.d,$(notdir $(1)))

SRC          := $(BOOT_SRC) $(EXTRA_SRC) $(filter-out $(EXCLUDE_SRC), $(foreach src,$(SRC_DIR),$(wildcard $(src)/*.cpp)) $(foreach src,$(SRC_DIR),$(wildcard $(src)/*.c)))
OBJ          ?= $(foreach src,$(SRC),$(call src2obj,$(src)))
DEPS         ?= $(foreach src,$(SRC),$(call src2dep,$(src)))

###############################################################################
# Rules: Compilation macro
###############################################################################
# Dependancy generation
DEPFLAGS      = -MT $$@ -MMD -MP -MF $(call src2dep,$(1))

define template_c
$(call src2obj,$(1)): $(1) | $(BUILD_DIR)/ $(BUILD_DIR)/
	@echo "# Compiling $(notdir $(1))"
	$(Q)$(GCC) $(CFLAGS) $(DEPFLAGS) -c $$< -o $$@
endef

###############################################################################
# Rules
###############################################################################
BUILD_TARGETS += $(BUILD_DIR)/$(TARGET)

ifeq ($(ENABLE_BIN),yes)
  BUILD_TARGETS += $(BUILD_DIR)/$(TARGET).bin
  BUILD_TARGETS += $(PREBUILT_DIR)/firmware.mem
endif
ifeq ($(ENABLE_LST),yes)
  BUILD_TARGETS += $(BUILD_DIR)/$(TARGET).lst
endif

.PHONY: all init_image run clean load update_ram

all: $(BUILD_TARGETS)

$(BUILD_DIR)/:
	$(Q)mkdir -p $@

$(foreach src,$(SRC),$(eval $(call template_c,$(src))))

$(BUILD_DIR)/$(TARGET): $(OBJ) | $(BUILD_DIR)/ 
	@echo "# Building $(notdir $@)"
ifeq ($(suffix $(BUILD_DIR)/$(TARGET)),.a)
	$(Q)$(AR) rcs $(BUILD_DIR)/$(TARGET) $(OBJ)
else
	$(Q)$(GCC) -o $(BUILD_DIR)/$(TARGET) $(OBJ) $(LFLAGS)
endif

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET)
	@echo "# Building $(notdir $@)"
	$(Q)$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/$(TARGET).lst: $(BUILD_DIR)/$(TARGET)
	@echo "# Building $(notdir $@)"
	$(Q)$(OBJDUMP) -d $< > $@

$(RTL_INIT_MEM): $(BUILD_DIR)/$(TARGET).bin
	@echo "# Building $(notdir $@)"
	$(Q)$(CREATE_MIF) -d 16384 -f mem -w 32 -o 0 -i 1 $(BUILD_DIR)/$(TARGET).bin > $(RTL_INIT_MEM)

init_image: all $(RTL_INIT_MEM)	

run: $(BUILD_DIR)/$(TARGET)
	$(RUN_PREFIX) $(BUILD_DIR)/$(TARGET) $(RUN_ARGS)

clean:
	$(Q)rm -rf $(BUILD_DIR)

load:
	$(Q)make -C $(PANO_RTL_DIR) load

update_ram:
	$(Q)make -C $(PANO_RTL_DIR) update_ram

###############################################################################
# Rules: Dependancies
###############################################################################
EXCLUDE_DEPS := clean
ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(EXCLUDE_DEPS))))
-include $(DEPS)
endif

include $(MAKE_DIR)/tools.mk

