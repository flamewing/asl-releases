#ifndef _INTPSEUDO_H
#define _INTPSEUDO_H
/* intpseudo.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Haeufiger benutzte Intel-Pseudo-Befehle                                   */
/*                                                                           */
/*****************************************************************************/
/* $Id: intpseudo.h,v 1.2 2004/09/26 12:57:19 alfred Exp $                   */
/*****************************************************************************
 * $Log: intpseudo.h,v $
 * Revision 1.2  2004/09/26 12:57:19  alfred
 * - remove trailing blanks
 *
 * Revision 1.1  2004/05/29 11:33:04  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 *****************************************************************************/
  
/*****************************************************************************
 * Global Functions
 *****************************************************************************/
   
extern Boolean DecodeIntelPseudo(Boolean Turn);

#endif /* _INTPSEUDO_H */
