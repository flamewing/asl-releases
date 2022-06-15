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

#define AS_FARG_MInt    (1 << TempInt)
#define AS_FARG_MFloat  (1 << TempFloat)
#define AS_FARG_MString (1 << TempString)
#define AS_FARG_MAll    (AS_FARG_MInt | AS_FARG_MFloat | AS_FARG_MString)

typedef struct {
    char const* pName;
    Byte        MinNumArgs, MaxNumArgs;
    Byte        ArgTypes[3];
    Boolean (*pFunc)(TempResult* pErg, TempResult const* pArgs, unsigned ArgCnt);
} tFunction;

extern tFunction const* function_find(char const* p_name);

#endif /* FUNCTION_H */
