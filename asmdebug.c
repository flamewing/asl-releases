/* asmdebug.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung der Debug-Informationen zur Assemblierzeit                     */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/


#include "stdinc.h"
#include <string.h>

#include "stringutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmfnums.h"

#include "asmdebug.h"


typedef struct 
         {
          Boolean InASM;
          LongInt LineNum;
          Integer FileName;
          ShortInt Space;
          LongInt Address;
         } TLineInfo;

typedef struct _TLineInfoList
         {
          struct _TLineInfoList *Next;
          TLineInfo Contents;
         } TLineInfoList,*PLineInfoList;

String TempFileName;
FILE *TempFile;
PLineInfoList LineInfoRoot;


	void AddLineInfo(Boolean InASM, LongInt LineNum, char *FileName,
                         ShortInt Space, LongInt Address)
BEGIN
   PLineInfoList P,Run;

   P=(PLineInfoList) malloc(sizeof(TLineInfoList));
   P->Contents.InASM=InASM;
   P->Contents.LineNum=LineNum;
   P->Contents.FileName=GetFileNum(FileName);
   P->Contents.Space=Space;
   P->Contents.Address=Address;

   Run=LineInfoRoot;
   if (Run==Nil)
    BEGIN
     LineInfoRoot=P; P->Next=Nil;
    END
   else
    BEGIN
     while ((Run->Next!=Nil) AND (Run->Next->Contents.Space<Space)) Run=Run->Next;
     while ((Run->Next!=Nil) AND (Run->Next->Contents.FileName<P->Contents.FileName)) Run=Run->Next;
     while ((Run->Next!=Nil) AND (Run->Next->Contents.Address<Address)) Run=Run->Next;
     P->Next=Run->Next; Run->Next=P;
    END
END


	void InitLineInfo(void)
BEGIN
   TempFileName[0]='\0'; LineInfoRoot=Nil;
END


        void ClearLineInfo(void)
BEGIN
   PLineInfoList Run;

   if (TempFileName[0]!='\0')
    BEGIN
     fclose(TempFile); unlink(TempFileName);
    END

   while (LineInfoRoot!=Nil)
    BEGIN
     Run=LineInfoRoot; LineInfoRoot=LineInfoRoot->Next;
     free(Run);
    END

   InitLineInfo();
END


	static void DumpDebugInfo_MAP(void)
BEGIN
   PLineInfoList Run;
   Integer ActFile,ModZ;
   ShortInt ActSeg;
   FILE *MAPFile;
   String MAPName;

   strmaxcpy(MAPName,SourceFile,255); KillSuffix(MAPName); AddSuffix(MAPName,MapSuffix);
   MAPFile=fopen(MAPName,"w"); if (MAPFile==Nil) ChkIO(10001);

   Run=LineInfoRoot; ActSeg=(-1); ActFile=(-1); ModZ=0;
   while (Run!=Nil)
    BEGIN
     if (Run->Contents.Space!=ActSeg)
      BEGIN
       ActSeg=Run->Contents.Space;
       if (ModZ!=0)
        BEGIN
         errno=0; fprintf(MAPFile,"\n"); ChkIO(10004);
        END
       ModZ=0;
       errno=0; fprintf(MAPFile,"Segment %s\n",SegNames[ActSeg]); ChkIO(10004);
       ActFile=(-1);
      END
     if (Run->Contents.FileName!=ActFile)
      BEGIN
       ActFile=Run->Contents.FileName;
       if (ModZ!=0)
        BEGIN
         errno=0; fprintf(MAPFile,"\n"); ChkIO(10004);
        END
       ModZ=0;
       errno=0; fprintf(MAPFile,"File %s\n",GetFileName(Run->Contents.FileName)); ChkIO(10004);
      END;
     errno=0; 
     fprintf(MAPFile,"%5d:%08x ",Run->Contents.LineNum,Run->Contents.Address); 
     ChkIO(10004);
     if (++ModZ==5)
      BEGIN
       errno=0; fprintf(MAPFile,"\n"); ChkIO(10004); ModZ=0;
      END
     Run=Run->Next;
    END
   if (ModZ!=0)
    BEGIN
     errno=0; fprintf(MAPFile,"\n"); ChkIO(10004);
    END

   PrintDebSymbols(MAPFile);

   PrintDebSections(MAPFile);

   fclose(MAPFile);
END


        void DumpDebugInfo(void)
BEGIN
   switch (DebugMode)
    BEGIN
     case DebugMAP: DumpDebugInfo_MAP(); break;
     default: break;
    END
END


	void asmdebug_init(void)
BEGIN
END
