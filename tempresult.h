#ifndef _TEMPRESULT_H
#define _TEMPRESULT_H
/* tempresult.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* internal holder for int/float/string                                      */
/*                                                                           */
/*****************************************************************************/

#include <stddef.h>

#include "datatypes.h"
#include "dynstring.h"
#include "symflags.h"
#include "symbolsize.h"

typedef enum
{
  TempNone = 0,
  TempInt = 1,
  TempFloat = 2,
  TempString = 4,
  TempReg = 8,
  TempAll = 15
} TempType;

struct sRelocEntry;

typedef unsigned tRegInt;

typedef void (*DissectRegProc)(
#ifdef __PROTOS__
char *pDest, size_t DestSize, tRegInt Value, tSymbolSize InpSize
#endif
);

typedef struct sRegDescr
{
  DissectRegProc Dissect;
  tRegInt Reg;
} tRegDescr;

struct sTempResult
{
  TempType Typ;
  tSymbolFlags Flags;
  unsigned AddrSpaceMask;
  tSymbolSize DataSize;
  struct sRelocEntry *Relocs;
  DissectRegProc DissectReg;
  union
  {
    LargeInt Int;
    Double Float;
    tDynString Ascii;
    tRegDescr RegDescr;
  } Contents;
};
typedef struct sTempResult TempResult;

extern int TempResultToFloat(TempResult *pResult);

extern int TempResultToPlainString(char *pDest, const TempResult *pResult, size_t DestSize);

#endif /* _TEMPRESULT_H */
