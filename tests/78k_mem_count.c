/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "code78k_common.h"

#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

static void expected_abort(int signal_number) {
    (void)signal_number;
    _Exit(0);
}

int main(int argc, char** argv) {
    if (argc > 1) {
        int count;
        if (!strcmp(argv[1], "zero")) {
            count = 0;
        } else if (!strcmp(argv[1], "four")) {
            count = 4;
        } else if (!strcmp(argv[1], "min")) {
            count = INT_MIN;
        } else if (!strcmp(argv[1], "max")) {
            count = INT_MAX;
        } else {
            return 1;
        }
        if (signal(SIGABRT, expected_abort) == SIG_ERR) {
            return 1;
        }
        (void)as_78k_mem_tail_count(count);
        return 1; /* Invalid decoder counts must abort, including with NDEBUG. */
    }
    /* Check the exact bytes copied for all three legal operand lengths. */
    for (int count = 1; count <= 3; count++) {
        unsigned char const source[3] = {0x80, 0x12, 0x34};
        unsigned char       dest[3]   = {0xa5, 0xa5, 0xa5};
        unsigned            tail      = as_78k_mem_tail_count(count);
        if (tail != (unsigned)(count - 1)) {
            return 1;
        }
        memcpy(dest, source + 1, tail);
        for (unsigned index = 0; index < sizeof(dest); index++) {
            if (dest[index] != (index < tail ? source[index + 1] : 0xa5)) {
                return 1;
            }
        }
    }
    return 0;
}
