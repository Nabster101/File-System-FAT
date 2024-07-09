# Makefile for the FAT file system project

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -I.

# Source files and executable name
SRC = main.c fs.c
EXEC = fscli

# Default rule
all: $(EXEC)

# Rule to build the executable
$(EXEC): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to clean up the build files
clean:
	rm -f $(EXEC) fs.img

.PHONY: all clean
