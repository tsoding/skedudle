#include <assert.h>
#include <stdio.h>

#include "s.h"
#include "memory.h"
#include "schedule.h"
#include "frozen.h"

String read_whole_file(Memory *memory, const char *filepath)
{
    assert(filepath);
    assert(memory);

    FILE *f = fopen(filepath, "rb");
    assert(f);

    int err = fseek(f, 0, SEEK_END); assert(!err && "fseek end");
    long size = ftell(f); assert(size >= 0 && "ftell");
    err = fseek(f, 0, SEEK_SET); assert(!err && "fseek set");
    char *buffer = memory_alloc(memory, size);

    size_t read_size = fread(buffer, 1, size, f); assert(read_size == (size_t) size && "fread");

    String result = {
        .len = (size_t) size,
        .data = buffer
    };

    fclose(f);

    return result;
}

#define MEMORY_CAPACITY (640 * KILO)

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: schedule_test <schedule.json>\n");
        exit(1);
    }

    Memory memory;
    memory.capacity = MEMORY_CAPACITY;
    memory.buffer = malloc(MEMORY_CAPACITY);

    String content = read_whole_file(&memory, argv[1]);
    struct Schedule schedule;
    memset(&schedule, 0, sizeof(schedule));
    json_scan_schedule(&memory, content, &schedule);

    // TODO: error report?
    // TODO: memory management

    for (size_t i = 0; i < schedule.projects_size; ++i) {
        printf("Title: %s\n", schedule.projects[i].name);
    }

    // printf("Memory used %zu bytes out of %zu bytes\n", json_memory.size, json_memory.capacity);

    free(memory.buffer);

    return 0;
}
