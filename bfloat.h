#ifndef BFLOAT_H
#define BFLOAT_H
/* bfloat.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* Brain Floating Point Handling                                             */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

extern int as_float_2_bfloat16(as_float_t inp, Byte* pDest, Boolean NeedsBig);

#endif /* BFLOAT_H */
