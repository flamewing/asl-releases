/* p2hex.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Konvertierung von AS-P-Dateien nach Hex                                   */
/*                                                                           */
/* Historie:  1. 6.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "endian.h"
#include "bpemu.h"
#include "hex.h"
#include "nls.h"
#include "stringutil.h"
#include "chunks.h"
#include "decodecmd.h"

#include "asmutils.h"

static char *HexSuffix=".hex";
#define MaxLineLen 254

typedef enum {Default,MotoS,IntHex,IntHex16,IntHex32,MOSHex,TekHex,TiDSK} OutFormat;
typedef void (*ProcessProc)(char *FileName, LongWord Offset);

static CMDProcessed ParProcessed;
static Integer z,z2;
static FILE *TargFile;
static String SrcName,TargName;

static LongWord StartAdr,StopAdr,LineLen;
static LongWord StartData,StopData,EntryAdr;
static Boolean StartAuto,StopAuto,EntryAdrPresent;
static Word Seg,Ofs;
static LongWord Dummy;
static Byte IntelMode;
static Byte MultiMode;   /* 0=8M, 1=16, 2=8L, 3=8H */
static Boolean Rec5;
static Boolean SepMoto;

static Boolean RelAdr,MotoOccured,IntelOccured,MOSOccured,DSKOccured;
static Byte MaxMoto,MaxIntel;

static OutFormat DestFormat;

static ChunkList UsedList;

#include "tools.rsc"
#include "p2hex.rsc"

        static void ParamError(Boolean InEnv, char *Arg)
BEGIN
   printf("%s%s\n",InEnv?ErrMsgInvEnvParam:ErrMsgInvParam,Arg);
   printf("%s\n",ErrMsgProgTerm);
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
   Byte InpHeader,InpSegment,InpGran,BSwap;
   LongInt InpStart,SumLen;
   Word InpLen,TransLen;
   Boolean doit,FirstBank=0;
   Byte Buffer[MaxLineLen];
   Word *WBuffer=(Word *) Buffer;
   LongWord ErgStart,ErgStop=0xffffffffu,NextPos,IntOffset=0,MaxAdr;
   Word ErgLen=0,ChkSum=0,RecCnt,Gran,SwapBase,HSeg;

   LongInt z;

   Byte MotRecType=0;

   OutFormat ActFormat;

   SrcFile=fopen(FileName,"r"); 
   if (SrcFile==Nil) ChkIO(FileName);

   if (NOT Read2(SrcFile,&TestID)) ChkIO(FileName);
   if (TestID!=FileMagic) FormatError(FileName,FormatInvHeaderMsg);

   errno=0; printf("%s==>>%s",FileName,TargName); ChkIO(OutName);

   SumLen=0;

   do
    BEGIN
     ReadRecordHeader(&InpHeader,&InpSegment,&InpGran,FileName,SrcFile);
     if (InpHeader==FileHeaderStartAdr)
      BEGIN
       if (NOT Read4(SrcFile,&ErgStart)) ChkIO(FileName);
       if (NOT EntryAdrPresent)
        BEGIN
         EntryAdr=ErgStart; EntryAdrPresent=True;
        END
      END
     else if (InpHeader!=FileHeaderEnd)
      BEGIN
       Gran=InpGran;
       
       if ((ActFormat=DestFormat)==Default)
        switch (InpHeader)
         BEGIN
          case 0x01: case 0x05: case 0x09: case 0x52: case 0x56: case 0x61:
          case 0x62: case 0x63: case 0x64: case 0x65: case 0x68: case 0x69:
          case 0x6c:
           ActFormat=MotoS; break;
          case 0x12: case 0x21: case 0x31: case 0x39: case 0x41: case 0x49:
          case 0x51: case 0x53: case 0x54: case 0x55: case 0x70: case 0x71:
          case 0x72: case 0x78: case 0x79: case 0x7a: case 0x7b:
           ActFormat=IntHex; break;
          case 0x42: case 0x4c:
           ActFormat=IntHex16; break;
          case 0x13: case 0x29: case 0x76:
           ActFormat=IntHex32; break;
          case 0x11: case 0x19:
           ActFormat=MOSHex; break;
          case 0x74: case 0x75: case 0x77:
           ActFormat=TiDSK; break;
          default: 
           FormatError(FileName,FormatInvRecordHeaderMsg);
         END

       switch (ActFormat)
        BEGIN
         case MotoS:
         case IntHex32:
          MaxAdr=0xffffffffu; break;
         case IntHex16:
          MaxAdr=0xffff0+0xffff; break;
         default:
          MaxAdr=0xffff;
        END

       if (NOT Read4(SrcFile,&InpStart)) ChkIO(FileName);
       if (NOT Read2(SrcFile,&InpLen)) ChkIO(FileName);

       NextPos=ftell(SrcFile)+InpLen;
       if (NextPos>=FileSize(SrcFile)-1)
        FormatError(FileName,FormatInvRecordLenMsg);

       doit=(FilterOK(InpHeader)) AND (InpSegment==SegCode);

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
             errno=0; printf(" %s\n",ErrMsgOverlap); ChkIO(OutName);
            END
          END
        END

       if (ErgStop>MaxAdr)
        BEGIN
         errno=0; printf(" %s\n",ErrMsgAdrOverflow); ChkIO(OutName);
        END

       if (doit)
        BEGIN
 	 /* an Anfang interessierender Daten */

 	 if (fseek(SrcFile,(ErgStart-InpStart)*Gran,SEEK_CUR)==-1) ChkIO(FileName);

 	 /* Statistik, Anzahl Datenzeilen ausrechnen */

         RecCnt=ErgLen/LineLen;
         if ((ErgLen%LineLen)!=0) RecCnt++;

 	 /* relative Angaben ? */

 	 if (RelAdr) ErgStart-=StartAdr;

 	 /* Kopf einer Datenzeilengruppe */

  	 switch (ActFormat)
          BEGIN
   	   case MotoS:
  	    if ((NOT MotoOccured) OR (SepMoto))
  	     BEGIN
  	      errno=0; fprintf(TargFile,"S0030000FC\n"); ChkIO(TargName);
  	     END
  	    if ((ErgStop>>24)!=0) MotRecType=2;
  	    else if ((ErgStop>>16)!=0) MotRecType=1;
  	    else MotRecType=0;
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
  	    IntelOccured=True;
  	    IntOffset=ErgStart&0xfffffff0u;
  	    HSeg=IntOffset>>4; ChkSum=4+Lo(HSeg)+Hi(HSeg);
            errno=0;
  	    fprintf(TargFile,":02000002%s%s\n",HexWord(HSeg),HexByte(0x100-ChkSum));
            if (MaxIntel<1) MaxIntel=1;
  	    ChkIO(TargName);
  	    break;
           case IntHex32:
  	    IntelOccured=True;
            IntOffset=ErgStart&0xffff0000u;
            HSeg=IntOffset>>16; ChkSum=6+Lo(HSeg)+Hi(HSeg);
            fprintf(TargFile,":02000004%s%s\n",HexWord(HSeg),HexByte(0x100-ChkSum));
            if (MaxIntel<2) MaxIntel=2;
  	    ChkIO(TargName);
            FirstBank=False;
            break;
           case TekHex:
            break;
  	   case TiDSK:
  	    if (NOT DSKOccured)
  	     BEGIN
  	      DSKOccured=True;
              errno=0; fprintf(TargFile,"%s%s\n",DSKHeaderLine,TargName); ChkIO(TargName);
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
             IntOffset+=0x10000;
             HSeg=IntOffset>>16; ChkSum=6+Lo(HSeg)+Hi(HSeg);
             errno=0;
             fprintf(TargFile,":02000004%s%s\n",HexWord(HSeg),HexByte(0x100-ChkSum));
             ChkIO(TargName);
             FirstBank=False;
            END

           /* Recordlaenge ausrechnen, fuer Intel32 auf 64K-Grenze begrenzen */

           TransLen=min(LineLen,ErgLen);
           if ((ActFormat==IntHex32) AND ((ErgStart&0xffff)+(TransLen/Gran)>=0x10000))
            BEGIN
             TransLen=Gran*(0x10000-(ErgStart&0xffff));
             FirstBank=True;
            END

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
 	      errno=0; fprintf(TargFile,";%s%s",HexByte(TransLen),HexWord(ErgStart AND 0xffff)); ChkIO(TargName);
 	      ChkSum+=TransLen+Lo(ErgStart)+Hi(ErgStart);
 	      break;
             case IntHex:
             case IntHex16:
             case IntHex32:
 	      errno=0; fprintf(TargFile,":"); ChkIO(TargName); ChkSum=0;
 	      if (MultiMode==0)
 	       BEGIN
 	        errno=0; fprintf(TargFile,"%s",HexByte(TransLen)); ChkIO(TargName);
 	        errno=0; fprintf(TargFile,"%s",HexWord((ErgStart-IntOffset)*Gran)); ChkIO(TargName);
 	        ChkSum+=TransLen;
 	        ChkSum+=Lo((ErgStart-IntOffset)*Gran);
 	        ChkSum+=Hi((ErgStart-IntOffset)*Gran);
 	       END
 	      else
 	       BEGIN
 	        errno=0; fprintf(TargFile,"%s",HexByte(TransLen/Gran)); ChkIO(TargName);
 	        errno=0; fprintf(TargFile,"%s",HexWord(ErgStart-IntOffset)); ChkIO(TargName);
 	        ChkSum+=TransLen/Gran;
 	        ChkSum+=Lo(ErgStart-IntOffset);
 	        ChkSum+=Hi(ErgStart-IntOffset);
 	       END
 	      errno=0; fprintf(TargFile,"00"); ChkIO(TargName);
 	      break;
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
             default:
              break;
 	    END

 	   /* Daten selber */

 	   if (fread(Buffer,1,TransLen,SrcFile)!=TransLen) ChkIO(FileName);
 	   if ((Gran!=1) AND (MultiMode==1))
 	    for (z=0; z<(TransLen/Gran); z++)
 	     BEGIN
 	      SwapBase=z*Gran;
 	      for (z2=0; z2<(Gran/2); z++)
 	       BEGIN
 	        BSwap=Buffer[SwapBase+z2];
 	        Buffer[SwapBase+z2]=Buffer[SwapBase+Gran-1-z2];
 	        Buffer[SwapBase+Gran-1-z2]=BSwap;
 	       END
 	     END
 	   if (ActFormat==TiDSK)
            BEGIN
             if (BigEndian) WSwap(WBuffer,TransLen);
 	     for (z=0; z<(TransLen/2); z++)
 	      BEGIN
               errno=0;
 	       if ((ErgStart+z >= StartData) AND (ErgStart+z <= StopData))
 	        fprintf(TargFile,"M%s",HexWord(WBuffer[z]));
 	       else
 	        fprintf(TargFile,"B%s",HexWord(WBuffer[z]));
 	       ChkIO(TargName);
 	       ChkSum+=WBuffer[z];
 	       SumLen+=Gran;
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
           default:
            break;
          END;
        END
       if (fseek(SrcFile,NextPos,SEEK_SET)==-1) ChkIO(FileName);
      END
    END
   while (InpHeader!=0);

   errno=0; printf("  (%d Byte)\n",SumLen); ChkIO(OutName);

   errno=0; fclose(SrcFile); ChkIO(FileName);
END

	static void ProcessGroup(char *GroupName, ProcessProc Processor)
BEGIN
/**   s:SearchRec;**/
   String /**Path,Name,**/Ext;
   LongWord Offset;

   strmaxcpy(Ext,GroupName,255);
   if (NOT RemoveOffset(GroupName,&Offset)) ParamError(False,Ext);
   AddSuffix(GroupName,Suffix);

   Processor(GroupName,Offset);
/**   FSplit(GroupName,Path,Name,Ext);

   FindFirst(GroupName,Archive,s);
   IF DosError<>0 THEN
    WriteLn(ErrMsgNullMaskA,GroupName,ErrMsgNullMaskB)
   ELSE
    WHILE DosError=0 DO
     BEGIN
      Processor(Path+s.Name,Offset);
      FindNext(s);
     END;**/
END

        static void MeasureFile(char *FileName, LongWord Offset)
BEGIN
   FILE *f;
   Byte Header,Segment,Gran;
   Word Length,TestID;
   LongWord Adr,EndAdr,NextPos;

   f=fopen(FileName,"r"); if (f==Nil) ChkIO(FileName);

   if (NOT Read2(f,&TestID)) ChkIO(FileName); 
   if (TestID!=FileMagic) FormatError(FileName,FormatInvHeaderMsg);

   do
    BEGIN 
     ReadRecordHeader(&Header,&Segment,&Gran,FileName,f);

     if (Header==FileHeaderStartAdr)
      BEGIN
       if (fseek(f,sizeof(LongWord),SEEK_CUR)==-1) ChkIO(FileName);
      END
     else if (Header!=FileHeaderEnd)
      BEGIN
       if (NOT Read4(f,&Adr)) ChkIO(FileName);
       if (NOT Read2(f,&Length)) ChkIO(FileName);
       NextPos=ftell(f)+Length;
       if (NextPos>FileSize(f))
        FormatError(FileName,FormatInvRecordLenMsg);

       if (FilterOK(Header))
        BEGIN
         Adr+=Offset;
 	 EndAdr=Adr+(Length/Gran)-1;
         if (StartAuto) if (StartAdr>Adr) StartAdr=Adr;
 	 if (StopAuto) if (EndAdr>StopAdr) StopAdr=EndAdr;
        END

       fseek(f,NextPos,SEEK_SET);
      END
    END
   while(Header!=0);

   fclose(f);
END

	static CMDResult CMD_AdrRange(Boolean Negate, char *Arg)
BEGIN
   char *p,Save;
   Boolean err;

   if (Negate)
    BEGIN
     StartAdr=0; StopAdr=0x7fff;
     return CMDOK;
    END
   else
    BEGIN
     p=strchr(Arg,'-'); if (p==Nil) return CMDErr;

     Save=*p; *p='\0'; 
     if ((StartAuto=(strcmp(Arg,"$")==0))) err=True;
     else StartAdr=ConstLongInt(Arg,&err);
     *p=Save;
     if (NOT err) return CMDErr;

     if ((StopAuto=(strcmp(p+1,"$")==0))) err=True;
     else StopAdr=ConstLongInt(p+1,&err);
     if (NOT err) return CMDErr;

     if ((NOT StartAuto) AND (NOT StopAuto) AND (StartAdr>StopAdr)) return CMDErr;

     return CMDArg;
    END
END

	static CMDResult CMD_RelAdr(Boolean Negate, char *Arg)
BEGIN
   RelAdr=(NOT Negate);
   return CMDOK;
END

        static CMDResult CMD_Rec5(Boolean Negate, char *Arg)
BEGIN
   Rec5=(NOT Negate);
   return CMDOK;
END

        static CMDResult CMD_SepMoto(Boolean Negate, char *Arg)
BEGIN
   SepMoto=(NOT Negate);
   return CMDOK;
END

        static CMDResult CMD_IntelMode(Boolean Negate, char *Arg)
BEGIN
   Integer Mode;
   Boolean err;

   if (*Arg=='\0') return CMDErr;
   else
    BEGIN
     Mode=ConstLongInt(Arg,&err);
     if ((NOT err) OR (Mode<0) OR (Mode>2)) return CMDErr;
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
   Integer Mode;
   Boolean err;

   if (*Arg=='\0') return CMDErr;
   else
    BEGIN
     Mode=ConstLongInt(Arg,&err);
     if ((NOT err) OR (Mode<0) OR (Mode>3)) return CMDErr;
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
#define NameCnt 8

   static char *Names[NameCnt]={"DEFAULT","MOTO","INTEL","INTEL16","INTEL32","MOS","TEK","DSK"};
   static OutFormat Format[NameCnt]={Default,MotoS,IntHex,IntHex16,IntHex32,MOSHex,TekHex,TiDSK};
   Integer z;

   for (z=0; z<strlen(Arg); z++) Arg[z]=toupper(Arg[z]);

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
   Boolean err;

   if (Negate)
    BEGIN
     StartData=0; StopData=0x1fff;
     return CMDOK;
    END
   else
    BEGIN
     p=strchr(Arg,'-'); if (p==Nil) return CMDErr;

     Save=*p; *p='\0';
     StartData=ConstLongInt(Arg,&err);
     *p=Save;
     if (NOT err) return CMDErr;

     StopData=ConstLongInt(p+1,&err);
     if (NOT err) return CMDErr;

     if (StartData>StopData) return CMDErr;

     return CMDArg;
    END
END

	static CMDResult CMD_EntryAdr(Boolean Negate, char *Arg)
BEGIN
   Boolean err;

   if (Negate)
    BEGIN
     EntryAdrPresent=False;
     return CMDOK;
    END
   else
    BEGIN
     EntryAdr=ConstLongInt(Arg,&err);
     if ((NOT err) OR (EntryAdr>0xffff)) return CMDErr;
     return CMDArg;
    END
END

        static CMDResult CMD_LineLen(Boolean Negate, char *Arg)
BEGIN
   Boolean err;

   if (Negate)
    if (*Arg!='\0') return CMDErr;
    else
     BEGIN
      LineLen=16; return CMDOK;
     END
   else if (*Arg=='\0') return CMDErr;
   else
    BEGIN
     LineLen=ConstLongInt(Arg,&err);
     if ((err!=0) OR (LineLen<1) OR (LineLen>MaxLineLen)) return CMDErr;
     else
      BEGIN
       LineLen+=(LineLen&1); return CMDArg;
      END
    END
END

#define P2HEXParamCnt 11
static CMDRec P2HEXParams[P2HEXParamCnt]=
	       {{"f", CMD_FilterList},
		{"r", CMD_AdrRange},
		{"a", CMD_RelAdr},
		{"i", CMD_IntelMode},
		{"m", CMD_MultiMode},
		{"F", CMD_DestFormat},
		{"5", CMD_Rec5},
		{"s", CMD_SepMoto},
		{"d", CMD_DataAdrRange},
                {"e", CMD_EntryAdr},
                {"l", CMD_LineLen}};

static Word ChkSum;

	int main(int argc, char **argv)
BEGIN
   ParamCount=argc-1; ParamStr=argv;
   hex_init();
   endian_init();
   bpemu_init();
   hex_init();
   nls_init();
   chunks_init();
   decodecmd_init();
   asmutils_init();

   /**NLS_Initialize;**/ WrCopyRight("P2HEX/C V1.41r5");

   InitChunk(&UsedList);

   if (ParamCount==0)
    BEGIN
     errno=0; printf("%s%s%s\n",InfoMessHead1,GetEXEName(),InfoMessHead2); ChkIO(OutName);
     for (z=0; z<InfoMessHelpCnt; z++)
      BEGIN
       errno=0; printf("%s\n",InfoMessHelp[z]); ChkIO(OutName);
      END
     exit(1);
    END

   StartAdr=0; StopAdr=0x7fff;
   StartAuto=False; StopAuto=False;
   StartData=0; StopData=0x1fff;
   EntryAdr=-1; EntryAdrPresent=False;
   RelAdr=False; Rec5=True; LineLen=16;
   IntelMode=0; MultiMode=0; DestFormat=Default;
   *TargName='\0';
   ProcessCMD(P2HEXParams,P2HEXParamCnt,ParProcessed,"P2HEXCMD",ParamError);

   if (ProcessedEmpty(ParProcessed))
    BEGIN
     errno=0; printf("%s\n",ErrMsgTargMissing); ChkIO(OutName);
     exit(1);
    END

   z=ParamCount;
   while ((z>0) AND (NOT ParProcessed[z])) z--;
   strmaxcpy(TargName,ParamStr[z],255);
   if (NOT RemoveOffset(TargName,&Dummy)) ParamError(False,ParamStr[z]);
   ParProcessed[z]=False;
   if (ProcessedEmpty(ParProcessed))
    BEGIN
     strmaxcpy(SrcName,ParamStr[z],255); DelSuffix(TargName);
    END
   AddSuffix(TargName,HexSuffix);

   if ((StartAuto) OR (StopAuto))
    BEGIN
     if (StartAuto) StartAdr=0xffffffffu;
     if (StopAuto) StopAdr=0;
     if (ProcessedEmpty(ParProcessed)) ProcessGroup(SrcName,MeasureFile);
     else for (z=1; z<=ParamCount; z++)
      if (ParProcessed[z]) ProcessGroup(ParamStr[z],MeasureFile);
     if (StartAdr>StopAdr)
      BEGIN
       errno=0; printf("%s\n",ErrMsgAutoFailed); ChkIO(OutName); exit(1);
      END
    END

   OpenTarget();
   MotoOccured=False; IntelOccured=False;
   MOSOccured=False;  DSKOccured=False;
   MaxMoto=0; MaxIntel=0;

   if (ProcessedEmpty(ParProcessed)) ProcessGroup(SrcName,ProcessFile);
   else for (z=1; z<=ParamCount; z++)
    if (ParProcessed[z]) ProcessGroup(ParamStr[z],ProcessFile);

   if ((MotoOccured) AND (NOT SepMoto))
    BEGIN
     errno=0; fprintf(TargFile,"S%c%s",'9'-MaxMoto,HexByte(3+MaxMoto)); ChkIO(TargName);
     ChkSum=3+MaxMoto;
     if (NOT EntryAdrPresent) EntryAdr=0;
     if (MaxMoto>=2)
      BEGIN
       errno=0; fprintf(TargFile,HexByte((EntryAdr>>24)&0xff)); ChkIO(TargName);
       ChkSum+=(EntryAdr>>24)&0xff;
      END
     if (MaxMoto>=1)
      BEGIN
       errno=0; fprintf(TargFile,HexByte((EntryAdr>>16)&0xff)); ChkIO(TargName);
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
       if (MaxIntel==2)
        BEGIN
         errno=0; fprintf(TargFile,":04000003"); ChkIO(TargName); ChkSum=4+3;
         errno=0; fprintf(TargFile,"%s",HexLong(EntryAdr)); ChkIO(TargName);
         ChkSum+=((EntryAdr>>24)&0xff)+
                 ((EntryAdr>>16)&0xff)+
                 ((EntryAdr>>8) &0xff)+
                 ( EntryAdr     &0xff);
        END
       else if (MaxIntel==1)
        BEGIN
         errno=0; fprintf(TargFile,":04000003"); ChkIO(TargName); ChkSum=4+3;
         Seg=(EntryAdr>>4)&0xffff;
         Ofs=EntryAdr&0x000f;
         errno=0; fprintf(TargFile,"%s%s",HexWord(Seg),HexWord(Ofs));
         ChkIO(TargName);
         ChkSum+=Lo(Seg)+Hi(Seg)+Ofs;
        END
       else
        BEGIN
         errno=0; fprintf(TargFile,":02000003%s",HexWord(EntryAdr&0xffff));
         ChkIO(TargName); ChkSum=2+3+Lo(EntryAdr)+Hi(EntryAdr);
        END
       errno=0; fprintf(TargFile,"%s\n",HexByte(0x100-ChkSum)); ChkIO(TargName);
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
   return 0;
END
