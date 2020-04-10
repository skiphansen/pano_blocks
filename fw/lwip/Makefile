include ../../../project.mk

TARGET       = lwip.a
TARGET_IS_LIB = y
NOLIBSTD := y

LWIP_GIT      := git://git.savannah.gnu.org/lwip.git
LWIP_BASE_COMMIT := 3f47b04f16a874c05def88dcda6b3e0a2d6684ce
LWIP_DIR := $(abspath ./lwip)
LWIP_PATCH_DIR := $(abspath ./patches)

# source to include
SRC_DIR	    += $(LWIP_DIR)/src/core
SRC_DIR	    += $(LWIP_DIR)/src/core/ipv4
EXTRA_SRC   += $(LWIP_DIR)/src/netif/ethernet.c

INCLUDE_PATH += $(LWIP_DIR)/src/include
INCLUDE_PATH += $(PROJECT_INC)

BUILD_TARGETS += $(LWIP_DIR)/.patched 

# build optons
OPT        = 2

include $(TOPDIR)/pano/make/c_prog.mk

.PHONY: distclean debug init

init: $(LWIP_DIR)/.patched

$(LWIP_DIR):
	git clone $(LWIP_GIT)

$(LWIP_DIR)/.patched: $(wildcard $(LWIP_PATCH_DIR)/*) | $(LWIP_DIR)
	(cd $(LWIP_DIR);git reset $(LWIP_BASE_COMMIT) --hard)
	(cd $(LWIP_DIR);git am $(LWIP_PATCH_DIR)/*)
	touch $@

distclean: clean
	rm -rf $(LWIP_DIR)

debug:
	echo "SRC_DIR: $(SRC_DIR)"
	echo "SRC: $(SRC)"
	echo "OBJ: $(OBJ)"
