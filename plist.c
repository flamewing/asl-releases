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

#include "stdinc.h"
#include <string.h>

#include "version.h"
#include "endian.h"
#include "hex.h"
#include "bpemu.h"
#include "cmdarg.h"
#include "nls.h"
#include "nlmessages.h"
#include "plist.rsc"
#include "ioerrs.h"
#include "strutil.h"
#include "toolutils.h"
#include "headids.h"

static char *SegNames[PCMax+1]={"NONE","CODE","DATA","IDATA","XDATA","YDATA",
                                "BITDATA","IO","REG","ROMDATA"};



	int main(int argc, char **argv)
BEGIN
   FILE *ProgFile;
   String ProgName;
   Byte Header, Segment, Gran, CPU;
   LongWord StartAdr, Sums[PCMax+1];
   Word Len, ID, z;
   int Ch;
   Boolean HeadFnd;
   char *ph1, *ph2;
   String Ver;
   PFamilyDescr FoundId;

   ParamCount = argc - 1; ParamStr = argv;

   nls_init(); NLS_Initialize();

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
    BEGIN
     errno = 0;
     printf("%s", getmessage(Num_MessFileRequest)); fgets(ProgName, 255, stdin);
     ChkIO(OutName);
    END
   else if (ParamCount == 1) strmaxcpy(ProgName, ParamStr[1], 255);
   else
    BEGIN
     errno = 0;
     printf("%s%s%s\n", getmessage(Num_InfoMessHead1), GetEXEName(), getmessage(Num_InfoMessHead2));
     ChkIO(OutName);
     for (ph1 = getmessage(Num_InfoMessHelp), ph2 = strchr(ph1, '\n'); ph2 != Nil; ph1 = ph2 + 1, ph2 = strchr(ph1, '\n'))
      BEGIN
       *ph2 = '\0';
       printf("%s\n", ph1);
       *ph2 = '\n';
      END
     exit(1);
    END

   AddSuffix(ProgName, getmessage(Num_Suffix));

   if ((ProgFile = fopen(ProgName, OPENRDMODE)) == Nil) ChkIO(ProgName);

   if (NOT Read2(ProgFile, &ID)) ChkIO(ProgName);
   if (ID != FileMagic)
    FormatError(ProgName, getmessage(Num_FormatInvHeaderMsg));

   errno = 0; printf("\n"); ChkIO(OutName);
   errno = 0; printf("%s\n", getmessage(Num_MessHeaderLine1)); ChkIO(OutName);
   errno = 0; printf("%s\n", getmessage(Num_MessHeaderLine2)); ChkIO(OutName);

   for (z = 0; z <= PCMax; Sums[z++] = 0);

   do
    BEGIN
     ReadRecordHeader(&Header, &CPU, &Segment, &Gran, ProgName, ProgFile);

     HeadFnd = False;

     if (Header == FileHeaderEnd)
      BEGIN
       errno = 0; printf(getmessage(Num_MessGenerator)); ChkIO(OutName);
       do 
        BEGIN
  	 errno = 0; Ch = fgetc(ProgFile); ChkIO(ProgName);
         if (Ch != EOF)
         BEGIN
	  errno = 0; putchar(Ch); ChkIO(OutName);
         END
        END
       while (Ch != EOF);
       errno = 0; printf("\n"); ChkIO(OutName); HeadFnd = True;
      END

     else if (Header == FileHeaderStartAdr)
      BEGIN
       if (NOT Read4(ProgFile, &StartAdr)) ChkIO(ProgName);
       errno = 0;
       printf("%s%s\n", getmessage(Num_MessEntryPoint), HexLong(StartAdr));
       ChkIO(OutName);
      END

     else if (Header == FileHeaderRelocInfo)
      BEGIN
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
      END

     else if ((Header == FileHeaderDataRec) || (Header == FileHeaderRDataRec) ||
              (Header == FileHeaderRelocRec) || (Header == FileHeaderRRelocRec))
      BEGIN
       errno = 0;
       if (Magic != 0) FoundId = Nil;
       else FoundId = FindFamilyById(CPU);
       if (FoundId == Nil) printf("???=%02x        ", Header);
       else printf("%-13s ", FoundId->Name);
       ChkIO(OutName);

       errno = 0; printf("%-5s   ", SegNames[Segment]); ChkIO(OutName);

       if (NOT Read4(ProgFile, &StartAdr)) ChkIO(ProgName);
       errno = 0; printf("%s          ", HexLong(StartAdr)); ChkIO(OutName);

       if (NOT Read2(ProgFile, &Len)) ChkIO(ProgName);
       errno = 0; printf("%s       ", HexWord(Len));  ChkIO(OutName);

       if (Len != 0) StartAdr += (Len / Gran) - 1;
       else StartAdr--;
       errno = 0; printf("%s\n", HexLong(StartAdr));  ChkIO(OutName);

       Sums[Segment] += Len;

       if (ftell(ProgFile) + Len >= FileSize(ProgFile))
        FormatError(ProgName, getmessage(Num_FormatInvRecordLenMsg));
       else if (fseek(ProgFile, Len, SEEK_CUR) != 0) ChkIO(ProgName);
      END
     else
      SkipRecord(Header, ProgName, ProgFile);
    END
   while (Header != 0);

   errno = 0; printf("\n"); ChkIO(OutName);
   errno = 0; printf("%s", getmessage(Num_MessSum1)); ChkIO(OutName);
   for (z = 0; z <= PCMax; z++)
    if ((z == SegCode) OR (Sums[z] != 0))
     BEGIN
      errno = 0;
      printf(LongIntFormat, Sums[z]);
      printf("%s%s\n%s", getmessage((Sums[z] == 1) ? Num_MessSumSing : Num_MessSumPlur),
                         SegNames[z], Blanks(strlen(getmessage(Num_MessSum1))));
     END
   errno = 0; printf("\n"); ChkIO(OutName);
   errno = 0; printf("\n"); ChkIO(OutName);

   errno = 0; fclose(ProgFile); ChkIO(ProgName);
   return 0;
END
