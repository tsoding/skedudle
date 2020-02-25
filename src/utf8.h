#ifndef UTF8_H_
#define UTF8_H_

#include <stdint.h>

#define UTF8_CHUNK_CAPACITY 4

typedef struct {
    size_t size;
    uint8_t buffer[UTF8_CHUNK_CAPACITY];
} Utf8_Chunk;

Utf8_Chunk utf8_encode_rune(uint32_t rune);

#endif  // UTF8_H_
