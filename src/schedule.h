#ifndef SCHEDULE_H_
#define SCHEDULE_H_

#include <stdlib.h>
#include <stdint.h>
#include "s.h"
#include "memory.h"

struct Project
{
    const char *name;
    const char *description;
    const char *url;
    uint8_t days;
    int time_min;
    const char *channel;
};

struct Schedule
{
    struct Project *projects;
    size_t projects_size;
};

void json_scan_schedule(Memory *memory,
                        String input,
                        struct Schedule *schedule);

#endif  // SCHEDULE_H_
