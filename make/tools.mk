.PHONY: clean_tools

CREATE_MIF   := $(TOOLS_DIR)/create_mif.rb   

MKSPIFFS     := $(TOOLS_DIR)/mkspiffs/mkspiffs
$(MKSPIFFS):
	(make -C $(TOOLS_DIR)/mkspiffs) > $(TTY)

clean_tools:
	(make -C $(TOOLS_DIR)/mkspiffs clean) > $(TTY)
	-(rm bin2c/bin2c) > $(TTY)
	-(rm bin2ram_init/bin2ram_init) > $(TTY)


