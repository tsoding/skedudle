#include <stdio.h>

#include "json.h"

#define MEMORY_CAPACITY (640 * 1000)

int main(void)
{
    Memory memory = {
        .capacity = MEMORY_CAPACITY,
        .buffer = malloc(MEMORY_CAPACITY)
    };

    assert(memory.buffer);

    String tests[] = {
        SLT("null"),
        SLT("true"),
        SLT("false"),
        SLT("1"),
        SLT("2"),
        SLT(".10"),
        SLT("1e9999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999"),
        SLT("-.10"),
        SLT("-10"),
        SLT("10.10"),
        SLT("-10.10e2"),
        SLT("-10.10e-2"),
        // TODO(#25): parse_json_number treats -10.-10e-2 as two separate numbers
        SLT("-10.-10e-2"),
        SLT("\"hello,\tworld\""),
        SLT("[]"),
        SLT("[1]"),
        SLT("[\"test\"]"),
        SLT("[1,2,3]"),
        SLT("[1,2,3 5]"),
        SLT("[\"hello,\tworld\", 123, \t \"abcd\", -10.10e-2, \"test\"]"),
        SLT("[[]]"),
        SLT("[123,[321,\"test\"],\"abcd\"]"),
    };
    size_t tests_count = sizeof(tests) / sizeof(tests[0]);

    for (size_t i = 0; i < tests_count; ++i) {
        Json_Result result = parse_json_value(&memory, tests[i]);
        if (result.is_error) {
            fwrite(tests[i].data, 1, tests[i].len, stdout); fputc('\n', stdout);
            size_t n = result.rest.data - tests[i].data;
            for (size_t j = 0; j < n; ++j) {
                fputc(' ', stdout);
            }
            fputc('^', stdout);
            fputc('\n', stdout);

            fputs(result.message, stdout);
            fputc('\n', stdout);
        }
        print_json_value(stdout, result.value); fputc('\n', stdout);
    }

    free(memory.buffer);

    return 0;
}
