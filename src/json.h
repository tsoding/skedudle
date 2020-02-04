#ifndef JSON_H_
#define JSON_H_

#include <stdlib.h>
#include <stdio.h>

#include "s.h"
#include "memory.h"

typedef enum {
    JSON_NULL = 0,
    JSON_BOOLEAN,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} Json_Type;

typedef struct Json_Value Json_Value;

typedef struct {
    size_t size;
    Json_Value *elements;
} Json_Array;

typedef struct {
    size_t size;
    String *keys;
    Json_Value *values;
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

typedef struct {
    Json_Value value;
    String rest;
    // TODO(#22): error reporting in Json_Result is very poor
    int is_error;
} Json_Result;

Json_Result parse_json_value(Memory *memory, String source);
void print_json_value(FILE *stream, Json_Value value);

#endif  // JSON_H_
