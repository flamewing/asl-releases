/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Include the implementation to exercise locale formats with weekday and
   day-of-year fields regardless of the machine's installed locale. */
#include "../nls.c"

int main(void) {
    char result[100];
    NLSInfo.DateFmtStr = "%Y-%m-%d %w %j";
    Locale_DateString(2024, 2, 29, result, sizeof(result));
    if (strcmp(result, "2024-02-29 4 060")) {
        return 1;
    }
    Locale_DateString(2026, 10, 11, result, sizeof(result));
    if (strcmp(result, "2026-10-11 0 284")) {
        return 1;
    }
    NLSInfo.TimeFmtStr = "%H:%M:%S %w %j";
    Locale_TimeString(23, 59, 58, 0, result, sizeof(result));
    return strcmp(result, "23:59:58 1 001") != 0;
}
