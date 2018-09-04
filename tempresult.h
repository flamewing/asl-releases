#ifndef _TEMPRESULT_H
#define _TEMPRESULT_H
/* tempresult.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* internal holder for int/float/string                                      */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"
#include "dynstring.h"

typedef enum {TempNone = 0, TempInt = 1, TempFloat = 2, TempString = 4, TempAll = 7} TempType;

struct sRelocEntry;

struct sTempResult
{
  TempType Typ;
  LongWord Flags;
  struct sRelocEntry *Relocs;
  union
  {
    LargeInt Int;
    Double Float;
    tDynString Ascii;
  } Contents;
};
typedef struct sTempResult TempResult;

extern int TempResultToFloat(TempResult *pResult);

extern int TempResultToPlainString(char *pDest, const TempResult *pResult, unsigned DestSize);

#endif /* _TEMPRESULT_H */
