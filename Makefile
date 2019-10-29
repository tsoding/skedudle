CFLAGS=-Wall -Wextra -Werror -Wconversion -pedantic -std=c11
CS=src/main.c
HS=src/s.h src/request.h src/response.h
LIBS=

nodec: $(CS) $(HS)
	gcc $(CFLAGS) -o nodec $(CS) $(LIBS)

