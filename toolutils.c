/* toolutils.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Unterroutinen fuer die AS-Tools                                           */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "toolutils.h"

#include "addrspace.h"
#include "as_endian.h"
#include "bpemu.h"
#include "cmdarg.h"
#include "fileformat.h"
#include "headids.h"
#include "ioerrs.h"
#include "nlmessages.h"
#include "strutil.h"
#include "tools.rsc"

#include <stdarg.h>
#include <string.h>
#ifdef _USE_MSH
#    include "tools.msh"
#endif

#include "toolutils.h"
#include "version.h"

/****************************************************************************/

static Boolean DoFilter;
static int     FilterCnt;
static Byte    FilterBytes[100];

Word        FileID  = 0x1489;   /* Dateiheader Eingabedateien */
char const* OutName = "STDOUT"; /* Pseudoname Output */

String target_name;

static TMsgCat MsgCat;

Boolean QuietMode, Verbose;

/****************************************************************************/

void WrCopyRight(char const* Msg) {
    printf("%s\n%s\n", Msg, InfoMessCopyright);
}

void DelSuffix(char* Name) {
    char *p, *z, *Part;

    p = NULL;
    for (z = Name; *z != '\0'; z++) {
        if (*z == '\\') {
            p = z;
        }
    }
    Part = (p != NULL) ? p : Name;
    Part = strchr(Part, '.');
    if (Part != NULL) {
        *Part = '\0';
    }
}

void AddSuffix(char* pName, unsigned NameSize, char const* Suff) {
    char *p, *z, *Part;

    p = NULL;
    for (z = pName; *z != '\0'; z++) {
        if (*z == '\\') {
            p = z;
        }
    }
    Part = (p != NULL) ? p : pName;
    if (strchr(Part, '.') == NULL) {
        strmaxcat(pName, Suff, NameSize);
    }
}

AS_NORETURN void FormatError(char const* Name, char const* Detail) {
    fprintf(stderr, "%s%s%s (%s)\n", catgetmessage(&MsgCat, Num_FormatErr1aMsg), Name,
            catgetmessage(&MsgCat, Num_FormatErr1bMsg), Detail);
    fprintf(stderr, "%s\n", catgetmessage(&MsgCat, Num_FormatErr2Msg));
    exit(3);
}

AS_NORETURN static void wr_io_str(char const* p_name, char const* p_error_msg) {
    fprintf(stderr, "%s%s%s\n", catgetmessage(&MsgCat, Num_IOErrAHeaderMsg), p_name,
            catgetmessage(&MsgCat, Num_IOErrBHeaderMsg));

    fprintf(stderr, "%s.\n", p_error_msg);

    fprintf(stderr, "%s\n", catgetmessage(&MsgCat, Num_ErrMsgTerminating));

    exit(2);
}

void ChkIO(char const* Name) {
    /* This helper returns when errno is zero; only wr_io_str is fatal. */
    if (errno) {
        wr_io_str(Name, GetErrorMsg(errno));
    }
}

AS_NORETURN void chk_wr_read_error(char const* p_name) {
    wr_io_str(p_name, errno ? GetErrorMsg(errno) : GetReadErrorMsg());
}

int chkio_fprintf(FILE* p_file, char const* p_name, char const* p_fmt, ...) {
    va_list ap;
    int     ret;

    va_start(ap, p_fmt);
    ret = vfprintf(p_file, p_fmt, ap);
    va_end(ap);
    if (ret < 0) {
        ChkIO(p_name);
    }
    return ret;
}

int chkio_printf(char const* p_name, char const* p_fmt, ...) {
    va_list ap;
    int     ret;

    va_start(ap, p_fmt);
    ret = vprintf(p_fmt, ap);
    va_end(ap);
    if (ret < 0) {
        ChkIO(p_name);
    }
    return ret;
}

void ReadRecordHeader(
        Byte* Header, Byte* CPU, Byte* Segment, Byte* Gran, char const* Name, FILE* f) {
#ifdef _WIN32
    /* CygWin B20 seems to mix up the file pointer under certain
       conditions. Difficult to reproduce, so we reposition it. */

    long pos;

    pos = ftell(f);
    fflush(f);
    rewind(f);
    fseek(f, pos, SEEK_SET);
#endif

    if (fread(Header, 1, 1, f) != 1) {
        chk_wr_read_error(Name);
    }
    if ((*Header != FileHeaderEnd) && (*Header != FileHeaderStartAdr)) {
        if ((*Header == FileHeaderDataRec) || (*Header == FileHeaderRDataRec)
            || (*Header == FileHeaderRelocRec) || (*Header == FileHeaderRRelocRec)) {
            if (fread(CPU, 1, 1, f) != 1) {
                chk_wr_read_error(Name);
            }
            if (fread(Segment, 1, 1, f) != 1) {
                chk_wr_read_error(Name);
            }
            if (fread(Gran, 1, 1, f) != 1) {
                chk_wr_read_error(Name);
            }
        } else if (*Header <= 0x7f) {
            TFamilyDescr const* p_descr;

            *CPU     = *Header;
            p_descr  = FindFamilyById(*CPU);
            if (!p_descr) {
                FormatError(Name, catgetmessage(&MsgCat, Num_FormatInvRecordHeaderMsg));
            }
            *Header  = FileHeaderDataRec;
            *Segment = SegCode;
            *Gran    = p_descr->get_granularity((as_addrspace_t)*Segment);
        }
        if ((*Header == FileHeaderDataRec) || (*Header == FileHeaderRDataRec)
            || (*Header == FileHeaderRelocRec) || (*Header == FileHeaderRRelocRec)) {
            if ((*Segment <= SegNone) || (*Segment >= SegCount) || !*Gran
                || ((*Gran > 0xf8) && (*Gran != 0xfc) && (*Gran != 0xfe)
                    && (*Gran != 0xff))) {
                FormatError(Name, catgetmessage(&MsgCat, Num_FormatInvRecordHeaderMsg));
            }
        }
    }
}

void WriteRecordHeader(
        Byte* Header, Byte* CPU, Byte* Segment, Byte* Gran, char const* Name, FILE* f) {
    if ((*Header == FileHeaderEnd) || (*Header == FileHeaderStartAdr)) {
        if (fwrite(Header, 1, 1, f) != 1) {
            ChkIO(Name);
        }
    } else if ((*Header == FileHeaderDataRec) || (*Header == FileHeaderRDataRec)) {
        TFamilyDescr const* p_descr = FindFamilyById(*CPU);
        Word                def_granularity
                = p_descr ? p_descr->get_granularity((as_addrspace_t)*Segment) : 1;

        if ((*Segment != SegCode) || (*Gran != def_granularity) || (*CPU >= 0x80)) {
            if (fwrite(Header, 1, 1, f)) {
                ChkIO(Name);
            }
            if (fwrite(CPU, 1, 1, f)) {
                ChkIO(Name);
            }
            if (fwrite(Segment, 1, 1, f)) {
                ChkIO(Name);
            }
            if (fwrite(Gran, 1, 1, f)) {
                ChkIO(Name);
            }
        } else {
            if (fwrite(CPU, 1, 1, f)) {
                ChkIO(Name);
            }
        }
    } else {
        if (fwrite(CPU, 1, 1, f)) {
            ChkIO(Name);
        }
    }
}

Byte record_gran_bits(Byte gran_encoded) {
    return (gran_encoded >= 0xf8) ? (256 - gran_encoded) : gran_encoded * 8;
}

LongWord record_byte_address(LongWord target_word_address, Byte gran_encoded) {
    return (gran_encoded > 0xf8) ? (target_word_address / (8 / (256 - gran_encoded)))
                                 : (target_word_address * gran_encoded);
}

LongWord record_target_word_last_address(
        LongWord target_word_start_address, LongWord length_bytes, Byte gran_encoded) {
    return (gran_encoded > 0xf8)
                   ? (target_word_start_address
                      + (((LongWord)length_bytes * 8) / (256 - gran_encoded)) - 1)
                   : (target_word_start_address + (length_bytes / gran_encoded) - 1);
}

LongWord record_target_word_length(LongWord byte_length, Byte gran_encoded) {
    return (gran_encoded > 0xf8) ? (byte_length * 8 / record_gran_bits(gran_encoded))
                                 : (byte_length / gran_encoded);
}

LongWord record_byte_length(
        LongWord target_word_first_address, LongWord target_word_last_address,
        Byte gran_encoded) {
    return (gran_encoded > 0xf8)
                   ? ((target_word_last_address - target_word_first_address + 1)
                      / (8 / (256 - gran_encoded)))
                   : ((target_word_last_address - target_word_first_address + 1)
                      * gran_encoded);
}

LongWord record_byte_offset(
        LongWord target_word_act_address, LongWord target_word_start_address,
        Byte gran_encoded) {
    return (gran_encoded > 0xf8) ? ((target_word_act_address - target_word_start_address)
                                    / (8 / (256 - gran_encoded)))
                                 : ((target_word_act_address - target_word_start_address)
                                    * gran_encoded);
}

void SkipRecord(Byte Header, char const* Name, FILE* f) {
    int      Length;
    LongWord Addr, RelocCount, ExportCount, StringLen;
    Word     Len;

    switch (Header) {
    case FileHeaderStartAdr:
        Length = 4;
        break;
    case FileHeaderEnd:
        Length = 0;
        break;
    case FileHeaderRelocInfo:
        if (!Read4(f, &RelocCount)) {
            ChkIO(Name);
        }
        if (!Read4(f, &ExportCount)) {
            ChkIO(Name);
        }
        if (!Read4(f, &StringLen)) {
            ChkIO(Name);
        }
        Length = (16 * RelocCount) + (16 * ExportCount) + StringLen;
        break;
    default:
        if (!Read4(f, &Addr)) {
            ChkIO(Name);
        }
        if (!Read2(f, &Len)) {
            ChkIO(Name);
        }
        Length = Len;
        break;
    }

    if (fseek(f, Length, SEEK_CUR) != 0) {
        ChkIO(Name);
    }
}

PRelocInfo ReadRelocInfo(FILE* f) {
    PRelocInfo PInfo = (PRelocInfo)calloc(1, sizeof(*PInfo));
    LongWord   StringLen, StringPos;
    LongInt    z;
    long       Position, Size;
    size_t     Remaining;

    if (!PInfo) {
        return NULL;
    }
    if (!Read4(f, &PInfo->RelocCount) || !Read4(f, &PInfo->ExportCount)
        || !Read4(f, &StringLen) || (PInfo->RelocCount < 0)
        || (PInfo->ExportCount < 0)) {
        goto invalid;
    }

    /* Each on-disk entry takes 16 bytes. Bound allocations by the input
       before multiplying counts or constructing pointers into the strings. */
    Position = ftell(f);
    Size = FileSize(f);
    if ((Position < 0) || (Size < Position)) {
        goto invalid;
    }
    Remaining = (size_t)(Size - Position);
    if ((size_t)PInfo->RelocCount > Remaining / 16
        || (size_t)PInfo->RelocCount > (size_t)-1 / sizeof(TRelocEntry)) {
        goto invalid;
    }
    Remaining -= (size_t)PInfo->RelocCount * 16;
    if ((size_t)PInfo->ExportCount > Remaining / 16
        || (size_t)PInfo->ExportCount > (size_t)-1 / sizeof(TExportEntry)) {
        goto invalid;
    }
    Remaining -= (size_t)PInfo->ExportCount * 16;
    if (StringLen > Remaining) {
        goto invalid;
    }

    PInfo->RelocEntries = (PRelocEntry)malloc(sizeof(TRelocEntry) * PInfo->RelocCount);
    PInfo->ExportEntries = (PExportEntry)malloc(sizeof(TExportEntry) * PInfo->ExportCount);
    PInfo->Strings = (char*)malloc(StringLen);
    if ((PInfo->RelocCount && !PInfo->RelocEntries)
        || (PInfo->ExportCount && !PInfo->ExportEntries)
        || (StringLen && !PInfo->Strings)) {
        goto invalid;
    }

    for (z = 0; z < PInfo->RelocCount; z++) {
        PRelocEntry PEntry = &PInfo->RelocEntries[z];
        if (!Read8(f, &PEntry->Addr) || !Read4(f, &StringPos)
            || (StringPos >= StringLen) || !Read4(f, &PEntry->Type)) {
            goto invalid;
        }
        PEntry->Name = PInfo->Strings + StringPos;
    }
    for (z = 0; z < PInfo->ExportCount; z++) {
        PExportEntry PExp = &PInfo->ExportEntries[z];
        if (!Read4(f, &StringPos) || (StringPos >= StringLen)
            || !Read4(f, &PExp->Flags) || !Read8(f, &PExp->Value)) {
            goto invalid;
        }
        PExp->Name = PInfo->Strings + StringPos;
    }
    if (StringLen && (fread(PInfo->Strings, 1, StringLen, f) != StringLen)) {
        goto invalid;
    }
    for (z = 0; z < PInfo->RelocCount; z++) {
        char const* Name = PInfo->RelocEntries[z].Name;
        if (!memchr(Name, '\0', StringLen - (Name - PInfo->Strings))) {
            goto invalid;
        }
    }
    for (z = 0; z < PInfo->ExportCount; z++) {
        char const* Name = PInfo->ExportEntries[z].Name;
        if (!memchr(Name, '\0', StringLen - (Name - PInfo->Strings))) {
            goto invalid;
        }
    }
    return PInfo;

invalid:
    DestroyRelocInfo(PInfo);
    return NULL;
}

void DestroyRelocInfo(PRelocInfo PInfo) {
    if (PInfo->Strings != NULL) {
        free(PInfo->Strings);
        PInfo->Strings = NULL;
    }
    if (PInfo->RelocEntries != NULL) {
        free(PInfo->RelocEntries);
        PInfo->RelocEntries = NULL;
    }
    if (PInfo->ExportEntries != NULL) {
        free(PInfo->ExportEntries);
        PInfo->ExportEntries = NULL;
    }
    free(PInfo);
}

as_cmd_result_t CMD_FilterList(Boolean Negate, char const* Arg) {
    Byte        FTemp;
    char const* p_end;
    char*       p;
    int         Search;
    String      Copy;

    if (*Arg == '\0') {
        return e_cmd_err;
    }
    strmaxcpy(Copy, Arg, STRINGSIZE);

    do {
        p = strchr(Copy, ',');
        if (p != NULL) {
            *p = '\0';
        }
        FTemp = as_cmd_strtol(Copy, &p_end);
        if (*p_end) {
            return e_cmd_err;
        }

        for (Search = 0; Search < FilterCnt; Search++) {
            if (FilterBytes[Search] == FTemp) {
                break;
            }
        }

        if ((Negate) && (Search < FilterCnt)) {
            FilterBytes[Search] = FilterBytes[--FilterCnt];
        }

        else if ((!Negate) && (Search >= FilterCnt)) {
            FilterBytes[FilterCnt++] = FTemp;
        }

        if (p != NULL) {
            strmov(Copy, p + 1);
        }
    } while (p != NULL);

    DoFilter = (FilterCnt != 0);

    return e_cmd_arg;
}

as_cmd_result_t CMD_Range(
        LongWord* pStartAddr, LongWord* pStopAddr, Boolean* pStartAuto,
        Boolean* pStopAuto, char const* Arg) {
    char const *p, *p_end;
    String      StartStr;

    p = strchr(Arg, '-');
    if (!p) {
        return e_cmd_err;
    }

    strmemcpy(StartStr, sizeof(StartStr), Arg, p - Arg);
    *pStartAuto = AddressWildcard(StartStr);
    if (!*pStartAuto) {
        *pStartAddr = as_cmd_strtol(StartStr, &p_end);
        if (*p_end) {
            return e_cmd_err;
        }
    }

    *pStopAuto = AddressWildcard(p + 1);
    if (!*pStopAuto) {
        *pStopAddr = as_cmd_strtol(p + 1, &p_end);
        if (*p_end) {
            return e_cmd_err;
        }
    }

    if (!*pStartAuto && !*pStopAuto && (*pStartAddr > *pStopAddr)) {
        return e_cmd_err;
    }

    return e_cmd_arg;
}

as_cmd_result_t CMD_QuietMode(Boolean Negate, char const* Arg) {
    UNUSED(Arg);

    QuietMode = !Negate;
    return e_cmd_ok;
}

as_cmd_result_t CMD_Verbose(Boolean Negate, char const* Arg) {
    UNUSED(Arg);

    Verbose = !Negate;
    return e_cmd_ok;
}

as_cmd_result_t cmd_target_name(Boolean negate, char const* p_arg) {
    if (negate) {
        *target_name = '\0';
        return e_cmd_ok;
    } else if (!p_arg || !*p_arg) {
        return e_cmd_err;
    } else {
        strmaxcpy(target_name, p_arg, sizeof target_name);
        return e_cmd_arg;
    }
}

Boolean FilterOK(Byte Header) {
    int z;

    if (DoFilter) {
        for (z = 0; z < FilterCnt; z++) {
            if (Header == FilterBytes[z]) {
                return True;
            }
        }
        return False;
    } else {
        return True;
    }
}

Boolean RemoveOffset(char* Name, LongWord* Offset) {
    int z, Nest;

    *Offset = 0;
    if ((*Name) && (Name[strlen(Name) - 1] == ')')) {
        z    = strlen(Name) - 2;
        Nest = 0;
        while ((z >= 0) && (Nest >= 0)) {
            switch (Name[z]) {
            case '(':
                Nest--;
                break;
            case ')':
                Nest++;
                break;
            }
            if (Nest != -1) {
                z--;
            }
        }
        if (Nest != -1) {
            return False;
        } else {
            char const* p_end;

            Name[strlen(Name) - 1] = '\0';
            *Offset                = as_cmd_strtol(Name + z + 1, &p_end);
            Name[z]                = '\0';
            return !*p_end;
        }
    } else {
        return True;
    }
}

void EraseFile(char const* FileName, LongWord Offset) {
    UNUSED(Offset);

    if (unlink(FileName) == -1) {
        ChkIO(FileName);
    }
}

void toolutils_init(char const* ProgPath) {
#ifdef _USE_MSH
    msg_catalog_open_buffer(
            &MsgCat, tools_msh_data, sizeof(tools_msh_data), MsgId1, MsgId2);
    UNUSED(ProgPath);
#else
    msg_catalog_open_file(&MsgCat, "tools.msg", ProgPath, MsgId1, MsgId2);
#endif

    FilterCnt = 0;
    DoFilter  = False;
}

Boolean AddressWildcard(char const* addr) {
    return ((strcmp(addr, "$") == 0) || (as_strcasecmp(addr, "0x") == 0));
}

void WrError(Word Num) {
    UNUSED(Num);
}
