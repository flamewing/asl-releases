#ifndef _CODEPSEUDO_H
#define _CODEPSEUDO_H
/* codepseudo.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Haeufiger benutzte Pseudo-Befehle                                         */
/*                                                                           */
/* Historie: 23. 5.1996 Grundsteinlegung                                     */
/*           2001-11-11 added DecodeTIPseudo                                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: codepseudo.h,v 1.5 2004/05/29 12:28:14 alfred Exp $                  */
/*****************************************************************************
 * $Log: codepseudo.h,v $
 * Revision 1.5  2004/05/29 12:28:14  alfred
 * - remove unneccessary dummy fcn
 *
 * Revision 1.4  2004/05/29 12:18:06  alfred
 * - relocated DecodeTIPseudo() to separate module
 *
 * Revision 1.3  2004/05/29 12:04:48  alfred
 * - relocated DecodeMot(16)Pseudo into separate module
 *
 * Revision 1.2  2004/05/29 11:33:03  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 *****************************************************************************/
  
typedef struct
         {
          char *Name;
          LongInt *Dest;
          LongInt Min,Max;
          LongInt NothingVal;
         } ASSUMERec;

extern int FindInst(void *Field, int Size, int Count);

extern Boolean IsIndirect(char *Asc);

extern void CodeEquate(ShortInt DestSeg, LargeInt Min, LargeInt Max);

extern void CodeASSUME(ASSUMERec *Def, Integer Cnt);
#endif /* _CODEPSEUDO_H */
