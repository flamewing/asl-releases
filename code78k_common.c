/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "code78k_common.h"

#include <stdlib.h>

unsigned as_78k_mem_tail_count(int count) {
    if (count < 1 || count > 3) {
        abort();
    }
    return (unsigned)(count - 1);
}
