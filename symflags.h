#ifndef _SYMFLAGS_H
#define _SYMFLAGS_H
/* symflags.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* Symbol Flags Used in Symbol Table and TempResult                          */
/*                                                                           */
/*****************************************************************************/

typedef enum eSymbolFlags
{
  eSymbolFlag_None = 0,
  eSymbolFlag_NextLabelAfterBSR = 1 << 0,
  eSymbolFlag_StringSingleQuoted = 1 << 1
} tSymbolFlags;

#endif /* _SYMFLAGS_H */
