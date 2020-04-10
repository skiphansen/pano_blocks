LWIP_LIB_DIR := $(PANO_FW_DIR)/lwip
LWIP_SRC     := $(LWIP_LIB_DIR)/lwip
LWIP_LIB     := $(LWIP_LIB_DIR)/build/lwip.a

EXTRA_LIBS += $(LWIP_LIB)
INCLUDE_PATH += $(LWIP_SRC)/src/include

.PHONY: all
all:

$(LWIP_SRC):
	make -C $(LWIP_LIB_DIR) init

$(LWIP_LIB): | $(LWIP_SRC)
	make -C $(LWIP_LIB_DIR) PROJECT_INC=$(PROJECT_INC)

