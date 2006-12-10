/* p2bin.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Umwandlung von AS-Codefiles in Binaerfiles                                */
/*                                                                           */
/* Historie:  3. 6.1996 Grundsteinlegung                                     */
/*           30. 5.1999 0x statt $ erlaubt                                   */
/*            9. 1.2000 plattformabhaengige Formatstrings benutzen           */
/*           24. 3.2000 added symbolic string for byte message               */
/*            4. 8.2000 renamed ParProcessed to ParUnprocessed               */
/*           14. 1.2001 silenced warnings about unused parameters            */
/*                                                                           */
/*****************************************************************************/
/* $Id: p2bin.c,v 1.4 2006/12/09 18:27:30 alfred Exp $                      */
/***************************************************************************** 
 * $Log: p2bin.c,v $
 * Revision 1.4  2006/12/09 18:27:30  alfred
 * - add warning about empty output
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "version.h"
#include "endian.h"
#include "bpemu.h"
#include "strutil.h"
#include "hex.h"
#include "nls.h"
#include "nlmessages.h"
#include "p2bin.rsc"
#include "ioerrs.h"
#include "chunks.h"
#include "stringlists.h"
#include "cmdarg.h"
#include "toolutils.h"

#define BinSuffix ".bin"


typedef void (*ProcessProc)(
#ifdef __PROTOS__
char *FileName, LongWord Offset
#endif
);


static CMDProcessed ParUnprocessed;

static FILE *TargFile;
static String SrcName,TargName;

static LongWord StartAdr,StopAdr,EntryAdr,RealFileLen;
static LongWord MaxGran,Dummy;
static Boolean StartAuto,StopAuto,AutoErase,EntryAdrPresent;

static Byte FillVal;
static Boolean DoCheckSum;

static Byte SizeDiv;
static LongInt ANDMask,ANDEq;
static ShortInt StartHeader;

static ChunkList UsedList;


#ifdef DEBUG
#define ChkIO(s) ChkIO_L(s,__LINE__)

	static void ChkIO_L(char *s, int line)
BEGIN
   if (errno!=0)
    BEGIN
     fprintf(stderr,"%s %d\n",s,line); exit(3);
    END
END
#endif

        static void ParamError(Boolean InEnv, char *Arg)
BEGIN
   printf("%s%s\n%s\n",getmessage((InEnv)?Num_ErrMsgInvEnvParam:Num_ErrMsgInvParam),Arg,getmessage(Num_ErrMsgProgTerm));
   exit(1);
END

#define BufferSize 4096
static Byte Buffer[BufferSize];

        static void OpenTarget(void)
BEGIN
   LongWord Rest, Trans, AHeader;

   TargFile=fopen(TargName,OPENWRMODE);
   if (TargFile==Nil) ChkIO(TargName); 
   RealFileLen=((StopAdr-StartAdr+1)*MaxGran)/SizeDiv;

   AHeader = abs(StartHeader);
   if (StartHeader!=0)
    BEGIN
     memset(Buffer,0,AHeader);
     if (fwrite(Buffer,1,abs(StartHeader),TargFile)!=AHeader) ChkIO(TargName);
    END

   memset(Buffer,FillVal,BufferSize);

   Rest=RealFileLen;
   while (Rest!=0)
    BEGIN
     Trans=min(Rest,BufferSize);
     if (fwrite(Buffer,1,Trans,TargFile)!=Trans) ChkIO(TargName);
     Rest-=Trans;
    END
END

	static void CloseTarget(void)
BEGIN
   LongWord Sum,Rest,Trans,Real,z,bpos, AHeader;

   AHeader = abs(StartHeader);
   if ((EntryAdrPresent) AND (StartHeader!=0))
    BEGIN
     rewind(TargFile); 
     bpos=((StartHeader>0) ? 0 : -1-StartHeader)<<3;
     for (z=0; z<AHeader; z++)
      BEGIN
       Buffer[z]=(EntryAdr >> bpos) & 0xff;
       bpos += (StartHeader>0) ? 8 : -8;
      END
     if (fwrite(Buffer,1,AHeader,TargFile)!=AHeader) ChkIO(TargName);
    END

   if (fclose(TargFile)==EOF) ChkIO(TargName);

   if (DoCheckSum)
    BEGIN
     TargFile=fopen(TargName,OPENUPMODE); if (TargFile==Nil) ChkIO(TargName);
     if (fseek(TargFile,AHeader,SEEK_SET)==-1) ChkIO(TargName);
     Rest=FileSize(TargFile)-1;
     Sum=0;
     while (Rest!=0)
      BEGIN
       Trans=min(Rest,BufferSize);
       Rest-=Trans;
       Real=fread(Buffer,1,Trans,TargFile);
       if (Real!=Trans) ChkIO(TargName);
       for (z=0; z<Trans; Sum+=Buffer[z++]);
      END
     errno=0; printf("%s%s\n",getmessage(Num_InfoMessChecksum),HexLong(Sum));
     Buffer[0]=0x100-(Sum&0xff); fflush(TargFile);
     if (fwrite(Buffer,1,1,TargFile)!=1) ChkIO(TargName); fflush(TargFile);
     if (fclose(TargFile)==EOF) ChkIO(TargName);
    END

   if (Magic!=0) unlink(TargName);
END

        static void ProcessFile(char *FileName, LongWord Offset)
BEGIN
   FILE *SrcFile;
   Word TestID;
   Byte InpHeader, InpCPU, InpSegment;
   LongWord InpStart,SumLen;
   Word InpLen,TransLen,ResLen;
   Boolean doit;
   LongWord ErgStart,ErgStop;
   LongInt NextPos;
   Word ErgLen=0;
   LongInt z;
   Byte Gran;

   SrcFile=fopen(FileName,OPENRDMODE);
   if (SrcFile==Nil) ChkIO(FileName);

   if (NOT Read2(SrcFile,&TestID)) ChkIO(FileName);
   if (TestID!=FileID) FormatError(FileName,getmessage(Num_FormatInvHeaderMsg));

   errno=0; printf("%s==>>%s",FileName,TargName); ChkIO(OutName);

   SumLen=0;

   do
    BEGIN
     ReadRecordHeader(&InpHeader, &InpCPU, &InpSegment, &Gran, FileName, SrcFile);

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
       if (NOT Read4(SrcFile,&InpStart)) ChkIO(FileName);
       if (NOT Read2(SrcFile,&InpLen)) ChkIO(FileName);

       NextPos=ftell(SrcFile)+InpLen;
       if (NextPos>=FileSize(SrcFile)-1)
        FormatError(FileName,getmessage(Num_FormatInvRecordLenMsg));

       doit=(FilterOK(InpHeader) AND (InpSegment==SegCode));

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

       if (doit)
        BEGIN
 	 /* an Anfang interessierender Daten */

 	 if (fseek(SrcFile,(ErgStart-InpStart)*Gran,SEEK_CUR)==-1) ChkIO(FileName);

 	 /* in Zieldatei an passende Stelle */

 	 if (fseek(TargFile,(((ErgStart-StartAdr)*Gran)/SizeDiv)+abs(StartHeader),SEEK_SET)==-1) ChkIO(TargName);

 	 /* umkopieren */

 	 while (ErgLen>0)
 	  BEGIN
 	   TransLen=min(BufferSize,ErgLen);
 	   if (fread(Buffer,1,TransLen,SrcFile)!=TransLen) ChkIO(FileName);
 	   if (SizeDiv==1) ResLen=TransLen;
 	   else
 	    BEGIN
 	     ResLen=0;
 	     for (z=0; z<(LongInt)TransLen; z++)
 	      if (((ErgStart*Gran+z)&ANDMask)==ANDEq)
 	       Buffer[ResLen++]=Buffer[z];
 	    END
 	   if (fwrite(Buffer,1,ResLen,TargFile)!=ResLen) ChkIO(TargName);
 	   ErgLen-=TransLen; ErgStart+=TransLen; SumLen+=ResLen;
 	  END
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

   if (fclose(SrcFile)==EOF) ChkIO(FileName);
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
   Byte Header, CPU, Gran, Segment;
   Word Length,TestID;
   LongWord Adr,EndAdr;
   LongInt NextPos;

   f=fopen(FileName,OPENRDMODE);
   if (f==Nil) ChkIO(FileName);

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

       if (FilterOK(Header) AND (Segment==SegCode))
        BEGIN
         Adr+=Offset;
 	 EndAdr=Adr+(Length/Gran)-1;
         if (Gran>MaxGran) MaxGran=Gran;
         if (StartAuto) if (StartAdr>Adr) StartAdr=Adr;
 	 if (StopAuto) if (EndAdr>StopAdr) StopAdr=EndAdr;
        END

       fseek(f,NextPos,SEEK_SET);
      END
     else
      SkipRecord(Header, FileName, f);
    END
   while(Header!=0);

   if (fclose(f)==EOF) ChkIO(FileName);
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

     Save=(*p); *p='\0'; 
     if ((StartAuto = AddressWildcard(Arg))) err = True;
     else StartAdr = ConstLongInt(Arg, &err, 10);
     *p = Save;
     if (NOT err) return CMDErr;

     if ((StopAuto = AddressWildcard(p + 1))) err = True;
     else StopAdr = ConstLongInt(p+1, &err, 10);
     if (NOT err) return CMDErr;

     if ((NOT StartAuto) AND (NOT StopAuto) AND (StartAdr>StopAdr)) return CMDErr;

     return CMDArg;
    END
END

	static CMDResult CMD_ByteMode(Boolean Negate, char *Arg)
BEGIN
#define ByteModeCnt 9
   static char *ByteModeStrings[ByteModeCnt]={"ALL","EVEN","ODD","BYTE0","BYTE1","BYTE2","BYTE3","WORD0","WORD1"};
   static Byte ByteModeDivs[ByteModeCnt]={1,2,2,4,4,4,4,2,2};
   static Byte ByteModeMasks[ByteModeCnt]={0,1,1,3,3,3,3,2,2};
   static Byte ByteModeEqs[ByteModeCnt]={0,0,1,0,1,2,3,0,2};

   int z;
   UNUSED(Negate);

   if (*Arg=='\0')
    BEGIN
     SizeDiv=1; ANDEq=0; ANDMask=0;
     return CMDOK;
    END
   else
    BEGIN
     for (z=0; z<(int)strlen(Arg); z++) Arg[z]=toupper(Arg[z]);
     ANDEq=0xff;
     for (z=0; z<ByteModeCnt; z++)
      if (strcmp(Arg,ByteModeStrings[z])==0)
       BEGIN
        SizeDiv=ByteModeDivs[z];
        ANDMask=ByteModeMasks[z];
        ANDEq  =ByteModeEqs[z];
       END
      if (ANDEq==0xff) return CMDErr; else return CMDArg;
    END
END

	static CMDResult CMD_StartHeader(Boolean Negate, char *Arg)
BEGIN
   Boolean err;
   ShortInt Sgn;
   
   if (Negate)
    BEGIN
     StartHeader=0; return CMDOK;
    END
   else
    BEGIN
     Sgn=1; if (*Arg=='\0') return CMDErr;
     switch (toupper(*Arg))
      BEGIN
       case 'B': Sgn=(-1);
       case 'L': Arg++;
      END
     StartHeader=ConstLongInt(Arg,&err, 10);
     if ((NOT err) OR (StartHeader>4)) return CMDErr;
     StartHeader*=Sgn;
     return CMDArg;
    END
END	

static CMDResult CMD_EntryAdr(Boolean Negate, char *Arg)
{
   Boolean err;
   
   if (Negate)
   {
     EntryAdrPresent = False;
     return CMDOK;
   }
   else
   {
     EntryAdr = ConstLongInt(Arg, &err, 10);
     if (err)
       EntryAdrPresent = True;
     return (err) ? CMDArg : CMDErr;
   }
}

	static CMDResult CMD_FillVal(Boolean Negate, char *Arg)
BEGIN
   Boolean err;
   UNUSED(Negate);

   FillVal=ConstLongInt(Arg,&err, 10);
   if (NOT err) return CMDErr; else return CMDArg;
END

	static CMDResult CMD_CheckSum(Boolean Negate, char *Arg)
BEGIN
   UNUSED(Arg);

   DoCheckSum=NOT Negate;
   return CMDOK;
END

	static CMDResult CMD_AutoErase(Boolean Negate, char *Arg)
BEGIN
   UNUSED(Arg);

   AutoErase=NOT Negate;
   return CMDOK;
END

#define P2BINParamCnt 8
static CMDRec P2BINParams[P2BINParamCnt]=
	       {{"f", CMD_FilterList},
		{"r", CMD_AdrRange},
		{"s", CMD_CheckSum},
		{"m", CMD_ByteMode},
		{"l", CMD_FillVal},
		{"e", CMD_EntryAdr},
		{"S", CMD_StartHeader},
		{"k", CMD_AutoErase}};

	int main(int argc, char **argv)	
BEGIN
   int z;
   char *ph1,*ph2;
   String Ver;

   ParamStr=argv; ParamCount=argc-1;

   nls_init(); NLS_Initialize();

   endian_init();
   strutil_init();
   bpemu_init();
   hex_init();
   nlmessages_init("p2bin.msg",*argv,MsgId1,MsgId2); ioerrs_init(*argv);
   chunks_init();
   cmdarg_init(*argv);
   toolutils_init(*argv);

   sprintf(Ver,"P2BIN/C V%s",Version);
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

   StartAdr=0; StopAdr=0x7fff; StartAuto=False; StopAuto=False;
   FillVal=0xff; DoCheckSum=False; SizeDiv=1; ANDEq=0;
   EntryAdr=(-1); EntryAdrPresent=False; AutoErase=False;
   StartHeader=0;
   ProcessCMD(P2BINParams,P2BINParamCnt,ParUnprocessed,"P2BINCMD",ParamError);

   if (ProcessedEmpty(ParUnprocessed))
    BEGIN
     errno=0;
     printf("%s\n",getmessage(Num_ErrMsgTargMissing));
     ChkIO(OutName);
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
   AddSuffix(TargName,BinSuffix);

   MaxGran=1;
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

   if (ProcessedEmpty(ParUnprocessed)) ProcessGroup(SrcName,ProcessFile);
   else for (z=1; z<=ParamCount; z++)
    if (ParUnprocessed[z]) ProcessGroup(ParamStr[z],ProcessFile);

   CloseTarget(); 

   if (AutoErase)
    BEGIN
     if (ProcessedEmpty(ParUnprocessed)) ProcessGroup(SrcName,EraseFile);
     else for (z=1; z<=ParamCount; z++)
      if (ParUnprocessed[z]) ProcessGroup(ParamStr[z],EraseFile);
    END

   return 0;
END
