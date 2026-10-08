/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "strutil.h"

#include <locale.h>
#include <stdio.h>

static int check_bytes(void) {
    int c;
    for (c = -512; c < 768; c++) {
        if (as_isspace(c) != !!isspace(chartouint(c))) {
            fprintf(stderr, "Incorrect whitespace classification for byte %u\n",
                    chartouint(c));
            return 1;
        }
    }
    return 0;
}

int main(void) {
    int failed = check_bytes(); /* Valid in the initial C locale before setup. */
    char const* locales[] = {"", "C", "C.UTF-8"};
    unsigned i;
    strutil_init();
    failed |= check_bytes();
    for (i = 0; i < sizeof(locales) / sizeof(locales[0]); i++) {
        if (setlocale(LC_CTYPE, locales[i])) {
            /* Explicit initialization refreshes a changed character locale. */
            strutil_init();
            failed |= check_bytes();
        }
    }
    if (!setlocale(LC_CTYPE, "C")) {
        return 1;
    }
    strutil_init();
    failed |= check_bytes();
    return failed;
}
