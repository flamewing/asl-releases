#ifndef DYNSTR_NLS_H
#define DYNSTR_NLS_H
/* dynstr.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Port                                                                   */
/*                                                                           */
/* Handling of strings with dynamic allocation - NLS extensions              */
/*                                                                           */
/*****************************************************************************/

#include <stddef.h>

struct as_dynstr;

extern size_t as_dynstr_append_upr(
        struct as_dynstr* p_dest, char const* p_src, size_t len);

extern size_t as_dynstr_append_c_str_upr(struct as_dynstr* p_dest, char const* p_src);

#endif /* DYNSTR_NLS_H */
