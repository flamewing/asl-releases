#ifndef CODEPSEUDO_H
#define CODEPSEUDO_H
/* codepseudo.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Haeufiger benutzte Pseudo-Befehle                                         */
/*                                                                           */
/*****************************************************************************/

#include "addrspace.h"
#include "datatypes.h"

struct tag_ASSUMERec;

extern int FindInst(void *Field, int Size, int Count);

extern Boolean IsIndirectGen(const char *Asc, const char *pBeginEnd);
extern Boolean IsIndirect(const char *Asc);

typedef int (*tDispBaseSplitQualifier)(const char *pArg, int StartPos, int SplitPos);
extern int FindDispBaseSplitWithQualifier(const char *pArg, int *pArgLen, tDispBaseSplitQualifier Qualifier);
#define FindDispBaseSplit(pArg, pArgLen) FindDispBaseSplitWithQualifier(pArg, pArgLen, NULL)

extern void CodeEquate(as_addrspace_t DestSeg, LargeInt Min, LargeInt Max);

extern Boolean QualifyQuote_SingleQuoteConstant(const char *pStart, const char *pQuotePos);

#endif /* CODEPSEUDO_H */
