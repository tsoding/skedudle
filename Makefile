CFLAGS=-Wall -Wextra -Wno-unused-result -pedantic -std=c11 -ggdb
CS=src/main.c src/schedule.c src/json.c src/utf8.c
HS=src/s.h src/request.h src/response.h src/error_page_template.h src/schedule.h src/json.h
LIBS=-lm

all: skedudle json_test json_check

skedudle: $(CS) $(HS)
	$(CC) $(CFLAGS) -o skedudle $(CS) $(LIBS)

tt: src/tt.c
	$(CC) $(CFLAGS) -o tt src/tt.c

src/error_page_template.h: tt src/error_page_template.h.tt
	./tt src/error_page_template.h.tt > src/error_page_template.h

json_test: src/json.c src/json_test.c src/s.h src/memory.h src/utf8.h src/utf8.c
	$(CC) $(CFLAGS) -o json_test src/json.c src/json_test.c src/utf8.c $(LIBS)

json_check: src/json.c src/json_check.c src/s.h src/memory.h src/utf8.h src/utf8.c
	$(CC) $(CFLAGS) -o json_check src/json.c src/json_check.c src/utf8.c $(LIBS)
