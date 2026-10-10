/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "strcomp_view.h"
#include "striter.h"
#include <stdio.h>

/* Oracle follows the existing QuotPos callback and shared quote iterator. */
typedef struct {
    as_quoted_iterator_cb_data_t quote;
    ShortInt parentheses, brackets;
    char const* found;
} search_t;
static int reference_callback(char const* p, as_quoted_iterator_cb_data_t* cb) {
    search_t* state = (search_t*)cb;
    if (!state->parentheses && !state->brackets && *p == ',') {
        state->found = p;
        return -1;
    }
    switch (*p) {
    case '(': if (!state->brackets) state->parentheses++; break;
    case ')': if (!state->brackets) state->parentheses--; break;
    case '[': if (!state->parentheses) state->brackets++; break;
    case ']': if (!state->parentheses) state->brackets--; break;
    }
    return 0;
}

int main(void) {
    static char const alphabet[] = "a,'\"\\()[]";
    static char const bytes[] = {'X', ' ', 'a', ' ', 'Y'};
    as_strcomp_view_t view = {{bytes + 1, 3}, {17, 999}};
    as_strcomp_view_t trimmed, slice;
    tStrComp source;
    char input[8], terminated[8];
    size_t length, combinations = 1, number, i;
    int failed = 0;
    strutil_init();
    source.Pos = view.pos;
    source.str.p_str = (char*)view.text.data;
    view = as_strcomp_view_from(&source, 3);
    trimmed = as_strcomp_view_trim(view);
    failed |= trimmed.text.data != bytes + 2 || trimmed.text.len != 1
              || trimmed.pos.StartCol != 18 || trimmed.pos.Len != 1;
    slice = as_strcomp_view_sub(view, 2, (size_t)-1);
    failed |= slice.text.data != bytes + 3 || slice.text.len != 1
              || slice.pos.StartCol != 19 || slice.pos.Len != 1;
    slice = as_strcomp_view_sub(view, (size_t)-1, (size_t)-1);
    failed |= slice.text.len || slice.pos.StartCol != 20;
    for (length = 0; length <= 5; length++) {
        for (number = 0; number < combinations; number++) {
            size_t digits = number;
            search_t reference;
            char const* found;
            memset(&reference, 0, sizeof(reference));
            reference.quote.callback_before = True;
            for (i = 0; i < length; i++) {
                input[i] = terminated[i] = alphabet[digits % (sizeof(alphabet) - 1)];
                digits /= sizeof(alphabet) - 1;
            }
            /* These bytes must never extend the bounded search. */
            input[length] = ',';
            input[length + 1] = '"';
            terminated[length] = 0;
            as_iterate_str_quoted(terminated, reference_callback, &reference.quote);
            found = as_strref_find_quoted((as_strref_t){input, length}, ',');
            if (!!found != !!reference.found
                || (found && found - input != reference.found - terminated)) {
                fprintf(stderr, "Counted delimiter disagrees with legacy scanner\n");
                return 1;
            }
        }
        combinations *= sizeof(alphabet) - 1;
    }
    return failed;
}
