LWIP_LIB_DIR := $(PANO_FW_DIR)/lwip
LWIP_SRC_DIR := $(abspath $(LWIP_LIB_DIR)/lwip)
LWIP_LIB     := $(PROJECT_DIR)/build/lwip/lwip.a

EXTRA_LIBS += $(LWIP_LIB)
INCLUDE_PATH += $(LWIP_SRC_DIR)/src/include

.PHONY: all
all:

$(LWIP_SRC):
	make -C $(LWIP_LIB_DIR) init

$(LWIP_LIB): | $(LWIP_SRC)
	make -C $(LWIP_LIB_DIR) PROJECT_DIR=$(PROJECT_DIR)

