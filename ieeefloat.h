#ifndef IEEEFLOAT_H
#define IEEEFLOAT_H
/* ieeefloat.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* IEEE Floating Point Handling                                              */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

enum
{
  AS_FP_NORMAL,
  AS_FP_SUBNORMAL,
  AS_FP_NAN,
  AS_FP_INFINITE
};

extern int as_fpclassify(Double inp);

extern Boolean Double_2_ieee2(Double inp, Byte *pDest, Boolean NeedsBig);

extern void Double_2_ieee4(Double inp, Byte *pDest, Boolean NeedsBig);

extern void Double_2_ieee8(Double inp, Byte *pDest, Boolean NeedsBig);

extern void Double_2_ieee10(Double inp, Byte *pDest, Boolean NeedsBig);

#endif /* IEEEFLOAT_H */
