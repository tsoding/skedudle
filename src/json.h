#ifndef JSON_H_
#define JSON_H_

#include <stdlib.h>
#include <stdio.h>

#include "s.h"
#include "memory.h"

#define JSON_DEPTH_MAX_LIMIT 100

typedef enum {
    JSON_NULL = 0,
    JSON_BOOLEAN,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} Json_Type;

static inline
const char *json_type_as_cstr(Json_Type type)
{
    switch (type) {
    case JSON_NULL: return "JSON_NULL";
    case JSON_BOOLEAN: return "JSON_BOOLEAN";
    case JSON_NUMBER: return "JSON_NUMBER";
    case JSON_STRING: return "JSON_STRING";
    case JSON_ARRAY: return "JSON_ARRAY";
    case JSON_OBJECT: return "JSON_OBJECT";
    }

    assert(!"Incorrect Json_Type");
}

typedef struct Json_Value Json_Value;

#define JSON_ARRAY_PAGE_CAPACITY 128

typedef struct Json_Array_Page Json_Array_Page;

typedef struct {
    Json_Array_Page *begin;
    Json_Array_Page *end;
} Json_Array;

void json_array_push(Memory *memory, Json_Array *array, Json_Value value);

typedef struct Json_Object_Page Json_Object_Page;

typedef struct {
    Json_Object_Page *begin;
    Json_Object_Page *end;
} Json_Object;

typedef struct {
    // TODO(#26): because of the use of String-s Json_Number can hold an incorrect value
    //   But you can only get an incorrect Json_Number if you construct it yourself.
    //   Anything coming from parse_json_value should be always a correct number.
    String integer;
    String fraction;
    String exponent;
} Json_Number;

int64_t json_number_to_integer(Json_Number number);

struct Json_Value {
    Json_Type type;
    union
    {
        int boolean;
        Json_Number number;
        String string;
        Json_Array array;
        Json_Object object;
    };
};

struct Json_Array_Page {
    Json_Array_Page *next;
    size_t size;
    Json_Value elements[JSON_ARRAY_PAGE_CAPACITY];
};

static inline
size_t json_array_size(Json_Array array)
{
    size_t size = 0;
    for (Json_Array_Page *page = array.begin;
         page != NULL;
         page = page->next)
    {
        size += page->size;
    }
    return size;
}

typedef struct {
    Json_Value value;
    String rest;
    int is_error;
    const char *message;
} Json_Result;

typedef struct {
    String key;
    Json_Value value;
} Json_Object_Member;

#define JSON_OBJECT_PAGE_CAPACITY 128

extern Json_Value json_null;
extern Json_Value json_true;
extern Json_Value json_false;

Json_Value json_string(String string);

struct Json_Object_Page {
    Json_Object_Page *next;
    size_t size;
    Json_Object_Member elements[JSON_OBJECT_PAGE_CAPACITY];
};

void json_object_push(Memory *memory, Json_Object *object, String key, Json_Value value);

// TODO(#40): parse_json_value is not aware of input encoding
Json_Result parse_json_value(Memory *memory, String source);
void print_json_error(FILE *stream, Json_Result result, String source, const char *prefix);
void print_json_value(FILE *stream, Json_Value value);
void print_json_value_fd(int fd, Json_Value value);

#endif  // JSON_H_
