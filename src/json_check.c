#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "json.h"

#define MEMORY_CAPACITY (10 * 1000 * 1000)

static uint8_t memory_buffer[MEMORY_CAPACITY];
static Memory memory = {
    .capacity = MEMORY_CAPACITY,
    .buffer = memory_buffer,
};

String mmap_file_to_string(const char *filepath)
{
    int fd = open(filepath, O_RDONLY);
    assert(fd >= 0);

    struct stat fd_stat;
    int err = fstat(fd, &fd_stat);
    assert(err == 0);

    String result;
    result.len = fd_stat.st_size;
    result.data = mmap(NULL, result.len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    assert(result.data);
    close(fd);

    return result;
}

int main(int argc, char *argv[])
{
    assert(argc >= 2);
    String file_content = mmap_file_to_string(argv[1]);
    Json_Result result = parse_json_value(&memory, file_content);
    return result.is_error || trim_begin(result.rest).len > 0;
}
