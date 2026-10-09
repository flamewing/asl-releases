/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "strref.h"
#include <stdio.h>

int main(void) {
    /* Nonterminated input with nonblank bytes immediately outside each view. */
    static char const source[] = {'X', ' ', '\t', 'a', 'b', ' ', '\r', 'Y'};
    static char const expected[] = {'a', 'b'};
    as_strref_t text = {source + 1, 6}, empty = {NULL, 0};
    as_strref_t value = {expected, sizeof(expected)};
    as_strref_t trimmed, prefix = {expected, 1};
    int failed = 0;
    size_t start, count;
    strutil_init();
    trimmed = as_strref_trim(text);
    failed |= trimmed.data != source + 3 || trimmed.len != 2;
    failed |= !as_strref_equal(trimmed, value);
    failed |= !as_strref_starts_with(trimmed, prefix);
    failed |= as_strref_starts_with(prefix, trimmed);
    failed |= as_strref_find(trimmed, 'b') != source + 4;
    failed |= as_strref_find(trimmed, 'Y') != NULL;
    failed |= as_strref_find(trimmed, 0) != NULL;
    failed |= as_strref_trim_left(text).len != 4;
    failed |= as_strref_trim_right(text).len != 4;
    failed |= !as_strref_equal(as_strref_trim(empty), empty);
    failed |= !as_strref_equal(as_strref_sub(empty, 1, (size_t)-1), empty);
    failed |= as_strref_find(empty, 'a') != NULL;
    failed |= !as_strref_starts_with(text, empty);
    for (start = 0; start < 10; start++) {
        for (count = 0; count < 10; count++) {
            as_strref_t slice = as_strref_sub(text, start, count);
            size_t offset = start > text.len ? text.len : start;
            size_t length = count > text.len - offset ? text.len - offset : count;
            failed |= slice.data != text.data + offset || slice.len != length;
        }
    }
    trimmed = as_strref_sub(text, 2, (size_t)-1);
    failed |= trimmed.data != source + 3 || trimmed.len != 4;
    if (failed) fprintf(stderr, "Incorrect borrowed string view bounds or trimming\n");
    return failed;
}
