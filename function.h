#ifndef FUNCTION_H
#define FUNCTION_H
/* function.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* built-in (non-user-defined) functions                                     */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"
#include "tempresult.h"

#include <stddef.h>

typedef struct {
    char const* pName;
    Byte        MinNumArgs, MaxNumArgs;
    Byte        ArgTypes[3];
    Boolean (*pFunc)(TempResult* pErg, TempResult const* pArgs, unsigned ArgCnt);
} tFunction;

extern tFunction const* function_find(char const* p_name);

#endif /* FUNCTION_H */
