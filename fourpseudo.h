#ifndef FOURPSEUDO_H
#define FOURPSEUDO_H
/* fourpseudo.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Pseudo Instructions used by some 4 bit devices                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
 * Global Functions
 *****************************************************************************/

#include "asmpars.h"
#include "datatypes.h"

extern void DecodeRES(Word Code);

extern void DecodeDATA(IntType CodeIntType, IntType DataIntType);

#endif /* FOURPSEUDO_H */
