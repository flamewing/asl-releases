/* strcomp.c */
/*****************************************************************************/
/* Macro Assembler AS                                                        */
/*                                                                           */
/* Definition of a source line's component present after parsing             */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include "datatypes.h"
#include "strcomp.h"

void StrCompAlloc(tStrComp *pComp)
{
  pComp->Str = (char*)malloc(STRINGSIZE);
  StrCompReset(pComp);
}

void StrCompReset(tStrComp *pComp)
{
  LineCompReset(&pComp->Pos);
  *pComp->Str = '\0';
}

void LineCompReset(tLineComp *pComp)
{
  pComp->StartCol = -1;
  pComp->Len = 0;
}

void StrCompCopy(tStrComp *pDest, tStrComp *pSrc)
{
  pDest->Pos = pSrc->Pos;
  strcpy(pDest->Str, pSrc->Str);
}
