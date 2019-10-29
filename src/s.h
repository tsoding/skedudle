#ifndef S_H_
#define S_H_

#include <ctype.h>

typedef struct  {
    uint64_t len;
    const char *data;
} String;

// Construct String from NULL-terminated C string
static inline
String string_nt(const char *s)
{
    String result = {
        .len = strlen(s),
        .data = s
    };
    return result;
}

static inline
String chop_line(String *input)
{
    // TODO: is this guard even needed?
    if (input->len == 0) {
        String result = { .data = NULL, .len = 0 };
        return result;
    }

    uint64_t i = 0;
    while (i < input->len && input->data[i] != '\n')
        ++i;

    String line;
    line.data = input->data;
    line.len = i;

    if (i == input->len) {
        input->data += input->len;
        input->len = 0;
    } else {
        input->data += i + 1;
        input->len -= i + 1;
    }

    return line;
}

static inline
String trim_begin(String s)
{
    while (s.len && isspace(*s.data)) {
        s.data++;
        s.len--;
    }
    return s;
}

static inline
String trim_end(String s)
{
    while (s.len && isspace(s.data[s.len - 1])) {
        s.len--;
    }
    return s;
}

static inline
String chop_word(String *input)
{
    // TODO: is this guard even needed?
    if (input->len == 0) {
        String result = { .data = NULL, .len = 0 };
        return result;
    }

    *input = trim_begin(*input);

    uint64_t i = 0;
    while (i < input->len && !isspace(input->data[i])) {
        ++i;
    }

    String word;
    word.data = input->data;
    word.len = i;

    input->data += i;
    input->len -= i;

    return word;
}

int string_equal(String a, String b)
{
    if (a.len != b.len) return 0;
    return memcmp(a.data, b.data, a.len) == 0;
}

#endif  // S_H_
