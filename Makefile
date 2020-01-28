CFLAGS=-Wall -Wextra -Wno-unused-result -pedantic -std=c11 -I./third_party/frozen/ -ggdb
CS=src/main.c src/schedule.c src/json.c
HS=src/s.h src/request.h src/response.h src/error_page_template.h src/schedule.h src/rest_map_template.h src/json.h
LIBS=

all: skedudle frozen_test json_test

skedudle: $(CS) $(HS) frozen.o
	clang $(CFLAGS) -o skedudle $(CS) frozen.o $(LIBS)

frozen_test: third_party/frozen/unit_test.c
	clang -std=c99 -O0 -fno-builtin -rdynamic -ggdb -Wall -pedantic -o frozen_test third_party/frozen/unit_test.c -lm

frozen.o: third_party/frozen/frozen.c
	clang -std=c99 -O0 -fno-builtin -rdynamic -ggdb -Wall -pedantic -c -o frozen.o third_party/frozen/frozen.c

tt: src/tt.c
	clang $(CFLAGS) -o tt src/tt.c

src/error_page_template.h: tt src/error_page_template.h.tt
	./tt src/error_page_template.h.tt > src/error_page_template.h

src/rest_map_template.h: tt src/rest_map_template.h.tt
	./tt src/rest_map_template.h.tt > src/rest_map_template.h

json_test: src/json.c src/json_test.c src/s.h src/memory.h
	clang $(CFLAGS) -o json_test src/json.c src/json_test.c
