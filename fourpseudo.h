#ifndef _FOURPSEUDO_H
#define _FOURPSEUDO_H
/* fourpseudo.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Pseudo Instructions used by some 4 bit devices                            */
/*                                                                           */
/*****************************************************************************/
/* $Id: fourpseudo.h,v 1.1 2016/10/07 19:40:43 alfred Exp $                  */
/*****************************************************************************
 * $Log: fourpseudo.h,v $
 * Revision 1.1  2016/10/07 19:40:43  alfred
 * - add missing files
 *
 *****************************************************************************/
  
/*****************************************************************************
 * Global Functions
 *****************************************************************************/

extern void DecodeRES(Word Code);
   
extern void DecodeDATA(IntType CodeIntType, IntType DataIntType);

#endif /* _FOURPSEUDO_H */
