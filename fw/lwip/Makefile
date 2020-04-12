include ../../../project.mk

TARGET       = lwip.a
TARGET_IS_LIB = y
NOLIBSTD := y

LWIP_GIT      := git://git.savannah.gnu.org/lwip.git
LWIP_BASE_COMMIT := 3f47b04f16a874c05def88dcda6b3e0a2d6684ce
LWIP_SRC := $(abspath ./lwip)
LWIP_PATCH_DIR := $(abspath ./patches)

ifneq ($(MAKECMDGOALS),distclean)
GIT_INIT := $(shell if [ ! -e $(LWIP_SRC) ]; then git clone $(LWIP_GIT);fi)
endif

# source to include
SRC_DIR	    += $(LWIP_SRC)/src/core
SRC_DIR	    += $(LWIP_SRC)/src/api
SRC_DIR	    += $(LWIP_SRC)/src/core/ipv4
EXTRA_SRC   += $(LWIP_SRC)/src/netif/ethernet.c

INCLUDE_PATH += $(LWIP_SRC)/src/include
INCLUDE_PATH += $(PROJECT_DIR)
BUILD_DIR    := $(PROJECT_DIR)/build/lwip

BUILD_TARGETS += $(LWIP_SRC)/.patched 

# build optons
OPT        = 2

include $(TOPDIR)/pano/make/c_prog.mk

PATCHES := $(wildcard $(LWIP_PATCH_DIR)/*)
.PHONY: distclean debug init

init: $(LWIP_SRC)/.patched

$(LWIP_SRC)/.patched: $(PATCHES)
ifneq ($(PATCHES),)
	(cd $(LWIP_SRC);git reset $(LWIP_BASE_COMMIT) --hard)
	(cd $(LWIP_SRC);git am $(LWIP_PATCH_DIR)/*)
endif
	touch $@

distclean: clean
	rm -rf $(LWIP_SRC)

debug:
	echo "SRC_DIR: $(SRC_DIR)"
	echo "SRC: $(SRC)"
	echo "OBJ: $(OBJ)"
