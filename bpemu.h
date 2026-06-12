#ifndef BPEMU_H
#define BPEMU_H
/* bpemu.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Emulation einiger Borland-Pascal-Funktionen                               */
/*                                                                           */
/* Historie: 20. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

#include <stddef.h>
#include <stdio.h>

typedef void (*charcallback)(char* Name);

extern void FExpand(char* p_dest, size_t dest_size, char const* p_src);

extern int FSearch(
        char* pDest, size_t DestSize, char const* FileToSearch, char const* pCurrFileName,
        char const* SearchPath);

extern long FileSize(FILE* file);

extern Byte Lo(Word inp);

extern Byte Hi(Word inp);

extern unsigned LoWord(LongWord Src);

extern unsigned HiWord(LongWord Src);

extern unsigned long LoDWord(LargeWord Src);

extern Boolean Odd(int inp);

extern Boolean DirScan(char const* Mask, charcallback callback);

extern LongInt MyGetFileTime(char* Name);

extern int as_fsize(char const* p_path, LargeWord* p_size);

#ifdef __CYGWIN32__
extern char* DeCygWinDirList(char* pStr);

extern char* DeCygwinPath(char* pStr);
#endif

extern void bpemu_init(void);
#endif /* BPEMU_H */
