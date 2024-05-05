#ifndef MATH64_H
#define MATH64_H
/* math64.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* 64 bit arithmetic for platforms not having a 64 bit integer               */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

typedef struct {
    LongWord low;
    LongWord high;
} t64;

extern void add64(t64* pRes, t64 const* pA, t64 const* pB);

extern void sub64(t64* pRes, t64 const* pA, t64 const* pB);

extern void mul64(t64* pRes, t64 const* pA, t64 const* pB);

extern void div64(t64* pRes, t64 const* pA, t64 const* pB);

#endif /* MATH64_H */
