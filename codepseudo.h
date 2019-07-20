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
/* Historie: 23. 5.1996 Grundsteinlegung                                     */
/*           2001-11-11 added DecodeTIPseudo                                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: codepseudo.h,v 1.6 2014/03/08 21:06:37 alfred Exp $                  */
/*****************************************************************************
 * $Log: codepseudo.h,v $
 * Revision 1.6  2014/03/08 21:06:37  alfred
 * - rework ASSUME framework
 *
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

struct _ASSUMERec;
  
extern int FindInst(void *Field, int Size, int Count);

extern Boolean IsIndirect(char *Asc);

extern void CodeEquate(ShortInt DestSeg, LargeInt Min, LargeInt Max);

#endif /* _CODEPSEUDO_H */
