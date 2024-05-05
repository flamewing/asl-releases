#ifndef FUNCTION_H
#define FUNCTION_H
/* function.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* internal holder for int/float/string                                      */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"
#include "tempresult.h"

typedef struct {
    char const* pName;
    Byte        MinNumArgs, MaxNumArgs;
    Byte        ArgTypes[3];
    void (*pFunc)(TempResult* pErg, TempResult const* pArgs, unsigned ArgCnt);
} tFunction;

extern tFunction const Functions[];

#endif /* FUNCTION_H */
