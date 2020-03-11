#include <assert.h>
#define __USE_XOPEN
#include <time.h>
#include <string.h>

#include "schedule.h"

static inline
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

struct tm json_as_date(Memory *memory, Json_Value input)
{
    assert(memory);
    expect_json_type(input, JSON_STRING);
    const char *input_cstr = string_as_cstr(memory, input.string);
    struct tm tm = {0};
    strptime(input_cstr, "%Y-%m-%d", &tm);
    return tm;
}

static
struct Project json_as_project(Memory *memory, Json_Value input)
{
    assert(memory);

    expect_json_type(input, JSON_OBJECT);

    struct Project project;
    memset(&project, 0, sizeof(project));

    for (Json_Object_Page *page = input.object.begin;
         page != NULL;
         page = page->next)
    {
        for (size_t page_index = 0; page_index < page->size; ++page_index) {
            if (string_equal(page->elements[page_index].key, SLT("name"))) {
                expect_json_type(page->elements[page_index].value, JSON_STRING);
                project.name = page->elements[page_index].value.string;
            } else if (string_equal(page->elements[page_index].key, SLT("description"))) {
                expect_json_type(page->elements[page_index].value, JSON_STRING);
                project.description = page->elements[page_index].value.string;
            } else if (string_equal(page->elements[page_index].key, SLT("url"))) {
                expect_json_type(page->elements[page_index].value, JSON_STRING);
                project.url = page->elements[page_index].value.string;
            } else if (string_equal(page->elements[page_index].key, SLT("days"))) {
                project.days = json_as_days(memory, page->elements[page_index].value);
            } else if (string_equal(page->elements[page_index].key, SLT("time"))) {
                project.time_min = json_as_time_min(memory, page->elements[page_index].value);
            } else if (string_equal(page->elements[page_index].key, SLT("channel"))) {
                expect_json_type(page->elements[page_index].value, JSON_STRING);
                project.channel = page->elements[page_index].value.string;
            } else if (string_equal(page->elements[page_index].key, SLT("starts"))) {
                project.starts = memory_alloc(memory, sizeof(*project.starts));
                memset(project.starts, 0, sizeof(*project.starts));
                *project.starts = json_as_date(memory, page->elements[page_index].value);
            } else if (string_equal(page->elements[page_index].key, SLT("ends"))) {
                project.ends = memory_alloc(memory, sizeof(*project.ends));
                memset(project.ends, 0, sizeof(*project.ends));
                *project.ends = json_as_date(memory, page->elements[page_index].value);
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

    const size_t array_size = json_array_size(input.array);
    const size_t memory_size = sizeof(schedule->projects[0]) * array_size;

    schedule->projects = memory_alloc(memory, memory_size);
    memset(schedule->projects, 0, memory_size);
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

static
void parse_schedule_cancelled_events(Memory *memory, Json_Value input, struct Schedule *schedule)
{
    assert(memory);
    assert(schedule);
    expect_json_type(input, JSON_ARRAY);

    const size_t array_size = json_array_size(input.array);
    const size_t memory_size = sizeof(schedule->cancelled_events[0]) * array_size;

    schedule->cancelled_events = memory_alloc(memory, memory_size);
    memset(schedule->cancelled_events, 0, memory_size);
    schedule->cancelled_events_count = 0;

    for (Json_Array_Page *page = input.array.begin;
         page != NULL;
         page = page->next)
    {
        for (size_t page_index = 0; page_index < page->size; ++page_index) {
            expect_json_type(page->elements[page_index], JSON_NUMBER);
            schedule->cancelled_events[schedule->cancelled_events_count++] =
                json_number_to_integer(page->elements[page_index].number);
        }
    }
}

static
struct Event json_as_event(Memory *memory, Json_Value input)
{
    assert(memory);
    expect_json_type(input, JSON_OBJECT);

    struct Event event = {0};

    for (Json_Object_Page *page = input.object.begin;
         page != NULL;
         page = page->next)
    {
        for (size_t page_index = 0; page_index < page->size; ++page_index) {
            if (string_equal(page->elements[page_index].key, SLT("date"))) {
                event.date = json_as_date(memory, page->elements[page_index].value);
            } else if (string_equal(page->elements[page_index].key, SLT("time"))) {
                event.time_min = json_as_time_min(memory, page->elements[page_index].value);
            } else if (string_equal(page->elements[page_index].key, SLT("title"))) {
                expect_json_type(page->elements[page_index].value, JSON_STRING);
                event.title = page->elements[page_index].value.string;
            } else if (string_equal(page->elements[page_index].key, SLT("description"))) {
                expect_json_type(page->elements[page_index].value, JSON_STRING);
                event.description = page->elements[page_index].value.string;
            } else if (string_equal(page->elements[page_index].key, SLT("url"))) {
                expect_json_type(page->elements[page_index].value, JSON_STRING);
                event.url = page->elements[page_index].value.string;
            } else if (string_equal(page->elements[page_index].key, SLT("channel"))) {
                expect_json_type(page->elements[page_index].value, JSON_STRING);
                event.channel = page->elements[page_index].value.string;
            }
        }
    }

    return event;
}

static
void parse_schedule_extra_events(Memory *memory, Json_Value input, struct Schedule *schedule)
{
    assert(memory);
    assert(schedule);
    expect_json_type(input, JSON_ARRAY);

    const size_t array_size = json_array_size(input.array);
    const size_t memory_size = sizeof(schedule->extra_events[0]) * array_size;

    schedule->extra_events = memory_alloc(memory, memory_size);
    memset(schedule->extra_events, 0, memory_size);
    schedule->extra_events_size = 0;

    for (Json_Array_Page *page = input.array.begin;
         page != NULL;
         page = page->next)
    {
        for (size_t page_index = 0; page_index < page->size; ++page_index) {
            assert(schedule->extra_events_size < array_size);
            schedule->extra_events[schedule->extra_events_size++] =
                json_as_event(memory, page->elements[page_index]);
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
                parse_schedule_cancelled_events(memory, page->elements[page_index].value, &schedule);
            } else if (string_equal(page->elements[page_index].key, SLT("extraEvents"))) {
                parse_schedule_extra_events(memory, page->elements[page_index].value, &schedule);
            } else if (string_equal(page->elements[page_index].key, SLT("timezone"))) {
                expect_json_type(page->elements[page_index].value, JSON_STRING);
                schedule.timezone = page->elements[page_index].value.string;
            }
        }
    }

    return schedule;
}
