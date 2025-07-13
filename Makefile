# Compiler and flags
CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lm

# SDL flags
SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LDFLAGS = $(shell sdl2-config --libs) -lSDL2_ttf -lm

# Output binary names
TERM_TARGET = flip-sim
SDL_TARGET = flip-sim-sdl

# Source files
TERM_SRC = main.c
SDL_SRC = main-sdl.c

# Default: build both
all: $(TERM_TARGET) $(SDL_TARGET)

# Build terminal version
$(TERM_TARGET): $(TERM_SRC)
	$(CC) $(CFLAGS) -o $(TERM_TARGET) $(TERM_SRC) $(LDFLAGS)

# Build SDL version with SDL2_ttf
$(SDL_TARGET): $(SDL_SRC)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $(SDL_TARGET) $(SDL_SRC) $(SDL_LDFLAGS)

# Run terminal version
run-term: $(TERM_TARGET)
	./$(TERM_TARGET)

# Run SDL version
run-sdl: $(SDL_TARGET)
	./$(SDL_TARGET)

# Clean build artifacts
clean:
	rm -f $(TERM_TARGET) $(SDL_TARGET)
