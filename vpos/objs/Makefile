OBJS = kernel_start.o machine_init.o exception_handler.o HAL_arch_startup.o hal_swi_handler.o led.o serial.o timer.o irq.o vpsh.o

IMAGE_TARGET = vpos_kernel_binary
PRE_TARGET = vpos_kernel-elf32

all : $(PRE_TARGET)
	$(OC) $(OCFLAGS) $(PRE_TARGET) $(IMAGE_TARGET)
	mv vpos_kernel_binary vpos_kernel-elf32 ../images

$(PRE_TARGET) : 
	$(LD) -v -T../hal/cpu/vpos_kernel-ld-script -Bstatic *.o -o $@ -L /opt/s5pc1xx/cross/armv7a/lib/gcc/arm-s5pc1xx-linux-gnueabi/4.2.1 -L /opt/s5pc1xx/staging/armv7a-s5pc1xx-linux-gnueabi/usr/lib -lgcc -lc
#	$(LD) -r -Bstatic *.o -o libvpos.a -L/usr/local/arm_huins/lib/gcc-lib/arm-linux/3.2 -lgcc -lc
	
clean:
	rm -f $(OBJS)
	rm -f $(PRE_TARGET)
	rm -f $(IMAGE_TARGET)
	
dep:
	gccmakedep $(INCLUDE) $(LIB_SRCS)
