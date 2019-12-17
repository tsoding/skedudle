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
static Memory memory;

static void *epic_alloc(size_t size)
{
    return memory_alloc(&memory, size);
}

static void epic_free(void *ptr) { (void)ptr;}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: schedule_test <schedule.json>\n");
        exit(1);
    }

    memory.capacity = MEMORY_CAPACITY;
    memory.buffer = malloc(MEMORY_CAPACITY);

    allocator.alloc = epic_alloc;
    allocator.free = epic_free;

    String content = read_whole_file(&memory, argv[1]);
    struct Schedule schedule;
    memset(&schedule, 0, sizeof(schedule));
    json_scan_schedule(&memory, content, &schedule);

    // TODO: error report?

    for (size_t i = 0; i < schedule.projects_size; ++i) {
        printf("Title: %s\n", schedule.projects[i].name);
    }

    free(memory.buffer);

    return 0;
}
