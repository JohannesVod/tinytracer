UNAME_S = $(shell uname -s)

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Wstrict-aliasing
CFLAGS += -Wno-pointer-arith -Wno-newline-eof -Wno-unused-parameter -Wno-gnu-statement-expression
CFLAGS += -Wno-gnu-compound-literal-initializer -Wno-gnu-zero-variadic-macro-arguments
CFLAGS += -Ilib/stb
LDFLAGS = -lm

SRC  = fancytracer.c
OBJ  = $(SRC:.c=.o)
BIN = bin

.PHONY: all clean run debug release

all: release

dirs:
	@mkdir -p ./$(BIN)

run: all
	$(BIN)/fancytracer

debug: CFLAGS += -g -O0
debug: clean dirs $(BIN)

release: CFLAGS += -O3
release: clean dirs $(BIN)

fancytracer: $(OBJ)
	$(CC) -o $(BIN)/fancytracer $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf $(BIN) $(OBJ)