/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#ifndef STRCOMP_VIEW_H
#define STRCOMP_VIEW_H
#include "stdinc.h"
#include "strcomp.h"
#include "strref.h"

/* Physical text bounds are separate from diagnostic position metadata.
   Views neither own storage nor permit mutation; a live parent owns it. */
typedef struct {
    as_strref_t text;
    tLineComp pos;
} as_strcomp_view_t;

static inline as_strcomp_view_t as_strcomp_view_from(tStrComp const* source,
                                                    size_t length) {
    as_strcomp_view_t result = {{source->str.p_str, length}, source->Pos};
    return result;
}

static inline as_strcomp_view_t as_strcomp_view_sub(as_strcomp_view_t source,
                                                   size_t offset, size_t count) {
    if (offset > source.text.len) offset = source.text.len;
    source.text = as_strref_sub(source.text, offset, count);
    source.pos.StartCol += offset;
    source.pos.Len = source.text.len;
    return source;
}

static inline as_strcomp_view_t as_strcomp_view_trim(as_strcomp_view_t source) {
    as_strref_t text = as_strref_trim_left(source.text);
    source.pos.StartCol += source.text.len - text.len;
    source.text = as_strref_trim_right(text);
    source.pos.Len = source.text.len;
    return source;
}

/* Counted equivalent of unqualified QuotPos, for function argument commas.
   Preserve its independent parenthesis/square-bracket depth rules. */
static inline char const* as_strref_find_quoted(as_strref_t text, char delimiter) {
    ShortInt parentheses = 0, brackets = 0;
    char quote = 0;
    Boolean escaped = False;
    size_t i;
    for (i = 0; i < text.len; i++) {
        char byte = text.data[i];
        if (quote) {
            if (byte == quote && !escaped) quote = 0;
            escaped = byte == '\\' && !escaped;
            continue;
        }
        if (!parentheses && !brackets && byte == delimiter) return text.data + i;
        switch (byte) {
        case '(':
            if (!brackets) parentheses++;
            break;
        case ')':
            if (!brackets) parentheses--;
            break;
        case '[':
            if (!parentheses) brackets++;
            break;
        case ']':
            if (!parentheses) brackets--;
            break;
        case '\'':
        case '"':
            quote = byte;
            escaped = False;
            break;
        }
    }
    return NULL;
}
#endif
