/* toolutils.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Unterroutinen fuer die AS-Tools                                           */
/*                                                                           */
/* Historie: 31. 5.1996 Grundsteinlegung                                     */
/*           27.10.1997 Routinen aus P2... heruebergenommen                  */
/*           27. 3.1999 Granularitaet SC144xx                                */
/*           30. 5.1999 Adresswildcard-Funktion                              */
/*           22. 1.2000 Funktion zum Lesen von RelocInfos                    */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*           26. 6.2000 added reading of export entries                      */
/*            2. 7.2000 updated copyright year                               */
/*            4. 7.2000 ReadRecordHeader transports record type              */
/*           14. 1.2001 silenced warnings about unused parameters            */
/*           30. 9.2001 added workaround for CygWin file pointer bug         */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include "endian.h"
#include <string.h>

#include "strutil.h"
#include "stringlists.h"
#include "cmdarg.h"
#include "stdhandl.h"
#include "ioerrs.h"

#include "nls.h"
#include "nlmessages.h"
#include "tools.rsc"

#include "toolutils.h"

#include "version.h"

/****************************************************************************/

static Boolean DoFilter;
static int FilterCnt;
static Byte FilterBytes[100];

Word FileID=0x1489;       /* Dateiheader Eingabedateien */
char *OutName="STDOUT";   /* Pseudoname Output */

static TMsgCat MsgCat;

/****************************************************************************/

	void WrCopyRight(char *Msg)
BEGIN
   printf("%s\n%s\n",Msg,InfoMessCopyright);
END

        void DelSuffix(char *Name)
BEGIN
   char *p,*z,*Part;

   p=Nil;
   for (z=Name; *z!='\0'; z++) if (*z=='\\') p=z;
   Part=(p!=Nil)?(p):(Name); Part=strchr(Part,'.');
   if (Part!=Nil) *Part='\0';
END

        void AddSuffix(char *s, char *Suff)
BEGIN
   char *p,*z,*Part;

   p=Nil;
   for (z=s; *z!='\0'; z++) if (*z=='\\') p=z;
   Part=(p!=Nil)?(p):(s);
   if (strchr(Part,'.')==Nil) strmaxcat(s,Suff,255);
END


	void FormatError(char *Name, char *Detail)
BEGIN
   fprintf(stderr,"%s%s%s (%s)\n",catgetmessage(&MsgCat,Num_FormatErr1aMsg),
                                  Name,catgetmessage(&MsgCat,Num_FormatErr1bMsg),Detail);
   fprintf(stderr,"%s\n",catgetmessage(&MsgCat,Num_FormatErr2Msg));
   exit(3);
END

	void ChkIO(char *Name)
BEGIN
   int io;

   io=errno;

   if (io==0) return;

   fprintf(stderr,"%s%s%s\n",catgetmessage(&MsgCat,Num_IOErrAHeaderMsg),Name,catgetmessage(&MsgCat,Num_IOErrBHeaderMsg));

   fprintf(stderr,"%s.\n",GetErrorMsg(io));

   fprintf(stderr,"%s\n",catgetmessage(&MsgCat,Num_ErrMsgTerminating));

   exit(2);
END

	Word Granularity(Byte Header)
BEGIN
   switch (Header)
    BEGIN
     case 0x09:
     case 0x76:
     case 0x7d:
      return 4;
     case 0x70:
     case 0x71:
     case 0x72:
     case 0x74:
     case 0x75:
     case 0x77:
     case 0x12:
     case 0x3b:
     case 0x6d:
      return 2;
     default:
      return 1;
    END
END

	void ReadRecordHeader(Byte *Header, Byte *CPU, Byte* Segment,
                              Byte *Gran, char *Name, FILE *f)
BEGIN
#ifdef _WIN32
   /* CygWin B20 seems to mix up the file pointer under certain 
      conditions difficult to reproduce, so we reposition it. */

   long pos;

   pos = ftell(f);
   fflush(f);
   rewind(f);
   fseek(f, pos, SEEK_SET);
#endif

   if (fread(Header, 1, 1, f) != 1) ChkIO(Name);
   if ((*Header != FileHeaderEnd) AND (*Header != FileHeaderStartAdr))
    BEGIN
     if ((*Header == FileHeaderDataRec) OR (*Header == FileHeaderRDataRec) OR
         (*Header == FileHeaderRelocRec) OR (*Header == FileHeaderRRelocRec))
      BEGIN
       if (fread(CPU, 1, 1, f) != 1) ChkIO(Name);
       if (fread(Segment, 1, 1, f) != 1) ChkIO(Name);
       if (fread(Gran, 1, 1, f) != 1) ChkIO(Name);
      END
     else if (*Header <= 0x7f)
      BEGIN
       *CPU = *Header;
       *Header = FileHeaderDataRec;
       *Segment = SegCode;
       *Gran = Granularity(*CPU);
      END
    END
END

        void WriteRecordHeader(Byte *Header, Byte *CPU, Byte *Segment,
                               Byte *Gran, char *Name, FILE *f)
BEGIN
   if ((*Header == FileHeaderEnd) OR (*Header == FileHeaderStartAdr))
    BEGIN
     if (fwrite(Header, 1, 1, f) != 1) ChkIO(Name);
    END
   else if ((*Header == FileHeaderDataRec) OR (*Header == FileHeaderRDataRec))
    BEGIN
     if ((*Segment != SegCode) OR (*Gran != Granularity(*CPU)) OR (*CPU >= 0x80))
      BEGIN
       if (fwrite(Header, 1, 1, f)) ChkIO(Name);
       if (fwrite(CPU, 1, 1, f)) ChkIO(Name);
       if (fwrite(Segment, 1, 1, f)) ChkIO(Name);
       if (fwrite(Gran, 1, 1, f)) ChkIO(Name);
      END
     else
      BEGIN
       if (fwrite(CPU,1,1,f)) ChkIO(Name);
      END
    END
   else
    BEGIN
     if (fwrite(CPU,1,1,f)) ChkIO(Name);
    END
END

	void SkipRecord(Byte Header, char *Name, FILE *f)
BEGIN
   int Length;
   LongWord Addr, RelocCount, ExportCount, StringLen;
   Word Len;

   switch (Header)
    BEGIN
     case FileHeaderStartAdr:
      Length = 4;
      break;
     case FileHeaderEnd:
      Length = 0;
      break;
     case FileHeaderRelocInfo:
      if (NOT Read4(f, &RelocCount)) ChkIO(Name);
      if (NOT Read4(f, &ExportCount)) ChkIO(Name);
      if (NOT Read4(f, &StringLen)) ChkIO(Name);
      Length = (16 * RelocCount) + (16 * ExportCount) + StringLen;
      break;
     default:
      if (NOT Read4(f, &Addr)) ChkIO(Name);
      if (NOT Read2(f, &Len)) ChkIO(Name);
      Length = Len;
      break;
    END

   if (fseek(f, Length, SEEK_CUR) != 0) ChkIO(Name);
END

	PRelocInfo ReadRelocInfo(FILE *f)
BEGIN
   PRelocInfo PInfo;
   PRelocEntry PEntry;
   PExportEntry PExp;
   Boolean OK = FALSE;
   LongWord StringLen, StringPos;
   LongInt z;
   
   /* get memory for structure */

   PInfo = (PRelocInfo) malloc(sizeof(TRelocInfo));
   if (PInfo != Nil)
    BEGIN
     PInfo->RelocEntries = Nil;
     PInfo->ExportEntries = Nil;
     PInfo->Strings = Nil;

     /* read global numbers */

     if ((Read4(f, &PInfo->RelocCount))
     AND (Read4(f, &PInfo->ExportCount))
     AND (Read4(f, &StringLen)))
      BEGIN
       /* allocate memory */

       PInfo->RelocEntries = (PRelocEntry) malloc(sizeof(TRelocEntry) * PInfo->RelocCount);
       if ((PInfo->RelocCount == 0) || (PInfo->RelocEntries != Nil))
        BEGIN
         PInfo->ExportEntries = (PExportEntry) malloc(sizeof(TExportEntry) * PInfo->ExportCount);
         if ((PInfo->ExportCount == 0) || (PInfo->ExportEntries != Nil))
          BEGIN
           PInfo->Strings = (char*) malloc(sizeof(char) * StringLen);
           if ((StringLen == 0) || (PInfo->Strings != Nil))
            BEGIN
             /* read relocation entries */

             for (z = 0, PEntry = PInfo->RelocEntries; z < PInfo->RelocCount; z++, PEntry++)
              BEGIN
               if (!Read8(f, &PEntry->Addr)) break;
               if (!Read4(f, &StringPos)) break; 
               PEntry->Name = PInfo->Strings + StringPos;
               if (!Read4(f, &PEntry->Type)) break;
              END

             /* read export entries */

             for (z = 0, PExp = PInfo->ExportEntries; z < PInfo->ExportCount; z++, PExp++)
              BEGIN
               if (!Read4(f, &StringPos)) break; 
               PExp->Name = PInfo->Strings + StringPos;
               if (!Read4(f, &PExp->Flags)) break;
               if (!Read8(f, &PExp->Value)) break;
              END

             /* read strings */

             if (z == PInfo->ExportCount)
              OK = ((fread(PInfo->Strings, 1, StringLen, f)) == StringLen);
            END
          END
        END
      END
    END

   if (NOT OK)
    BEGIN
     if (PInfo != Nil)
      BEGIN
       if ((StringLen > 0) && (PInfo->Strings != Nil))
        free(PInfo->Strings);
       if ((PInfo->ExportCount > 0) && (PInfo->RelocEntries != Nil))
        free(PInfo->RelocEntries);
       if ((PInfo->RelocCount > 0) && (PInfo->ExportEntries != Nil))
        free(PInfo->ExportEntries);
       free (PInfo);
       PInfo = Nil;
      END
    END

   return PInfo;
END

	void DestroyRelocInfo(PRelocInfo PInfo)
BEGIN
   UNUSED(PInfo);
END

	CMDResult CMD_FilterList(Boolean Negate, char *Arg)
BEGIN
   Byte FTemp;
   Boolean err;
   char *p;
   int Search;
   String Copy;

   if (*Arg=='\0') return CMDErr;
   strmaxcpy(Copy,Arg,255);

   do
    BEGIN
     p=strchr(Copy,','); if (p!=Nil) *p='\0';
     FTemp=ConstLongInt(Copy,&err);
     if (NOT err) return CMDErr;

     for (Search=0; Search<FilterCnt; Search++)
      if (FilterBytes[Search]==FTemp) break;

     if ((Negate) AND (Search<FilterCnt))
      FilterBytes[Search]=FilterBytes[--FilterCnt];

     else if ((NOT Negate) AND (Search>=FilterCnt))
      FilterBytes[FilterCnt++]=FTemp;

     if (p!=Nil) strcpy(Copy,p+1);
    END
   while (p!=Nil);

   DoFilter=(FilterCnt!=0);

   return CMDArg;
END

	Boolean FilterOK(Byte Header)
BEGIN
   int z;

   if (DoFilter)
    BEGIN
     for (z=0; z<FilterCnt; z++)
      if (Header==FilterBytes[z]) return True;
     return False;
    END
   else return True;
END

        Boolean RemoveOffset(char *Name, LongWord *Offset)
BEGIN
   int z,Nest;
   Boolean err;

   *Offset=0;
   if ((*Name) && (Name[strlen(Name)-1]==')'))
    BEGIN
     z=strlen(Name)-2; Nest=0;
     while ((z>=0) AND (Nest>=0))
      BEGIN
       switch (Name[z])
        BEGIN
         case '(':Nest--; break;
         case ')':Nest++; break;
        END
       if (Nest!=-1) z--;
      END
     if (Nest!=-1) return False;
     else
      BEGIN
       Name[strlen(Name)-1]='\0';
       *Offset=ConstLongInt(Name+z+1,&err);
       Name[z]='\0';
       return err;
      END
    END
   else return True;
END

	void EraseFile(char *FileName, LongWord Offset)
BEGIN
   UNUSED(Offset);
   
   if (unlink(FileName)==-1) ChkIO(FileName);
END

	void toolutils_init(char *ProgPath)
BEGIN
   version_init();

   opencatalog(&MsgCat,"tools.msg",ProgPath,MsgId1,MsgId2);

   FilterCnt=0; DoFilter=False;
END

	Boolean AddressWildcard(char *addr)
BEGIN
   return ((strcmp(addr, "$") == 0) OR (strcasecmp(addr, "0x") == 0));
END

#ifdef CKMALLOC
#undef malloc
#undef realloc

        void *ckmalloc(size_t s)
BEGIN
   void *tmp=malloc(s);
   if (tmp==NULL) 
    BEGIN
     fprintf(stderr,"allocation error(malloc): out of memory");
     exit(255);
    END
   return tmp;
END

        void *ckrealloc(void *p, size_t s)
BEGIN
   void *tmp=realloc(p,s);
   if (tmp==NULL)
    BEGIN
     fprintf(stderr,"allocation error(realloc): out of memory");
     exit(255);
    END
   return tmp;
END
#endif


