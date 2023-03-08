#ifndef CMDARG_H
#define CMDARG_H
/* cmdarg.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Verarbeitung Kommandozeilenparameter                                      */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*            2001-10-20: option string is pointer instead of array          */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"
#include "stringlists.h"

#include <stddef.h>

typedef enum {
    e_cmd_err,
    e_cmd_file,
    e_cmd_ok,
    e_cmd_arg
} as_cmd_result_t;

typedef as_cmd_result_t (*as_cmd_callback_t)(Boolean neg_flag, char const* p_arg);

typedef void (*as_cmd_err_callback_t)(Boolean in_env, char* p_arg);

typedef struct {
    char const*       p_ident;
    as_cmd_callback_t callback;
} as_cmd_rec_t;

#define MAXPARAM 256
typedef Boolean as_cmd_processed_t[MAXPARAM + 1];

extern StringList FileArgList;

extern Boolean as_cmd_processed_empty(as_cmd_processed_t const processed);

extern void as_cmd_extend(
        as_cmd_rec_t** p_cmd_recs, size_t* p_cmd_rec_cnt, as_cmd_rec_t const* p_new_recs,
        size_t new_rec_cnt);

extern void as_cmd_process(
        int argc, char** argv, as_cmd_rec_t const* p_cmd_recs, size_t cmd_rec_cnt,
        as_cmd_processed_t unprocessed, char const* p_env_name,
        as_cmd_err_callback_t err_proc);

extern char const* GetEXEName(char const* argv0);

extern void as_cmdarg_init(char* ProgPath);

#endif /* CMDARG_H */
