/* plist.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Anzeige des Inhalts einer Code-Datei                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "version.h"
#include "endian.h"
#include "bpemu.h"
#include "stringlists.h"
#include "cmdarg.h"
#include "nls.h"
#include "nlmessages.h"
#include "plist.rsc"
#include "ioerrs.h"
#include "strutil.h"
#include "toolutils.h"
#include "headids.h"

static char *SegNames[PCMax + 1] =
{
  "NONE","CODE","DATA","IDATA","XDATA","YDATA",
  "BITDATA","IO","REG","ROMDATA"
};

int main(int argc, char **argv)
{
  FILE *ProgFile;
  String ProgName;
  Byte Header, Segment, Gran, CPU;
  LongWord StartAdr, Sums[PCMax + 1];
  Word Len, ID, z;
  int Ch;
  Boolean HeadFnd;
  char *ph1, *ph2;
  String Ver;
  PFamilyDescr FoundId;

  nls_init();
  if (!NLS_Initialize(&argc, argv))
    exit(4);

  endian_init();
  bpemu_init();
  strutil_init();
  nlmessages_init("plist.msg", *argv, MsgId1, MsgId2); ioerrs_init(*argv);
  cmdarg_init(*argv);
  toolutils_init(*argv);

  as_snprintf(Ver, sizeof(Ver), "PLIST/C V%s", Version);
  WrCopyRight(Ver);

  if (argc <= 1)
  {
    int l;

    errno = 0;
    printf("%s", getmessage(Num_MessFileRequest));
    if (!fgets(ProgName, STRINGSIZE, stdin))
      return 0;
    l = strlen(ProgName);
    if ((l > 0) && (ProgName[l - 1] == '\n'))
      ProgName[--l] = '\0';
    ChkIO(OutName);
  }
  else if (argc == 2)
    strmaxcpy(ProgName, argv[1], STRINGSIZE);
  else
  {
    errno = 0;
    printf("%s%s%s\n", getmessage(Num_InfoMessHead1), GetEXEName(argv[0]), getmessage(Num_InfoMessHead2));
    ChkIO(OutName);
    for (ph1 = getmessage(Num_InfoMessHelp), ph2 = strchr(ph1, '\n'); ph2; ph1 = ph2 + 1, ph2 = strchr(ph1, '\n'))
    {
      *ph2 = '\0';
      printf("%s\n", ph1);
      *ph2 = '\n';
    }
    exit(1);
  }

  AddSuffix(ProgName, STRINGSIZE, getmessage(Num_Suffix));

  ProgFile = fopen(ProgName, OPENRDMODE);
  if (!ProgFile)
    ChkIO(ProgName);

  if (!Read2(ProgFile, &ID))
    ChkIO(ProgName);
  if (ID != FileMagic)
    FormatError(ProgName, getmessage(Num_FormatInvHeaderMsg));

  errno = 0; printf("\n"); ChkIO(OutName);
  errno = 0; printf("%s\n", getmessage(Num_MessHeaderLine1)); ChkIO(OutName);
  errno = 0; printf("%s\n", getmessage(Num_MessHeaderLine2)); ChkIO(OutName);

  for (z = 0; z <= PCMax; Sums[z++] = 0);

  do
  {
    ReadRecordHeader(&Header, &CPU, &Segment, &Gran, ProgName, ProgFile);

    HeadFnd = False;

    if (Header == FileHeaderEnd)
    {
      errno = 0; fputs(getmessage(Num_MessGenerator), stdout); ChkIO(OutName);
      do 
      {
        errno = 0; Ch = fgetc(ProgFile); ChkIO(ProgName);
        if (Ch != EOF)
        {
          errno = 0; putchar(Ch); ChkIO(OutName);
        }
      }
      while (Ch != EOF);
      errno = 0; printf("\n"); ChkIO(OutName);
      HeadFnd = True;
    }

    else if (Header == FileHeaderStartAdr)
    {
      if (!Read4(ProgFile, &StartAdr))
        ChkIO(ProgName);
      errno = 0;
      printf("%s%08lX\n", getmessage(Num_MessEntryPoint), LoDWord(StartAdr));
      ChkIO(OutName);
    }

    else if (Header == FileHeaderRelocInfo)
    {
      PRelocInfo RelocInfo;
      PRelocEntry PEntry;
      PExportEntry PExp;
      int z;

      RelocInfo = ReadRelocInfo(ProgFile);
      for (z = 0,  PEntry = RelocInfo->RelocEntries; z < RelocInfo->RelocCount; z++, PEntry++)
        printf("%s  %08lX        %3d:%d(%c)     %c%s\n",
               getmessage(Num_MessRelocInfo),
               LoDWord(PEntry->Addr), RelocBitCnt(PEntry->Type) >> 3,
               RelocBitCnt(PEntry->Type) & 7,
               (PEntry->Type & RelocFlagBig) ? 'B' : 'L',
               (PEntry->Type & RelocFlagSUB) ? '-' : '+', PEntry->Name);

      for (z = 0,  PExp = RelocInfo->ExportEntries; z < RelocInfo->ExportCount; z++, PExp++)
        printf("%s  %08lX          %c          %s\n",
               getmessage(Num_MessExportInfo),
               LoDWord(PExp->Value), 
               (PExp->Flags & RelFlag_Relative) ? 'R' : ' ',
               PExp->Name);

      DestroyRelocInfo(RelocInfo);
    }

    else if ((Header == FileHeaderDataRec) || (Header == FileHeaderRDataRec)
          || (Header == FileHeaderRelocRec) || (Header == FileHeaderRRelocRec))
    {
      errno = 0;
      if (Magic != 0)
        FoundId = NULL;
      else
        FoundId = FindFamilyById(CPU);
      if (!FoundId)
        printf("\?\?\?=%02x        ", Header);
      else
        printf("%-13s ", FoundId->Name);
      ChkIO(OutName);

      errno = 0; printf("%-5s   ", SegNames[Segment]); ChkIO(OutName);

      if (!Read4(ProgFile, &StartAdr))
        ChkIO(ProgName);
      errno = 0; printf("%08lX          ", LoDWord(StartAdr)); ChkIO(OutName);

      if (!Read2(ProgFile, &Len))
        ChkIO(ProgName);
      errno = 0; printf("%04X       ", LoWord(Len));  ChkIO(OutName);

      if (Len != 0)
        StartAdr += (Len / Gran) - 1;
      else
        StartAdr--;
      errno = 0; printf("%08lX\n", LoDWord(StartAdr));  ChkIO(OutName);

      Sums[Segment] += Len;

      if (ftell(ProgFile) + Len >= FileSize(ProgFile))
        FormatError(ProgName, getmessage(Num_FormatInvRecordLenMsg));
      else if (fseek(ProgFile, Len, SEEK_CUR) != 0)
        ChkIO(ProgName);
    }
    else
     SkipRecord(Header, ProgName, ProgFile);
  }
  while (Header != 0);

  errno = 0; printf("\n"); ChkIO(OutName);
  errno = 0; printf("%s", getmessage(Num_MessSum1)); ChkIO(OutName);
  for (z = 0; z <= PCMax; z++)
    if ((z == SegCode) || (Sums[z] != 0))
    {
      errno = 0;
      printf(LongIntFormat, Sums[z]);
      printf("%s%s\n%s", getmessage((Sums[z] == 1) ? Num_MessSumSing : Num_MessSumPlur),
                        SegNames[z], Blanks(strlen(getmessage(Num_MessSum1))));
    }
  errno = 0; printf("\n"); ChkIO(OutName);
  errno = 0; printf("\n"); ChkIO(OutName);

  errno = 0; fclose(ProgFile); ChkIO(ProgName);
  (void)HeadFnd;
  return 0;
}
