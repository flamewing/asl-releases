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

extern int FindInst(void* Field, int Size, int Count);

extern Boolean IsIndirectGen(char const* Asc, char const* pBeginEnd);
extern Boolean IsIndirect(char const* Asc);

typedef int (*tDispBaseSplitQualifier)(char const* pArg, int StartPos, int SplitPos);
extern int FindDispBaseSplitWithQualifier(
        char const* pArg, int* pArgLen, tDispBaseSplitQualifier Qualifier);
#define FindDispBaseSplit(pArg, pArgLen) \
    FindDispBaseSplitWithQualifier(pArg, pArgLen, NULL)

extern void CodeEquate(as_addrspace_t DestSeg, LargeInt Min, LargeInt Max);

extern Boolean QualifyQuote_SingleQuoteConstant(
        char const* pStart, char const* pQuotePos);

#endif /* CODEPSEUDO_H */
