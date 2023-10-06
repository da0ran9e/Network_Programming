CC = gcc
CFLAGS = -Wall -Wextra
SRC = resolver.c 
OBJ = $(SRC:.c=.o)

.PHONY: all clean

all: resolver

resolver: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f resolver $(OBJ)