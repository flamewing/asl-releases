#ifndef AS_TIFLOAT_H
#define AS_TIFLOAT_H
/* tifloat.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* IEEE -> TI DSP Floating Point Conversion on host                          */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

int as_float_2_ti2(as_float_t inp, Word* p_dest);

int as_float_2_ti4(as_float_t inp, LongWord* p_dest);

int as_float_2_ti5(as_float_t Inp, LongWord* p_dest_l, LongWord* p_dest_h);

int as_float_2_tms340_flt4(as_float_t inp, Byte* p_dest, Boolean needs_big);
int as_float_2_tms340_flt8(as_float_t inp, Byte* p_dest, Boolean needs_big);

#endif /* AS_TIFLOAT_H */
