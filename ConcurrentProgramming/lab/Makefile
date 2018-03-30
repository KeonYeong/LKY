CC=g++
CFLAGS=-g -O3 --std=c++11

run: firstThreading.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f *.o run
