/* p2bin.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Umwandlung von AS-Codefiles in Binaerfiles                                */
/*                                                                           */
/* Historie:  3. 6.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "endian.h"
#include "bpemu.h"
#include "stringutil.h"
#include "hex.h"
#include "nls.h"
#include "chunks.h"
#include "decodecmd.h"
#include "asmutils.h"

#define BinSuffix ".bin"


typedef void (*ProcessProc)(char *FileName, LongWord Offset);


static CMDProcessed ParProcessed;
static Integer z;

static FILE *TargFile;
static String SrcName,TargName;

static LongWord StartAdr,StopAdr,RealFileLen;
static LongWord MaxGran,Dummy;
static Boolean StartAuto,StopAuto;

static Byte FillVal;
static Boolean DoCheckSum;

static Byte SizeDiv;
static LongInt ANDMask,ANDEq;

static ChunkList UsedList;

#include "ioerrors.rsc"
#include "tools.rsc"
#include "p2bin.rsc"

#ifndef DEBUG
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
   printf("%s%s\n%s\n",(InEnv)?ErrMsgInvEnvParam:ErrMsgInvParam,Arg,ErrMsgProgTerm);
   exit(1);
END

#define BufferSize 4096

        static void OpenTarget(void)
BEGIN
   LongWord Rest,Trans;
   Byte Buffer[BufferSize];

   TargFile=fopen(TargName,"w");
   if (TargFile==Nil) ChkIO(TargName); 
   RealFileLen=((StopAdr-StartAdr+1)*MaxGran)/SizeDiv;

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
   LongWord Sum,Rest,Trans,Real,z;
   Byte Buffer[BufferSize];

   if (fclose(TargFile)==EOF) ChkIO(TargName);

   if (DoCheckSum)
    BEGIN
     TargFile=fopen(TargName,"r+"); if (TargFile==Nil) ChkIO(TargName);
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
     errno=0; printf("%s%s\n",InfoMessChecksum,HexLong(Sum));
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
   Byte InpHeader,InpSegment;
   LongWord InpStart,SumLen;
   Word InpLen,TransLen,ResLen;
   Boolean doit;
   Byte Buffer[BufferSize];
   LongWord ErgStart,ErgStop,NextPos;
   Word ErgLen=0;
   LongInt z;
   Byte Gran;

   SrcFile=fopen(FileName,"r");
   if (SrcFile==Nil) ChkIO(FileName);

   if (NOT Read2(SrcFile,&TestID)) ChkIO(FileName);
   if (TestID!=FileID) FormatError(FileName,FormatInvHeaderMsg);

   errno=0; printf("%s==>>%s",FileName,TargName); ChkIO(OutName);

   SumLen=0;

   do
    BEGIN
     ReadRecordHeader(&InpHeader,&InpSegment,&Gran,FileName,SrcFile);
     if (InpHeader==FileHeaderStartAdr)
      BEGIN
       if (NOT Read4(SrcFile,&ErgStart)) ChkIO(FileName);
      END
     else if (InpHeader!=FileHeaderEnd)
      BEGIN
       if (NOT Read4(SrcFile,&InpStart)) ChkIO(FileName);
       if (NOT Read2(SrcFile,&InpLen)) ChkIO(FileName);

       NextPos=ftell(SrcFile)+InpLen;
       if (NextPos>=FileSize(SrcFile)-1)
        FormatError(FileName,FormatInvRecordLenMsg);

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
             errno=0; printf(" %s\n",ErrMsgOverlap); ChkIO(OutName);
            END
          END
        END

       if (doit)
        BEGIN
 	 /* an Anfang interessierender Daten */

 	 if (fseek(SrcFile,(ErgStart-InpStart)*Gran,SEEK_CUR)==-1) ChkIO(FileName);

 	 /* in Zieldatei an passende Stelle */

 	 if (fseek(TargFile,((ErgStart-StartAdr)*Gran)/SizeDiv,SEEK_SET)==-1) ChkIO(TargName);

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
    END
   while (InpHeader!=0);

   errno=0; printf("  (%d Byte)\n",SumLen); ChkIO(OutName);

   if (fclose(SrcFile)==EOF) ChkIO(FileName);
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
   Byte Header,Gran,Segment;
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

	static CMDResult CMD_ByteMode(Boolean Negate, char *Arg)
BEGIN
#define ByteModeCnt 9
   static char *ByteModeStrings[ByteModeCnt]={"ALL","EVEN","ODD","BYTE0","BYTE1","BYTE2","BYTE3","WORD0","WORD1"};
   static Byte ByteModeDivs[ByteModeCnt]={1,2,2,4,4,4,4,2,2};
   static Byte ByteModeMasks[ByteModeCnt]={0,1,1,3,3,3,3,2,2};
   static Byte ByteModeEqs[ByteModeCnt]={0,0,1,0,1,2,3,0,2};

   Integer z;

   if (*Arg=='\0')
    BEGIN
     SizeDiv=1; ANDEq=0; ANDMask=0;
     return CMDOK;
    END
   else
    BEGIN
     for (z=0; z<strlen(Arg); z++) Arg[z]=toupper(Arg[z]);
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

	static CMDResult CMD_FillVal(Boolean Negate, char *Arg)
BEGIN
   Boolean err;

   FillVal=ConstLongInt(Arg,&err);
   if (NOT err) return CMDErr; else return CMDArg;
END

	static CMDResult CMD_CheckSum(Boolean Negate, char *Arg)
BEGIN
   DoCheckSum=NOT Negate;
   return CMDOK;
END

#define P2BINParamCnt 5
static CMDRec P2BINParams[P2BINParamCnt]=
	       {{"f", CMD_FilterList},
		{"r", CMD_AdrRange},
		{"s", CMD_CheckSum},
		{"m", CMD_ByteMode},
		{"l", CMD_FillVal}};

	int main(int argc, char **argv)	
BEGIN
   ParamStr=argv; ParamCount=argc-1;
   endian_init();
   stringutil_init();
   bpemu_init();
   hex_init();
   nls_init();
   chunks_init();
   decodecmd_init();
   asmutils_init();

   /**NLS_Initialize;**/ WrCopyRight("P2BIN/C V1.41r5");

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

   StartAdr=0; StopAdr=0x7fff; StartAuto=False; StopAuto=False;
   FillVal=0xff; DoCheckSum=False; SizeDiv=1; ANDEq=0;
   ProcessCMD(P2BINParams,P2BINParamCnt,ParProcessed,"P2BINCMD",ParamError);

   if (ProcessedEmpty(ParProcessed))
    BEGIN
     errno=0;
     printf("%s\n",ErrMsgTargMissing);
     ChkIO(OutName);
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
   AddSuffix(TargName,BinSuffix);

   MaxGran=1;
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

   if (ProcessedEmpty(ParProcessed)) ProcessGroup(SrcName,ProcessFile);
   else for (z=1; z<=ParamCount; z++)
    if (ParProcessed[z]) ProcessGroup(ParamStr[z],ProcessFile);

   CloseTarget(); 
   return 0;
END
