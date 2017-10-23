#compile option
CC = g++
CXXFLAGS = -g -Wall -std=c++11

#Specifying Paths for compile
SRCS := $(wildcard src/*.cc)
OBJS := $(SRCS:.cc=.o)
BIN = ./bin/
INC = ./include/
LIB = ./lib/ -lpthread

#Pre-Processor
CPPFLAGS += -I$(INC)

#Compile Command
TARGET = print_hello
$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) $(CPPFLAGS) -o $(BIN)$(TARGET) $(OBJS) -L$(LIB)

#Delete Command
clean:
	rm $(BIN)$(TARGET) $(OBJS)
