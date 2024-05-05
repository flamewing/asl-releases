#ifndef ASMFNUMS_H
#define ASMFNUMS_H
/* asmfnums.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung von Datei-Nummern                                              */
/*                                                                           */
/* Historie: 15. 5.96 Grundsteinlegung                                       */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

extern void InitFileList(void);

extern void ClearFileList(void);

extern void AddFile(char* FName);

extern Integer GetFileNum(char* Name);

extern char const* GetFileName(int Num);

extern Integer GetFileCount(void);

extern void AddAddressRange(int File, LargeWord Start, LargeWord Len);

extern void GetAddressRange(int File, LargeWord* Start, LargeWord* End);

extern void ResetAddressRanges(void);

extern void asmfnums_init(void);
#endif /* ASMFNUMS_H */
