CFLAGS=-Wall -Wextra -Wno-unused-result -pedantic -std=c11 -I./third_party/frozen/ -ggdb
CS=src/main.c src/schedule.c src/json.c src/utf8.c
HS=src/s.h src/request.h src/response.h src/error_page_template.h src/schedule.h src/rest_map_template.h src/json.h
LIBS=-lm

all: skedudle frozen_test json_test json_check

skedudle: $(CS) $(HS) frozen.o
	$(CC) $(CFLAGS) -o skedudle $(CS) frozen.o $(LIBS)

frozen_test: third_party/frozen/unit_test.c
	$(CC) -std=c99 -O0 -fno-builtin -rdynamic -ggdb -Wall -pedantic -o frozen_test third_party/frozen/unit_test.c -lm

frozen.o: third_party/frozen/frozen.c
	$(CC) -std=c99 -O0 -fno-builtin -rdynamic -ggdb -Wall -pedantic -c -o frozen.o third_party/frozen/frozen.c

tt: src/tt.c
	$(CC) $(CFLAGS) -o tt src/tt.c

src/error_page_template.h: tt src/error_page_template.h.tt
	./tt src/error_page_template.h.tt > src/error_page_template.h

src/rest_map_template.h: tt src/rest_map_template.h.tt
	./tt src/rest_map_template.h.tt > src/rest_map_template.h

json_test: src/json.c src/json_test.c src/s.h src/memory.h src/utf8.h src/utf8.c
	$(CC) $(CFLAGS) -o json_test src/json.c src/json_test.c src/utf8.c $(LIBS)

json_check: src/json.c src/json_check.c src/s.h src/memory.h src/utf8.h src/utf8.c
	$(CC) $(CFLAGS) -o json_check src/json.c src/json_check.c src/utf8.c $(LIBS)
