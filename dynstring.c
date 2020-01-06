/* dynstring.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Port                                                                   */
/*                                                                           */
/* Dynamic String Handling                                                   */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include "strutil.h"

#include "dynstring.h"

unsigned DynString2CString(char *pDest, const tDynString *pSrc, unsigned DestLen)
{
  if (DestLen > 0)
  {
    unsigned TransLen = DestLen - 1;

    if (TransLen > pSrc->Length)
      TransLen = pSrc->Length;
    memcpy(pDest, pSrc->Contents, TransLen);
    pDest[TransLen] = '\0';
    return TransLen;
  }
  else
    return 0;
}

unsigned DynString2DynString(tDynString *pDest, const tDynString *pSrc)
{
  memcpy(pDest->Contents, pSrc->Contents, pDest->Length = pSrc->Length);
  return pDest->Length;
}

unsigned DynStringAppend(tDynString *pDest, const char *pSrc, int SrcLen)
{
  int TransLen;

  if (SrcLen < 0)
    SrcLen = strlen(pSrc);

  TransLen = sizeof(pDest->Contents) - pDest->Length;
  if (SrcLen < TransLen)
    TransLen = SrcLen;
  memcpy(pDest->Contents + pDest->Length, pSrc, TransLen);
  pDest->Length += TransLen;
  return TransLen;
}

unsigned CString2DynString(tDynString *pDest, const char *pSrc)
{
  pDest->Length = 0;
  return DynStringAppend(pDest, pSrc, -1);
}

unsigned DynStringAppendDynString(tDynString *pDest, const tDynString *pSrc)
{
  return DynStringAppend(pDest, pSrc->Contents, pSrc->Length);
}

int DynStringCmp(const tDynString *pStr1, const tDynString *pStr2)
{
  return strlencmp(pStr1->Contents, pStr1->Length, pStr2->Contents, pStr2->Length);
}

int DynStringFind(const tDynString *pHaystack, const tDynString *pNeedle)
{
  int pos, maxpos = ((int)pHaystack->Length) - ((int)pNeedle->Length);

  for (pos = 0; pos <= maxpos; pos++)
    if (!memcmp(pHaystack->Contents + pos, pNeedle->Contents, pNeedle->Length))
      return pos;

  return -1;
}
