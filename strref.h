/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#ifndef STRREF_H
#define STRREF_H

#include "strutil.h"
#include <stddef.h>
#include <string.h>

/* Borrowed immutable bytes. The owner must remain alive and unchanged while
   this view is used. There is no NUL-termination or ownership guarantee.
   A NULL pointer is permitted only for an empty view. */
typedef struct {
    char const* data;
    size_t len;
} as_strref_t;

/* Clamp slices to the view, including oversized counts without overflow. */
static inline as_strref_t as_strref_sub(as_strref_t text, size_t offset, size_t count) {
    if (offset > text.len) offset = text.len;
    if (offset) text.data += offset;
    text.len -= offset;
    if (count < text.len) text.len = count;
    return text;
}

static inline as_strref_t as_strref_trim_left(as_strref_t text) {
    while (text.len && as_isspace(*text.data)) {
        text.data++;
        text.len--;
    }
    return text;
}

static inline as_strref_t as_strref_trim_right(as_strref_t text) {
    while (text.len && as_isspace(text.data[text.len - 1])) text.len--;
    return text;
}

static inline as_strref_t as_strref_trim(as_strref_t text) {
    return as_strref_trim_right(as_strref_trim_left(text));
}

static inline int as_strref_equal(as_strref_t left, as_strref_t right) {
    return left.len == right.len
           && (!left.len || !memcmp(left.data, right.data, left.len));
}

static inline int as_strref_starts_with(as_strref_t text, as_strref_t prefix) {
    return prefix.len <= text.len
           && (!prefix.len || !memcmp(text.data, prefix.data, prefix.len));
}

static inline char const* as_strref_find(as_strref_t text, int byte) {
    return text.len ? (char const*)memchr(text.data, byte, text.len) : NULL;
}

#endif
