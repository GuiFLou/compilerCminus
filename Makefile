
CC=gcc
CFLAGS = -std=c99 -Wall -Wextra \
         -Wno-unused-variable -Wno-unused-function \
         -Wno-switch -Wno-parentheses

SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)

BIN=compilador

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(BIN)
