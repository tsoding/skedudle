#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "s.h"

String file_as_content(const char *filepath) {
    assert(filepath);

    FILE *f = fopen(filepath, "rb");
    assert(f);

    fseek(f, 0, SEEK_END);
    long m = ftell(f);
    assert(m >= 0);
    fseek(f, 0, SEEK_SET);
    char *buffer = calloc(1, sizeof(char) * (size_t) m + 1);
    assert(buffer);

    size_t n = fread(buffer, 1, (size_t) m, f);
    assert(n == (size_t) m);

    fclose(f);
    return string(n, buffer);
}

void compile_c_code(String s) {
    printf("%.*s\n", (int) s.len, s.data);
}

void compile_byte_array(String s) {
    printf("write(OUT, \"");
    for (uint64_t i = 0; i < s.len; ++i) {
        printf("\\x%02x", s.data[i]);
    }
    printf("\", %lu);\n", s.len);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: ./tt <template.h.tt>\n");
        return 1;
    }
    const char *filepath = argv[1];
    String template = file_as_content(filepath);
    int c_code_mode = 0;
    while (template.len) {
        String token = chop_until_char(&template, '%');
        if (c_code_mode) {
            compile_c_code(token);
        } else {
            compile_byte_array(token);
        }
        c_code_mode = !c_code_mode;
    }

    return 0;
}
