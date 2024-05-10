CC = gcc
CFLAGS = -std=c11 -O3 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing
CFLAGS += -Wno-pointer-arith -Wno-newline-eof -Wno-unused-parameter -Wno-gnu-statement-expression
CFLAGS += -Wno-gnu-compound-literal-initializer -Wno-gnu-zero-variadic-macro-arguments
CFLAGS += -Ilib/stb

LDFLAGS = -lm

SRC  = fancytracer.c
OBJ  = $(SRC:.c=.o)
BIN = bin

.PHONY: all clean run

all: dirs fancytracer

dirs:
	mkdir -p ./$(BIN)

run: fancytracer
	$(BIN)/fancytracer

fancytracer: $(OBJ)
	$(CC) -o $(BIN)/fancytracer $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf $(BIN) $(OBJ)
