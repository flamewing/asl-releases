#ifndef CODECR16_H
#define CODECR16_H

/* codecr16.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Code Generator National Semiconductor CR16A/B                             */
/*                                                                           */
/*****************************************************************************/

#include "asmdef.h"

typedef enum {
    e_cr16_gen_a = 0,
    e_cr16_gen_b = 1,
    e_cr16_gen_c = 2
} cr16_gen_t;

extern void codecr16_init(void);

extern void codecr16_iter_conditions(void (*callback)(char const*, Word));

extern Boolean decode_cr16_vector(tStrComp const* p_arg, Word* p_dest, cr16_gen_t gen);

#endif /* CODECR16_H */
