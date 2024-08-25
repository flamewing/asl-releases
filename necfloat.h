#ifndef AS_NECFLOAT_H
#define AS_NECFLOAT_H
/* necfloat.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* NEC<->IEEE Floating Point Conversion on host                              */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

int as_float_2_nec_4(as_float_t inp, LongWord* p_dest);

#endif /* AS_NECFLOAT_H */
