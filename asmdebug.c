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

#include "endian.h"
#include "strutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmfnums.h"

#include "asmdebug.h"


typedef struct 
         {
          Boolean InMacro;
          LongInt LineNum;
          Integer FileName;
          ShortInt Space;
          LongInt Address;
          Word Code;
         } TLineInfo;

typedef struct _TLineInfoList
         {
          struct _TLineInfoList *Next;
          TLineInfo Contents;
         } TLineInfoList,*PLineInfoList;

String TempFileName;
FILE *TempFile;
PLineInfoList LineInfoRoot;


	void AddLineInfo(Boolean InMacro, LongInt LineNum, char *FileName,
                         ShortInt Space, LongInt Address)
BEGIN
   PLineInfoList P,Run;

   P=(PLineInfoList) malloc(sizeof(TLineInfoList));
   P->Contents.InMacro=InMacro;
   P->Contents.LineNum=LineNum;
   P->Contents.FileName=GetFileNum(FileName);
   P->Contents.Space=Space;
   P->Contents.Address=Address;
   P->Contents.Code=(CodeLen<1) ? 0 : WAsmCode[0];

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
   Integer ActFile;
   int ModZ;
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

        static void DumpDebugInfo_Atmel(void)
BEGIN
   static char *OBJString="AVR Object File";
   PLineInfoList Run;
   LongInt FNamePos,RecPos;
   FILE *OBJFile;
   String OBJName;
   char *FName;
   Byte TByte,TNum,NameCnt;
   int z;
   LongInt LTurn;
   Word WTurn;

   strmaxcpy(OBJName,SourceFile,255);
   KillSuffix(OBJName); AddSuffix(OBJName,OBJSuffix);
   OBJFile=fopen(OBJName,OPENWRMODE); if (OBJFile==Nil) ChkIO(10001);

   /* initialer Kopf, Positionen noch unbekannt */

   FNamePos=0; RecPos=0;
   if (NOT Write4(OBJFile,&FNamePos)) ChkIO(10004);
   if (NOT Write4(OBJFile,&RecPos)) ChkIO(10004);
   TByte=9; if (fwrite(&TByte,1,1,OBJFile)!=1) ChkIO(10004);
   NameCnt=GetFileCount()-1; if (fwrite(&NameCnt,1,1,OBJFile)!=1) ChkIO(10004);
   if (fwrite(OBJString,1,strlen(OBJString)+1,OBJFile)!=strlen(OBJString)+1) ChkIO(10004);

   /* Objekt-Records */

   RecPos=ftell(OBJFile);
   for (Run=LineInfoRoot; Run!=Nil; Run=Run->Next)
    if (Run->Contents.Space==SegCode)
     BEGIN
      LTurn=Run->Contents.Address; if (NOT BigEndian) DSwap(&LTurn,4);
      if (fwrite(((Byte *) &LTurn)+1,1,3,OBJFile)!=3) ChkIO(10004);
      WTurn=Run->Contents.Code; if (NOT BigEndian) WSwap(&WTurn,2);
      if (fwrite(&WTurn,1,2,OBJFile)!=2) ChkIO(10004);
      TNum=Run->Contents.FileName-1; if (fwrite(&TNum,1,1,OBJFile)!=1) ChkIO(10004);
      WTurn=Run->Contents.LineNum; if (NOT BigEndian) WSwap(&WTurn,2);
      if (fwrite(&WTurn,1,2,OBJFile)!=2) ChkIO(10004);
      TNum=Ord(Run->Contents.InMacro); if (fwrite(&TNum,1,1,OBJFile)!=1) ChkIO(10004);
     END

   /* Dateinamen */

   FNamePos=ftell(OBJFile);
   for (z=1; z<=NameCnt; z++)
    BEGIN
     FName=NamePart(GetFileName(z));
     if (fwrite(FName,1,strlen(FName)+1,OBJFile)!=strlen(FName)+1) ChkIO(10004);
    END
   TByte=0;
   if (fwrite(&TByte,1,1,OBJFile)!=1) ChkIO(10004);

   /* korrekte Positionen in Kopf schreiben */

   rewind(OBJFile);
   if (NOT BigEndian) DSwap(&FNamePos,4);
   if (fwrite(&FNamePos,1,4,OBJFile)!=4) ChkIO(10004);
   if (NOT BigEndian) DSwap(&RecPos,4);
   if (fwrite(&RecPos,1,4,OBJFile)!=4) ChkIO(10004);

   fclose(OBJFile);
END


        void DumpDebugInfo(void)
BEGIN
   switch (DebugMode)
    BEGIN
     case DebugMAP: DumpDebugInfo_MAP(); break;
     case DebugAtmel: DumpDebugInfo_Atmel(); break;
     default: break;
    END
END


	void asmdebug_init(void)
BEGIN
END
