CFLAGS=-Wall -Wextra -Werror -Wno-unused-result -pedantic -std=c11 -I./third_party/frozen/ -ggdb
CS=src/main.c src/schedule.c
HS=src/s.h src/request.h src/response.h src/error_page.h src/schedule.h
LIBS=

all: nodec schedule_test frozen_test

nodec: $(CS) $(HS) frozen.o
	gcc $(CFLAGS) -o nodec $(CS) frozen.o $(LIBS)

frozen_test: frozen.o third_party/frozen/unit_test.c
	gcc -std=c99 -g -O1 -W -Wall -pedantic -o frozen_test frozen.o third_party/frozen/unit_test.c

frozen.o: third_party/frozen/frozen.c
	gcc -std=c99 -g -O1 -W -Wall -pedantic -c -o frozen.o third_party/frozen/frozen.c

tt: src/tt.c
	gcc $(CFLAGS) -o tt src/tt.c

src/error_page.h: tt src/error_page.h.tt
	./tt src/error_page.h.tt > src/error_page.h

schedule_test: src/schedule_test.c src/schedule.c frozen.o
	gcc $(CFLAGS) -o schedule_test src/schedule_test.c src/schedule.c frozen.o
