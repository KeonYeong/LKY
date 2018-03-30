CC=g++
CFLAGS=-g -O3 --std=c++11

run: signalFinder_final.o
	$(CC) $(CFLAGS) $^ -o $@ -lpthread
%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@ -lpthread

clean:
	rm -f *.o run
