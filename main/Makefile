INC_PATH = ../inc
SRC_PATH = ../src
CXX = g++
CC = $(CXX)

CFLAGS = -Wall -Werror
CFLAGS+= -Wextra
CFLAGS+= -pedantic
CFLAGS+= -O3
CFLAGS+= -DDEBUG
CFLAGS+= -g

CXXFLAGS = $(CFLAGS)
CXXFLAGS += -std=c++17

# flags for preprocessor
CPPFLAGS = -I$(INC_PATH)/

OBJECTS = $(SRC_PATH)/char_str.o $(SRC_PATH)/timer.o $(SRC_PATH)/server_buffer.o $(SRC_PATH)/server_data.o \
			$(SRC_PATH)/server.o $(SRC_PATH)/topic.o $(SRC_PATH)/client.o utest.o

TARGET = utest

all: utest

utest : $(OBJECTS)

vala: $(TARGET)
	valgrind --leak-check=yes -s ./$(TARGET)

clean:
	@$(RM) $(TARGET) $(OBJECTS)

.PHONY: clean all check


# dependencies
char_str.o: $(INC_PATH)/char_str.hpp
client.o: $(INC_PATH)/client.hpp
topic.o: $(INC_PATH)/topic.hpp
server.o: $(INC_PATH)/server.hpp $(INC_PATH)/tcp_server.hpp
server_data.o: $(INC_PATH)/server_data.hpp
server_buffer.o: $(INC_PATH)/server_buffer.hpp
timer.o: $(INC_PATH)/timer.hpp
utest.o: $(INC_PATH)/server.hpp
