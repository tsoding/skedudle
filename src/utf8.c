#include <assert.h>
#include <stdlib.h>
#include "utf8.h"

Utf8_Chunk utf8_encode_rune(uint32_t rune)
{
    const uint8_t b00000111 = (1 << 3) - 1;
    const uint8_t b00001111 = (1 << 4) - 1;
    const uint8_t b00011111 = (1 << 5) - 1;
    const uint8_t b00111111 = (1 << 6) - 1;
    const uint8_t b10000000 = ~((1 << 7) - 1);
    const uint8_t b11000000 = ~((1 << 6) - 1);
    const uint8_t b11100000 = ~((1 << 5) - 1);
    const uint8_t b11110000 = ~((1 << 4) - 1);

    if (rune <= 0x007F) {
        return (Utf8_Chunk) {
            .size = 1,
            .buffer = {rune}
        };
    } else if (0x0080 <= rune && rune <= 0x07FF) {
        return (Utf8_Chunk) {
            .size = 2,
            .buffer = {((rune >> 6) & b00011111) | b11000000,
                       (rune        & b00111111) | b10000000}
        };
    } else if (0x0800 <= rune && rune <= 0xFFFF) {
        return (Utf8_Chunk){
            .size = 3,
            .buffer = {((rune >> 12) & b00001111) | b11100000,
                       ((rune >> 6)  & b00111111) | b10000000,
                       (rune         & b00111111) | b10000000}
        };
    } else if (0x10000 <= rune && rune <= 0x10FFFF) {
        return (Utf8_Chunk){
            .size = 4,
            .buffer = {((rune >> 18) & b00000111) | b11110000,
                       ((rune >> 12) & b00111111) | b10000000,
                       ((rune >> 6)  & b00111111) | b10000000,
                       (rune         & b00111111) | b10000000}
        };
    } else {
        return (Utf8_Chunk){0};
    }
}
