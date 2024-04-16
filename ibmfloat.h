#ifndef IBMLFLOAT_H
#define IBMLFLOAT_H
/* ibmfloat.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* IBM Floating Point Format                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
 * Includes
 *****************************************************************************/

#include "stdinc.h"
#include <math.h>

Boolean Double2IBMFloat(Word *pDest, double Src, Boolean ToDouble);

#endif /* IBMLFLOAT_H */
