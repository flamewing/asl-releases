/* plist.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Anzeige des Inhalts einer Code-Datei                                      */
/*                                                                           */
/* Historie: 31. 5.1996 Grundsteinlegung                                     */
/*           29. 8.1998 Tabellen auf HeadIds umgestellt                      */
/*                      main-lokale Variablen dorthin verschoben             */
/*           11. 9.1998 ROMDATA-Segment hinzugenommen                        */
/*           15. 8.1999 Einrueckung der Endadresse korrigiert                */
/*           21. 1.2000 Auflisten externe Referenzen                         */
/*           26. 6.2000 list exports                                         */
/*           30. 5.2001 move copy buffer to heap to avoid stack overflows on */
/*                      DOS platforms                                        */
/*                                                                           */
/*****************************************************************************/
/* $Id: plist.c,v 1.7 2014/12/07 19:14:02 alfred Exp $                       */
/*****************************************************************************
 * $Log: plist.c,v $
 * Revision 1.7  2014/12/07 19:14:02  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.6  2014/12/05 11:15:29  alfred
 * - eliminate AND/OR/NOT
 *
 * Revision 1.5  2014/12/05 11:09:11  alfred
 * - eliminate Nil
 *
 * Revision 1.4  2014/12/05 08:28:38  alfred
 * - rework to current style
 *
 * Revision 1.3  2014/05/29 10:59:06  alfred
 * - some const cleanups
 *
 * Revision 1.2  2012-09-02 16:55:23  alfred
 * - silence compiler warning about non-literal printf() format strinf
 *
 * Revision 1.1  2003/11/06 02:49:24  alfred
 * - recreated
 *
 * Revision 1.3  2003/03/29 18:45:51  alfred
 * - allow source file spec in key files
 *
 * Revision 1.2  2002/03/31 22:59:01  alfred
 * - added CVS header
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "version.h"
#include "endian.h"
#include "hex.h"
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

  ParamCount = argc - 1;
  ParamStr = argv;

  nls_init();
  NLS_Initialize();

  endian_init();
  hex_init();
  bpemu_init();
  strutil_init();
  nlmessages_init("plist.msg", *argv, MsgId1, MsgId2); ioerrs_init(*argv);
  cmdarg_init(*argv);
  toolutils_init(*argv);

  sprintf(Ver,"PLIST/C V%s",Version);
  WrCopyRight(Ver);

  if (ParamCount == 0)
  {
    errno = 0;
    printf("%s", getmessage(Num_MessFileRequest));
    fgets(ProgName, 255, stdin);
    ChkIO(OutName);
  }
  else if (ParamCount == 1)
    strmaxcpy(ProgName, ParamStr[1], 255);
  else
  {
    errno = 0;
    printf("%s%s%s\n", getmessage(Num_InfoMessHead1), GetEXEName(), getmessage(Num_InfoMessHead2));
    ChkIO(OutName);
    for (ph1 = getmessage(Num_InfoMessHelp), ph2 = strchr(ph1, '\n'); ph2; ph1 = ph2 + 1, ph2 = strchr(ph1, '\n'))
    {
      *ph2 = '\0';
      printf("%s\n", ph1);
      *ph2 = '\n';
    }
    exit(1);
  }

  AddSuffix(ProgName, getmessage(Num_Suffix));

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
      printf("%s%s\n", getmessage(Num_MessEntryPoint), HexLong(StartAdr));
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
        printf("%s  %s        %3d:%d(%c)     %c%s\n",
               getmessage(Num_MessRelocInfo),
               HexLong(PEntry->Addr), RelocBitCnt(PEntry->Type) >> 3,
               RelocBitCnt(PEntry->Type) & 7,
               (PEntry->Type & RelocFlagBig) ? 'B' : 'L',
               (PEntry->Type & RelocFlagSUB) ? '-' : '+', PEntry->Name);

      for (z = 0,  PExp = RelocInfo->ExportEntries; z < RelocInfo->ExportCount; z++, PExp++)
        printf("%s  %s          %c          %s\n",
               getmessage(Num_MessExportInfo),
               HexLong(PExp->Value), 
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
      errno = 0; printf("%s          ", HexLong(StartAdr)); ChkIO(OutName);

      if (!Read2(ProgFile, &Len))
        ChkIO(ProgName);
      errno = 0; printf("%s       ", HexWord(Len));  ChkIO(OutName);

      if (Len != 0)
        StartAdr += (Len / Gran) - 1;
      else
        StartAdr--;
      errno = 0; printf("%s\n", HexLong(StartAdr));  ChkIO(OutName);

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
