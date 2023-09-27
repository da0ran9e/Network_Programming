CC = gcc
CFLAGS = -Wall -Wextra
SRC = grading.c 
OBJ = $(SRC:.c=.o)

.PHONY: all clean

all: exercise2

exercise1: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f exercise2 $(OBJ)
