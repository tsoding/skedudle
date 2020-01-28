#ifndef S_H_
#define S_H_

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef struct  {
    size_t len;
    const char *data;
} String;

static inline
String string(size_t len, const char *data)
{
    String result = {
        .len = len,
        .data = data
    };

    return result;
}

#define SLT(literal) string(sizeof(literal) - 1, literal)

static inline
String string_empty(void)
{
    String result = {
        .len = 0,
        .data = NULL
    };
    return result;
}

static inline
String chop_until_char(String *input, char delim)
{
    if (input->len == 0) {
        return string_empty();
    }

    size_t i = 0;
    while (i < input->len && input->data[i] != delim)
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
String chop_line(String *input) {
    return chop_until_char(input, '\n');
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
String trim(String s)
{
    return trim_begin(trim_end(s));
}

static inline
String chop_word(String *input)
{
    if (input->len == 0) {
        return string_empty();
    }

    *input = trim_begin(*input);

    size_t i = 0;
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

static inline
int string_equal(String a, String b)
{
    if (a.len != b.len) return 0;
    return memcmp(a.data, b.data, a.len) == 0;
}

static inline
String take(String s, size_t n)
{
    if (s.len < n) return s;
    return (String) {
        .len = n,
        .data = s.data
    };
}

static inline
String drop(String s, size_t n)
{
    if (s.len < n) return SLT("");
    return (String) {
        .len = s.len - n,
        .data = s.data + n
    };
}

#endif  // S_H_
