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
        SLT("nullptr"),
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
        SLT("[\n"
            "  true,\n"
            "  false,\n"
            "  null\n"
            "]"),
        SLT("[\n"
            "  true,\n"
            "  false #\n"
            "  null\n"
            "]"),
        SLT("{\n"
            "   \"null\": null,\n"
            "   \"boolean\": true,\n"
            "   \"boolean\": false,\n"
            "   \"number\": 69420,\n"
            "   \"string\": \"hello\",\n"
            "   \"array\": [null, true, false, 69420, \"hello\"],\n"
            "   \"object\": {}\n"
            "}"),
        SLT("{\n"
            "   \"null\": null,\n"
            "   \"boolean\": true\n"
            "   \"boolean\": false,\n"
            "   \"number\": 69420,\n"
            "   \"string\": \"hello\",\n"
            "   \"array\": [null, true, false, 69420, \"hello\"],\n"
            "   \"object\": {}\n"
            "}")
    };
    size_t tests_count = sizeof(tests) / sizeof(tests[0]);

    for (size_t i = 0; i < tests_count; ++i) {
        fputs("PARSING: \n", stdout);
        fwrite(tests[i].data, 1, tests[i].len, stdout);
        fputc('\n', stdout);

        Json_Result result = parse_json_value(&memory, tests[i]);
        if (result.is_error) {
            fputs("FAILURE: \n", stdout);
            print_json_error(stdout, result, tests[i], "<test>");
        } else if (trim_begin(result.rest).len != 0) {
            fputs("FAILURE: \n", stdout);
            fputs("parsed ", stdout);
            print_json_value(stdout, result.value);
            fputc('\n', stdout);

            fputs("but left unparsed input: ", stdout);
            fwrite(result.rest.data, 1, result.rest.len, stdout);
            fputc('\n', stdout);
        } else {
            fputs("SUCCESS: \n", stdout);
            print_json_value(stdout, result.value);
            fputc('\n', stdout);
        }

        printf("MEMORY USAGE: %lu bytes\n", memory.size);
        fputs("------------------------------\n", stdout);
        memory_clean(&memory);
    }

    free(memory.buffer);

    return 0;
}
