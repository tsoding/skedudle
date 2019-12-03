CFLAGS=-Wall -Wextra -Werror -Wconversion -Wno-unused-result -pedantic -std=c11
CS=src/main.c
HS=src/s.h src/request.h src/response.h src/error_page.h
LIBS=

nodec: $(CS) $(HS) frozen.o
	gcc $(CFLAGS) -o nodec $(CS) frozen.o $(LIBS)

frozen.o: third_party/frozen/frozen.c
	gcc -std=c99 -g -O1 -W -Wall -pedantic -c -o frozen.o third_party/frozen/frozen.c

tt: src/tt.c
	gcc $(CFLAGS) -o tt src/tt.c

src/error_page.h: tt src/error_page.h.tt
	./tt src/error_page.h.tt > src/error_page.h

.PHONY: clean

clean:
	rm -rf nodec tt

