/* bind.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Bearbeitung von AS-P-Dateien                                              */
/*                                                                           */
/* Historie:  1. 6.1996 Grundsteinlegung                                     */
/*            9. 1.2000 plattformabhaengige Formatstrings benutzen           */
/*           24. 3.2000 added symbolic string for byte message               */
/*            4. 7.2000 renamed ParProcessed to ParUnprocessed               */
/*           30. 5.2001 move copy buffer to heap to avoid stack overflows on */
/*                      DOS platforms                                        */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "version.h"
#include "endian.h"
#include "stdhandl.h"
#include "bpemu.h"
#include "strutil.h"
#include "stringlists.h"
#include "cmdarg.h"
#include "toolutils.h"
#include "nls.h"
#include "nlmessages.h"
#include "pbind.rsc"
#include "ioerrs.h"

#define BufferSize 8192

static char *Creator="BIND/C 1.42";

static CMDProcessed ParUnprocessed;

static FILE *TargFile;
static String TargName;
static Byte *Buffer;

	static void OpenTarget(void)
BEGIN
   TargFile=fopen(TargName,OPENWRMODE);
   if (TargFile==Nil) ChkIO(TargName);
   if (NOT Write2(TargFile,&FileID)) ChkIO(TargName); 
END

	static void CloseTarget(void)
BEGIN
   Byte EndHeader=FileHeaderEnd;

   if (fwrite(&EndHeader,1,1,TargFile)!=1) ChkIO(TargName);
   if (fwrite(Creator,1,strlen(Creator),TargFile)!=strlen(Creator)) ChkIO(TargName);
   if (fclose(TargFile)==EOF) ChkIO(TargName);
   if (Magic!=0) unlink(TargName);
END

	static void ProcessFile(char *FileName)
BEGIN
   FILE *SrcFile;
   Word TestID;
   Byte InpHeader, InpCPU, InpSegment, InpGran;
   LongInt InpStart,SumLen;
   Word InpLen,TransLen;
   Boolean doit;

   SrcFile=fopen(FileName,OPENRDMODE);
   if (SrcFile==Nil) ChkIO(FileName);  

   if (NOT Read2(SrcFile,&TestID)) ChkIO(FileName);
   if (TestID!=FileMagic) FormatError(FileName,getmessage(Num_FormatInvHeaderMsg));

   errno=0; printf("%s==>>%s",FileName,TargName); ChkIO(OutName);

   SumLen=0;

   do
    BEGIN
     ReadRecordHeader(&InpHeader, &InpCPU, &InpSegment, &InpGran, FileName, SrcFile);

     if (InpHeader == FileHeaderStartAdr)
      BEGIN
       if (NOT Read4(SrcFile,&InpStart)) ChkIO(FileName);
       WriteRecordHeader(&InpHeader, &InpCPU, &InpSegment, &InpGran, TargName, TargFile);
       if (NOT Write4(TargFile,&InpStart)) ChkIO(TargName);
      END

     else if (InpHeader == FileHeaderDataRec)
      BEGIN
       if (NOT Read4(SrcFile,&InpStart)) ChkIO(FileName);
       if (NOT Read2(SrcFile,&InpLen)) ChkIO(FileName);

       if (ftell(SrcFile)+InpLen>=FileSize(SrcFile)-1)
        FormatError(FileName,getmessage(Num_FormatInvRecordLenMsg));

       doit=FilterOK(InpCPU);

       if (doit)
        BEGIN
	 SumLen+=InpLen;
         WriteRecordHeader(&InpHeader, &InpCPU, &InpSegment, &InpGran, TargName, TargFile);
	 if (NOT Write4(TargFile,&InpStart)) ChkIO(TargName);
	 if (NOT Write2(TargFile,&InpLen)) ChkIO(TargName);
	 while (InpLen>0)
	  BEGIN
           TransLen=min(BufferSize,InpLen);
	   if (fread(Buffer,1,TransLen,SrcFile)!=TransLen) ChkIO(FileName);
	   if (fwrite(Buffer,1,TransLen,TargFile)!=TransLen) ChkIO(TargName);
	   InpLen-=TransLen;
	  END
        END
       else
        BEGIN
         if (fseek(SrcFile,InpLen,SEEK_CUR)==-1) ChkIO(FileName);
        END
      END
     else
      SkipRecord(InpHeader, FileName, SrcFile);
    END 
   while (InpHeader!=FileHeaderEnd);

   errno = 0; printf("  ("); ChkIO(OutName);
   errno = 0; printf(Integ32Format, SumLen); ChkIO(OutName);
   errno = 0; printf(" %s)\n", getmessage((SumLen == 1) ? Num_Byte : Num_Bytes)); ChkIO(OutName);

   if (fclose(SrcFile)==EOF) ChkIO(FileName);
END

	static void ParamError(Boolean InEnv, char *Arg)
BEGIN
   printf("%s%s\n",getmessage(InEnv ? Num_ErrMsgInvEnvParam : Num_ErrMsgInvParam),Arg);
   printf("%s\n",getmessage(Num_ErrMsgProgTerm));
   exit(1);
END

#define BINDParamCnt 1
static CMDRec BINDParams[BINDParamCnt]=
	      {{"f", CMD_FilterList}};

	int main(int argc, char **argv)
BEGIN
   int z;
   char *ph1,*ph2;
   String Ver;

   ParamCount=argc-1; ParamStr=argv;

   NLS_Initialize();
   endian_init();

   sprintf(Ver,"BIND/C V%s",Version);
   WrCopyRight(Ver);

   stdhandl_init(); cmdarg_init(*argv); toolutils_init(*argv); nls_init();
   nlmessages_init("pbind.msg",*argv,MsgId1,MsgId2); ioerrs_init(*argv);

   Buffer = (Byte*) malloc(sizeof(Byte) * BufferSize);

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

   ProcessCMD(BINDParams,BINDParamCnt,ParUnprocessed,"BINDCMD",ParamError);

   z=ParamCount;
   while ((z>0) AND (NOT ParUnprocessed[z])) z--;
   if (z==0)
    BEGIN
     errno=0; printf("%s\n",getmessage(Num_ErrMsgTargetMissing));
     ChkIO(OutName);
     exit(1);
    END
   else
    BEGIN
     strmaxcpy(TargName,ParamStr[z],255); ParUnprocessed[z]=False;
     AddSuffix(TargName,getmessage(Num_Suffix));
    END

   OpenTarget();

   for (z=1; z<=ParamCount; z++)
    if (ParUnprocessed[z]) DirScan(ParamStr[z],ProcessFile);

   CloseTarget();

   return 0;
END
