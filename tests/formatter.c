/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "strutil.h"
#include "dynstr.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static int check(size_t capacity, char const* expected, char const* format, ...) {
    unsigned char storage[130];
    va_list args;
    int count;
    size_t length = strlen(expected);
    memset(storage, 0xa5, sizeof(storage));
    va_start(args, format);
    count = as_vsnprintf((char*)storage + 1, capacity, format, args);
    va_end(args);
    if (count != (int)length || memcmp(storage + 1, expected, length + 1)
        || storage[0] != 0xa5 || storage[capacity + 1] != 0xa5) {
        fprintf(stderr, "Incorrect format result, count, or bounds: %s\n", format);
        return 1;
    }
    return 0;
}

int main(void) {
    as_dynstr_t dynamic;
    char raw[16], empty_guard[2] = {'x', '\0'};
    int failed = 0, count;
    strutil_init();
    failed |= check(80, "literal text", "literal text");
    failed |= check(80, "pre 007 % tail", "pre %03u %% tail", 7u);
    failed |= check(80, "[  abc][xy   ]", "[%5.3s][%-5s]", "abcdef", "xy");
    failed |= check(80, "[00FF][+7]", "[%04.16u][%+d]", 255u, 7);
    failed |= check(5, "abcd", "abcdef%sTAIL", "XYZ");
    failed |= check(1, "", "literal %% %u", 7u);
    failed |= as_snprintf(empty_guard, 0, "abc") != 0 || empty_guard[0] != 'x';
    count = as_snprintf(raw, sizeof(raw), "head%cTAIL", 0);
    failed |= count != 9 || memcmp(raw, "head\0TAIL", 10);
    as_dynstr_ini(&dynamic, 4);
    count = as_sdprintf(&dynamic, "prefix %u and a long literal suffix", 17u);
    failed |= count != 35
              || strcmp(dynamic.p_str, "prefix 17 and a long literal suffix");
    count = as_sdprcatf(&dynamic, " / %s %% end", "text");
    failed |= count != 13
              || strcmp(dynamic.p_str, "prefix 17 and a long literal suffix / text % end");
    as_dynstr_free(&dynamic);
    return failed;
}
