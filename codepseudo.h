#ifndef _CODEPSEUDO_H
#define _CODEPSEUDO_H
/* codepseudo.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Haeufiger benutzte Pseudo-Befehle                                         */
/*                                                                           */
/*****************************************************************************/

struct _ASSUMERec;
  
extern int FindInst(void *Field, int Size, int Count);

extern Boolean IsIndirectGen(const char *Asc, const char *pBeginEnd);
extern Boolean IsIndirect(const char *Asc);

extern int FindDispBaseSplit(const char *pArg, int *pArgLen);

extern void CodeEquate(ShortInt DestSeg, LargeInt Min, LargeInt Max);

extern Boolean QualifyQuote_SingleQuoteConstant(const char *pStart, const char *pQuotePos);

#endif /* _CODEPSEUDO_H */
