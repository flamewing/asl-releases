/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung der Debug-Informationen zur Assemblierzeit                     */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*           24. 7.1998 NoICE-Format                                         */
/*           25. 7.1998 Adresserfassung Dateien                              */
/*           16. 8.1998 Case-Sensitivitaet NoICE                             */
/*                      NoICE-Zeileninfo nach Dateien sortiert               */
/*           29. 1.1999 uninitialisierten Speicherzugriff beseitigt          */
/*            2. 5.1999 optional mehrere Records im Atmel-Format schreiben   */
/*            1. 6.2000 explicitly write addresses as hex numbers for NoICE  */
/*           2001-09-29 do not accept line info for pseudo segments          */
/*                                                                           */
/*****************************************************************************/


#include "stdinc.h"
#include <string.h>
#include "strutil.h"

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
          LargeInt Address;
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
                         ShortInt Space, LargeInt Address, LargeInt Len)
BEGIN
   PLineInfoList PNeu, PFirst, PLast, Run, Link;
   int RecCnt, z;
   Integer FNum;

   /* do not accept line infor for pseudo segments */

   if (Space > PCMax)
     return;

   /* wieviele Records schreiben ? */

   if ((DebugMode == DebugAtmel) AND (CodeLen > 1)) RecCnt = CodeLen;
   else RecCnt = 1;

   FNum = GetFileNum(FileName);

   /* Einfuegepunkt in Liste finden */

   Run = LineInfoRoot;
   if (Run == Nil)
    Link = Nil;
   else
    BEGIN
     while ((Run->Next != Nil) AND (Run->Next->Contents.Space < Space)) Run = Run->Next;
     while ((Run->Next != Nil) AND (Run->Next->Contents.FileName < FNum)) Run = Run->Next;
     while ((Run->Next != Nil) AND (Run->Next->Contents.Address < Address)) Run = Run->Next;
     Link = Run->Next;
    END

   /* neue Teilliste bilden */

   PLast = PFirst = NULL;
   for (z = 0; z < RecCnt; z++)
    BEGIN
     PNeu = (PLineInfoList) malloc(sizeof(TLineInfoList));
     PNeu->Contents.InMacro = InMacro;
     PNeu->Contents.LineNum = LineNum;
     PNeu->Contents.FileName = FNum;
     PNeu->Contents.Space = Space;
     PNeu->Contents.Address = Address + z;
     PNeu->Contents.Code = ((CodeLen < z + 1) OR (DontPrint)) ? 0 : WAsmCode[z];
     if (z == 0) PFirst = PNeu;
     if (PLast != NULL) PLast->Next = PNeu;
     PLast = PNeu;
    END

   /* Teilliste einhaengen */

   if (Run == Nil) LineInfoRoot = PFirst;
   else Run->Next = PFirst;
   PLast->Next = Link;

   if (Space == SegCode)
    AddAddressRange(FNum, Address, Len);
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
   String MAPName,Tmp;

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
       ActFile = -1;
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
     sprintf(Tmp,LongIntFormat,Run->Contents.LineNum);
     fprintf(MAPFile,"%5s:%s ",Tmp,HexString(Run->Contents.Address,8)); 
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
   if ((int)fwrite(OBJString,1,strlen(OBJString)+1,OBJFile)!=strlen(OBJString)+1) ChkIO(10004);

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
     if ((int)fwrite(FName,1,strlen(FName)+1,OBJFile)!=strlen(FName)+1) ChkIO(10004);
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

	static void DumpDebugInfo_NOICE(void)
BEGIN
   PLineInfoList Run;
   Integer ActFile;
   FILE *MAPFile;
   String MAPName,Tmp1,Tmp2;
   LargeWord Start,End;
   Boolean HadLines;

   strmaxcpy(MAPName, SourceFile, 255); KillSuffix(MAPName); AddSuffix(MAPName, ".noi");
   MAPFile = fopen(MAPName, "w"); if (MAPFile == Nil) ChkIO(10001);

   fprintf(MAPFile, "CASE %d\n", (CaseSensitive) ? 1 : 0);

   PrintNoISymbols(MAPFile);

   for (ActFile = 0; ActFile < GetFileCount(); ActFile++)
    BEGIN
     HadLines = FALSE;
     Run = LineInfoRoot;
     while (Run != Nil)
      BEGIN
       if ((Run->Contents.Space == SegCode) AND (Run->Contents.FileName == ActFile))
        BEGIN
         if (NOT HadLines)
          BEGIN
           GetAddressRange(ActFile, &Start, &End);
           sprintf(Tmp1, LargeHIntFormat, Start);
           errno = 0; 
           fprintf(MAPFile,"FILE %s 0x%s\n", GetFileName(Run->Contents.FileName), Tmp1);
           ChkIO(10004);
          END
         errno = 0; 
         sprintf(Tmp1, LongIntFormat, Run->Contents.LineNum);
         sprintf(Tmp2, LargeHIntFormat, Run->Contents.Address - Start);
         fprintf(MAPFile, "LINE %s 0x%s\n", Tmp1, Tmp2); 
         ChkIO(10004);
         HadLines = TRUE;
        END
       Run = Run->Next;
      END
     if (HadLines)
      BEGIN
       sprintf(Tmp1, LargeHIntFormat, End);
       errno = 0; fprintf(MAPFile, "ENDFILE 0x%s\n", Tmp1); ChkIO(10004);
      END
    END

   fclose(MAPFile);
END


        void DumpDebugInfo(void)
BEGIN
   switch (DebugMode)
    BEGIN
     case DebugMAP: DumpDebugInfo_MAP(); break;
     case DebugAtmel: DumpDebugInfo_Atmel(); break;
     case DebugNoICE: DumpDebugInfo_NOICE(); break;
     default: break;
    END
END


	void asmdebug_init(void)
BEGIN
END
