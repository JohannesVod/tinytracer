UNAME_S = $(shell uname -s)

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Wstrict-aliasing
CFLAGS += -Wno-pointer-arith -Wno-newline-eof -Wno-unused-parameter -Wno-gnu-statement-expression
CFLAGS += -Wno-gnu-compound-literal-initializer -Wno-gnu-zero-variadic-macro-arguments
CFLAGS += -Ilib/stb
LDFLAGS = -lm

# Add OpenMP flag
CFLAGS += -fopenmp
LDFLAGS += -fopenmp

BIN = bin
SRC = $(wildcard src/**/*.c) $(wildcard src/*.c) $(wildcard src/**/**/*.c) $(wildcard src/**/**/**/*.c)
OBJ = $(subst src, $(BIN), $(SRC:.c=.o))

.PHONY: all clean debug release

all: fast

dirs:
	mkdir -p ./$(BIN)

run: all
	$(BIN)/fancytracer

debug: CFLAGS += -g -O2
debug: clean dirs fancytracer

fast: CFLAGS += -O2
fast: clean dirs fancytracer

# release: CFLAGS += -O3
# release: clean dirs fancytracer

fancytracer: $(OBJ)
	$(CC) -o $(BIN)/fancytracer $^ $(LDFLAGS)

$(BIN)/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf $(BIN) $(OBJ)
