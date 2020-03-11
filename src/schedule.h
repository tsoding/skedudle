#ifndef SCHEDULE_H_
#define SCHEDULE_H_

#include <stdlib.h>
#include <stdint.h>
#include "s.h"
#include "memory.h"
#include "json.h"

struct Project
{
    String name;
    String description;
    String url;
    uint8_t days;
    int time_min;
    String channel;
    struct tm *starts;
    struct tm *ends;
};

struct Event
{
    struct tm date;
    int time_min;
    String title;
    String description;
    String url;
    String channel;
};

struct Schedule
{
    struct Project *projects;
    size_t projects_size;
    time_t *cancelled_events;
    size_t cancelled_events_count;
    struct Event *extra_events;
    size_t extra_events_size;
    String timezone;
};

struct Schedule json_as_schedule(Memory *memory, Json_Value input);

#endif  // SCHEDULE_H_
