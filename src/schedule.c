#include <assert.h>
#define __USE_XOPEN
#include <time.h>
#include <string.h>

#include "frozen.h"
#include "schedule.h"

static
void json_scan_days(const char *str, int str_len,
                    uint8_t *days)
{
    assert(str);
    assert(days);
    assert(str_len >= 0);

    *days = 0;
    struct json_token t;
    for (int i = 0;
         json_scanf_array_elem(str, str_len, "", i, &t) > 0;
         i++)
    {
        int x;
        json_scanf(t.ptr, t.len, "%d", &x);
        // NOTE:
        // - schedule.json (1-7, Monday = 1)
        // - POSIX         (0-6, Sunday = 0)
        //
        // the mask is expected to be POSIX compliant.
        //
        //     JSON  POSIX
        //  Mon  1 -> 1
        //  Tue  2 -> 2
        //  Wed  3 -> 3
        //  Thu  4 -> 4
        //  Fri  5 -> 5
        //  Sat  6 -> 6
        //  Sun  7 -> 0
        *days |= 1 << (x % 7);
    }
}

static
void json_scan_time(const char *str, int str_len,
                    int *time_min)
{
    assert(str);
    assert(time_min);
    assert(str_len >= 0);

    char buffer[str_len + 1];

    memcpy(buffer, str, str_len);
    buffer[str_len] = 0;

    struct tm tm;
    strptime(buffer, "%H:%M", &tm);

    *time_min = tm.tm_hour * 60 + tm.tm_min;
}

void json_scan_date(const char *str, int str_len,
                    struct tm **tm)
{
    *tm = allocator.alloc(sizeof(**tm));
    char *str_null = allocator.alloc(str_len + 1);
    memcpy(str_null, str, str_len);
    str_null[str_len] = '\0';
    strptime(str_null, "%Y-%m-%d", *tm);
}

static
void json_scan_project(const char *str, int str_len,
                       struct Project *project)
{
    json_scanf(
        str, str_len,
        "{"
        "    name: %Q,"
        "    description: %Q,"
        "    url: %Q,"
        "    days: %M,"
        "    time: %M,"
        "    channel: %Q,"
        "    starts: %M,"
        "    ends: %M"
        "}",
        &project->name,
        &project->description,
        &project->url,
        json_scan_days, &project->days,
        json_scan_time, &project->time_min,
        &project->channel,
        json_scan_date, &project->starts,
        json_scan_date, &project->ends);
}

static
size_t json_array_len(const char *str, int str_len)
{
    size_t n = 0;
    struct json_token t;
    while (json_scanf_array_elem(str, str_len, "", n, &t) > 0)
        ++n;
    return n;
}

typedef struct {
    Memory *memory;
    struct Schedule *schedule;
} Context;

static
void json_scan_projects(const char *str, int str_len,
                        struct Schedule* schedule)
{
    schedule->projects_size = json_array_len(str, str_len);
    schedule->projects = allocator.alloc(
        schedule->projects_size * sizeof(struct Project));

    struct json_token t;
    for (int i = 0;
         json_scanf_array_elem(str, str_len, "", i, &t) > 0;
         i++)
    {
        json_scan_project(
            t.ptr, t.len,
            schedule->projects + i);
    }
}

static
void json_scan_cancelled_events(const char *str, int str_len,
                                struct Schedule *schedule)
{
    schedule->cancelled_events_count = json_array_len(str, str_len);
    schedule->cancelled_events = allocator.alloc(
        schedule->cancelled_events_count * sizeof(time_t));

    struct json_token t;
    for (int i = 0;
         json_scanf_array_elem(str, str_len, "", i, &t) > 0;
         i++)
    {
        json_scanf(t.ptr, t.len, "%ld", schedule->cancelled_events + i);
    }
}

void json_scan_schedule(String input, struct Schedule *schedule)
{
    json_scanf(
        input.data, input.len,
        "{"
        "    projects: %M,"
        "    timezone: %Q,"
        "    cancelledEvents: %M"
        "}",
        json_scan_projects, schedule,
        &schedule->timezone,
        json_scan_cancelled_events, schedule);
}
