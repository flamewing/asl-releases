/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "strutil.h"

#include <stdio.h>
#include <string.h>

static int Check(char const* prefix, char const* original, size_t capacity,
                 char const* expected) {
    unsigned char storage[96], before[96];
    char* dest = (char*)storage + 16;
    size_t i, length = strlen(expected);

    memset(storage, 0xa5, sizeof(storage));
    memcpy(dest, original, strlen(original) + 1);
    memcpy(before, storage, sizeof(storage));
    strmaxprep2(dest, prefix, capacity);
    if (!memchr(dest, '\0', capacity)
        || memcmp(dest, expected, length + 1)) {
        fprintf(stderr, "Incorrect prepend at capacity %lu (prefix %lu, dest %lu)\n",
                (unsigned long)capacity, (unsigned long)strlen(prefix),
                (unsigned long)strlen(original));
        return 1;
    }
    for (i = 0; i < sizeof(storage); ++i) {
        if ((i < 16 || i >= 16 + capacity) && storage[i] != before[i]) {
            fprintf(stderr, "Prepend wrote outside capacity %lu\n",
                    (unsigned long)capacity);
            return 1;
        }
    }
    return 0;
}

int main(void) {
    char prefix[129], original[65], expected[193];
    size_t capacity, src_len, dest_len;

    /* Independent expected strings for the reported bug and its boundaries. */
    if (Check("LONGNAME", ".abc", 8, "LONGNAM")
        || Check("LONG", ".abc", 8, "LONG.ab")
        || Check("pre", "abcd", 8, "preabcd")
        || Check("", "abcd", 5, "abcd")
        || Check("LONGNAME", "", 1, "")
        || Check("ABC", "x", 2, "A")) {
        return 1;
    }

    /* The contract is the concatenation, truncated to capacity - 1 bytes. */
    for (capacity = 1; capacity <= 64; ++capacity) {
        for (src_len = 0; src_len <= 128; ++src_len) {
            memset(prefix, 'P', src_len);
            prefix[src_len] = '\0';
            for (dest_len = 0; dest_len < capacity; ++dest_len) {
                size_t length = src_len + dest_len;
                memset(original, 'D', dest_len);
                original[dest_len] = '\0';
                memcpy(expected, prefix, src_len);
                memcpy(expected + src_len, original, dest_len);
                if (length >= capacity) {
                    length = capacity - 1;
                }
                expected[length] = '\0';
                if (Check(prefix, original, capacity, expected)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}
