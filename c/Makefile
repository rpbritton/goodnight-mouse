CC = gcc
CFLAGS = -g -Wall
CFLAGS += `pkg-config --libs --cflags atspi-2`
CFLAGS += `pkg-config --libs --cflags x11`
TARGET = goodnight_mouse.o

SRC := $(wildcard ./src/*.c)
OBJ := $(addsuffix .o,$(basename $(SRC)))

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(CFLAGS)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJ)