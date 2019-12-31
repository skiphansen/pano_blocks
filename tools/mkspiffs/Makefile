mkspiffs: build/mkspiffs

build/mkspiffs:
	git clone https://github.com/igrr/mkspiffs.git build
	(cd build; patch -p1 < ../0001-Configure-for-RISC-V-processor.patch)
	(cd build; git clone https://github.com/pellepl/spiffs.git)
	make -C build
	cp build/mkspiffs .

clean:
	-rm -rf build
	-rm mkspiffs




