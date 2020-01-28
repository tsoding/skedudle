#include <stdio.h>

#include "json.h"

#define MEMORY_CAPACITY (640 * 1000)

int main(int argc, char *argv[])
{
    Memory memory = {
        .capacity = MEMORY_CAPACITY,
        .buffer = malloc(MEMORY_CAPACITY)
    };

    assert(memory.buffer);

    Json_Result result = parse_json_value(&memory, SLT("null"));
    assert(!result.is_error);
    print_json_value(stdout, result.value);

    free(memory.buffer);

    return 0;
}
