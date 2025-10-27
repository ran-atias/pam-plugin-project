# Name of the output shared library
TARGET = hook.so

# Compiler and flags
CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -shared -ldl

# Source file
SRC = hook.c

# Default rule
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

# Clean rule
clean:
	rm -f $(TARGET)


.PHONY: all clean run
