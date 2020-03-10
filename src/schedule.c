#include <assert.h>
#define __USE_XOPEN
#include <time.h>
#include <string.h>

#include "schedule.h"

// static
// void json_scan_cancelled_events(const char *str, int str_len,
//                                 struct Schedule *schedule)
// {
//     schedule->cancelled_events_count = json_array_len(str, str_len);
//     schedule->cancelled_events = allocator.alloc(
//         schedule->cancelled_events_count * sizeof(time_t));

//     struct json_token t;
//     for (int i = 0;
//          json_scanf_array_elem(str, str_len, "", i, &t) > 0;
//          i++)
//     {
//         json_scanf(t.ptr, t.len, "%ld", schedule->cancelled_events + i);
//     }
// }

// static
// void json_scan_event(const char *str, int str_len,
//                      struct Event *event)
// {
//     json_scanf(
//         str, str_len,
//         "{"
//         "    date: %M,"
//         "    time: %M,"
//         "    title: %Q,"
//         "    description: %Q,"
//         "    url: %Q,"
//         "    channel: %Q"
//         "}",
//         json_scan_date, &event->date,
//         json_scan_time, &event->time_min,
//         &event->title,
//         &event->description,
//         &event->url,
//         &event->channel);
// }

// static
// void json_scan_extra_events(const char *str, int str_len,
//                             struct Schedule *schedule)
// {
//     schedule->extra_events_size = json_array_len(str, str_len);
//     schedule->extra_events = allocator.alloc(
//         schedule->extra_events_size * sizeof(struct Event));

//     struct json_token t;
//     for (int i = 0;
//          json_scanf_array_elem(str, str_len, "", i, &t) > 0;
//          i++)
//     {
//         json_scan_event(t.ptr, t.len, &schedule->extra_events[i]);
//     }
// }

static
void expect_json_type(Json_Value value, Json_Type type)
{
    if (value.type != type) {
        fprintf(stderr,
                "Expected %s, but got %s\n",
                json_type_as_cstr(type),
                json_type_as_cstr(value.type));
        abort();
    }
}

uint8_t json_as_days(Memory *memory, Json_Value input)
{
    assert(memory);
    expect_json_type(input, JSON_ARRAY);

    uint8_t days = 0;
    for (Json_Array_Page *page = input.array.begin;
         page != NULL;
         page = page->next)
    {
        for (size_t page_index = 0; page_index < page->size; ++page_index) {
            expect_json_type(page->elements[page_index], JSON_NUMBER);
            int64_t x = json_number_to_integer(page->elements[page_index].number);
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
            days |= 1 << (x % 7);
        }
    }

    return days;
}

int json_as_time_min(Memory *memory, Json_Value input)
{
    assert(memory);
    expect_json_type(input, JSON_STRING);
    const char *input_cstr = string_as_cstr(memory, input.string);
    struct tm tm = {0};
    strptime(input_cstr, "%H:%M", &tm);
    return tm.tm_hour * 60 + tm.tm_min;
}

struct tm *json_as_date(Memory *memory, Json_Value input)
{
    assert(memory);
    expect_json_type(input, JSON_STRING);
    const char *input_cstr = string_as_cstr(memory, input.string);
    struct tm *tm = memory_alloc(memory, sizeof(struct tm));
    strptime(input_cstr, "%Y-%m-%d", tm);
    return tm;
}

static
struct Project json_as_project(Memory *memory, Json_Value input)
{
    assert(memory);

    expect_json_type(input, JSON_OBJECT);

    struct Project project = {0};

    for (Json_Object_Page *page = input.object.begin;
         page != NULL;
         page = page->next)
    {
        for (size_t page_index = 0; page_index < page->size; ++page_index) {
            if (string_equal(page->elements[page_index].key, SLT("name"))) {
                expect_json_type(page->elements[page_index].value, JSON_STRING);
                project.name = string_as_cstr(
                    memory,
                    page->elements[page_index].value.string);
            } else if (string_equal(page->elements[page_index].key, SLT("description"))) {
                expect_json_type(page->elements[page_index].value, JSON_STRING);
                project.description = string_as_cstr(
                    memory,
                    page->elements[page_index].value.string);
            } else if (string_equal(page->elements[page_index].key, SLT("url"))) {
                expect_json_type(page->elements[page_index].value, JSON_STRING);
                project.url = string_as_cstr(
                    memory,
                    page->elements[page_index].value.string);
            } else if (string_equal(page->elements[page_index].key, SLT("days"))) {
                project.days = json_as_days(memory, page->elements[page_index].value);
            } else if (string_equal(page->elements[page_index].key, SLT("time"))) {
                project.time_min = json_as_time_min(memory, page->elements[page_index].value);
            } else if (string_equal(page->elements[page_index].key, SLT("channel"))) {
                expect_json_type(page->elements[page_index].value, JSON_STRING);
                project.channel = string_as_cstr(
                    memory,
                    page->elements[page_index].value.string);
            } else if (string_equal(page->elements[page_index].key, SLT("starts"))) {
                project.starts = json_as_date(memory, page->elements[page_index].value);
            } else if (string_equal(page->elements[page_index].key, SLT("ends"))) {
                project.ends = json_as_date(memory, page->elements[page_index].value);
            }
        }
    }

    return project;
}

static
void parse_schedule_projects(Memory *memory, Json_Value input, struct Schedule *schedule)
{
    assert(memory);
    assert(schedule);

    expect_json_type(input, JSON_ARRAY);
    schedule->projects = memory_alloc(
        memory,
        sizeof(struct Project) * json_array_size(input.array));
    schedule->projects_size = 0;

    for (Json_Array_Page *page = input.array.begin;
         page != NULL;
         page = page->next)
    {
        for (size_t page_index = 0; page_index < page->size; ++page_index) {
            schedule->projects[schedule->projects_size++] =
                json_as_project(memory, page->elements[page_index]);
        }
    }
}

struct Schedule json_as_schedule(Memory *memory, Json_Value input)
{
    assert(memory);
    expect_json_type(input, JSON_OBJECT);

    struct Schedule schedule = {0};

    for (Json_Object_Page *page = input.object.begin;
         page != NULL;
         page = page->next)
    {
        for (size_t page_index = 0; page_index < page->size; ++page_index)
        {
            if (string_equal(page->elements[page_index].key, SLT("projects"))) {
                parse_schedule_projects(memory, page->elements[page_index].value, &schedule);
            } else if (string_equal(page->elements[page_index].key, SLT("cancelledEvents"))) {
                // TODO(#47): cancelledEvents deserialization is not implemented
            } else if (string_equal(page->elements[page_index].key, SLT("extraEvents"))) {
                // TODO(#48): extraEvents deserialization is not implemented
            } else if (string_equal(page->elements[page_index].key, SLT("timezone"))) {
                expect_json_type(page->elements[page_index].value, JSON_STRING);
                schedule.timezone = string_as_cstr(
                    memory,
                    page->elements[page_index].value.string);
            }
        }
    }

    return schedule;
}
