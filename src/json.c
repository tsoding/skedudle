#include "json.h"

static Json_Value json_null = { .type = JSON_NULL };
static Json_Value json_true = { .type = JSON_BOOLEAN, .boolean = 1 };
static Json_Value json_false = { .type = JSON_BOOLEAN, .boolean = 0 };
static Json_Value json_number(double value)
{
    return (Json_Value) {
        .type = JSON_NUMBER,
        .boolean = value
    };
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

static Json_Result json_error = { .is_error = 1 };

static Json_Result parse_token(String source, String token, Json_Value value)
{
    String source_trimmed = trim_begin(source);

    if (string_equal(take(source_trimmed, token.len), token)) {
        return (Json_Result) {
            .rest = drop(source_trimmed, token.len),
            .value = value
        };
    }

    return json_error;
}

static Json_Result parse_json_null(String source)
{
    return parse_token(source, SLT("null"), json_null);
}

static Json_Result parse_json_boolean(String source)
{
    Json_Result result;

    result = parse_token(source, SLT("true"), json_true);
    if (!result.is_error) return result;

    result = parse_token(source, SLT("false"), json_false);
    if (!result.is_error) return result;

    return json_error;
}

static Json_Result parse_json_number(String source)
{
    String source_trimmed = trim_begin(source);

    String integer = {0};
    String fraction = {0};
    String exponent = {0};

    integer.data = source_trimmed.data;

    if (source_trimmed.len && *source_trimmed.data == '-') {
        integer.len += 1;
        chop(&source_trimmed, 1);
    }

    while (source_trimmed.len && isdigit(*source_trimmed.data)) {
        integer.len += 1;
        chop(&source_trimmed, 1);
    }

    if (integer.len == 0 || string_equal(integer, SLT("-"))) {
        return json_error;
    }

    if (source_trimmed.len && *source_trimmed.data == '.') {
        chop(&source_trimmed, 1);
        fraction.data = source_trimmed.data;

        while (source_trimmed.len && isdigit(*source_trimmed.data)) {
            fraction.len  += 1;
            chop(&source_trimmed, 1);
        }
    }

    if (source_trimmed.len && tolower(*source_trimmed.data) == 'e') {
        chop(&source_trimmed, 1);
        exponent.data = source_trimmed.data;

        if (source_trimmed.len && *source_trimmed.data == '-') {
            exponent.len  += 1;
            chop(&source_trimmed, 1);
        }

        while (source_trimmed.len && isdigit(*source_trimmed.data)) {
            exponent.len  += 1;
            chop(&source_trimmed, 1);
        }

        if (exponent.len == 0 || string_equal(exponent, SLT("-"))) {
            return json_error;
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
        .rest = source_trimmed
    };
}

static Json_Result parse_json_string_literal(String source)
{
    String source_trimmed = trim_begin(source);

    if (source_trimmed.len == 0 || *source_trimmed.data != '"') {
        return json_error;
    }

    chop(&source_trimmed, 1);

    String s = {
        .data = source_trimmed.data,
        .len = 0
    };

    int escape = 0;
    while (source_trimmed.len && (*source_trimmed.data != '"' || escape)) {
        escape = 0;
        if (*source_trimmed.data == '\\') {
            escape = 1;
        }

        s.len++;
        chop(&source_trimmed, 1);
    }

    if (source_trimmed.len == 0) {
        return json_error;
    }

    chop(&source_trimmed, 1);

    return (Json_Result) {
        .value = {
            .type = JSON_STRING,
            .string = s
        },
        .rest = source_trimmed
    };
}

static Json_Result parse_json_string(Memory *memory, String source)
{
    Json_Result result = parse_json_string_literal(source);
    if (result.is_error) return result;
    assert(result.value.type == JSON_STRING);

    char *buffer = memory_alloc(memory, result.value.string.len);
    size_t buffer_size = 0;

    static char unescape_map[][2] = {
        {'b', '\b'},
        {'f', '\f'},
        {'n', '\n'},
        {'r', '\r'},
        {'t', '\t'},
    };
    static const size_t unescape_map_size = sizeof(unescape_map) / sizeof(unescape_map[0]);

    for (size_t i = 0; i < result.value.string.len; ++i) {
        if (result.value.string.data[i] == '\\' && i + 1 < result.value.string.len) {
            int unescaped = 0;
            for (size_t j = 0; j < unescape_map_size; ++j) {
                if (unescape_map[j][0] == result.value.string.data[i + 1]) {
                    buffer[buffer_size++] = unescape_map[j][1];
                    unescaped = 1;
                    break;
                }
            }

            if (unescaped) continue;

            // TODO(#29): parse_json_string does not support \u
            buffer[buffer_size++] = result.value.string.data[i + 1];
        } else {
            buffer[buffer_size++] = result.value.string.data[i];
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
        .rest = result.rest
    };
}

static Json_Result parse_json_array(Memory *memory, String source)
{
    String source_trimmed = trim_begin(source);

    if(source_trimmed.len == 0 || *source_trimmed.data != '[') {
        return json_error;
    }

    chop(&source_trimmed, 1);

    source_trimmed = trim_begin(source_trimmed);

    if (source_trimmed.len == 0) {
        return json_error;
    } else if(*source_trimmed.data == ']') {
        return (Json_Result) {
            .value = {
                .type = JSON_ARRAY,
                .array = {
                    .begin = NULL,
                    .end = NULL
                },
            },
            .rest = drop(source_trimmed, 1)
        };
    }

    Json_Array array = {
        .begin = NULL,
        .end = NULL
    };

    while(source_trimmed.len > 0) {
        Json_Result item_result = parse_json_value(memory, source_trimmed);
        if(item_result.is_error) {
            return item_result;
        }

        json_array_push(memory, &array, item_result.value);

        source_trimmed = trim_begin(item_result.rest);

        if(*source_trimmed.data == ']') {
            return (Json_Result) {
                .value = {
                    .type = JSON_ARRAY,
                    .array = array
                },
                .rest = drop(source_trimmed, 1)
            };
        } else if (*source_trimmed.data == ',') {
            source_trimmed = trim_begin(drop(source_trimmed, 1));
        } else {
            return json_error;
        }
    }

    return json_error;
}

static Json_Result parse_json_object(Memory *memory, String source)
{
    assert(!"TODO(#21): parse_json_object is not implemented");
    return json_error;
}

Json_Result parse_json_value(Memory *memory, String source)
{
    String trimmed_source = trim_begin(source);

    if (trimmed_source.len == 0) {
        return (Json_Result) {
            .is_error = 1,
            .message = "EOF"
        };
    }

    switch (*trimmed_source.data) {
    case 'n': return parse_json_null(trimmed_source);
    case 't':
    case 'f': return parse_json_boolean(trimmed_source);
    case '"': return parse_json_string(memory, trimmed_source);
    case '[': return parse_json_array(memory, trimmed_source);
    case '{': return parse_json_object(memory, trimmed_source);
    }

    return parse_json_number(trimmed_source);
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
    for (size_t i = 0; i < object.size; ++i) {
        if (i > 0) fprintf(stream, ",");
        print_json_string(stream, object.keys[i]);
        fprintf(stream, ":");
        print_json_value(stream, object.values[i]);
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
