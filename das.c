#include "stdinc.h"
#include "nlmessages.h"
#include "stringlists.h"
#include "codechunks.h"
#include "entryaddress.h"
#include "cmdarg.h"
#include "dasmdef.h"
#include "das.rsc"

String EnvName;

static void DumpChunks(const ChunkList *NChunk)
{
  Word z;

  for (z = 0; z < NChunk->RealLen; z++)
    printf(" %llx-%llx",
           NChunk->Chunks[z].Start,
           NChunk->Chunks[z].Start + NChunk->Chunks[z].Length - 1);
}

static void ParamError(Boolean InEnv, char *Arg)
{
  printf("%s%s\n", getmessage((InEnv) ? Num_ErrMsgInvEnvParam : Num_ErrMsgInvParam), Arg);
  exit(4);
}

#define DASParamCnt (sizeof(DASParams) / sizeof(*DASParams))

static CMDRec DASParams[] =
{
};

extern void Disassemble(LargeWord Address, tDisassInfo *pInfo);


int main(int argc, char **argv)
{
  CMDProcessed ParUnprocessed;     /* bearbeitete Kommandozeilenparameter */
  LargeWord Address;
  tDisassInfo Info;
  unsigned z;
  Byte Code[10];

  dasmdef_init();
  nlmessages_init("das.msg", *argv, MsgId1, MsgId2);

  ParamCount = argc - 1;
  ParamStr = argv;

  if (ParamCount == 0)
  {
    printf("help\n");
  }

  ProcessCMD(DASParams, DASParamCnt, ParUnprocessed, EnvName, ParamError);

#if 1
  {
    tCodeChunk Chunk;
    Byte ResetVector[2];

    InitCodeChunk(&Chunk);
    if (ReadCodeChunk(&Chunk, "PC_final_dump.bin", 0x8000, 0, 1))
      return 1;
    MoveCodeChunkToList(&CodeChunks, &Chunk, TRUE);
    if (RetrieveCodeFromChunkList(&CodeChunks, 0xfffe, ResetVector, 2))
    {
      AddEntryAddress((((LongWord)ResetVector[1]) << 8) | ResetVector[0]);
    }
  }
#endif

  while (EntryAddressAvail())
  {
    Address = GetEntryAddress();
    Disassemble(Address, &Info);
    AddChunk(&UsedChunks, Address, Info.CodeLen, True);
#if 0
    printf("<<");
    DumpChunks(&UsedChunks);
    printf(">>\n");
#endif
    printf("%04x:", (Word)Address);
    RetrieveCodeFromChunkList(&CodeChunks, Address, Code, Info.CodeLen);
    for (z = 0; z < Info.CodeLen; z++)
      printf(" %02x", Code[z]);
    for (; z < 6 ; z++)
      printf("   ");
    printf(" %s\n", Info.SrcLine);
    for (z = 0; z < Info.NextAddressCount; z++)
      if (!AddressInChunk(&UsedChunks, Info.NextAddresses[z]))
        AddEntryAddress(Info.NextAddresses[z]);
  }

  return 0;
}
