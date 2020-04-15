#ifndef MEMORY_H_
#define MEMORY_H_

#include <assert.h>
#include <stdint.h>
#include <stdalign.h>

#define KILO 1024
#define MEGA (1024 * KILO)
#define GIGA (1024 * MEGA)

typedef struct {
    size_t capacity;
    size_t size;
    uint8_t *buffer;
} Memory;

static inline
void *memory_alloc(Memory *memory, size_t size)
{
    assert(memory);
    assert(memory->size + size <= memory->capacity);


    void *result = memory->buffer + memory->size;
    memory->size += size;

    return result;
}

static inline
void *memory_alloc_aligned(Memory *memory, size_t size, size_t alignment)
{
    assert(memory);

    // this gets aligns the address *upwards*, to the next alignment.
    // the assumption here is that 'alignment' is a power of two.
    assert((alignment & (alignment - 1)) == 0);

    uintptr_t ptr = (uintptr_t) (memory->buffer + memory->size + (alignment - 1));
    uint8_t *result = (uint8_t *) (ptr & ~(alignment - 1));

    // since result and buffer are uint8_t*, this gives us bytes.
    size_t real_size = (result + size) - (memory->buffer + memory->size);
    assert(memory->size + real_size <= memory->capacity);

    memory->size += real_size;
    return result;
}

static inline
void memory_clean(Memory *memory)
{
    assert(memory);
    memory->size = 0;
}

#endif  // MEMORY_H_
