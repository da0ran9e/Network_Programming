CC = gcc
CFLAGS = -Wall -Wextra
SRC = grading.c 
OBJ = $(SRC:.c=.o)
EXECUTABLE = exercise2

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(EXECUTABLE) $(OBJ)
