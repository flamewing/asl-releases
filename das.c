/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
#include "stdinc.h"

#include "das.rsc"

#include "cmdarg.h"
#include "codechunks.h"
#include "console.h"
#include "cpulist.h"
#include "dasmdef.h"
#include "deco4004.h"
#include "deco68.h"
#include "deco87c800.h"
#include "entryaddress.h"
#include "invaddress.h"
#include "nlmessages.h"
#include "nls.h"
#include "strutil.h"

#include <string.h>

#define TABSIZE 8

char* pEnvName = "DASCMD";

typedef void (*tChunkCallback)(OneChunk const* pChunk, Boolean IsData, void* pUser);

static void IterateChunks(tChunkCallback Callback, void* pUser) {
    Word            NextCodeChunk, NextDataChunk;
    OneChunk const* pChunk;
    Boolean         IsData;

    NextCodeChunk = NextDataChunk = 0;
    while ((NextCodeChunk < UsedCodeChunks.RealLen)
           || (NextDataChunk < UsedDataChunks.RealLen)) {
        if (NextCodeChunk >= UsedCodeChunks.RealLen) {
            pChunk = UsedDataChunks.Chunks + (NextDataChunk++);
            IsData = True;
        } else if (NextDataChunk >= UsedDataChunks.RealLen) {
            pChunk = UsedCodeChunks.Chunks + (NextCodeChunk++);
            IsData = False;
        } else if (
                UsedDataChunks.Chunks[NextDataChunk].Start
                < UsedCodeChunks.Chunks[NextCodeChunk].Start) {
            pChunk = UsedDataChunks.Chunks + (NextDataChunk++);
            IsData = True;
        } else {
            pChunk = UsedCodeChunks.Chunks + (NextCodeChunk++);
            IsData = False;
        }

        Callback(pChunk, IsData, pUser);
    }
}

typedef struct {
    FILE*     pDestFile;
    LargeWord Sum;
} tDumpIteratorData;

static void DumpIterator(OneChunk const* pChunk, Boolean IsData, void* pUser) {
    String             Str;
    tDumpIteratorData* pData = (tDumpIteratorData*)pUser;

    HexString(Str, sizeof(Str), pChunk->Start, 0);
    fprintf(pData->pDestFile, "\t\t; %s...", Str);
    HexString(Str, sizeof(Str), pChunk->Start + pChunk->Length - 1, 0);
    fprintf(pData->pDestFile, "%s (%s)\n", Str, IsData ? "data" : "code");
    pData->Sum += pChunk->Length;
}

static void DumpChunks(ChunkList const* NChunk, FILE* pDestFile) {
    tDumpIteratorData Data;
    String            Str;

    UNUSED(NChunk);

    Data.pDestFile = pDestFile;
    Data.Sum       = 0;
    fprintf(pDestFile, "\t\t; disassembled area:\n");
    IterateChunks(DumpIterator, &Data);
    as_snprintf(
            Str, sizeof(Str), "\t\t; %" PRIx64 "/%" PRIx64 " bytes disassembled",
            Data.Sum, GetCodeChunksStored(&CodeChunks));
    fprintf(pDestFile, "%s\n", Str);
}

static int tabbedstrlen(char const* s) {
    int Result = 0;

    for (; *s; s++) {
        if (*s == '\t') {
            Result += TABSIZE - (Result % TABSIZE);
        } else {
            Result++;
        }
    }
    return Result;
}

static void PrTabs(FILE* pDestFile, int TargetLen, int ThisLen) {
    while (ThisLen < TargetLen) {
        fputc('\t', pDestFile);
        ThisLen += TABSIZE - (ThisLen % TABSIZE);
    }
}

static void ParamError(Boolean InEnv, char* Arg) {
    fprintf(stderr, "%s%s\n",
            getmessage((InEnv) ? Num_ErrMsgInvEnvParam : Num_ErrMsgInvParam), Arg);
    exit(4);
}

static CMDResult ArgError(int MsgNum, char const* pArg) {
    if (pArg) {
        fprintf(stderr, "%s:", pArg);
    }
    fprintf(stderr, "%s\n", getmessage(MsgNum));

    return CMDErr;
}

static CMDResult CMD_BinFile(Boolean Negate, char const* pArg) {
    LargeWord  Start = 0, Len = 0, Gran = 1;
    char *     pStart = NULL, *pLen = NULL, *pGran = NULL;
    String     Arg;
    Boolean    OK;
    tCodeChunk Chunk;

    if (Negate || !*pArg) {
        return ArgError(Num_ErrMsgFileArgumentMissing, NULL);
    }

    strmaxcpy(Arg, pArg, sizeof(Arg));
    if ((pStart = strchr(Arg, '@'))) {
        *pStart++ = '\0';
        if ((pLen = strchr(pStart, ','))) {
            *pLen++ = '\0';
            if ((pGran = strchr(pLen, ','))) {
                *pGran++ = '\0';
            }
        }
    }

    if (pStart && *pStart) {
        Start = ConstLongInt(pStart, &OK, 10);
        if (!OK) {
            return ArgError(Num_ErrMsgInvalidNumericValue, pStart);
        }
    } else {
        Start = 0;
    }

    if (pLen && *pLen) {
        Len = ConstLongInt(pLen, &OK, 10);
        if (!OK) {
            return ArgError(Num_ErrMsgInvalidNumericValue, pLen);
        }
    } else {
        Len = 0;
    }

    if (pGran && *pGran) {
        Gran = ConstLongInt(pGran, &OK, 10);
        if (!OK) {
            return ArgError(Num_ErrMsgInvalidNumericValue, pGran);
        }
    } else {
        Gran = 1;
    }

    InitCodeChunk(&Chunk);
    if (ReadCodeChunk(&Chunk, Arg, Start, Len, Gran)) {
        return ArgError(Num_ErrMsgCannotReadBinaryFile, Arg);
    }
    MoveCodeChunkToList(&CodeChunks, &Chunk, TRUE);

    return CMDArg;
}

static void ResizeBuffer(Byte** ppBuffer, LargeWord* pAllocLen, LargeWord ReqLen) {
    if (ReqLen > *pAllocLen) {
        void* pNew = *ppBuffer ? realloc(*ppBuffer, ReqLen) : malloc(ReqLen);
        if (pNew) {
            *ppBuffer  = (Byte*)pNew;
            *pAllocLen = ReqLen;
        }
    }
}

static Boolean GetByte(char** ppLine, Byte* pResult) {
    if (!as_isxdigit(**ppLine)) {
        return False;
    }
    *pResult = isdigit(**ppLine) ? (**ppLine - '0') : (as_toupper(**ppLine) - 'A' + 10);
    (*ppLine)++;
    if (!as_isxdigit(**ppLine)) {
        return False;
    }
    *pResult = (*pResult << 4)
               | (isdigit(**ppLine) ? (**ppLine - '0')
                                    : (as_toupper(**ppLine) - 'A' + 10));
    (*ppLine)++;
    return True;
}

static void FlushChunk(tCodeChunk* pChunk) {
    pChunk->Granularity = 1;
    pChunk->pLongCode   = (LongWord*)pChunk->pCode;
    pChunk->pWordCode   = (Word*)pChunk->pCode;
    MoveCodeChunkToList(&CodeChunks, pChunk, TRUE);
    InitCodeChunk(pChunk);
}

static CMDResult CMD_HexFile(Boolean Negate, char const* pArg) {
    FILE*      pFile;
    char       Line[300], *pLine;
    size_t     Len;
    Byte *     pLineBuffer = NULL, *pDataBuffer = 0, RecordType, Tmp;
    LargeWord  LineBufferStart = 0, LineBufferAllocLen = 0, LineBufferLen = 0, Sum;
    tCodeChunk Chunk;
    unsigned   z;

    if (Negate || !*pArg) {
        return ArgError(Num_ErrMsgFileArgumentMissing, NULL);
    }

    pFile = fopen(pArg, "r");
    if (!pFile) {
        return ArgError(Num_ErrMsgCannotReadHexFile, pArg);
    }

    InitCodeChunk(&Chunk);
    while (!feof(pFile)) {
        fgets(Line, sizeof(Line), pFile);
        Len = strlen(Line);
        if ((Len > 0) && (Line[Len - 1] == '\n')) {
            Line[--Len] = '\0';
        }
        if ((Len > 0) && (Line[Len - 1] == '\r')) {
            Line[--Len] = '\0';
        }
        if (*Line != ':') {
            continue;
        }

        Sum   = 0;
        pLine = Line + 1;
        if (!GetByte(&pLine, &Tmp)) {
            return ArgError(Num_ErrMsgInvalidHexData, pArg);
        }
        ResizeBuffer(&pLineBuffer, &LineBufferAllocLen, Tmp);
        LineBufferLen = Tmp;
        Sum += Tmp;

        LineBufferStart = 0;
        for (z = 0; z < 2; z++) {
            if (!GetByte(&pLine, &Tmp)) {
                return ArgError(Num_ErrMsgInvalidHexData, pArg);
            }
            LineBufferStart = (LineBufferStart << 8) | Tmp;
            Sum += Tmp;
        }

        if (!GetByte(&pLine, &RecordType)) {
            return ArgError(Num_ErrMsgInvalidHexData, pArg);
        }
        Sum += RecordType;
        if (RecordType != 0) {
            continue;
        }

        for (z = 0; z < LineBufferLen; z++) {
            if (!GetByte(&pLine, &pLineBuffer[z])) {
                return ArgError(Num_ErrMsgInvalidHexData, pArg);
            }
            Sum += pLineBuffer[z];
        }

        if (!GetByte(&pLine, &Tmp)) {
            return ArgError(Num_ErrMsgInvalidHexData, pArg);
        }
        Sum += Tmp;
        if (Sum & 0xff) {
            return ArgError(Num_ErrMsgHexDataChecksumError, pArg);
        }

        if (Chunk.Start + Chunk.Length == LineBufferStart) {
            ResizeBuffer(&Chunk.pCode, &Chunk.Length, Chunk.Length + LineBufferLen);
            memcpy(&Chunk.pCode[Chunk.Length - LineBufferLen], pLineBuffer,
                   LineBufferLen);
        } else {
            if (Chunk.Length) {
                FlushChunk(&Chunk);
            }
            ResizeBuffer(&Chunk.pCode, &Chunk.Length, LineBufferLen);
            memcpy(Chunk.pCode, pLineBuffer, LineBufferLen);
            Chunk.Start = LineBufferStart;
        }
    }
    if (Chunk.Length) {
        FlushChunk(&Chunk);
    }

    if (pLineBuffer) {
        free(pLineBuffer);
    }
    if (pDataBuffer) {
        free(pDataBuffer);
    }
    fclose(pFile);
    return CMDOK;
}

static CMDResult CMD_EntryAddress(Boolean Negate, char const* pArg) {
    LargeWord Address;
    char*     pName = NULL;
    String    Arg, Str;
    Boolean   OK;

    if (Negate || !*pArg) {
        return ArgError(Num_ErrMsgAddressArgumentMissing, NULL);
    }

    strmaxcpy(Arg, pArg, sizeof(Arg));
    if ((pName = ParenthPos(Arg, ','))) {
        *pName++ = '\0';
    }

    if (*Arg) {
        if (*Arg == '(') {
            Byte      Vector[8];
            char *    pVectorAddress = NULL, *pAddrLen = NULL, *pEndianess = NULL;
            LargeWord AddrLen, VectorAddress = 0, z;
            Boolean   VectorMSB;
            int       l;

            pVectorAddress = Arg + 1;
            l              = strlen(pVectorAddress);
            if (pVectorAddress[l - 1] != ')') {
                return ArgError(Num_ErrMsgClosingPatentheseMissing, pVectorAddress);
            }
            pVectorAddress[l - 1] = '\0';

            if ((pAddrLen = strchr(pVectorAddress, ','))) {
                *pAddrLen++ = '\0';
                if ((pEndianess = strchr(pAddrLen, ','))) {
                    *pEndianess++ = '\0';
                }
            }

            if (pVectorAddress && *pVectorAddress) {
                VectorAddress = ConstLongInt(pVectorAddress, &OK, 10);
                if (!OK) {
                    return ArgError(Num_ErrMsgInvalidNumericValue, pVectorAddress);
                }
            } else {
                pVectorAddress = 0;
            }

            if (pAddrLen && *pAddrLen) {
                AddrLen = ConstLongInt(pAddrLen, &OK, 10);
                if (!OK || (AddrLen > sizeof(Vector))) {
                    return ArgError(Num_ErrMsgInvalidNumericValue, pAddrLen);
                }
            } else {
                AddrLen = 1;
            }

            if (pEndianess && *pEndianess) {
                if (!as_strcasecmp(pEndianess, "MSB")) {
                    VectorMSB = True;
                } else if (!as_strcasecmp(pEndianess, "LSB")) {
                    VectorMSB = False;
                } else {
                    return ArgError(Num_ErrMsgInvalidEndinaness, pEndianess);
                }
            } else {
                VectorMSB = True; /* TODO: depend on CPU */
            }

            if (!RetrieveCodeFromChunkList(&CodeChunks, VectorAddress, Vector, AddrLen)) {
                return ArgError(Num_ErrMsgCannotRetrieveEntryAddressData, NULL);
            }

            Address = 0;
            for (z = 0; z < AddrLen; z++) {
                Address <<= 8;
                Address |= VectorMSB ? Vector[z] : Vector[AddrLen - 1 - z];
            }
            as_snprintf(
                    Str, sizeof Str, "indirect address @ %" PRIx64 " -> 0x%" PRIx64,
                    VectorAddress, Address);
            printf("%s\n", Str);
            AddChunk(&UsedDataChunks, VectorAddress, AddrLen, True);

            if (pName && *pName) {
                String Str;

                as_snprintf(Str, sizeof(Str), "Vector_2_%s", pName);
                AddInvSymbol(Str, VectorAddress);
            }
        } else {
            Address = ConstLongInt(pArg, &OK, 10);
            if (!OK) {
                return ArgError(Num_ErrMsgInvalidNumericValue, pArg);
            }
        }
    } else {
        Address = 0;
    }

    if (pName && *pName) {
        AddInvSymbol(pName, Address);
    }
    AddEntryAddress(Address);

    return CMDArg;
}

static CMDResult CMD_Symbol(Boolean Negate, char const* pArg) {
    LargeWord Address;
    char*     pName = NULL;
    String    Arg;
    Boolean   OK;

    if (Negate || !*pArg) {
        return ArgError(Num_ErrMsgSymbolArgumentMissing, NULL);
    }

    strmaxcpy(Arg, pArg, sizeof(Arg));
    if ((pName = strchr(Arg, '='))) {
        *pName++ = '\0';
    }

    if (*Arg) {
        Address = ConstLongInt(Arg, &OK, 10);
        if (!OK) {
            return ArgError(Num_ErrMsgInvalidNumericValue, Arg);
        }
    } else {
        Address = 0;
    }

    if (pName && *pName) {
        AddInvSymbol(pName, Address);
    }

    return CMDArg;
}

static CMDResult CMD_CPU(Boolean Negate, char const* pArg) {
    tCPUDef const* pCPUDef;

    if (Negate || !*pArg) {
        return ArgError(Num_ErrMsgCPUArgumentMissing, NULL);
    }

    pCPUDef = LookupCPUDefByName(pArg);
    if (!pCPUDef) {
        return ArgError(Num_ErrMsgUnknownCPU, pArg);
    }

    pCPUDef->SwitchProc(pCPUDef->pUserData);

    return CMDArg;
}

static CMDResult CMD_HexLowerCase(Boolean Negate, char const* Arg) {
    UNUSED(Arg);

    HexStartCharacter = Negate ? 'A' : 'a';
    return CMDOK;
}

#define DASParamCnt (sizeof(DASParams) / sizeof(*DASParams))

static CMDRec DASParams[] = {
        {         "CPU",          CMD_CPU},
        {     "BINFILE",      CMD_BinFile},
        {     "HEXFILE",      CMD_HexFile},
        {"ENTRYADDRESS", CMD_EntryAddress},
        {      "SYMBOL",       CMD_Symbol},
        {           "h", CMD_HexLowerCase},
};

static void NxtLine(void) {
    static int LineZ;

    if (++LineZ == 23) {
        LineZ = 0;
#if 0
    if (Redirected != NoRedir)
      return;
#endif
        WrConsoleLine(getmessage(Num_KeyWaitMsg), False);
        fflush(stdout);
        while (getchar() != '\n')
            ;
#if 0
    printf("%s", CursUp);
#endif
    }
}

typedef struct {
    FILE* pDestFile;
    int   MaxSrcLineLen, MaxLabelLen;
} tDisasmData;

static void DisasmIterator(OneChunk const* pChunk, Boolean IsData, void* pUser) {
    LargeWord    Address;
    char         NumString[50];
    tDisassInfo  Info;
    tDisasmData* pData = (tDisasmData*)pUser;
    char const*  pLabel;
    Byte         Code[100];
    unsigned     z;
    int          DataSize = -1;

    Address = pChunk->Start;
    HexString(NumString, sizeof(NumString), Address, 0);
    fprintf(pData->pDestFile, "\n");
    PrTabs(pData->pDestFile, pData->MaxLabelLen, 0);
    fprintf(pData->pDestFile, "org\t$%s\n", NumString);
    while (Address < pChunk->Start + pChunk->Length) {
        pLabel = LookupInvSymbol(Address);
        if (pLabel && !strncmp(pLabel, "Vector_", 7) && IsData) {
            String Num;
            char*  pEnd = strchr(pLabel + 7, '_');

            if (pEnd) {
                int l = pEnd - (pLabel + 7);

                memcpy(Num, pLabel + 7, l);
                Num[l]   = '\0';
                DataSize = strtol(Num, &pEnd, 10);
                if (*pEnd) {
                    DataSize = -1;
                }
            }
        }

        Disassemble(Address, &Info, IsData, DataSize);
        if (Info.pRemark) {
            PrTabs(pData->pDestFile, pData->MaxLabelLen, 0);
            fprintf(pData->pDestFile, "; %s\n", Info.pRemark);
        }

        if (pLabel) {
            fprintf(pData->pDestFile, "%s:", pLabel);
            PrTabs(pData->pDestFile, pData->MaxLabelLen, tabbedstrlen(pLabel) + 1);
        } else {
            PrTabs(pData->pDestFile, pData->MaxLabelLen, 0);
        }
        fprintf(pData->pDestFile, "%s", Info.SrcLine);

        PrTabs(pData->pDestFile, pData->MaxSrcLineLen, tabbedstrlen(Info.SrcLine));
        fprintf(pData->pDestFile, ";");
        RetrieveCodeFromChunkList(&CodeChunks, Address, Code, Info.CodeLen);
        for (z = 0; z < Info.CodeLen; z++) {
            HexString(NumString, sizeof(NumString), Code[z], 2);
            fprintf(pData->pDestFile, " %s", NumString);
        }
        fputc('\n', pData->pDestFile);

        Address += Info.CodeLen;
    }
}

int main(int argc, char** argv) {
    CMDProcessed ParUnprocessed; /* bearbeitete Kommandozeilenparameter */
    LargeWord    Address, NextAddress;
    Boolean      NextAddressValid;
    tDisassInfo  Info;
    unsigned     z;
    tDisasmData  Data;
    int          ThisSrcLineLen;

    strutil_init();
    nls_init();
    NLS_Initialize(&argc, argv);
    dasmdef_init();
    cpulist_init();
    nlmessages_init("das.msg", *argv, MsgId1, MsgId2);
    deco68_init();
    deco87c800_init();
    deco4004_init();

    if (argc <= 1) {
        char *ph1, *ph2;

        printf("%s%s%s\n", getmessage(Num_InfoMessHead1), GetEXEName(argv[0]),
               getmessage(Num_InfoMessHead2));
        NxtLine();
        for (ph1 = getmessage(Num_InfoMessHelp), ph2 = strchr(ph1, '\n'); ph2;
             ph1 = ph2 + 1, ph2 = strchr(ph1, '\n')) {
            *ph2 = '\0';
            printf("%s\n", ph1);
            NxtLine();
            *ph2 = '\n';
        }
        PrintCPUList(NxtLine);
        exit(1);
    }

    ProcessCMD(argc, argv, DASParams, DASParamCnt, ParUnprocessed, pEnvName, ParamError);

    if (!Disassemble) {
        fprintf(stderr, "no CPU set, aborting\n");
        exit(3);
    }

    /* walk through code */

    NextAddress        = 0;
    NextAddressValid   = False;
    Data.MaxSrcLineLen = 0;
    while (EntryAddressAvail()) {
        Address = GetEntryAddress(NextAddressValid, NextAddress);
        Disassemble(Address, &Info, False, -1);
        AddChunk(&UsedCodeChunks, Address, Info.CodeLen, True);
        if ((ThisSrcLineLen = tabbedstrlen(Info.SrcLine)) > Data.MaxSrcLineLen) {
            Data.MaxSrcLineLen = ThisSrcLineLen;
        }
        for (z = 0; z < Info.NextAddressCount; z++) {
            if (!AddressInChunk(&UsedCodeChunks, Info.NextAddresses[z])) {
                AddEntryAddress(Info.NextAddresses[z]);
            }
        }
        NextAddress      = Address + Info.CodeLen;
        NextAddressValid = True;
    }

    /* round up src line & symbol length to next multiple of tabs */

    Data.MaxSrcLineLen += TABSIZE - (Data.MaxSrcLineLen % TABSIZE);
    Data.MaxLabelLen = GetMaxInvSymbolNameLen() + 1;
    Data.MaxLabelLen += TABSIZE - (Data.MaxLabelLen % TABSIZE);
    Data.pDestFile = stdout;

    /* bring areas into order */

    SortChunks(&UsedCodeChunks);
    SortChunks(&UsedDataChunks);

    /* dump them out */

    IterateChunks(DisasmIterator, &Data);

    /* summary */

    DumpChunks(&UsedCodeChunks, Data.pDestFile);

    return 0;
}
