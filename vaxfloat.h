#ifndef VAXFLOAT_H
#define VAXFLOAT_H
/* vaxfloat.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* VAX->IEEE Floating Point Conversion on host                               */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

extern void VAXF_2_Single(Byte *pDest, float inp);

extern void VAXD_2_Double(Byte *pDest, Double inp);

extern void VAXD_2_LongDouble(Byte *pDest, Double inp);

#endif /* VAXFLOAT_H */
