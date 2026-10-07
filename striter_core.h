#ifndef STRITER_CORE_H
#define STRITER_CORE_H
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */

/* Shared private quote handling for generic and specialized scanners. */
#if defined(_MSC_VER)
static Boolean iterate_quote_core(
        char const* p_run, as_quoted_iterator_cb_data_t* p_cb_data,
        iter_data_t* p_iter_data) {
#else
static inline Boolean as_iterate_quote_core(
        char const* p_run, as_quoted_iterator_cb_data_t* p_cb_data,
        as_quoted_iterator_state_t* p_iter_data) {
#endif
    p_iter_data->next_escaped = False;
    switch (*p_run) {
    case '\\':
        if ((p_cb_data->in_single_quote || p_cb_data->in_double_quote)
            && !p_iter_data->this_escaped) {
            p_iter_data->next_escaped = True;
        }
        return True;
    case '\'':
        if (p_cb_data->in_double_quote) {
        } else if (
                !p_cb_data->in_single_quote
                && (!p_cb_data->qualify_quote
                    || p_cb_data->qualify_quote(p_cb_data->p_str, p_run))) {
            p_cb_data->in_single_quote = True;
        } else if (!p_iter_data->this_escaped) { /* skip escaped ' in '...' */
            p_cb_data->in_single_quote = False;
        }
        return True;
    case '"':
        if (p_cb_data->in_single_quote) {
        } else if (!p_cb_data->in_double_quote) {
            p_cb_data->in_double_quote = True;
        } else if (!p_iter_data->this_escaped) { /* skip escaped " in "..." */
            p_cb_data->in_double_quote = False;
        }
        return True;
    default:
        return False;
    }
}

#endif /* STRITER_CORE_H */
