CFLAGS=-Wall -Wextra -Werror -Wconversion -pedantic -std=c11
LIBS=

nodec: main.c
	gcc $(CFLAGS) -o nodec main.c $(LIBS)
