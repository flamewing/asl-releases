/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#ifndef AS_RUNTIME_H
#define AS_RUNTIME_H

/* Internal boundary shared by the assembler and standalone command-line tools. */
typedef void (*as_exit_handler_t)(int status);
as_exit_handler_t as_set_exit_handler(as_exit_handler_t handler);
_Noreturn void    as_runtime_exit(int status);

#endif
