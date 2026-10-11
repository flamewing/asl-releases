/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "dynstr.h"
#include "strutil.h"

#include <string.h>

static int vformat(int append, char* dest, size_t capacity, char const* format, ...) {
    va_list ap;
    va_start(ap, format);
    int result = append ? as_vsnprcatf(dest, capacity, format, ap)
                        : as_vsnprintf(dest, capacity, format, ap);
    va_end(ap);
    return result;
}

int main(void) {
    strutil_init();
    if (as_snprintf(NULL, 0, "%s", "ignored") != 0
        || as_snprcatf(NULL, 0, "%s", "ignored") != 0 || vformat(0, NULL, 0, NULL) != 0
        || vformat(1, NULL, 0, NULL) != 0) {
        return 1;
    }
    char const text[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    for (size_t capacity = 1; capacity <= 33; ++capacity) {
        if (capacity == sizeof(char*)) {
            continue; /* Intentional fatal pointer-size capacity guard. */
        }
        for (int append = 0; append <= 1; ++append) {
            for (int via_va = 0; via_va <= 1; ++via_va) {
                unsigned char storage[64];
                memset(storage, 0xa5, sizeof(storage));
                char*  dest   = (char*)storage + 8;
                size_t prefix = append && capacity > 1 ? 1 : 0;
                if (append) {
                    if (prefix) {
                        dest[0] = 'P';
                    }
                    dest[prefix] = '\0';
                }
                int    result = via_va   ? vformat(append, dest, capacity, "%s", text)
                                : append ? as_snprcatf(dest, capacity, "%s", text)
                                         : as_snprintf(dest, capacity, "%s", text);
                size_t stored = capacity - prefix - 1;
                if (result != (int)stored || strlen(dest) != capacity - 1
                    || memcmp(dest + prefix, text, stored)
                    || (prefix && dest[0] != 'P')) {
                    fprintf(stderr,
                            "Format boundary: capacity=%zu append=%d va=%d return=%d "
                            "output='%s'\n",
                            capacity, append, via_va, result, dest);
                    return 1;
                }
                for (size_t z = 0; z < sizeof(storage); ++z) {
                    if ((z < 8 || z >= 8 + capacity) && storage[z] != 0xa5) {
                        return 1;
                    }
                }
            }
        }
    }
    char dest[64];
    /* Project integer precision selects radix, and lll selects LargeWord. */
    char const* custom = "%.*lllu";
    if (as_snprintf(dest, sizeof(dest), custom, 2, (LargeWord)13) != 4
        || strcmp(dest, "1101")) {
        fprintf(stderr, "Custom radix output: '%s'\n", dest);
        return 1;
    }
    if (as_snprintf(dest, sizeof(dest), "") != 0 || dest[0]) {
        return 1;
    }
    as_dynstr_t dynamic;
    as_dynstr_ini(&dynamic, 1);
    char long_text[300];
    memset(long_text, 'X', sizeof(long_text) - 1);
    long_text[sizeof(long_text) - 1] = '\0';
    int result                       = as_sdprintf(&dynamic, "%s", long_text);
    int failed                       = result != 299 || strcmp(dynamic.p_str, long_text);
    result                           = as_sdprcatf(&dynamic, "%s", "tail");
    failed |= result != 4 || strlen(dynamic.p_str) != 303
              || strcmp(dynamic.p_str + 299, "tail");
    as_dynstr_free(&dynamic);
    if (failed) {
        fprintf(stderr, "Dynamic formatting failed\n");
    }
    return failed;
}
