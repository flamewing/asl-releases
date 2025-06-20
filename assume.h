#ifndef AS_ASSUME_H
#define AS_ASSUME_H
/* assume.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* ASL                                                                       */
/*                                                                           */
/* ASSUME settings keeper                                                    */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

#include <stddef.h>

typedef struct as_assume_rec {
    char const* p_name;
    LongInt*    p_dest;
    LongInt     min_value, max_value;
    LongInt     nothing_value;
    void (*p_post_proc)(void);
} as_assume_rec_t;

extern void assume_set(as_assume_rec_t const* p_recs, size_t rec_cnt);

extern as_assume_rec_t const* assume_lookup(char const* p_name);

extern void assume_append(
        as_assume_rec_t** pp_recs, size_t* p_rec_cnt, char const* p_name, LongInt* p_dest,
        LongInt min_value, LongInt max_value, LongInt nothing_value,
        void (*p_post_proc)(void));

extern void assume_dump(as_assume_rec_t const* p_recs, size_t rec_cnt);

#endif /* AS_ASSUME_H */
