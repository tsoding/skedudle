#include <assert.h>
#include <stdio.h>

#include "s.h"
#include "schedule.h"
#include "frozen.h"

String read_whole_file(const char *filepath)
{
    assert(filepath);

    FILE *f = fopen(filepath, "rb");
    assert(f);

    int err = fseek(f, 0, SEEK_END); assert(!err && "fseek end");
    long size = ftell(f); assert(size >= 0 && "ftell");
    err = fseek(f, 0, SEEK_SET); assert(!err && "fseek set");
    char *buffer = malloc(size); assert(buffer && "malloc");

    size_t read_size = fread(buffer, 1, size, f); assert(read_size == (size_t) size && "fread");

    String result = {
        .len = (size_t) size,
        .data = buffer
    };

    fclose(f);

    return result;
}


int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: schedule_test <schedule.json>\n");
        exit(1);
    }

    String content = read_whole_file(argv[1]);
    struct Schedule schedule;
    json_scan_schedule(content, &schedule);

    // TODO: error report?
    // TODO: memory management

    for (size_t i = 0; i < schedule.projects_size; ++i) {
        printf("Title:       %s\n", schedule.projects[i].name);
        printf("Description: %s\n", schedule.projects[i].description);
        printf("------------------------------\n");
    }

    return 0;
}
