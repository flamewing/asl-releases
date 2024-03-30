#ifndef STRITER_H
#define STRITER_H
/* striter.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* String Iteration                                                          */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

typedef Boolean (*as_qualify_quote_fnc_t)(char const* p_start, char const* p_quote_pos);

typedef struct as_quoted_iterator_cb_data {
    /* initialized by function */
    char const* p_str;
    Boolean     in_single_quote, in_double_quote;
    /* initialized by caller */
    Boolean                callback_before;
    as_qualify_quote_fnc_t qualify_quote;
} as_quoted_iterator_cb_data_t;

/* callback return values:
   <0 -> terminate
   >=0 -> continue and skip next n characters */
typedef int (*as_quoted_iterator_cb_t)(
        char const* p_pos, as_quoted_iterator_cb_data_t* p_cb_data);

extern void as_iterate_str(
        char const* p_str, as_quoted_iterator_cb_t callback,
        as_quoted_iterator_cb_data_t* p_cb_data);
extern void as_iterate_str_quoted(
        char const* p_str, as_quoted_iterator_cb_t callback,
        as_quoted_iterator_cb_data_t* p_cb_data);

#endif /* STRITER_H */
