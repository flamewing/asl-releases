/* toolutils.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Unterroutinen fuer die AS-Tools                                           */
/*                                                                           */
/* Historie: 31. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "stringutil.h"
#include "decodecmd.h"
#include "stdhandl.h"
#include "ioerrors.h"

#include "toolutils.h"

LongWord Magic=0x1b342b4d;

#include "tools.rsc"

/****************************************************************************/

static Boolean DoFilter;
static Integer FilterCnt;
static Byte FilterBytes[100];

static char *InfoMessCopyright="(C) 1992,1997 Alfred Arnold";

Word FileID=0x1489;       /* Dateiheader Eingabedateien */
char *OutName="STDOUT";   /* Pseudoname Output */

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
   fprintf(stderr,"%s%s%s (%s)\n",FormatErr1aMsg,Name,FormatErr1bMsg,Detail);
   fprintf(stderr,"%s\n",FormatErr2Msg);
   exit(3);
END

	void ChkIO(char *Name)
BEGIN
   int io;

   io=errno;

   if (io==0) return;

   fprintf(stderr,"%s%s%s\n",IOErrAHeaderMsg,Name,IOErrBHeaderMsg);

   fprintf(stderr,"%s.\n",GetErrorMsg(io));

   fprintf(stderr,"%s\n",ErrMsgTerminating);

   exit(2);
END

	Word Granularity(Byte Header)
BEGIN
   switch (Header)
    BEGIN
     case 0x09:
     case 0x76:
      return 4;
     case 0x70:
     case 0x71:
     case 0x72:
     case 0x74:
     case 0x75:
     case 0x77:
     case 0x12:
     case 0x3b:
      return 2;
     default:
      return 1;
    END
END

	void ReadRecordHeader(Byte *Header, Byte* Segment, Byte *Gran,
                              char *Name, FILE *f)
BEGIN
   if (fread(Header,1,1,f)!=1) ChkIO(Name);
   if ((*Header!=FileHeaderEnd) AND (*Header!=FileHeaderStartAdr))
    if (*Header==FileHeaderDataRec)
     BEGIN
      if (fread(Header,1,1,f)!=1) ChkIO(Name);
      if (fread(Segment,1,1,f)!=1) ChkIO(Name);
      if (fread(Gran,1,1,f)!=1) ChkIO(Name);
     END
    else
     BEGIN
      *Segment=SegCode;
      *Gran=Granularity(*Header);
     END
END

        void WriteRecordHeader(Byte *Header, Byte *Segment, Byte *Gran,
                               char *Name, FILE *f)
BEGIN
   Byte h;

   if ((*Header==FileHeaderEnd) OR (*Header==FileHeaderStartAdr))
    BEGIN
     if (fwrite(Header,1,1,f)!=1) ChkIO(Name);
    END
   else if ((*Segment!=SegCode) OR (*Gran!=Granularity(*Header)))
    BEGIN
     h=FileHeaderDataRec;
     if (fwrite(&h,1,1,f)) ChkIO(Name);
     if (fwrite(Header,1,1,f)) ChkIO(Name);
     if (fwrite(Segment,1,1,f)) ChkIO(Name);
     if (fwrite(Gran,1,1,f)) ChkIO(Name);
    END
   else
    BEGIN
     if (fwrite(Header,1,1,f)) ChkIO(Name);
    END
END

	CMDResult CMD_FilterList(Boolean Negate, char *Arg)
BEGIN
   Byte FTemp;
   Boolean err;
   char *p;
   Integer Search;
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
   Integer z;

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
   Integer z,Nest;
   Boolean err;

   *Offset=0;
   if (Name[strlen(Name)-1]==')')
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

	void toolutils_init(void)
BEGIN
   Word z;
   LongWord XORVal;

   FilterCnt=0; DoFilter=False;
   for (z=0; z<strlen(InfoMessCopyright); z++)
    BEGIN
     XORVal=InfoMessCopyright[z];
     XORVal=XORVal << (((z+1)%4)*8);
     Magic=Magic^XORVal;
    END
END
