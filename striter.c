/* striter.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* String Iteration                                                          */
/*                                                                           */
/*****************************************************************************/

#include "striter.h"

#if defined(_MSC_VER)
/* Preserve the original MSVC iterator implementation and code layout. */
typedef struct {
    Boolean this_escaped, next_escaped;
} iter_data_t;
#include "striter_core.h"
#define as_iterate_quote_core iterate_quote_core
#else
typedef as_quoted_iterator_state_t iter_data_t;
#endif

/*!------------------------------------------------------------------------
 * \fn     as_iterate_str(const char *p_str, as_quoted_iterator_cb_t callback, as_quoted_iterator_cb_data_t *p_cb_data)
 * \brief  iterate through string, detecting quoted areas
 * \param  p_str string to iterate through
 * \param  callback is called for all characters outside quoted areas
 * \param  p_cb_data callback data
 * ------------------------------------------------------------------------ */

void as_iterate_str(
        char const* p_str, as_quoted_iterator_cb_t callback,
        as_quoted_iterator_cb_data_t* p_cb_data) {
    iter_data_t iter_data;
    int         n_extra_skip;
    char const* p_run;

    p_cb_data->p_str           = p_str;
    p_cb_data->in_single_quote = p_cb_data->in_double_quote = False;

    for (p_run = p_str, iter_data.this_escaped               = False; *p_run;
         p_run += (1 + n_extra_skip), iter_data.this_escaped = iter_data.next_escaped) {
        n_extra_skip = 0;

        (void)as_iterate_quote_core(p_run, p_cb_data, &iter_data);
        n_extra_skip = callback(p_run, p_cb_data);
        if (n_extra_skip < 0) {
            return;
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     as_iterate_str_quoted(const char *p_str, as_quoted_iterator_cb_t callback, as_quoted_iterator_cb_data_t *p_cb_data)
 * \brief  iterate through string, skipping quoted areas
 * \param  p_str string to iterate through
 * \param  callback is called for all characters outside quoted areas
 * \param  p_cb_data callback data
 * ------------------------------------------------------------------------ */

void as_iterate_str_quoted(
        char const* p_str, as_quoted_iterator_cb_t callback,
        as_quoted_iterator_cb_data_t* p_cb_data) {
    iter_data_t iter_data;
    int         n_extra_skip;
    char const* p_run;

    p_cb_data->p_str           = p_str;
    p_cb_data->in_single_quote = p_cb_data->in_double_quote = False;

    for (p_run = p_str, iter_data.this_escaped               = False; *p_run;
         p_run += (1 + n_extra_skip), iter_data.this_escaped = iter_data.next_escaped) {
        if (p_cb_data->callback_before && !p_cb_data->in_single_quote
            && !p_cb_data->in_double_quote) {
            n_extra_skip = callback(p_run, p_cb_data);
            if (n_extra_skip < 0) {
                return;
            }
        } else {
            n_extra_skip = 0;
        }

        if (!as_iterate_quote_core(p_run, p_cb_data, &iter_data)
            && !p_cb_data->callback_before && !p_cb_data->in_single_quote
            && !p_cb_data->in_double_quote) {
            n_extra_skip = callback(p_run, p_cb_data);
            if (n_extra_skip < 0) {
                return;
            }
        }
    }
}
