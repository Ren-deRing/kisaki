// SPDX-License-Identifier: GPL-2.0-only

#include <kisaki/string.h>

#include <stdint.h>

size_t strlen(const char *s) {
    const char *p = s;
    while (*p) { p++; } // loop until reaches \0
    return (size_t)(p - s);
}

void *memcpy(void *dest, const void *src, size_t count) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    for (size_t i = 0; i < count; i++) {
        d[i] = s[i];
    }
    return dest;
}
