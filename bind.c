/* bind.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Bearbeitung von AS-P-Dateien                                              */
/*                                                                           */
/* Historie:  1. 6.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "endian.h"
#include "stdhandl.h"
#include "bpemu.h"
#include "stringutil.h"
#include "decodecmd.h"
#include "asmutils.h"
#include "nls.h"

static char *Creator="BIND/C 1.41r5";


static CMDProcessed ParProcessed;
static Integer z;

static FILE *TargFile;
static String TargName;

#include "tools.rsc"
#include "bind.rsc"

	static void OpenTarget(void)
BEGIN
   TargFile=fopen(TargName,"w");
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
#define BufferSize 8192
   FILE *SrcFile;
   Word TestID;
   Byte InpHeader,InpSegment,InpGran;
   LongInt InpStart,SumLen;
   Word InpLen,TransLen;
   Boolean doit;
   Byte Buffer[BufferSize];

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
       if (NOT Read4(SrcFile,&InpStart)) ChkIO(FileName);
       WriteRecordHeader(&InpHeader,&InpSegment,&InpGran,TargName,TargFile);
       if (NOT Write4(TargFile,&InpStart)) ChkIO(TargName);
      END
     else if (InpHeader!=FileHeaderEnd)
      BEGIN
       if (NOT Read4(SrcFile,&InpStart)) ChkIO(FileName);
       if (NOT Read2(SrcFile,&InpLen)) ChkIO(FileName);

       if (ftell(SrcFile)+InpLen>=FileSize(SrcFile)-1)
        FormatError(FileName,FormatInvRecordLenMsg);

       doit=FilterOK(InpHeader);

       if (doit)
        BEGIN
	 SumLen+=InpLen;
         WriteRecordHeader(&InpHeader,&InpSegment,&InpGran,TargName,TargFile);
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
    END 
   while (InpHeader!=FileHeaderEnd);

   errno=0; printf("  (%d Byte",SumLen); ChkIO(OutName);

   if (fclose(SrcFile)==EOF) ChkIO(FileName);
END

	static void ProcessGroup(char *GroupName)
BEGIN/**
   s:SearchRec;
   Path,Name,Ext:String;**/
   String Name;

   strmaxcpy(Name,GroupName,255);
   AddSuffix(Name,Suffix);
   ProcessFile(Name);
/**
   FSplit(GroupName,Path,Name,Ext);

   FindFirst(GroupName,Archive,s);
   WHILE DosError=0 DO
    BEGIN
     ProcessFile(Path+s.Name);
     FindNext(s);
    END;**/
END

	static void ParamError(Boolean InEnv, char *Arg)
BEGIN
   printf("%s%s\n",InEnv ? ErrMsgInvEnvParam : ErrMsgInvParam,Arg);
   printf("%s\n",ErrMsgProgTerm);
   exit(1);
END

#define BINDParamCnt 1
static CMDRec BINDParams[BINDParamCnt]=
	      {{"f", CMD_FilterList}};

	void main(int argc, char **argv)
BEGIN
   ParamCount=argc-1; ParamStr=argv;

   NLS_Initialize(); WrCopyRight("BIND V1.41r5");

   stdhandl_init(); decodecmd_init(); asmutils_init(); nls_init();
   if (ParamCount==0)
    BEGIN
     errno=0; printf("%s%s%s\n",InfoMessHead1,GetEXEName(),InfoMessHead2); ChkIO(OutName);
     for (z=0; z<InfoMessHelpCnt; z++)
      BEGIN
       errno=0; printf("%s\n",InfoMessHelp[z]); ChkIO(OutName);
      END
     exit(1);
    END

   ProcessCMD(BINDParams,BINDParamCnt,ParProcessed,"BINDCMD",ParamError);

   z=ParamCount;
   while ((z>0) AND (NOT ParProcessed[z])) z--;
   if (z==0)
    BEGIN
     errno=0; printf("%s\n",ErrMsgTargetMissing);
     ChkIO(OutName);
     exit(1);
    END
   else
    BEGIN
     strmaxcpy(TargName,ParamStr[z],255); ParProcessed[z]=False;
     AddSuffix(TargName,Suffix);
    END

   OpenTarget();

   for (z=1; z<=ParamCount; z++)
    if (ParProcessed[z]) ProcessGroup(ParamStr[z]);

   CloseTarget();
END
