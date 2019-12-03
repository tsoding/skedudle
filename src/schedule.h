#ifndef SCHEDULE_H_
#define SCHEDULE_H_

#include <stdint.h>

struct Project
{
    const char *name;
    const char *description;
    const char *url;
    uint8_t days;
    int time_min;
    const char *channel;
};

void json_scan_project(const char *str, int str_len,
                       struct Project *project);

#endif  // SCHEDULE_H_
