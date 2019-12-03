#include <assert.h>
#define __USE_XOPEN
#include <time.h>
#include <string.h>

#include "frozen.h"
#include "schedule.h"

static
void scan_days(const char *str, int str_len,
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
        *days |= 1 << (x - 1);
    }
}

static
void scan_time(const char *str, int str_len,
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
        "    channel: %Q"
        "}",
        &project->name,
        &project->description,
        &project->url,
        scan_days, &project->days,
        scan_time, &project->time_min,
        &project->channel);
}
