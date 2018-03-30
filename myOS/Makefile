all: BootLoader Kernel32 Disk.img

BootLoader:
	@echo
	@echo ============ Build Boot Loader ============
	@echo	
	make -C 00BootLoader
	@echo
	@echo ============ Build Complete ============
	@echo

Kernel32:
	@echo
	@echo ============ Build 32bit kernel ============
	@echo
	make -C 01Kernel32
	@echo
	@echo ============ Build Complete ============
	@echo

Disk.img: 00BootLoader/BootLoader.bin 01Kernel32/Kernel32.bin
	@echo
	@echo ============ Disk Image Build Start ============
	@echo
	./ImageMaker.exe $^
	#cat $^ > Disk.img
	@echo
	@echo ============ All Build Complete ============
	@echo

clean:
	make -C 00BootLoader clean
	make -C 01Kernel32 clean
	rm -f Disk.img
