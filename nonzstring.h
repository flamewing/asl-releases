#ifndef NONZSTRING_H
#define NONZSTRING_H
/* nonzstring.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Port                                                                   */
/*                                                                           */
/* Handling of strings without NUL termination                               */
/*                                                                           */
/*****************************************************************************/

#include <stddef.h>
#include <stdio.h>

struct as_nonz_dynstr {
    size_t len, capacity;
    char*  p_str;
};
typedef struct as_nonz_dynstr as_nonz_dynstr_t;

#define as_nonz_dynstr_roundup_len(len) (((len) + 127) & ~127)

extern void as_nonz_dynstr_ini(as_nonz_dynstr_t* p_str, size_t ini_capacity);

extern void as_nonz_dynstr_ini_c_str(as_nonz_dynstr_t* p_str, char const* p_src);

extern void as_nonz_dynstr_realloc(as_nonz_dynstr_t* p_str, size_t new_capacity);

extern void as_nonz_dynstr_free(as_nonz_dynstr_t* p_str);

extern size_t as_nonz_dynstr_to_c_str(
        char* p_dest, as_nonz_dynstr_t const* p_src, size_t dest_len);

extern size_t as_nonz_dynstr_copy(
        as_nonz_dynstr_t* p_dest, as_nonz_dynstr_t const* p_src);

extern size_t as_nonz_dynstr_append_raw(
        as_nonz_dynstr_t* p_dest, char const* p_src, int src_len); /* -1 -> strlen */

extern size_t as_nonz_dynstr_copy_c_str(as_nonz_dynstr_t* p_dest, char const* p_src);

extern size_t as_nonz_dynstr_append(
        as_nonz_dynstr_t* p_dest, as_nonz_dynstr_t const* p_src);

extern int as_nonz_dynstr_cmp(
        as_nonz_dynstr_t const* p_str1, as_nonz_dynstr_t const* p_str2);

extern int as_nonz_dynstr_find(
        as_nonz_dynstr_t const* p_haystack, as_nonz_dynstr_t const* p_needle);

extern void as_nonz_dynstr_dump_hex(FILE* p_file, as_nonz_dynstr_t const* p_str);

#endif /* NONZSTRING_H */
