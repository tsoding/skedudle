#ifndef SCHEDULE_H_
#define SCHEDULE_H_

#include <stdlib.h>
#include <stdint.h>
#include "s.h"
#include "memory.h"
#include "json.h"

struct Project
{
    const char *name;
    const char *description;
    const char *url;
    uint8_t days;
    int time_min;
    const char *channel;
    struct tm *starts;
    struct tm *ends;
};

struct Event
{
    struct tm date;
    int time_min;
    const char *title;
    const char *description;
    const char *url;
    const char *channel;
};

struct Schedule
{
    struct Project *projects;
    size_t projects_size;
    time_t *cancelled_events;
    size_t cancelled_events_count;
    struct Event *extra_events;
    size_t extra_events_size;
    const char *timezone;
};

struct Schedule json_as_schedule(Memory *memory, Json_Value input);

#endif  // SCHEDULE_H_
