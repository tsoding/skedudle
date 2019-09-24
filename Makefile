CFLAGS=-Wall -Wextra -Werror -Wconversion -Wunused-macros -pedantic -std=c11
LIBS=

nodec: main.c
	gcc $(CFLAGS) -o nodec main.c $(LIBS)
