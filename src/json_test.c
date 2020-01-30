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

    String tests[] = {
        SLT("null"),
        SLT("true"),
        SLT("false")
    };
    size_t tests_count = sizeof(tests) / sizeof(tests[0]);

    for (size_t i = 0; i < tests_count; ++i) {
        Json_Result result = parse_json_value(&memory, tests[i]);
        assert(!result.is_error);
        print_json_value(stdout, result.value); printf("\n");
    }

    free(memory.buffer);

    return 0;
}
