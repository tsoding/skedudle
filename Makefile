CFLAGS=-Wall -Wextra -Wno-unused-result -pedantic -std=c11 -I./third_party/frozen/ -ggdb
CS=src/main.c src/schedule.c
HS=src/s.h src/request.h src/response.h src/error_page.h src/schedule.h src/rest_map.h
LIBS=

all: skedudle frozen_test

skedudle: $(CS) $(HS) frozen.o
	gcc $(CFLAGS) -o skedudle $(CS) frozen.o $(LIBS)

frozen_test: third_party/frozen/unit_test.c
	gcc -std=c99 -O0 -fno-builtin -rdynamic -ggdb -Wall -pedantic -o frozen_test third_party/frozen/unit_test.c -lm

frozen.o: third_party/frozen/frozen.c
	gcc -std=c99 -O0 -fno-builtin -rdynamic -ggdb -Wall -pedantic -c -o frozen.o third_party/frozen/frozen.c

tt: src/tt.c
	gcc $(CFLAGS) -o tt src/tt.c

src/error_page.h: tt src/error_page.h.tt
	./tt src/error_page.h.tt > src/error_page.h

src/rest_map.h: src/rest_map.h.tt
	./tt src/rest_map.h.tt > src/rest_map.h
