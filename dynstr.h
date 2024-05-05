#ifndef DYNSTR_H
#define DYNSTR_H
/* dynstr.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Port                                                                   */
/*                                                                           */
/* Handling of strings with dynamic allocation                               */
/*                                                                           */
/*****************************************************************************/

#include <stddef.h>
#include <stdio.h>

typedef struct as_dynstr {
    size_t capacity;
    char*  p_str;
    int    dynamic;
} as_dynstr_t;

/* add one character more for terminating NUL */

#define as_dynstr_roundup_len(len) (((len) + 128) & ~127)

extern void as_dynstr_ini(as_dynstr_t* p_str, size_t ini_alloc_len);

extern void as_dynstr_ini_clone(as_dynstr_t* p_str, as_dynstr_t const* p_src);

extern void as_dynstr_ini_c_str(as_dynstr_t* p_str, char const* p_src);

extern int as_dynstr_realloc(as_dynstr_t* p_str, size_t new_alloc_len);

extern void as_dynstr_free(as_dynstr_t* p_str);

extern size_t as_dynstr_copy(as_dynstr_t* p_dest, as_dynstr_t const* p_src);

extern size_t as_dynstr_copy_c_str(as_dynstr_t* p_dest, char const* p_src);

extern size_t as_dynstr_append_c_str(as_dynstr_t* p_dest, char const* p_src);

extern void as_dynstr_dump_hex(FILE* p_file, as_dynstr_t const* p_str);

#endif /* DYNSTR_H */
