/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "as_runtime.h"

#include <stdlib.h>

static as_exit_handler_t exit_handler;

as_exit_handler_t as_set_exit_handler(as_exit_handler_t handler) {
    as_exit_handler_t previous = exit_handler;
    exit_handler               = handler;
    return previous;
}

_Noreturn void as_runtime_exit(int status) {
    if (exit_handler) {
        exit_handler(status);
    }
    exit(status);
}
