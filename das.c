#include "stdinc.h"
#include "nlmessages.h"
#include "stringlists.h"
#include "codechunks.h"
#include "entryaddress.h"
#include "invaddress.h"
#include "strutil.h"
#include "cmdarg.h"
#include "dasmdef.h"
#include "cpulist.h"
#include "das.rsc"

#include "deco68.h"
#include "deco87c800.h"

#define TABSIZE 8

char *pEnvName = "DASCMD";

typedef void (*tChunkCallback)(const OneChunk *pChunk, Boolean IsData, void *pUser);

static void IterateChunks(tChunkCallback Callback, void *pUser)
{
  Word NextCodeChunk, NextDataChunk;
  const OneChunk *pChunk;
  Boolean IsData;

  NextCodeChunk = NextDataChunk = 0;
  while ((NextCodeChunk < UsedCodeChunks.RealLen) || (NextDataChunk < UsedDataChunks.RealLen))
  {
    if (NextCodeChunk >= UsedCodeChunks.RealLen)
    {
      pChunk = UsedDataChunks.Chunks + (NextDataChunk++);
      IsData = True;
    }
    else if (NextDataChunk >= UsedDataChunks.RealLen)
    {
      pChunk = UsedCodeChunks.Chunks + (NextCodeChunk++);
      IsData = False;
    }
    else if (UsedDataChunks.Chunks[NextDataChunk].Start < UsedCodeChunks.Chunks[NextCodeChunk].Start)
    {
      pChunk = UsedDataChunks.Chunks + (NextDataChunk++);
      IsData = True;
    }
    else
    {
      pChunk = UsedCodeChunks.Chunks + (NextCodeChunk++);
      IsData = False;
    }

    Callback(pChunk, IsData, pUser);
  }
}

typedef struct
{
  FILE *pDestFile;
  LargeWord Sum;
} tDumpIteratorData;

static void DumpIterator(const OneChunk *pChunk, Boolean IsData, void *pUser)
{
  String Str;
  tDumpIteratorData *pData = (tDumpIteratorData*)pUser;

  HexString(Str, sizeof(Str), pChunk->Start, 0);
  fprintf(pData->pDestFile, "\t\t; %s...", Str);
  HexString(Str, sizeof(Str), pChunk->Start + pChunk->Length - 1, 0);
  fprintf(pData->pDestFile, "%s (%s)\n", Str, IsData ? "data" :"code");
  pData->Sum += pChunk->Length;
}

static void DumpChunks(const ChunkList *NChunk, FILE *pDestFile)
{
  tDumpIteratorData Data;

  Data.pDestFile = pDestFile;
  Data.Sum = 0;
  fprintf(pDestFile, "\t\t; disassembled area:\n");
  IterateChunks(DumpIterator, &Data);
  fprintf(pDestFile, "\t\t; %llu/%llu bytes disassembled\n", Data.Sum, GetCodeChunksStored(&CodeChunks));
}

static int tabbedstrlen(const char *s)
{
  int Result = 0;

  for (; *s; s++)
  {
    if (*s == '\t')
      Result += TABSIZE - (Result % TABSIZE);
    else
      Result++;
  }
  return Result;
}

static void PrTabs(FILE *pDestFile, int TargetLen, int ThisLen)
{
  while (ThisLen < TargetLen)
  {
    fputc('\t', pDestFile);
    ThisLen += TABSIZE - (ThisLen % TABSIZE);
  }
}

static void ParamError(Boolean InEnv, char *Arg)
{
  fprintf(stderr, "%s%s\n", getmessage((InEnv) ? Num_ErrMsgInvEnvParam : Num_ErrMsgInvParam), Arg);
  exit(4);
}

static CMDResult ArgError(int MsgNum, const char *pArg)
{
  if (pArg)
    fprintf(stderr, "%s:", pArg);
  fprintf(stderr, "%s\n", getmessage(MsgNum));

  return CMDErr;
}

static CMDResult CMD_BinFile(Boolean Negate, const char *pArg)
{
  LargeWord Start = 0, Len = 0, Gran = 1;
  char *pStart = NULL, *pLen = NULL, *pGran = NULL;
  String Arg;
  Boolean OK;
  tCodeChunk Chunk;

  if (Negate || !*pArg)
    return ArgError(Num_ErrMsgFileArgumentMissing, NULL);

  strmaxcpy(Arg, pArg, sizeof(Arg));
  if ((pStart = strchr(Arg, '@')))
  {
    *pStart++ = '\0';
    if ((pLen = strchr(pStart, ',')))
    {
      *pLen++ = '\0';
      if ((pGran = strchr(pLen, ',')))
        *pGran++ = '\0';
    }
  }

  if (pStart && *pStart)
  {
    Start = ConstLongInt(pStart, &OK, 10);
    if (!OK)
      return ArgError(Num_ErrMsgInvalidNumericValue, pStart);
  }
  else
    Start = 0;

  if (pLen && *pLen)
  {
    Len = ConstLongInt(pLen, &OK, 10);
    if (!OK)
      return ArgError(Num_ErrMsgInvalidNumericValue, pLen);
  }
  else
    Len = 0;

  if (pGran && *pGran)
  {
    Gran = ConstLongInt(pGran, &OK, 10);
    if (!OK)
      return ArgError(Num_ErrMsgInvalidNumericValue, pGran);
  }
  else
    Gran = 1;

  InitCodeChunk(&Chunk);
  if (ReadCodeChunk(&Chunk, Arg, Start, Len, Gran))
    return ArgError(Num_ErrMsgCannotReadBinaryFile, Arg);
  MoveCodeChunkToList(&CodeChunks, &Chunk, TRUE);

  return CMDArg;
}

static CMDResult CMD_EntryAddress(Boolean Negate, const char *pArg)
{
  LargeWord Address;
  char *pName = NULL;
  String Arg;
  Boolean OK;

  if (Negate || !*pArg)
    return ArgError(Num_ErrMsgAddressArgumentMissing, NULL);

  strmaxcpy(Arg, pArg, sizeof(Arg));
  if ((pName = ParenthPos(Arg, ',')))
    *pName++ = '\0';

  if (*Arg)
  {
    if (*Arg == '(')
    {
      Byte Vector[8];
      char *pVectorAddress = NULL, *pAddrLen = NULL, *pEndianess = NULL;
      LargeWord AddrLen, VectorAddress = 0;
      Boolean VectorMSB;
      int z, l;

      pVectorAddress = Arg + 1;
      l = strlen(pVectorAddress);
      if (pVectorAddress[l - 1] != ')')
        return ArgError(Num_ErrMsgClosingPatentheseMissing, pVectorAddress);
      pVectorAddress[l - 1] = '\0';

      if ((pAddrLen = strchr(pVectorAddress, ',')))
      {
        *pAddrLen++ = '\0';
        if ((pEndianess = strchr(pAddrLen, ',')))
          *pEndianess++ = '\0';
      }

      if (pVectorAddress && *pVectorAddress)
      {
        VectorAddress = ConstLongInt(pVectorAddress, &OK, 10);
        if (!OK)
          return ArgError(Num_ErrMsgInvalidNumericValue, pVectorAddress);
      }
      else
        pVectorAddress = 0;

      if (pAddrLen && *pAddrLen)
      {
        AddrLen = ConstLongInt(pAddrLen, &OK, 10);
        if (!OK || (AddrLen > sizeof(Vector)))
          return ArgError(Num_ErrMsgInvalidNumericValue, pAddrLen);
      }
      else
        AddrLen = 1;

      if (pEndianess && *pEndianess)
      {
        if (!strcasecmp(pEndianess, "MSB"))
          VectorMSB = True;
        else if (!strcasecmp(pEndianess, "LSB"))
          VectorMSB = False;
        else
          return ArgError(Num_ErrMsgInvalidEndinaness, pEndianess);
      }
      else
        VectorMSB = True; /* TODO: depend on CPU */

      if (!RetrieveCodeFromChunkList(&CodeChunks, VectorAddress, Vector, AddrLen))
        return ArgError(Num_ErrMsgCannotRetrieveEntryAddressData, NULL);

      Address = 0;
      for (z = 0; z < AddrLen; z++)
      {
        Address <<= 8;
        Address |= VectorMSB ? Vector[z] : Vector[AddrLen - 1 - z];
      }
      printf("indirect address @ %llx -> 0x%llx\n", VectorAddress, Address);
      AddChunk(&UsedDataChunks, VectorAddress, AddrLen, True);

      if (pName && *pName)
      {
        String Str;

        sprintf(Str, "Vector_2_"); strmaxcat(Str, pName, sizeof(Str));
        AddInvSymbol(Str, VectorAddress);
      }
    }
    else
    {
      Address = ConstLongInt(pArg, &OK, 10);
      if (!OK)
        return ArgError(Num_ErrMsgInvalidNumericValue, pArg);
    }
  }
  else
    Address = 0;

  if (pName && *pName)
    AddInvSymbol(pName, Address);
  AddEntryAddress(Address);  

  return CMDArg;
}

static CMDResult CMD_Symbol(Boolean Negate, const char *pArg)
{
  LargeWord Address;
  char *pName = NULL;
  String Arg;
  Boolean OK;

  if (Negate || !*pArg)
    return ArgError(Num_ErrMsgSymbolArgumentMissing, NULL);

  strmaxcpy(Arg, pArg, sizeof(Arg));
  if ((pName = strchr(Arg, '=')))
    *pName++ = '\0';

  if (*Arg)
  {
    Address = ConstLongInt(Arg, &OK, 10);
    if (!OK)
      return ArgError(Num_ErrMsgInvalidNumericValue, Arg);
  }
  else
    Address = 0;

  if (pName && *pName)
    AddInvSymbol(pName, Address);

  return CMDArg;
}

static CMDResult CMD_CPU(Boolean Negate, const char *pArg)
{
  const tCPUDef *pCPUDef;

  if (Negate || !*pArg)
    return ArgError(Num_ErrMsgCPUArgumentMissing, NULL);

  pCPUDef = LookupCPUDefByName(pArg);
  if (!pCPUDef)
    return ArgError(Num_ErrMsgUnknownCPU, pArg);

  pCPUDef->SwitchProc();

  return CMDArg;
}

static CMDResult CMD_HexLowerCase(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  HexLowerCase = !Negate;
  return CMDOK;
}

#define DASParamCnt (sizeof(DASParams) / sizeof(*DASParams))

static CMDRec DASParams[] =
{
  { "CPU"             , CMD_CPU             },
  { "BINFILE"         , CMD_BinFile         },
  { "ENTRYADDRESS"    , CMD_EntryAddress    },
  { "SYMBOL"          , CMD_Symbol          },
  { "h"               , CMD_HexLowerCase    },
};

static void NxtLine(void)
{
  static int LineZ;

  if (++LineZ == 23)
  {
    LineZ = 0;
//    if (Redirected != NoRedir)
//      return;
    printf("%s", getmessage(Num_KeyWaitMsg));
    fflush(stdout);
    while (getchar() != '\n');
//    printf("%s%s", CursUp, ClrEol);
  }
}

typedef struct
{
  FILE *pDestFile;
  int MaxSrcLineLen, MaxLabelLen;
} tDisasmData;

static void DisasmIterator(const OneChunk *pChunk, Boolean IsData, void *pUser)
{
  LargeWord Address;
  char NumString[50];
  tDisassInfo Info;
  tDisasmData *pData = (tDisasmData*)pUser;
  const char *pLabel;
  Byte Code[100];
  int z, DataSize = -1;

  Address = pChunk->Start;
  HexString(NumString, sizeof(NumString), Address, 0);
  fprintf(pData->pDestFile, "\n");
  PrTabs(pData->pDestFile, pData->MaxLabelLen, 0);
  fprintf(pData->pDestFile, "org\t$%s\n", NumString);
  while (Address < pChunk->Start + pChunk->Length)
  {
    pLabel = LookupInvSymbol(Address);
    if (pLabel && !strncmp(pLabel, "Vector_", 7) && IsData)
    {
      String Num;
      char *pEnd = strchr(pLabel + 7, '_');

      if (pEnd)
      {
        int l = pEnd - (pLabel + 7);

        memcpy(Num, pLabel + 7, l);
        Num[l] = '\0';
        DataSize = strtol(Num, &pEnd, 10);
        if (*pEnd)
          DataSize = -1;
      }
    }

    Disassemble(Address, &Info, IsData, DataSize);
    if (Info.pRemark)
    {
      PrTabs(pData->pDestFile, pData->MaxLabelLen, 0);
      fprintf(pData->pDestFile, "; %s\n", Info.pRemark);
    }

    if (pLabel)
    {
      fprintf(pData->pDestFile, "%s:", pLabel);
      PrTabs(pData->pDestFile, pData->MaxLabelLen, tabbedstrlen(pLabel) + 1);
    }
    else
      PrTabs(pData->pDestFile, pData->MaxLabelLen, 0);
    fprintf(pData->pDestFile, "%s", Info.SrcLine);

    PrTabs(pData->pDestFile, pData->MaxSrcLineLen, tabbedstrlen(Info.SrcLine));
    fprintf(pData->pDestFile, ";");
    RetrieveCodeFromChunkList(&CodeChunks, Address, Code, Info.CodeLen);
    for (z = 0; z < Info.CodeLen; z++)
    {
      HexString(NumString, sizeof(NumString),  Code[z], 2);
      fprintf(pData->pDestFile, " %s", NumString);
    }
    fputc('\n', pData->pDestFile);

    Address += Info.CodeLen;
  }
}

int main(int argc, char **argv)
{
  CMDProcessed ParUnprocessed;     /* bearbeitete Kommandozeilenparameter */
  LargeWord Address, NextAddress;
  Boolean NextAddressValid;
  tDisassInfo Info;
  unsigned z;
  tDisasmData Data;
  int ThisSrcLineLen;

  dasmdef_init();
  cpulist_init();
  nlmessages_init("das.msg", *argv, MsgId1, MsgId2);
  deco68_init();
  deco87c800_init();

  ParamCount = argc - 1;
  ParamStr = argv;

  if (ParamCount == 0)
  {
    char *ph1, *ph2;

    printf("%s%s%s\n", getmessage(Num_InfoMessHead1), GetEXEName(), getmessage(Num_InfoMessHead2));
    NxtLine();
    for (ph1 = getmessage(Num_InfoMessHelp), ph2 = strchr(ph1, '\n'); ph2; ph1 = ph2 + 1, ph2 = strchr(ph1, '\n'))
    {
      *ph2 = '\0';
      printf("%s\n", ph1);
      NxtLine();
      *ph2 = '\n';
    }
    PrintCPUList(NxtLine);
    exit(1);
  }

  ProcessCMD(DASParams, DASParamCnt, ParUnprocessed, pEnvName, ParamError);

  if (!Disassemble)
  {
    fprintf(stderr, "no CPU set, aborting\n");
    exit(3);
  }

  /* walk through code */

  NextAddress = 0;
  NextAddressValid = False;
  Data.MaxSrcLineLen = 0;
  while (EntryAddressAvail())
  {
    Address = GetEntryAddress(NextAddressValid, NextAddress);
    Disassemble(Address, &Info, False, -1);
    AddChunk(&UsedCodeChunks, Address, Info.CodeLen, True);
    if ((ThisSrcLineLen = tabbedstrlen(Info.SrcLine)) > Data.MaxSrcLineLen)
      Data.MaxSrcLineLen = ThisSrcLineLen;
    for (z = 0; z < Info.NextAddressCount; z++)
      if (!AddressInChunk(&UsedCodeChunks, Info.NextAddresses[z]))
        AddEntryAddress(Info.NextAddresses[z]);
    NextAddress = Address + Info.CodeLen;
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
