/* p2hex.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Konvertierung von AS-P-Dateien nach Hex                                   */
/*                                                                           */
/* Historie:  1. 6.1996 Grundsteinlegung                                     */
/*           29. 8.1998 HeadIds verwendet fuer Default-Hex-Format            */
/*           30. 5.1999 0x statt $ erlaubt                                   */
/*            6. 7.1999 minimal S-Record-Adresslaenge setzbar                */
/*                      Fehlerabfrage in CMD_Linelen war falsch              */
/*           12.10.1999 Startadresse 16-Bit-Hex geaendert                    */
/*           13.10.1999 Startadressen 20+32 Bit Intel korrigiert             */
/*           24.10.1999 Relokation von Adressen (Thomas Eschenbach)          */
/*           16.11.1999 CS:IP-Intel-Record korrigiert                        */
/*            9. 1.2000 plattformabhaengige Formatstrings benutzen           */
/*           24. 3.2000 added symbolic string for byte message               */
/*            4. 7.2000 renamed ParProcessed to ParUnprocessed               */
/*           14. 1.2001 silenced warnings about unused parameters            */
/*           30. 5.2001 added avrlen parameter                               */
/*           2001-08-30 set EntryAddrPresent when address given as argument  */
/*                                                                           */
/*****************************************************************************/
/* $Id: p2hex.c,v 1.9 2012-08-19 09:37:51 alfred Exp $                       */
/*****************************************************************************
 * $Log: p2hex.c,v $
 * Revision 1.9  2012-08-19 09:37:51  alfred
 * - silence compiler warnings about printf without arguments
 *
 * Revision 1.8  2009/04/13 07:55:57  alfred
 * - silence Borland C++ warnings
 *
 * Revision 1.7  2007/11/24 22:48:08  alfred
 * - some NetBSD changes
 *
 * Revision 1.6  2006/12/19 17:26:42  alfred
 * - correctly regard IntOffset with granularities != 1
 *
 * Revision 1.5  2006/12/09 18:27:30  alfred
 * - add warning about empty output
 *
 *****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "version.h"
#include "endian.h"
#include "bpemu.h"
#include "hex.h"
#include "nls.h"
#include "nlmessages.h"
#include "p2hex.rsc"
#include "ioerrs.h"
#include "strutil.h"
#include "chunks.h"
#include "stringlists.h"
#include "cmdarg.h"

#include "toolutils.h"
#include "headids.h"

static char *HexSuffix=".hex";
#define MaxLineLen 254
#define AVRLEN_DEFAULT 3

typedef void (*ProcessProc)(
#ifdef __PROTOS__
char *FileName, LongWord Offset
#endif
);

static CMDProcessed ParUnprocessed;
static int z;
static FILE *TargFile;
static String SrcName, TargName;

static LongWord StartAdr, StopAdr, LineLen;
static LongWord StartData, StopData, EntryAdr;
static LargeInt Relocate;
static Boolean StartAuto, StopAuto, AutoErase, EntryAdrPresent;
static Word Seg, Ofs;
static LongWord Dummy;
static Byte IntelMode;
static Byte MultiMode;   /* 0=8M, 1=16, 2=8L, 3=8H */
static Byte MinMoto;
static Boolean Rec5;
static Boolean SepMoto;
static LongWord AVRLen, ValidSegs;

static Boolean RelAdr, MotoOccured, IntelOccured, MOSOccured, DSKOccured;
static Byte MaxMoto, MaxIntel;

static THexFormat DestFormat;

static ChunkList UsedList;

        static void ParamError(Boolean InEnv, char *Arg)
BEGIN
   printf("%s%s\n",getmessage(InEnv?Num_ErrMsgInvEnvParam:Num_ErrMsgInvParam),Arg);
   printf("%s\n",getmessage(Num_ErrMsgProgTerm));
   exit(1);
END

        static void OpenTarget(void)
BEGIN
   TargFile=fopen(TargName,"w");
   if (TargFile==Nil) ChkIO(TargName);
END

	static void CloseTarget(void)
BEGIN
   errno=0; fclose(TargFile); ChkIO(TargName);
   if (Magic!=0) unlink(TargName);
END

        static void ProcessFile(char *FileName, LongWord Offset)
BEGIN
   FILE *SrcFile;
   Word TestID;
   Byte InpHeader, InpCPU, InpSegment, InpGran;
   LongWord InpStart,SumLen;
   Word InpLen,TransLen;
   Boolean doit,FirstBank=0;
   Byte Buffer[MaxLineLen];
   Word *WBuffer=(Word *) Buffer;
   LongWord ErgStart,
#ifdef __STDC__
            ErgStop=0xffffffffu,
#else
            ErgStop=0xffffffff,
#endif
            IntOffset=0,MaxAdr;
   LongInt NextPos;
   Word ErgLen=0,ChkSum=0,RecCnt,Gran,HSeg;

   LongInt z;

   Byte MotRecType=0;

   THexFormat ActFormat;
   PFamilyDescr FoundDscr;

   SrcFile=fopen(FileName,OPENRDMODE); 
   if (SrcFile==Nil) ChkIO(FileName);

   if (NOT Read2(SrcFile,&TestID)) ChkIO(FileName);
   if (TestID!=FileMagic) FormatError(FileName,getmessage(Num_FormatInvHeaderMsg));

   errno = 0; printf("%s==>>%s", FileName, TargName); ChkIO(OutName);

   SumLen = 0;

   do
    BEGIN
     ReadRecordHeader(&InpHeader, &InpCPU, &InpSegment, &InpGran, FileName, SrcFile);

     if (InpHeader == FileHeaderStartAdr)
      BEGIN
       if (NOT Read4(SrcFile,&ErgStart)) ChkIO(FileName);
       if (NOT EntryAdrPresent)
        BEGIN
         EntryAdr=ErgStart; EntryAdrPresent=True;
        END
      END

     else if (InpHeader == FileHeaderDataRec)
      BEGIN
       Gran=InpGran;
       
       if ((ActFormat=DestFormat)==Default)
        BEGIN
         FoundDscr=FindFamilyById(InpCPU);
         if (FoundDscr==Nil)
          FormatError(FileName,getmessage(Num_FormatInvRecordHeaderMsg));
         else ActFormat=FoundDscr->HexFormat;
        END

       ValidSegs = (1 << SegCode);
       switch (ActFormat)
        BEGIN
         case MotoS:
         case IntHex32:
#ifdef __STDC__
          MaxAdr=0xffffffffu; break;
#else
          MaxAdr=0xffffffff; break;
#endif
         case IntHex16:
          MaxAdr=0xffff0+0xffff; break;
         case Atmel:
          MaxAdr = (1 << (AVRLen << 3)) - 1; break;
         case TiDSK:
          ValidSegs = (1 << SegCode) | (1 << SegData);
          /* no break!!! */
         default:
          MaxAdr=0xffff;
        END

       if (NOT Read4(SrcFile,&InpStart)) ChkIO(FileName);
       if (NOT Read2(SrcFile,&InpLen)) ChkIO(FileName);

       NextPos=ftell(SrcFile)+InpLen;
       if (NextPos>=FileSize(SrcFile)-1)
        FormatError(FileName,getmessage(Num_FormatInvRecordLenMsg));

       doit=(FilterOK(InpCPU)) AND (ValidSegs & (1 << InpSegment));

       if (doit)
        BEGIN
         InpStart+=Offset;
 	 ErgStart=max(StartAdr,InpStart);
 	 ErgStop=min(StopAdr,InpStart+(InpLen/Gran)-1);
 	 doit=(ErgStop>=ErgStart);
 	 if (doit)
 	  BEGIN
 	   ErgLen=(ErgStop+1-ErgStart)*Gran;
           if (AddChunk(&UsedList,ErgStart,ErgStop-ErgStart+1,True))
            BEGIN
             errno=0; printf(" %s\n",getmessage(Num_ErrMsgOverlap)); ChkIO(OutName);
            END
          END
        END

       if (ErgStop>MaxAdr)
        BEGIN
         errno=0; printf(" %s\n",getmessage(Num_ErrMsgAdrOverflow)); ChkIO(OutName);
        END

       if (doit)
        BEGIN
 	 /* an Anfang interessierender Daten */

 	 if (fseek(SrcFile,(ErgStart-InpStart)*Gran,SEEK_CUR)==-1) ChkIO(FileName);

 	 /* Statistik, Anzahl Datenzeilen ausrechnen */

         RecCnt=ErgLen/LineLen;
         if ((ErgLen%LineLen)!=0) RecCnt++;

 	 /* relative Angaben ? */

 	 if (RelAdr) ErgStart -= StartAdr;

         /* Auf Zieladressbereich verschieben */

         ErgStart += Relocate;

 	 /* Kopf einer Datenzeilengruppe */

  	 switch (ActFormat)
          BEGIN
   	   case MotoS:
  	    if ((NOT MotoOccured) OR (SepMoto))
  	     BEGIN
  	      errno=0; fprintf(TargFile,"S0030000FC\n"); ChkIO(TargName);
  	     END
  	    if ((ErgStop >> 24) != 0) MotRecType=2;
  	    else if ((ErgStop>>16)!=0) MotRecType=1;
  	    else MotRecType=0;
            if (MotRecType < (MinMoto - 1)) MotRecType = (MinMoto - 1);
            if (MaxMoto<MotRecType) MaxMoto=MotRecType;
  	    if (Rec5)
  	     BEGIN
  	      ChkSum=Lo(RecCnt)+Hi(RecCnt)+3;
              errno=0;
  	      fprintf(TargFile,"S503%s%s\n",HexWord(RecCnt),HexByte(Lo(ChkSum^0xff)));
  	      ChkIO(TargName);
  	     END
  	    MotoOccured=True;
  	    break;
  	   case MOSHex:
            MOSOccured=True;
            break;
  	   case IntHex:
  	    IntelOccured=True;
  	    IntOffset=0;
  	    break;
  	   case IntHex16:
  	    IntelOccured = True;
            IntOffset = (ErgStart * Gran);
#ifdef __STDC__
  	    IntOffset &= 0xfffffff0u;
#else
            IntOffset &= 0xfffffff0;
#endif
  	    HSeg = IntOffset >> 4; ChkSum = 4 + Lo(HSeg) + Hi(HSeg);
            IntOffset /= Gran;
            errno = 0; fprintf(TargFile, ":02000002%s%s\n", HexWord(HSeg),HexByte(0x100 - ChkSum)); ChkIO(TargName);
            if (MaxIntel < 1) MaxIntel = 1;
  	    break;
           case IntHex32:
  	    IntelOccured = True;
            IntOffset = (ErgStart * Gran);
#ifdef __STDC__
            IntOffset &= 0xffff0000u;
#else
            IntOffset &= 0xffff0000;
#endif
            HSeg = IntOffset >> 16; ChkSum = 6 + Lo(HSeg) + Hi(HSeg);
            IntOffset /= Gran;
            errno = 0; fprintf(TargFile, ":02000004%s%s\n",HexWord(HSeg),HexByte(0x100-ChkSum)); ChkIO(TargName);
            if (MaxIntel < 2) MaxIntel = 2;
            FirstBank = False;
            break;
           case TekHex:
            break;
           case Atmel:
            break;
  	   case TiDSK:
  	    if (NOT DSKOccured)
  	     BEGIN
  	      DSKOccured=True;
              errno=0; fprintf(TargFile,"%s%s\n",getmessage(Num_DSKHeaderLine),TargName); ChkIO(TargName);
  	     END
            break;
           default: 
            break;
 	  END

 	 /* Datenzeilen selber */

 	 while (ErgLen>0)
 	  BEGIN
           /* evtl. Folgebank fuer Intel32 ausgeben */

           if ((ActFormat==IntHex32) AND (FirstBank))
            BEGIN
             IntOffset += (0x10000 / Gran);
             HSeg=IntOffset>>16; ChkSum=6+Lo(HSeg)+Hi(HSeg);
             errno=0;
             fprintf(TargFile,":02000004%s%s\n",HexWord(HSeg),HexByte(0x100-ChkSum));
             ChkIO(TargName);
             FirstBank=False;
            END

           /* Recordlaenge ausrechnen, fuer Intel32 auf 64K-Grenze begrenzen
              bei Atmel nur 2 Byte pro Zeile! */

           TransLen=min(LineLen,ErgLen);
           if ((ActFormat==IntHex32) AND ((ErgStart&0xffff)+(TransLen/Gran)>=0x10000))
            BEGIN
             TransLen=Gran*(0x10000-(ErgStart&0xffff));
             FirstBank=True;
            END
           else if (ActFormat==Atmel) TransLen=min(2,TransLen);

 	   /* Start der Datenzeile */

 	   switch (ActFormat)
            BEGIN
 	     case MotoS:
              errno=0;
              fprintf(TargFile,"S%c%s",'1'+MotRecType,HexByte(TransLen+3+MotRecType));
 	      ChkIO(TargName);
 	      ChkSum=TransLen+3+MotRecType;
 	      if (MotRecType>=2)
 	       BEGIN
 	        errno=0; fprintf(TargFile,"%s",HexByte((ErgStart>>24)&0xff)); ChkIO(TargName);
 	        ChkSum+=((ErgStart>>24)&0xff);
 	       END
 	      if (MotRecType>=1)
 	       BEGIN
 	        errno=0; fprintf(TargFile,"%s",HexByte((ErgStart>>16)&0xff)); ChkIO(TargName);
 	        ChkSum+=((ErgStart>>16)&0xff);
 	       END
 	      errno=0; fprintf(TargFile,"%s",HexWord(ErgStart&0xffff)); ChkIO(TargName);
 	      ChkSum+=Hi(ErgStart)+Lo(ErgStart);
 	      break;
 	     case MOSHex:
 	      errno=0; fprintf(TargFile,";%s%s",HexByte(TransLen),HexWord(ErgStart & 0xffff)); ChkIO(TargName);
 	      ChkSum+=TransLen+Lo(ErgStart)+Hi(ErgStart);
 	      break;
             case IntHex:
             case IntHex16:
             case IntHex32:
             {
               Word WrTransLen;
               LongWord WrErgStart;

               WrTransLen = (MultiMode < 2) ? TransLen : (TransLen / Gran);
               WrErgStart = (ErgStart-IntOffset) * ((MultiMode < 2) ? Gran : 1);
               errno = 0;
               fprintf(TargFile, ":%s%s00", HexByte(WrTransLen), HexWord(WrErgStart));
               ChkIO(TargName);
               ChkSum = Lo(WrTransLen) + Hi(WrErgStart) + Lo(WrErgStart);

               break;
             }
 	     case TekHex:
 	      errno=0; 
              fprintf(TargFile,"/%s%s%s",HexWord(ErgStart),HexByte(TransLen),
 		                         HexByte(Lo(ErgStart)+Hi(ErgStart)+TransLen));
 	      ChkIO(TargName);
 	      ChkSum=0;
 	      break;
 	     case TiDSK:
              errno=0; fprintf(TargFile,"9%s",HexWord(/*Gran**/ErgStart));
 	      ChkIO(TargName);
 	      ChkSum=0;
 	      break;
             case Atmel:
              for (z = (AVRLen - 1) << 3; z >= 0; z -= 8)
               {
                errno = 0; 
                fputs(HexByte((ErgStart >> z) & 0xff), TargFile);
                ChkIO(TargName);
               }
              errno = 0;
              fputc(':', TargFile);
              ChkIO(TargName);
              break;
             default:
              break;
 	    END

 	   /* Daten selber */

 	   if (fread(Buffer,1,TransLen,SrcFile)!=TransLen) ChkIO(FileName);
           if (MultiMode == 1)
             switch (Gran)
             {
               case 4:
                 DSwap(Buffer, TransLen);
                 break;
               case 2:
                 WSwap(Buffer, TransLen);
                 break;
               case 1:
                 break;
             }
 	   if (ActFormat==TiDSK)
            BEGIN
             if (BigEndian) WSwap(WBuffer,TransLen);
 	     for (z=0; z<(TransLen/2); z++)
 	      BEGIN
               errno=0;
 	       if (((ErgStart+z >= StartData) AND (ErgStart+z <= StopData))
                OR (InpSegment == SegData))
 	        fprintf(TargFile,"M%s",HexWord(WBuffer[z]));
 	       else
 	        fprintf(TargFile,"B%s",HexWord(WBuffer[z]));
 	       ChkIO(TargName);
 	       ChkSum+=WBuffer[z];
 	       SumLen+=Gran;
 	      END
            END
           else if (ActFormat==Atmel)
            BEGIN
             if (TransLen>=2)
              BEGIN
               fprintf(TargFile,"%s",HexWord(WBuffer[0])); SumLen+=2;
              END
            END
 	   else
 	    for (z=0; z<(LongInt)TransLen; z++)
 	     if ((MultiMode<2) OR (z%Gran==MultiMode-2))
 	      BEGIN
 	       errno=0; fprintf(TargFile,"%s",HexByte(Buffer[z])); ChkIO(TargName);
 	       ChkSum+=Buffer[z];
 	       SumLen++;
 	      END

 	   /* Ende Datenzeile */

 	   switch (ActFormat)
            BEGIN
 	     case MotoS:
 	      errno=0;
 	      fprintf(TargFile,"%s\n",HexByte(Lo(ChkSum^0xff)));
 	      ChkIO(TargName);
 	      break;
 	     case MOSHex:
 	      errno=0;
              fprintf(TargFile,"%s\n",HexWord(ChkSum));
              break;
             case IntHex:
             case IntHex16:
             case IntHex32:
              errno=0;
 	      fprintf(TargFile,"%s\n",HexByte(Lo(1+(ChkSum^0xff))));
 	      ChkIO(TargName);
 	      break;
 	     case TekHex:
 	      errno=0;
              fprintf(TargFile,"%s\n",HexByte(Lo(ChkSum)));
 	      ChkIO(TargName);
 	      break;
 	     case TiDSK:
 	      errno=0;
              fprintf(TargFile,"7%sF\n",HexWord(ChkSum));
 	      ChkIO(TargName);
 	      break;
             case Atmel:
              errno=0;
              fprintf(TargFile,"\n");
              ChkIO(TargName);
              break;
             default:
              break;
 	    END

 	   /* Zaehler rauf */

 	   ErgLen-=TransLen;
 	   ErgStart+=TransLen/Gran;
 	  END

         /* Ende der Datenzeilengruppe */

         switch (ActFormat)
          BEGIN
           case MotoS:
            if (SepMoto)
             BEGIN
              errno=0;
 	      fprintf(TargFile,"S%c%s",'9'-MotRecType,HexByte(3+MotRecType));
              ChkIO(TargName);
              for (z=1; z<=2+MotRecType; z++)
               BEGIN
 	        errno=0; fprintf(TargFile,"%s",HexByte(0)); ChkIO(TargName);
               END
              errno=0;
              fprintf(TargFile,"%s\n",HexByte(0xff-3-MotRecType)); 
              ChkIO(TargName);
             END
            break;
           case MOSHex:
            break;
           case IntHex:
           case IntHex16:
           case IntHex32:
            break;
           case TekHex:
            break;
           case TiDSK:
            break;
           case Atmel:
            break;
           default:
            break;
          END;
        END
       if (fseek(SrcFile,NextPos,SEEK_SET)==-1) ChkIO(FileName);
      END
     else
      SkipRecord(InpHeader, FileName, SrcFile);
    END
   while (InpHeader!=0);

   errno = 0; printf("  ("); ChkIO(OutName);
   errno = 0; printf(Integ32Format, SumLen); ChkIO(OutName);
   errno = 0; printf(" %s)\n", getmessage((SumLen == 1) ? Num_Byte : Num_Bytes)); ChkIO(OutName);
   if (!SumLen)
   {
     errno = 0; fputs(getmessage(Num_WarnEmptyFile), stdout); ChkIO(OutName);
   }

   errno=0; fclose(SrcFile); ChkIO(FileName);
END

static ProcessProc CurrProcessor;
static LongWord CurrOffset;

	static void Callback(char *Name)
BEGIN
   CurrProcessor(Name,CurrOffset);
END

	static void ProcessGroup(char *GroupName_O, ProcessProc Processor)
BEGIN
   String Ext,GroupName;

   CurrProcessor=Processor;
   strmaxcpy(GroupName,GroupName_O,255); strmaxcpy(Ext,GroupName,255);
   if (NOT RemoveOffset(GroupName,&CurrOffset)) ParamError(False,Ext);
   AddSuffix(GroupName,getmessage(Num_Suffix));

   if (NOT DirScan(GroupName,Callback))
    fprintf(stderr,"%s%s%s\n",getmessage(Num_ErrMsgNullMaskA),GroupName,getmessage(Num_ErrMsgNullMaskB));
END

        static void MeasureFile(char *FileName, LongWord Offset)
BEGIN
   FILE *f;
   Byte Header, CPU, Segment, Gran;
   Word Length,TestID;
   LongWord Adr,EndAdr;
   LongInt NextPos;

   f=fopen(FileName,OPENRDMODE); if (f==Nil) ChkIO(FileName);

   if (NOT Read2(f,&TestID)) ChkIO(FileName); 
   if (TestID!=FileMagic) FormatError(FileName,getmessage(Num_FormatInvHeaderMsg));

   do
    BEGIN 
     ReadRecordHeader(&Header, &CPU, &Segment, &Gran, FileName, f);

     if (Header == FileHeaderDataRec)
      BEGIN
       if (NOT Read4(f,&Adr)) ChkIO(FileName);
       if (NOT Read2(f,&Length)) ChkIO(FileName);
       NextPos=ftell(f)+Length;
       if (NextPos>FileSize(f))
        FormatError(FileName,getmessage(Num_FormatInvRecordLenMsg));

       if (FilterOK(Header))
        BEGIN
         Adr+=Offset;
 	 EndAdr=Adr+(Length/Gran)-1;
         if (StartAuto) if (StartAdr>Adr) StartAdr=Adr;
 	 if (StopAuto) if (EndAdr>StopAdr) StopAdr=EndAdr;
        END

       fseek(f,NextPos,SEEK_SET);
      END
     else
      SkipRecord(Header, FileName, f);
    END
   while(Header!=0);

   fclose(f);
END

	static CMDResult CMD_AdrRange(Boolean Negate, char *Arg)
BEGIN
   char *p,Save;
   Boolean ok;

   if (Negate)
    BEGIN
     StartAdr=0; StopAdr=0x7fff;
     return CMDOK;
    END
   else
    BEGIN
     p=strchr(Arg,'-'); if (p==Nil) return CMDErr;

     Save = (*p); *p = '\0'; 
     StartAuto = AddressWildcard(Arg);
     if (StartAuto) ok = True;
     else StartAdr = ConstLongInt(Arg, &ok, 10);
     *p = Save;
     if (NOT ok) return CMDErr;

     StopAuto = AddressWildcard(p + 1);
     if (StopAuto) ok = True;
     else StopAdr = ConstLongInt(p + 1, &ok, 10);
     if (NOT ok) return CMDErr;

     if ((NOT StartAuto) AND (NOT StopAuto) AND (StartAdr>StopAdr)) return CMDErr;

     return CMDArg;
    END
END

	static CMDResult CMD_RelAdr(Boolean Negate, char *Arg)
BEGIN
   UNUSED(Arg);

   RelAdr=(NOT Negate);
   return CMDOK;
END

       static CMDResult CMD_AdrRelocate(Boolean Negate, char *Arg)
BEGIN
   Boolean ok;
   UNUSED(Arg);

   if (Negate)
    BEGIN
     Relocate = 0;
     return CMDOK;
    END
   else
    BEGIN
     Relocate = ConstLongInt(Arg,&ok,10);
     if (NOT ok) return CMDErr;

     return CMDArg;
    END
END
   
        static CMDResult CMD_Rec5(Boolean Negate, char *Arg)
BEGIN
   UNUSED(Arg);

   Rec5=(NOT Negate);
   return CMDOK;
END

        static CMDResult CMD_SepMoto(Boolean Negate, char *Arg)
BEGIN
   UNUSED(Arg);

   SepMoto=(NOT Negate);
   return CMDOK;
END

        static CMDResult CMD_IntelMode(Boolean Negate, char *Arg)
BEGIN
   int Mode;
   Boolean ok;

   if (*Arg=='\0') return CMDErr;
   else
    BEGIN
     Mode=ConstLongInt(Arg,&ok,10);
     if ((NOT ok) OR (Mode<0) OR (Mode>2)) return CMDErr;
     else
      BEGIN
       if (NOT Negate) IntelMode=Mode;
       else if (IntelMode==Mode) IntelMode=0;
       return CMDArg;
      END
    END
END

	static CMDResult CMD_MultiMode(Boolean Negate, char *Arg)
BEGIN
   int Mode;
   Boolean ok;

   if (*Arg=='\0') return CMDErr;
   else
    BEGIN
     Mode=ConstLongInt(Arg,&ok,10);
     if ((NOT ok) OR (Mode<0) OR (Mode>3)) return CMDErr;
     else
      BEGIN
       if (NOT Negate) MultiMode=Mode;
       else if (MultiMode==Mode) MultiMode=0;
       return CMDArg;
      END
    END
END

        static CMDResult CMD_DestFormat(Boolean Negate, char *Arg)
BEGIN
#define NameCnt 9

   static char *Names[NameCnt]={"DEFAULT","MOTO","INTEL","INTEL16","INTEL32","MOS","TEK","DSK","ATMEL"};
   static THexFormat Format[NameCnt]={Default,MotoS,IntHex,IntHex16,IntHex32,MOSHex,TekHex,TiDSK,Atmel};
   int z;

   for (z=0; z<(int)strlen(Arg); z++) Arg[z]=mytoupper(Arg[z]);

   z=0;
   while ((z<NameCnt) AND (strcmp(Arg,Names[z])!=0)) z++;
   if (z>=NameCnt) return CMDErr;

   if (NOT Negate) DestFormat=Format[z];
   else if (DestFormat==Format[z]) DestFormat=Default;

   return CMDArg;
END

	static CMDResult CMD_DataAdrRange(Boolean Negate, char *Arg)
BEGIN
   char *p,Save;
   Boolean ok;

   fputs(getmessage(Num_WarnDOption), stderr);
   fflush(stdout);

   if (Negate)
    BEGIN
     StartData=0; StopData=0x1fff;
     return CMDOK;
    END
   else
    BEGIN
     p=strchr(Arg,'-'); if (p==Nil) return CMDErr;

     Save=(*p); *p='\0';
     StartData=ConstLongInt(Arg,&ok,10);
     *p=Save;
     if (NOT ok) return CMDErr;

     StopData=ConstLongInt(p+1,&ok,10);
     if (NOT ok) return CMDErr;

     if (StartData>StopData) return CMDErr;

     return CMDArg;
    END
END

	static CMDResult CMD_EntryAdr(Boolean Negate, char *Arg)
BEGIN
   Boolean ok;

   if (Negate)
    BEGIN
     EntryAdrPresent=False;
     return CMDOK;
    END
   else
    BEGIN
     EntryAdr=ConstLongInt(Arg,&ok,10);
     if ((NOT ok) OR (EntryAdr>0xffff)) return CMDErr;
     EntryAdrPresent = True;
     return CMDArg;
    END
END

        static CMDResult CMD_LineLen(Boolean Negate, char *Arg)
BEGIN
   Boolean ok;

   if (Negate)
    if (*Arg!='\0') return CMDErr;
    else
     BEGIN
      LineLen=16; return CMDOK;
     END
   else if (*Arg=='\0') return CMDErr;
   else
    BEGIN
     LineLen=ConstLongInt(Arg,&ok,10);
     if ((NOT ok) OR (LineLen<1) OR (LineLen>MaxLineLen)) return CMDErr;
     else
      BEGIN
       LineLen+=(LineLen&1); return CMDArg;
      END
    END
END

        static CMDResult CMD_MinMoto(Boolean Negate, char *Arg)
BEGIN
   Boolean ok;

   if (Negate)
    if (*Arg != '\0') return CMDErr;
    else
     BEGIN
      MinMoto = 0; return CMDOK;
     END
   else if (*Arg == '\0') return CMDErr;
   else
    BEGIN
     MinMoto = ConstLongInt(Arg,&ok,10);
     if ((NOT ok) OR (MinMoto < 1) OR (MinMoto > 3)) return CMDErr;
     else return CMDArg;
    END
END

	static CMDResult CMD_AutoErase(Boolean Negate, char *Arg)
BEGIN
   UNUSED(Arg);

   AutoErase=NOT Negate;
   return CMDOK;
END

	static CMDResult CMD_AVRLen(Boolean Negate, char *Arg)
{
   Word Temp;
   Boolean ok;

   if (Negate)  
    {
      AVRLen = AVRLEN_DEFAULT;
      return CMDOK;
    }
   else
    {
      Temp = ConstLongInt(Arg, &ok, 10);
      if ((NOT ok) || (Temp < 2) || (Temp > 3)) return CMDErr;
      else
       {
         AVRLen = Temp;
         return CMDArg;
       }
    }
}

#define P2HEXParamCnt 15
static CMDRec P2HEXParams[P2HEXParamCnt]=
	       {{"f", CMD_FilterList},
		{"r", CMD_AdrRange},
                {"R", CMD_AdrRelocate},
		{"a", CMD_RelAdr},
		{"i", CMD_IntelMode},
		{"m", CMD_MultiMode},
		{"F", CMD_DestFormat},
		{"5", CMD_Rec5},
		{"s", CMD_SepMoto},
		{"d", CMD_DataAdrRange},
                {"e", CMD_EntryAdr},
                {"l", CMD_LineLen},
                {"k", CMD_AutoErase},
                {"M", CMD_MinMoto},
                {"AVRLEN", CMD_AVRLen}};

static Word ChkSum;

	int main(int argc, char **argv)
BEGIN
   char *ph1,*ph2;
   String Ver;

   ParamCount=argc-1; ParamStr=argv;

   nls_init(); NLS_Initialize();

   hex_init();
   endian_init();
   bpemu_init();
   hex_init();
   chunks_init();
   cmdarg_init(*argv);
   toolutils_init(*argv);
   nlmessages_init("p2hex.msg",*argv,MsgId1,MsgId2); ioerrs_init(*argv);

   sprintf(Ver,"P2HEX/C V%s",Version);
   WrCopyRight(Ver);

   InitChunk(&UsedList);

   if (ParamCount==0)
    BEGIN
     errno=0; printf("%s%s%s\n",getmessage(Num_InfoMessHead1),GetEXEName(),getmessage(Num_InfoMessHead2)); ChkIO(OutName);
     for (ph1=getmessage(Num_InfoMessHelp),ph2=strchr(ph1,'\n'); ph2!=Nil; ph1=ph2+1,ph2=strchr(ph1,'\n'))
      BEGIN
       *ph2='\0';
       printf("%s\n",ph1);
       *ph2='\n';
      END
     exit(1);
    END

   StartAdr = 0; StopAdr = 0x7fff;
   StartAuto = False; StopAuto = False;
   StartData = 0; StopData = 0x1fff;
   EntryAdr = (-1); EntryAdrPresent = False; AutoErase = False;
   RelAdr = False; Rec5 = True; LineLen = 16;
   AVRLen = AVRLEN_DEFAULT;
   IntelMode = 0; MultiMode = 0; DestFormat = Default; MinMoto = 1;
   *TargName = '\0';
   Relocate = 0;
   ProcessCMD(P2HEXParams, P2HEXParamCnt, ParUnprocessed, "P2HEXCMD", ParamError);

   if (ProcessedEmpty(ParUnprocessed))
    BEGIN
     errno=0; printf("%s\n",getmessage(Num_ErrMsgTargMissing)); ChkIO(OutName);
     exit(1);
    END

   z=ParamCount;
   while ((z>0) AND (NOT ParUnprocessed[z])) z--;
   strmaxcpy(TargName,ParamStr[z],255);
   if (NOT RemoveOffset(TargName,&Dummy)) ParamError(False,ParamStr[z]);
   ParUnprocessed[z]=False;
   if (ProcessedEmpty(ParUnprocessed))
    BEGIN
     strmaxcpy(SrcName,ParamStr[z],255); DelSuffix(TargName);
    END
   AddSuffix(TargName,HexSuffix);

   if ((StartAuto) OR (StopAuto))
    BEGIN
#ifdef __STDC__
     if (StartAuto) StartAdr=0xffffffffu;
#else
     if (StartAuto) StartAdr=0xffffffff;
#endif
     if (StopAuto) StopAdr=0;
     if (ProcessedEmpty(ParUnprocessed)) ProcessGroup(SrcName,MeasureFile);
     else for (z=1; z<=ParamCount; z++)
      if (ParUnprocessed[z]) ProcessGroup(ParamStr[z],MeasureFile);
     if (StartAdr>StopAdr)
      BEGIN
       errno=0; printf("%s\n",getmessage(Num_ErrMsgAutoFailed)); ChkIO(OutName); exit(1);
      END
    END

   OpenTarget();
   MotoOccured=False; IntelOccured=False;
   MOSOccured=False;  DSKOccured=False;
   MaxMoto=0; MaxIntel=0;

   if (ProcessedEmpty(ParUnprocessed)) ProcessGroup(SrcName,ProcessFile);
   else for (z=1; z<=ParamCount; z++)
    if (ParUnprocessed[z]) ProcessGroup(ParamStr[z],ProcessFile);

   if ((MotoOccured) AND (NOT SepMoto))
    BEGIN
     errno=0; fprintf(TargFile,"S%c%s",'9'-MaxMoto,HexByte(3+MaxMoto)); ChkIO(TargName);
     ChkSum=3+MaxMoto;
     if (NOT EntryAdrPresent) EntryAdr=0;
     if (MaxMoto>=2)
      BEGIN
       errno=0; fputs(HexByte((EntryAdr>>24)&0xff), TargFile); ChkIO(TargName);
       ChkSum+=(EntryAdr>>24)&0xff;
      END
     if (MaxMoto>=1)
      BEGIN
       errno=0; fputs(HexByte((EntryAdr>>16)&0xff), TargFile); ChkIO(TargName);
       ChkSum+=(EntryAdr>>16)&0xff;
      END
     errno=0; fprintf(TargFile,"%s",HexWord(EntryAdr&0xffff)); ChkIO(TargName);
     ChkSum+=(EntryAdr>>8)&0xff;
     ChkSum+=EntryAdr&0xff;
     errno=0; fprintf(TargFile,"%s\n",HexByte(0xff-(ChkSum&0xff))); ChkIO(TargName);
    END

   if (IntelOccured)
    BEGIN
     if (EntryAdrPresent)
      BEGIN
       if (MaxIntel == 2)
        BEGIN
         errno = 0; fprintf(TargFile, ":04000005"); ChkIO(TargName); ChkSum = 4 + 5;
         errno = 0; fprintf(TargFile, "%s", HexLong(EntryAdr)); ChkIO(TargName);
         ChkSum+=((EntryAdr >> 24)& 0xff) +
                 ((EntryAdr >> 16)& 0xff) +
                 ((EntryAdr >> 8) & 0xff) +
                 ( EntryAdr       & 0xff);
        END
       else if (MaxIntel == 1)
        BEGIN
         Seg = (EntryAdr >> 4) & 0xffff;
         Ofs = EntryAdr & 0x000f;
         errno = 0; fprintf(TargFile, ":04000003%s%s", HexWord(Seg), HexWord(Ofs));
         ChkIO(TargName); ChkSum = 4 + 3 + Lo(Seg) + Hi(Seg) + Ofs;
        END
       else
        BEGIN
         errno = 0; fprintf(TargFile, ":00%s03", HexWord(EntryAdr & 0xffff));
         ChkIO(TargName); ChkSum = 3 + Lo(EntryAdr) + Hi(EntryAdr);
        END
       errno = 0; fprintf(TargFile, "%s\n", HexByte(0x100 - ChkSum)); ChkIO(TargName);
      END
     errno=0;
     switch (IntelMode)
      BEGIN
       case 0: fprintf(TargFile,":00000001FF\n"); break;
       case 1: fprintf(TargFile,":00000001\n"); break;
       case 2: fprintf(TargFile,":0000000000\n"); break;
      END
     ChkIO(TargName);
    END

   if (MOSOccured)
    BEGIN
     errno=0; fprintf(TargFile,";0000040004\n"); ChkIO(TargName);
    END

   if (DSKOccured)
    BEGIN
     if (EntryAdrPresent)
      BEGIN
       errno=0;
       fprintf(TargFile,"1%s7%sF\n",HexWord(EntryAdr),HexWord(EntryAdr));
       ChkIO(TargName);
      END
     errno=0; fprintf(TargFile,":\n"); ChkIO(TargName);
    END

   CloseTarget();

   if (AutoErase)
    BEGIN
     if (ProcessedEmpty(ParUnprocessed)) ProcessGroup(SrcName,EraseFile);
     else for (z=1; z<=ParamCount; z++)
      if (ParUnprocessed[z]) ProcessGroup(ParamStr[z],EraseFile);
    END

   return 0;
END
