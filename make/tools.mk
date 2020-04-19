.PHONY: clean_tools start_console

CREATE_MIF   := $(TOOLS_DIR)/create_mif.rb   

MKSPIFFS     := $(TOOLS_DIR)/mkspiffs/mkspiffs
$(MKSPIFFS):
	(make -C $(TOOLS_DIR)/mkspiffs) > $(TTY)

clean_tools:
	(make -C $(TOOLS_DIR)/mkspiffs clean) > $(TTY)
	-(rm bin2c/bin2c) > $(TTY)
	-(rm bin2ram_init/bin2ram_init) > $(TTY)

start_console:
	$(CONSOLE_PREFIX)

