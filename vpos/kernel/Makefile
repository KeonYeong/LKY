
OBJS = kernel_start.o machine_init.o exception_handler.o 

SRCS = kernel_start.c machine_init.c exception_handler.c 

all : $(OBJS)
	mv *.o ../objs

clean:
	rm -f *.o
	
dep:
	gccmakedep $(INCLUDE) $(LIB_SRCS)

