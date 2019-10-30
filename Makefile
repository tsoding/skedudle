CFLAGS=-Wall -Wextra -Werror -Wconversion -pedantic -std=c11
CS=src/main.c
HS=src/s.h src/request.h src/response.h
LIBS=

all: tt nodec

tt: src/tt.c
	gcc $(CFLAGS) -o tt src/tt.c

nodec: $(CS) $(HS)
	gcc $(CFLAGS) -o nodec $(CS) $(LIBS)

.PHONY: clean

clean:
	rm -rf nodec tt

