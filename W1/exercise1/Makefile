CC = gcc
CFLAGS = -Wall -Wextra
SRC = HAZCHEM.c 
OBJ = $(SRC:.c=.o)

.PHONY: all clean

all: exercise1

exercise1: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f exercise1 $(OBJ)
