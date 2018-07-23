all: BootLoader Kernel32 Kernel64 Disk.img

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

Kernel64:
	@echo
	@echo ============ Build 64bit kernel ============
	@echo
	make -C 02Kernel64
	@echo
	@echo ============ Build Complete ============
	@echo

Disk.img: 00BootLoader/BootLoader.bin 01Kernel32/Kernel32.bin 02Kernel64/Kernel64.bin
	@echo
	@echo ============ Disk Image Build Start ============
	@echo
	./ImageMaker.exe $^
	@echo
	@echo ============ All Build Complete ============
	@echo

clean:
	make -C 00BootLoader clean
	make -C 01Kernel32 clean
	make -C 02Kernel64 clean
	rm -f Disk.img
