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

static int check_number(size_t capacity, char const* expected, LargeWord value,
                        Boolean is_signed, int width, Boolean lead_zero, char split) {
    unsigned char storage[260];
    size_t length = strlen(expected);
    int count;
    memset(storage, 0xa5, sizeof(storage));
    count = SysString((char*)storage + 1, capacity, value, 16, is_signed, width,
                      lead_zero, 'A', split);
    if (count != (int)length || storage[0] != 0xa5
        || storage[capacity + 1] != 0xa5
        || (capacity && memcmp(storage + 1, expected, length + 1))) {
        fprintf(stderr, "Incorrect bounded number conversion: capacity %u\n",
                (unsigned)capacity);
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
    failed |= check_number(0, "", 1, False, 100, False, '_');
    failed |= check_number(1, "", 1, False, 100, False, '_');
    failed |= check_number(8, "2_34_56", 0x123456, False, 100, False, '_');
    failed |= check_number(8, "0000000", 0, False, 100, False, '_');
    failed |= check_number(1, "", (LargeWord)-0x1234, True, 0, False, '_');
    failed |= check_number(2, "-", (LargeWord)-0x1234, True, 0, False, '_');
    failed |= check_number(3, "-4", (LargeWord)-0x1234, True, 0, False, '_');
    failed |= check_number(4, "-34", (LargeWord)-0x1234, True, 100, False, '_');
    failed |= check_number(5, "-_34", (LargeWord)-0x1234, True, 0, False, '_');
    failed |= check_number(3, "-F", (LargeWord)-15, True, 0, True, 0);
    failed |= check_number(4, "-0F", (LargeWord)-15, True, 0, True, 0);
    {
        char expected[130];
        size_t digits = sizeof(LargeWord) * 2;
        expected[0] = '-';
        expected[1] = '8';
        memset(expected + 2, '0', digits - 1);
        expected[digits + 1] = '\0';
        failed |= check_number(sizeof(expected), expected,
                               ((LargeWord)-1 >> 1) + 1, True, 0, False, 0);
    }
    SplitByteCharacter = '_';
    {
        char expected[100];
        memset(expected, '0', 91);
        strcpy(expected + 91, "12_34_56");
        failed |= check(100, expected, "%0100.16lllu", (LargeWord)0x123456);
    }
    return failed;
}
