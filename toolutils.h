#ifndef TOOLUTILS_H
#define TOOLUTILS_H
/* toolutils.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/*****************************************************************************/

#include "cmdarg.h"
#include "datatypes.h"

#include <stdio.h>

typedef struct {
    LargeInt Addr;
    LongInt  Type;
    char*    Name;
} TRelocEntry, *PRelocEntry;

typedef struct {
    char*    Name;
    LargeInt Value;
    LongInt  Flags;
} TExportEntry, *PExportEntry;

typedef struct {
    LongInt      RelocCount, ExportCount;
    PRelocEntry  RelocEntries;
    PExportEntry ExportEntries;
    char*        Strings;
} TRelocInfo, *PRelocInfo;

extern Word FileID;

extern char const* OutName;

extern Boolean QuietMode;

extern void WrCopyRight(char const* Msg);

extern void DelSuffix(char* Name);

extern void AddSuffix(char* Name, unsigned NameSize, char const* Suff);

extern void FormatError(char const* Name, char const* Detail);

extern void ChkIO(char const* Name);

extern Word Granularity(Byte Header, Byte Segment);

extern void ReadRecordHeader(
        Byte* Header, Byte* Target, Byte* Segment, Byte* Gran, char const* Name, FILE* f);

extern void WriteRecordHeader(
        Byte* Header, Byte* Target, Byte* Segment, Byte* Gran, char const* Name, FILE* f);

extern void SkipRecord(Byte Header, char const* Name, FILE* f);

extern PRelocInfo ReadRelocInfo(FILE* f);

extern void DestroyRelocInfo(PRelocInfo PInfo);

extern CMDResult CMD_FilterList(Boolean Negate, char const* Arg);

extern CMDResult CMD_Range(
        LongWord* pStart, LongWord* pStop, Boolean* pStartAuto, Boolean* pStopAuto,
        char const* Arg);

extern CMDResult CMD_QuietMode(Boolean Negate, char const* Arg);

extern Boolean FilterOK(Byte Header);

extern Boolean RemoveOffset(char* Name, LongWord* Offset);

extern void EraseFile(char const* FileName, LongWord Offset);

extern Boolean AddressWildcard(char const* addr);

extern void toolutils_init(char const* ProgPath);

#endif /* TOOLUTILS_H */
