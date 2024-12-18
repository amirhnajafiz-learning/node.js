# compiler
CC = gcc

# compiler flags
CFLAGS = -Wall -Wextra -pedantic

# source files
SRCS = src/main.c src/helpers.c

# executable name
TARGET = safex

# default target
all: $(TARGET)

# rule to build the executable
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# clean target to remove the executable
clean:
	rm -f $(TARGET)
	rm -rf bin

# building test modules
test:
	mkdir bin
	$(CC) $(CFLAGS) -o bin/bad tests/bad.c
	$(CC) $(CFLAGS) -o bin/normal tests/normal.c
	$(CC) $(CFLAGS) -o bin/normalfork tests/normalfork.c
	$(CC) $(CFLAGS) -o bin/badfork tests/badfork.c
	$(CC) $(CFLAGS) -o bin/trace_fork tests/trace_fork.c
	$(CC) $(CFLAGS) -o bin/openat tests/openat.c
	$(CC) $(CFLAGS) -o bin/user tests/user.c
	$(CC) $(CFLAGS) -o bin/multitrace tests/multitrace.c

# phony targets
.PHONY: all clean test
