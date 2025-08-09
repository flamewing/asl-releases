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

extern void WrCopyRight(char const* Msg);

extern void DelSuffix(char* Name);

extern void AddSuffix(char* Name, unsigned NameSize, char const* Suff);

extern void FormatError(char const* Name, char const* Detail);

extern void ChkIO(char const* Name);
extern void chk_wr_read_error(char const* p_name);
extern int  chkio_fprintf(FILE* p_file, char const* p_name, char const* p_fmt, ...)
#ifdef __GNUC__
        __attribute__((format(printf, 3, 4)))
#endif
        ;
extern int chkio_printf(char const* p_name, char const* p_fmt, ...)
#ifdef __GNUC__
        __attribute__((format(printf, 2, 3)))
#endif
        ;

extern void ReadRecordHeader(
        Byte* Header, Byte* Target, Byte* Segment, Byte* Gran, char const* Name, FILE* f);

extern void WriteRecordHeader(
        Byte* Header, Byte* Target, Byte* Segment, Byte* Gran, char const* Name, FILE* f);

extern void SkipRecord(Byte Header, char const* Name, FILE* f);

extern PRelocInfo ReadRelocInfo(FILE* f);

extern void DestroyRelocInfo(PRelocInfo PInfo);

extern as_cmd_result_t CMD_FilterList(Boolean Negate, char const* Arg);

extern as_cmd_result_t CMD_Range(
        LongWord* pStart, LongWord* pStop, Boolean* pStartAuto, Boolean* pStopAuto,
        char const* Arg);

extern Boolean FilterOK(Byte Header);

extern Boolean RemoveOffset(char* Name, LongWord* Offset);

extern void EraseFile(char const* FileName, LongWord Offset);

extern Boolean AddressWildcard(char const* addr);

extern void toolutils_init(char const* ProgPath);

#endif /* TOOLUTILS_H */
