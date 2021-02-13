#ifndef _INTPSEUDO_H
#define _INTPSEUDO_H
/* intpseudo.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Commonly used 'Intel Style' Pseudo Instructions                           */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"
  
typedef enum
{
  eSyntaxNeither = 0,
  eSyntax808x = 1,
  eSyntaxZ80 = 2,
  eSyntaxBoth = 3
} tZ80Syntax;

extern tZ80Syntax CurrZ80Syntax;

/*****************************************************************************
 * Global Functions
 *****************************************************************************/

extern void DecodeIntelDN(Word BigEndian);
extern void DecodeIntelDB(Word BigEndian);
extern void DecodeIntelDW(Word BigEndian);
extern void DecodeIntelDD(Word BigEndian);
extern void DecodeIntelDQ(Word BigEndian);
extern void DecodeIntelDT(Word BigEndian);
   
extern Boolean DecodeIntelPseudo(Boolean BigEndian);

extern void DecodeZ80SYNTAX(Word Code);

extern Boolean ChkZ80Syntax(tZ80Syntax InstrSyntax);

extern void intpseudo_init(void);

#endif /* _INTPSEUDO_H */
