OBJS = HAL_arch_startup.o hal_swi_handler.o 

SRCS = HAL_arch_startup.S hal_swi_handler.c 


all : $(OBJS)
	mv *.o ../../objs

clean:
	rm -f *.o
	
dep:
	gccmakedep $(INCLUDE) $(LIB_SRCS)
