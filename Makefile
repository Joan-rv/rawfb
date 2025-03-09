CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -Werror -g
LDFLAGS=-lm
OBJ=main.o keyboard.o display.o

main: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $^
