#include <stdio.h>
#include "json.h"
#include "utf8.h"

Json_Value json_null = { .type = JSON_NULL };
Json_Value json_true = { .type = JSON_BOOLEAN, .boolean = 1 };
Json_Value json_false = { .type = JSON_BOOLEAN, .boolean = 0 };

static
Json_Result parse_json_value_impl(Memory *memory, String source, int level);

int json_isspace(char c)
{
    return c == 0x20 || c == 0x0A || c == 0x0D || c == 0x09;
}

String json_trim_begin(String s)
{
    while (s.len && json_isspace(*s.data)) {
        s.data++;
        s.len--;
    }
    return s;
}

void json_array_push(Memory *memory, Json_Array *array, Json_Value value)
{
    assert(memory);
    assert(array);

    if (array->begin == NULL) {
        assert(array->end == NULL);
        array->begin = memory_alloc(memory, sizeof(Json_Array_Page));
        array->end = array->begin;
        memset(array->begin, 0, sizeof(Json_Array_Page));
    }

    if (array->end->size >= JSON_ARRAY_PAGE_CAPACITY) {
        Json_Array_Page *next = memory_alloc(memory, sizeof(Json_Array_Page));
        memset(next, 0, sizeof(Json_Array_Page));
        array->end->next = next;
        array->end = next;
    }

    assert(array->end->size < JSON_ARRAY_PAGE_CAPACITY);

    array->end->elements[array->end->size++] = value;
}

void json_object_push(Memory *memory, Json_Object *object, String key, Json_Value value)
{
    assert(memory);
    assert(object);

    if (object->begin == NULL) {
        assert(object->end == NULL);
        object->begin = memory_alloc(memory, sizeof(Json_Object_Page));
        object->end = object->begin;
        memset(object->begin, 0, sizeof(Json_Object_Page));
    }

    if (object->end->size >= JSON_OBJECT_PAGE_CAPACITY) {
        Json_Object_Page *next = memory_alloc(memory, sizeof(Json_Object_Page));
        memset(next, 0, sizeof(Json_Object_Page));
        object->end->next = next;
        object->end = next;
    }

    assert(object->end->size < JSON_OBJECT_PAGE_CAPACITY);

    object->end->elements[object->end->size].key = key;
    object->end->elements[object->end->size].value = value;
    object->end->size += 1;
}

int64_t stoi64(String integer)
{
    if (integer.len == 0) {
        return 0;
    }

    int64_t result = 0;
    int64_t sign = 1;

    if (*integer.data == '-') {
        sign = -1;
        chop(&integer, 1);
    } else if (*integer.data == '+') {
        sign = 1;
        chop(&integer, 1);
    }

    while (integer.len) {
        assert(isdigit(*integer.data));
        result = result * 10 + (*integer.data - '0');
        chop(&integer, 1);
    }

    return result * sign;
}

int64_t json_number_to_integer(Json_Number number)
{
    int64_t exponent = stoi64(number.exponent);
    int64_t result = stoi64(number.integer);

    if (exponent > 0) {
        int64_t sign = result >= 0 ? 1 : -1;

        for (; exponent > 0; exponent -= 1) {
            int64_t x = 0;

            if (number.fraction.len) {
                x = *number.fraction.data - '0';
                chop(&number.fraction, 1);
            }

            result = result * 10 + sign * x;
        }
    }

    for (; exponent < 0 && result; exponent += 1) {
        result /= 10;
    }

    return result;
}

static Json_Result parse_token(String source, String token,
                               Json_Value value,
                               const char *message)
{
    if (string_equal(take(source, token.len), token)) {
        return (Json_Result) {
            .rest = drop(source, token.len),
            .value = value
        };
    }

    return (Json_Result) {
        .is_error = 1,
        .message = message,
        .rest = source
    };
}

static Json_Result parse_json_number(String source)
{
    String integer = {0};
    String fraction = {0};
    String exponent = {0};

    integer.data = source.data;

    if (source.len && *source.data == '-') {
        integer.len += 1;
        chop(&source, 1);
    }

    while (source.len && isdigit(*source.data)) {
        integer.len += 1;
        chop(&source, 1);
    }

    // TODO(#34): empty integer with fraction is not taken into account
    if (integer.len == 0
        || string_equal(integer, SLT("-"))
        || (integer.len > 1 && *integer.data == '0')
        || (integer.len > 2 && prefix_of(SLT("-0"), integer))) {
        return (Json_Result) {
            .is_error = 1,
            .rest = source,
            .message = "Incorrect number literal"
        };
    }

    if (source.len && *source.data == '.') {
        chop(&source, 1);
        fraction.data = source.data;

        while (source.len && isdigit(*source.data)) {
            fraction.len  += 1;
            chop(&source, 1);
        }

        if (fraction.len == 0) {
            return (Json_Result) {
                .is_error = 1,
                .rest = source,
                .message = "Incorrect number literal"
            };
        }
    }

    if (source.len && tolower(*source.data) == 'e') {
        chop(&source, 1);

        exponent.data = source.data;

        if (source.len && (*source.data == '-' || *source.data == '+')) {
            exponent.len  += 1;
            chop(&source, 1);
        }

        while (source.len && isdigit(*source.data)) {
            exponent.len  += 1;
            chop(&source, 1);
        }

        if (exponent.len == 0 ||
            string_equal(exponent, SLT("-")) ||
            string_equal(exponent, SLT("+"))) {
            return (Json_Result) {
                .is_error = 1,
                .rest = source,
                .message = "Incorrect number literal"
            };
        }
    }

    return (Json_Result) {
        .value = {
            .type = JSON_NUMBER,
            .number = {
                .integer = integer,
                .fraction = fraction,
                .exponent = exponent
            }
        },
        .rest = source
    };
}

static Json_Result parse_json_string_literal(String source)
{
    if (source.len == 0 || *source.data != '"') {
        return (Json_Result) {
            .is_error = 1,
            .rest = source,
            .message = "Expected '\"'",
        };
    }

    chop(&source, 1);

    String s = {
        .data = source.data,
        .len = 0
    };

    while (source.len && *source.data != '"') {
        if (*source.data == '\\') {
            s.len++;
            chop(&source, 1);

            if (source.len == 0) {
                return (Json_Result) {
                    .is_error = 1,
                    .rest = source,
                    .message = "Unfinished escape sequence",
                };
            }
        }

        s.len++;
        chop(&source, 1);
    }

    if (source.len == 0) {
        return (Json_Result) {
            .is_error = 1,
            .rest = source,
            .message = "Expected '\"'",
        };
    }

    chop(&source, 1);

    return (Json_Result) {
        .value = {
            .type = JSON_STRING,
            .string = s
        },
        .rest = source
    };
}

static int32_t unhex(char x)
{
    x = tolower(x);

    if ('0' <= x && x <= '9') {
        return x - '0';
    } else if ('a' <= x && x <= 'f') {
        return x - 'a' + 10;
    }

    return -1;
}

static Json_Result parse_escape_sequence(Memory *memory, String source)
{
    static char unescape_map[][2] = {
        {'b', '\b'},
        {'f', '\f'},
        {'n', '\n'},
        {'r', '\r'},
        {'t', '\t'},
        {'/', '/'},
        {'\\', '\\'},
        {'"', '"'},
    };
    static const size_t unescape_map_size =
        sizeof(unescape_map) / sizeof(unescape_map[0]);

    if (source.len == 0 || *source.data != '\\') {
        return (Json_Result) {
            .is_error = 1,
            .rest = source,
            .message = "Expected '\\'",
        };
    }
    chop(&source, 1);

    if (source.len <= 0) {
        return (Json_Result) {
            .is_error = 1,
            .rest = source,
            .message = "Unfinished escape sequence",
        };
    }

    for (size_t i = 0; i < unescape_map_size; ++i) {
        if (unescape_map[i][0] == *source.data) {
            return (Json_Result) {
                .rest = drop(source, 1),
                .value = {
                    .type = JSON_STRING,
                    .string = {
                        .len = 1,
                        .data = &unescape_map[i][1]
                    }
                }
            };
        }
    }

    if (*source.data != 'u') {
        return (Json_Result) {
            .is_error = 1,
            .rest = source,
            .message = "Unknown escape sequence"
        };
    }
    chop(&source, 1);

    if (source.len < 4) {
        return (Json_Result) {
            .is_error = 1,
            .rest = source,
            .message = "Incomplete unicode point escape sequence"
        };
    }

    uint32_t rune = 0;
    for (int i = 0; i < 4; ++i) {
        int32_t x = unhex(*source.data);
        if (x < 0) {
            return (Json_Result) {
                .is_error = 1,
                .rest = source,
                .message = "Incorrect hex digit"
            };
        }
        rune = rune * 0x10 + x;
        chop(&source, 1);
    }

    if (0xD800 <= rune && rune <= 0xDBFF) {
        if (source.len < 6) {
            return (Json_Result) {
                .is_error = 1,
                .rest = source,
                .message = "Unfinished surrogate pair"
            };
        }

        if (*source.data != '\\') {
            return (Json_Result) {
                .is_error = 1,
                .rest = source,
                .message = "Unfinished surrogate pair. Expected '\\'",
            };
        }
        chop(&source, 1);

        if (*source.data != 'u') {
            return (Json_Result) {
                .is_error = 1,
                .rest = source,
                .message = "Unfinished surrogate pair. Expected 'u'",
            };
        }
        chop(&source, 1);

        uint32_t surrogate = 0;
        for (int i = 0; i < 4; ++i) {
            int32_t x = unhex(*source.data);
            if (x < 0) {
                return (Json_Result) {
                    .is_error = 1,
                        .rest = source,
                        .message = "Incorrect hex digit"
                        };
            }
            surrogate = surrogate * 0x10 + x;
            chop(&source, 1);
        }

        if (!(0xDC00 <= surrogate && surrogate <= 0xDFFF)) {
            return (Json_Result) {
                .is_error = 1,
                .rest = source,
                .message = "Invalid surrogate pair"
            };
        }

        rune = 0x10000 + (((rune - 0xD800) << 10) |(surrogate - 0xDC00));
    }

    if (rune > 0x10FFFF) {
        rune = 0xFFFD;
    }

    Utf8_Chunk utf8_chunk = utf8_encode_rune(rune);
    assert(utf8_chunk.size > 0);

    char *data = memory_alloc(memory, utf8_chunk.size);
    memcpy(data, utf8_chunk.buffer, utf8_chunk.size);

    return (Json_Result){
        .value = {
            .type = JSON_STRING,
            .string = {
                .len = utf8_chunk.size,
                .data = data
            }
        },
        .rest = source
    };
}

static Json_Result parse_json_string(Memory *memory, String source)
{
    Json_Result result = parse_json_string_literal(source);
    if (result.is_error) return result;
    assert(result.value.type == JSON_STRING);

    const size_t buffer_capacity = result.value.string.len;
    source = result.value.string;
    String rest = result.rest;

    char *buffer = memory_alloc(memory, buffer_capacity);
    size_t buffer_size = 0;

    while (source.len) {
        if (*source.data == '\\') {
            result = parse_escape_sequence(memory, source);
            if (result.is_error) return result;
            assert(result.value.type == JSON_STRING);
            assert(buffer_size + result.value.string.len <= buffer_capacity);
            memcpy(buffer + buffer_size,
                   result.value.string.data,
                   result.value.string.len);
            buffer_size += result.value.string.len;

            source = result.rest;
        } else {
            // TODO(#37): json parser is not aware of the input encoding
            assert(buffer_size < buffer_capacity);
            buffer[buffer_size++] = *source.data;
            chop(&source, 1);
        }
    }

    return (Json_Result) {
        .value = {
            .type = JSON_STRING,
            .string = {
                .data = buffer,
                .len = buffer_size
            },
        },
        .rest = rest
    };
}

static Json_Result parse_json_array(Memory *memory, String source, int level)
{
    assert(memory);

    if(source.len == 0 || *source.data != '[') {
        return (Json_Result) {
            .is_error = 1,
            .rest = source,
            .message = "Expected '['",
        };
    }

    chop(&source, 1);

    source = json_trim_begin(source);

    if (source.len == 0) {
        return (Json_Result) {
            .is_error = 1,
            .rest = source,
            .message = "Expected ']'",
        };
    } else if(*source.data == ']') {
        return (Json_Result) {
            .value = { .type = JSON_ARRAY },
            .rest = drop(source, 1)
        };
    }

    Json_Array array = {0};

    while(source.len > 0) {
        Json_Result item_result = parse_json_value_impl(memory, source, level + 1);
        if(item_result.is_error) {
            return item_result;
        }

        json_array_push(memory, &array, item_result.value);

        source = json_trim_begin(item_result.rest);

        if (source.len == 0) {
            return (Json_Result) {
                .is_error = 1,
                .rest = source,
                .message = "Expected ']' or ','",
            };
        }

        if (*source.data == ']') {
            return (Json_Result) {
                .value = {
                    .type = JSON_ARRAY,
                    .array = array
                },
                .rest = drop(source, 1)
            };
        }

        if (*source.data != ',') {
            return (Json_Result) {
                .is_error = 1,
                .rest = source,
                .message = "Expected ']' or ','",
            };
        }

        source = json_trim_begin(drop(source, 1));
    }

    return (Json_Result) {
        .is_error = 1,
        .rest = source,
        .message = "EOF",
    };
}

static Json_Result parse_json_object(Memory *memory, String source, int level)
{
    assert(memory);

    if (source.len == 0 || *source.data != '{') {
        return (Json_Result) {
            .is_error = 1,
            .rest = source,
            .message = "Expected '{'"
        };
    }

    chop(&source, 1);

    source = json_trim_begin(source);

    if (source.len == 0) {
        return (Json_Result) {
            .is_error = 1,
            .rest = source,
            .message = "Expected '}'"
        };
    } else if (*source.data == '}') {
        return (Json_Result) {
            .value = { .type = JSON_OBJECT },
            .rest = drop(source, 1)
        };
    }

    Json_Object object = {0};

    while (source.len > 0) {
        source = json_trim_begin(source);

        Json_Result key_result = parse_json_string(memory, source);
        if (key_result.is_error) {
            return key_result;
        }
        source = json_trim_begin(key_result.rest);

        if (source.len == 0 || *source.data != ':') {
            return (Json_Result) {
                .is_error = 1,
                .rest = source,
                .message = "Expected ':'"
            };
        }

        chop(&source, 1);

        Json_Result value_result = parse_json_value_impl(memory, source, level + 1);
        if (value_result.is_error) {
            return value_result;
        }
        source = json_trim_begin(value_result.rest);

        assert(key_result.value.type == JSON_STRING);
        json_object_push(memory, &object, key_result.value.string, value_result.value);

        if (source.len == 0) {
            return (Json_Result) {
                .is_error = 1,
                .rest = source,
                .message = "Expected '}' or ','",
            };
        }

        if (*source.data == '}') {
            return (Json_Result) {
                .value = {
                    .type = JSON_OBJECT,
                    .object = object
                },
                .rest = drop(source, 1)
            };
        }

        if (*source.data != ',') {
            return (Json_Result) {
                .is_error = 1,
                .rest = source,
                .message = "Expected '}' or ','",
            };
        }

        source = drop(source, 1);
    }

    return (Json_Result) {
        .is_error = 1,
        .rest = source,
        .message = "EOF",
    };
}

static
Json_Result parse_json_value_impl(Memory *memory, String source, int level)
{
    if (level >= JSON_DEPTH_MAX_LIMIT) {
        return (Json_Result) {
            .is_error = 1,
            .message = "Reach the max limit of depth",
            .rest = source
        };
    }

    source = json_trim_begin(source);

    if (source.len == 0) {
        return (Json_Result) {
            .is_error = 1,
            .message = "EOF",
            .rest = source
        };
    }

    switch (*source.data) {
    case 'n': return parse_token(source, SLT("null"), json_null, "Expected `null`");
    case 't': return parse_token(source, SLT("true"), json_true, "Expected `true`");
    case 'f': return parse_token(source, SLT("false"), json_false, "Expected `false`");
    case '"': return parse_json_string(memory, source);
    case '[': return parse_json_array(memory, source, level);
    case '{': return parse_json_object(memory, source, level);
    }

    return parse_json_number(source);
}

Json_Result parse_json_value(Memory *memory, String source)
{
    return parse_json_value_impl(memory, source, 0);
}

static
void print_json_null(FILE *stream)
{
    fprintf(stream, "null");
}

static
void print_json_boolean(FILE *stream, int boolean)
{
    if (boolean) {
        fprintf(stream, "true");
    } else {
        fprintf(stream, "false");
    }
}

static
void print_json_number(FILE *stream, Json_Number number)
{
    fwrite(number.integer.data, 1, number.integer.len, stream);

    if (number.fraction.len > 0) {
        fputc('.', stream);
        fwrite(number.fraction.data, 1, number.fraction.len, stream);
    }

    if (number.exponent.len > 0) {
        fputc('e', stream);
        fwrite(number.exponent.data, 1, number.exponent.len, stream);
    }
}

static
int json_get_utf8_char_len(unsigned char ch) {
    if ((ch & 0x80) == 0) return 1;
    switch (ch & 0xf0) {
        case 0xf0:
            return 4;
        case 0xe0:
            return 3;
        default:
            return 2;
    }
}

static
void print_json_string(FILE *stream, String string)
{
    const char *hex_digits = "0123456789abcdef";
    const char *specials = "btnvfr";
    const char *p = string.data;

    fputc('"', stream);
    size_t cl;
    for (size_t i = 0; i < string.len; i++) {
        unsigned char ch = ((unsigned char *) p)[i];
        if (ch == '"' || ch == '\\') {
            fwrite("\\", 1, 1, stream);
            fwrite(p + i, 1, 1, stream);
        } else if (ch >= '\b' && ch <= '\r') {
            fwrite("\\", 1, 1, stream);
            fwrite(&specials[ch - '\b'], 1, 1, stream);
        } else if (isprint(ch)) {
            fwrite(p + i, 1, 1, stream);
        } else if ((cl = json_get_utf8_char_len(ch)) == 1) {
            fwrite("\\u00", 1, 4, stream);
            fwrite(&hex_digits[(ch >> 4) % 0xf], 1, 1, stream);
            fwrite(&hex_digits[ch % 0xf], 1, 1, stream);
        } else {
            fwrite(p + i, 1, cl, stream);
            i += cl - 1;
        }
    }
    fputc('"', stream);
}

static
void print_json_array(FILE *stream, Json_Array array)
{
    fprintf(stream, "[");
    int t = 0;
    for (Json_Array_Page *page = array.begin; page != NULL; page = page->next) {
        for (size_t i = 0; i < page->size; ++i) {
            if (t) {
                printf(",");
            } else {
                t = 1;
            }
            print_json_value(stream, page->elements[i]);
        }
    }
    fprintf(stream, "]");
}

void print_json_object(FILE *stream, Json_Object object)
{
    fprintf(stream, "{");
    int t = 0;
    for (Json_Object_Page *page = object.begin; page != NULL; page = page->next) {
        for (size_t i = 0; i < page->size; ++i) {
            if (t) {
                printf(",");
            } else {
                t = 1;
            }
            print_json_string(stream, page->elements[i].key);
            fprintf(stream, ":");
            print_json_value(stream, page->elements[i].value);
        }
    }
    fprintf(stream, "}");
}

void print_json_value(FILE *stream, Json_Value value)
{
    switch (value.type) {
    case JSON_NULL: {
        print_json_null(stream);
    } break;
    case JSON_BOOLEAN: {
        print_json_boolean(stream, value.boolean);
    } break;
    case JSON_NUMBER: {
        print_json_number(stream, value.number);
    } break;
    case JSON_STRING: {
        print_json_string(stream, value.string);
    } break;
    case JSON_ARRAY: {
        print_json_array(stream, value.array);
    } break;
    case JSON_OBJECT: {
        print_json_object(stream, value.object);
    } break;
    }
}

void print_json_error(FILE *stream, Json_Result result,
                      String source, const char *prefix)
{
    assert(stream);
    assert(source.data <= result.rest.data);

    size_t n = result.rest.data - source.data;

    for (size_t line_number = 1; source.len; ++line_number) {
        String line = chop_line(&source);

        if (n <= line.len) {
            fprintf(stream, "%s:%ld: %s\n", prefix, line_number, result.message);
            fwrite(line.data, 1, line.len, stream);
            fputc('\n', stream);

            for (size_t j = 0; j < n; ++j) {
                fputc(' ', stream);
            }
            fputc('^', stream);
            fputc('\n', stream);
            break;
        }

        n -= line.len + 1;
    }

    for (int i = 0; source.len && i < 3; ++i) {
        String line = chop_line(&source);
        fwrite(line.data, 1, line.len, stream);
        fputc('\n', stream);
    }
}

void print_json_value_fd(int fd, Json_Value value)
{
    (void) fd;
    (void) value;
    assert(!"TODO: print_json_value_fd is not implemented");
}
