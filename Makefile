CC = gcc
CFLAGS = -Wall -Wextra -std=c11

SRC = raytracer.c
OBJ = $(SRC:.c=.o)
EXECUTABLE = raytracer

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(EXECUTABLE) $(OBJ)