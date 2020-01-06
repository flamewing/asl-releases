#ifndef _FUNCTION_H
#define _FUNCTION_H
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

typedef struct
{
  const char *pName;
  Byte MinNumArgs, MaxNumArgs;
  Byte ArgTypes[3];
  void (*pFunc)(TempResult *pErg, const TempResult *pArgs, unsigned ArgCnt);
} tFunction;

extern const tFunction Functions[];

#endif /* _FUNCTION_H */
