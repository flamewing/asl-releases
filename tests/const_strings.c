/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "stdinc.h"

#include "asmsub.h"
#include "nls.h"
#include "strutil.h"

#include <string.h>

static int check(int mismatch, int line) {
    if (mismatch) {
        fprintf(stderr, "String contract mismatch at line %d\n", line);
    }
    return mismatch;
}

static Boolean ignore_quote(char const* start, char const* quote) {
    UNUSED(start);
    UNUSED(quote);
    return False;
}

int main(void) {
    static char const readonly[] = "a'b,c',d";
    static char const strings[]  = ",\0";
    char              writable[] = "a'b,c',d";
    char              upper[]    = "ab 'cd' \"ef\" gh";
    char              empty[]    = "";
    char              matched[]  = "xabc";
    char              capture    = 0;
    char*             position;
    int               failed = 0;
    strutil_init();
    nls_init();
    if (!NLS_Initialize(NULL, NULL)) {
        return 1;
    }
    QualifyQuote = NULL;
    failed |= check((QuotPosConst(readonly, ',') != readonly + 6), __LINE__);
    failed |= check((QuotMultPosConst(readonly, ",;") != readonly + 6), __LINE__);
    failed |= check((QuotSMultPosConst(readonly, strings) != readonly + 6), __LINE__);
    failed |= check(
            (QuotPosConstQualify(readonly, ',', ignore_quote) != readonly + 3), __LINE__);
    failed |= check((QuotPosConst(readonly, ';') != NULL), __LINE__);
    position = QuotPos(writable, ',');
    failed |= check((position != writable + 6), __LINE__);
    if (position) {
        *position = ';';
    }
    failed |= check((strcmp(writable, "a'b,c';d") != 0), __LINE__);
    failed |= check((RQuotPos(writable, ';') != writable + 6), __LINE__);
    UpString(upper);
    failed |= check((strcmp(upper, "AB 'cd' \"ef\" GH") != 0), __LINE__);
    failed |= check((strrmultchr("", ",;") != NULL), __LINE__);
    failed |= check((strrmultchr(readonly, ",") != readonly + 6), __LINE__);
    failed |= check((strrmultchr(readonly, ";") != NULL), __LINE__);
    failed |= check((MatchChars(empty, "?", "a", &capture) != NULL), __LINE__);
    failed |= check((MatchCharsRev(empty, "?", "a", &capture) != NULL), __LINE__);
    failed |= check((MatchCharsRev(empty, "") != empty), __LINE__);
    failed |= check((MatchCharsRev(matched, "ABC") != matched + 1), __LINE__);
    failed |= check(
            (MatchCharsRev(matched, " ?", "C", &capture) != matched + 3
             || capture != 'c'),
            __LINE__);
    failed |= check(
            (MatchChars(matched, "X?", "A", &capture) != matched + 2 || capture != 'a'),
            __LINE__);
    if (failed) {
        fprintf(stderr, "String search constness or boundary behavior changed\n");
    }
    return failed;
}
