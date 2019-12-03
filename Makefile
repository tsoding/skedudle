CFLAGS=-Wall -Wextra -Werror -Wconversion -Wno-unused-result -pedantic -std=c11
CS=src/main.c third_party/frozen/frozen.c
HS=src/s.h src/request.h src/response.h src/error_page.h third_party/frozen/frozen.h
LIBS=

nodec: $(CS) $(HS)
	gcc $(CFLAGS) -o nodec $(CS) $(LIBS)

tt: src/tt.c
	gcc $(CFLAGS) -o tt src/tt.c

src/error_page.h: tt src/error_page.h.tt
	./tt src/error_page.h.tt > src/error_page.h

.PHONY: clean

clean:
	rm -rf nodec tt

