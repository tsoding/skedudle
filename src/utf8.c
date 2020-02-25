#include <assert.h>
#include <stdlib.h>
#include "utf8.h"

Utf8_Chunk utf8_encode_rune(uint32_t rune)
{
    if (0x0000 <= rune && rune <= 0x007F) {
        return (Utf8_Chunk) {
            .size = 1,
            .buffer = {rune}
        };
    } else if (0x0080 <= rune && rune <= 0x07FF) {
        return (Utf8_Chunk) {
            .size = 2,
            .buffer = {((rune >> 6) & 0b00011111) | 0b11000000,
                       (rune        & 0b00111111) | 0b10000000}
        };
    } else if (0x0800 <= rune && rune <= 0xFFFF) {
        return (Utf8_Chunk){
            .size = 3,
            .buffer = {((rune >> 12) & 0b00001111) | 0b11100000,
                       ((rune >> 6)  & 0b00111111) | 0b10000000,
                       (rune         & 0b00111111) | 0b10000000}
        };
    } else if (0x10000 <= rune && rune <= 0x10FFFF) {
        return (Utf8_Chunk){
            .size = 4,
            .buffer = {((rune >> 18) & 0b00000111) | 0b11110000,
                       ((rune >> 12) & 0b00111111) | 0b10000000,
                       ((rune >> 6)  & 0b00111111) | 0b10000000,
                       (rune         & 0b00111111) | 0b10000000}
        };
    } else {
        return (Utf8_Chunk){0};
    }
}
