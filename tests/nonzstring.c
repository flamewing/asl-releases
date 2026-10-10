/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "nonzstring.h"

#include <stdio.h>
#include <string.h>

int main(void) {
    as_nonz_dynstr_t empty, dest;
    char text[8] = "dirty";
    int failed = 0;

    as_nonz_dynstr_ini_c_str(&empty, "");
    as_nonz_dynstr_ini(&dest, 0);
    if (as_nonz_dynstr_append_raw(&dest, NULL, 0)
        || as_nonz_dynstr_append_raw(&dest, "", -1)
        || as_nonz_dynstr_copy(&dest, &empty)
        || as_nonz_dynstr_to_c_str(text, &empty, sizeof(text))
        || text[0] || dest.len || dest.capacity || dest.p_str) {
        fprintf(stderr, "Empty string operation changed the result or allocation\n");
        failed = 1;
    }
    if (as_nonz_dynstr_append_raw(&dest, "abc", 3) != 3
        || as_nonz_dynstr_append_raw(&dest, NULL, 0)
        || dest.len != 3 || memcmp(dest.p_str, "abc", 3)
        || as_nonz_dynstr_copy(&dest, &empty) || dest.len
        || as_nonz_dynstr_to_c_str(text, &dest, sizeof(text)) || text[0]) {
        fprintf(stderr, "Empty append or copy mishandled a populated string\n");
        failed = 1;
    }
    as_nonz_dynstr_free(&dest);
    as_nonz_dynstr_free(&empty);
    return failed;
}
