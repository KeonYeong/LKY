.EXPORT_ALL_VARIABLES:

TOPDIR	:= $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)

DIRS = kernel hal/cpu hal/io shell objs
CROSSTOOL = arm-s5pc1xx-linux-gnueabi-
CC = $(CROSSTOOL)gcc
OC = $(CROSSTOOL)objcopy
LD = $(CROSSTOOL)ld

INCLUDE = -I. -I$(TOPDIR)/include -I$(TOPDIR)/hal/include -I$(TOPDIR)/fs/include -I/opt/s5pc1xx/cross/armv7a/lib/gcc/arm-s5pc1xx-linux-gnueabi/4.2.1/include

CFLAGS  = -g -O0 -Wall -Wstrict-prototypes -fPIC -msoft-float -nostdinc -nostartfiles -nostdlib -march=armv5 -fno-builtin $(INCLUDE)

OCFLAGS = -O binary -R .note -R .comment -R .stab -R .stabstr -S 

all:
	for i in $(DIRS) ; do make -C $$i || exit $? ; done
	dd if=images/vpos_kernel_binary of=images/vpos.bin bs=1k conv=sync

	date

imgmerge:
	$(OC) $(OCFLAGS) ./images/vpos_kernel-elf32 ./images/vpos_kernel_binary
	dd if=images/vpos_kernel_binary of=images/vpos.bin bs=1k conv=sync
	dd if=utils4/alpha of=images/vpos.bin bs=1k seek=3040
	

clean:
	for i in $(DIRS) ; do make -C $$i clean; done
	rm -f ./images/vpos_bootloader_binary;rm -f ./images/vpos_bootloader-elf32;rm -f ./images/vpos_kernel_binary;rm -f ./images/vpos_kernel-elf32;rm -f ./images/vpos.bin
	
dep:
	for i in $(DIRS) ; do make -C $$i dep; done
