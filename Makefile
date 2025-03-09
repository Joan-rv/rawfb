CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -Werror -g
LDFLAGS=-lm
OBJ=main.o keyboard.o display.o

.PHONY: clean

main: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f main $(OBJ)
