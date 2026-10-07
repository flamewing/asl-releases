/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "strcomp.h"

#include <stdio.h>
#include <string.h>

static int check(tStrComp const* dest, char const* text, int column) {
    if (strcmp(dest->str.p_str, text) || dest->Pos.StartCol != column
        || dest->Pos.Len != strlen(text)) {
        fprintf(stderr, "Incorrect bounded substring or source position\n");
        return 1;
    }
    return 0;
}

int main(void) {
    char source[] = "0123456789abcdef";
    char storage[10];
    tStrComp src, fixed, dynamic, ref;
    int failed = 0;

    StrCompMkTemp(&src, source, sizeof(source));
    src.Pos.StartCol = 100;
    src.Pos.Len = 1; /* Diagnostic span metadata need not equal string length. */
    memset(storage, 0x5a, sizeof(storage));
    StrCompMkTemp(&fixed, storage + 1, 8);
    StrCompCopySubKnown(&fixed, &src, 3, 13);
    failed |= check(&fixed, "3456789", 103);
    failed |= storage[0] != 0x5a || storage[9] != 0x5a;

    StrCompAlloc(&dynamic, 4);
    StrCompCopySubKnown(&dynamic, &src, 4, 12);
    failed |= check(&dynamic, "456789abcdef", 104);
    StrCompCopySubKnown(&dynamic, &src, 16, 0);
    failed |= check(&dynamic, "", 116);

    StrCompRefRight(&ref, &src, 5);
    StrCompCopySubKnown(&dynamic, &ref, 2, 3);
    failed |= check(&dynamic, "789", 107);
    StrCompFree(&dynamic);
    return failed;
}
