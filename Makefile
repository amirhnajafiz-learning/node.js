# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -pedantic

# Source files
SRCS = safex.c

# Executable name
TARGET = safex

# Default target
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) -lptrace

# Clean target to remove the executable
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: all clean
