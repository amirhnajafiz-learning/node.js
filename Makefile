# Makefile for safex

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -pedantic

# Source files
SRCS = safex.c file_access.c child_trace.c safenoread.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = safex

# Default target
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lptrace

# Rule for compiling object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target to remove the executable and object files
clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: all clean
