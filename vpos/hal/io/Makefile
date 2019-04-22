OBJS = serial.o timer.o led.o 

SRCS = serial.c timer.c led.c

all : $(OBJS)
	mv *.o ../../objs
clean:
	rm -f *.o
	
dep:
	gccmakedep $(INCLUDE) $(LIB_SRCS)
