#ifndef _HEX_H
#define _HEX_H
/* hex.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Dezimal-->Hexadezimal-Wandlung, Grossbuchstaben                           */
/*                                                                           */
/* Historie: 2. 6.1996                                                       */
/*                                                                           */
/*****************************************************************************/

extern char *HexNibble(Byte inp);

extern char *HexByte(Byte inp);

extern char *HexWord(Word inp);

extern char *HexLong(LongWord inp);

extern void hex_init(void);
#endif /* _HEX_H */
