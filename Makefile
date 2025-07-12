# Compiler and flags
CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lm

# Output binary name
TARGET = flip-sim

# Source file(s)
SRC = main.c

# Default target
all: $(TARGET)

# Build the binary
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Run the binary
run: $(TARGET)
	./$(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET)
