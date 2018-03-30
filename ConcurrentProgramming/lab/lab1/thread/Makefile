#compile option
CC = g++
CXXFLAGS = -g -Wall -std=c++11

#Specifying Paths for compile
SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:.cpp=.o)
BIN = ./bin/
INC = ./include/
LIB = ./lib/ -lpthread

#Pre-Processor
CPPFLAGS += -I$(INC)

#Compile Command
TARGET = prac_pthread
$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) $(CPPFLAGS) -o $(BIN)$(TARGET) $(OBJS) -L$(LIB)

#Delete Command
clean:
	rm $(BIN)$(TARGET) $(OBJS)
