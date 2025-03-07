CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -Werror -g
OBJ=main.o

main: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $^
