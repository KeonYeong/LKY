CC=gcc
CFLAGS= -g -O2 -Wall
LDFLAGS=-L./ -lbpt
LIB=libbpt.a
OBJS=marker.o

marker: $(OBJS) $(LIB)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $^ -c -o $@

clean:
	rm -f *.o marker
