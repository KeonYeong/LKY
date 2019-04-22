
OBJS = vpsh.o 

SRCS = vpsh.c

all : $(OBJS)
	mv *.o ../objs

clean:
	rm -f *.o
	
dep:
	gccmakedep $(INCLUDE) $(LIB_SRCS)
