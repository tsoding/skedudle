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
    assert(!"TODO(#18): parse_json_number is not implemented");
    return json_error;
}

static Json_Result parse_json_string(Memory *memory, String source)
{
    assert(!"TODO: parse_json_string is not implemented");
    return json_error;
}

static Json_Result parse_json_array(Memory *memory, String source)
{
    assert(!"TODO: parse_json_array is not implemented");
    return json_error;
}

static Json_Result parse_json_object(Memory *memory, String source)
{
    assert(!"TODO: parse_json_object is not implemented");
    return json_error;
}

Json_Result parse_json_value(Memory *memory, String source)
{
    Json_Result result;

    result = parse_json_null(source);
    if (!result.is_error) return result;

    result = parse_json_boolean(source);
    if (!result.is_error) return result;

    result = parse_json_number(source);
    if (!result.is_error) return result;

    result = parse_json_string(memory, source);
    if (!result.is_error) return result;

    result = parse_json_array(memory, source);
    if (!result.is_error) return result;

    result = parse_json_object(memory, source);
    if (!result.is_error) return result;

    return json_error;
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
void print_json_number(FILE *stream, double number)
{
    fprintf(stream, "%lf", number);
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
}

static
void print_json_array(FILE *stream, Json_Array array)
{
    fprintf(stream, "[");
    for (size_t i = 0; i < array.size; ++i) {
        if (i > 0) fprintf(stream, ",");
        print_json_value(stream, array.elements[i]);
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
