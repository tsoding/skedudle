CFLAGS=-Wall -Wextra -Werror -Wconversion -pedantic -std=c11
CS=src/main.c
LIBS=

nodec: $(CS)
	gcc $(CFLAGS) -o nodec $(CS) $(LIBS)

