/* asmcode.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung der Code-Datei                                                 */
/*                                                                           */
/* Historie: 18. 5.1996 Grundsteinlegung                                     */
/*           19. 1.2000 Patchlistenverarbeitung begonnen                     */
/*           18. 6.2000 moved code buffer to heap                            */
/*           26. 6.2000 added export list                                    */
/*            4. 7.2000 only write data records in extended format           */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "version.h"
#include "endian.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmrelocs.h"

#define CodeBufferSize 512

static Word LenSoFar;
static LongInt RecPos, LenPos;
static Boolean ThisRel;

static Word CodeBufferFill;
static Byte *CodeBuffer;

PPatchEntry PatchList, PatchLast;
PExportEntry ExportList, ExportLast;
LongInt SectSymbolCounter;
String SectSymbolName;

        static void FlushBuffer(void)
BEGIN
   if (CodeBufferFill>0)
    BEGIN
     if (fwrite(CodeBuffer,1,CodeBufferFill,PrgFile)!=CodeBufferFill) ChkIO(10004);
     CodeBufferFill=0;
    END
END

	void DreheCodes(void)
BEGIN
   int z;
   LongInt l=CodeLen*Granularity();

    switch (ActListGran)
     BEGIN
      case 2: 
       for (z=0; z<(l>>1); z++) 
        WAsmCode[z]=((WAsmCode[z]&0xff)<<8)+((WAsmCode[z]&0xff00)>>8);
       break;
      case 4:
       for (z=0; z<(l>>2); z++)
        {
#ifdef __STDC__
	DAsmCode[z]=((DAsmCode[z]&0xff000000u)>>24)+
		    ((DAsmCode[z]&0x00ff0000u)>>8)+
		    ((DAsmCode[z]&0x0000ff00u)<<8)+
		    ((DAsmCode[z]&0x000000ffu)<<24);
#else
	DAsmCode[z]=((DAsmCode[z]&0xff000000)>>24)+
		    ((DAsmCode[z]&0x00ff0000)>>8)+
		    ((DAsmCode[z]&0x0000ff00)<<8)+
		    ((DAsmCode[z]&0x000000ff)<<24);
#endif
        }
       break;
     END
END

	static void WrPatches(void)
BEGIN
   LongWord Cnt, ExportCnt, StrLen;
   Byte T8;

   if ((PatchList != Nil) OR (ExportList != Nil))
    BEGIN
     /* find out length of string field */

     Cnt = StrLen = 0;
     for (PatchLast = PatchList; PatchLast != Nil; PatchLast = PatchLast->Next)
      BEGIN
       Cnt++;
       StrLen += (PatchLast->len = strlen(PatchLast->Ref) + 1);
      END
     ExportCnt = 0;
     for (ExportLast = ExportList; ExportLast != Nil; ExportLast = ExportLast->Next)
      BEGIN
       ExportCnt++;
       StrLen += (ExportLast->len = strlen(ExportLast->Name) + 1);
      END

     /* write header */

     T8 = FileHeaderRelocInfo; if (fwrite(&T8, 1, 1, PrgFile) != 1) ChkIO(10004);
     if (NOT Write4(PrgFile, &Cnt)) ChkIO(10004);
     if (NOT Write4(PrgFile, &ExportCnt)) ChkIO(10004);
     if (NOT Write4(PrgFile, &StrLen)) ChkIO(10004);

     /* write patch entries */

     StrLen = 0;
     for (PatchLast = PatchList; PatchLast != Nil; PatchLast = PatchLast->Next)
      BEGIN
       if (NOT Write8(PrgFile, &(PatchLast->Address))) ChkIO(10004);
       if (NOT Write4(PrgFile, &StrLen)) ChkIO(10004);
       if (NOT Write4(PrgFile, &(PatchLast->RelocType))) ChkIO(10004);
       StrLen += PatchLast->len;
      END

     /* write export entries */

     for (ExportLast = ExportList; ExportLast != Nil; ExportLast = ExportLast->Next)
      BEGIN
       if (NOT Write4(PrgFile, &StrLen)) ChkIO(10004);
       if (NOT Write4(PrgFile, &(ExportLast->Flags))) ChkIO(10004);
       if (NOT Write8(PrgFile, &(ExportLast->Value))) ChkIO(10004);
       StrLen += ExportLast->len;
      END

     /* write string table, free structures */

     while (PatchList != Nil)
      BEGIN
       PatchLast = PatchList;
       if (fwrite(PatchLast->Ref, 1, PatchLast->len, PrgFile) != PatchLast->len) ChkIO(10004);
       free(PatchLast->Ref);
       PatchList = PatchLast->Next;
       free(PatchLast);
      END
     PatchLast = Nil;

     while (ExportList != Nil)
      BEGIN
       ExportLast = ExportList;
       if (fwrite(ExportLast->Name, 1, ExportLast->len, PrgFile) != ExportLast->len) ChkIO(10004);
       free(ExportLast->Name);
       ExportList = ExportLast->Next;
       free(ExportLast);
      END
     ExportLast = Nil;
    END
END

/*--- neuen Record in Codedatei anlegen.  War der bisherige leer, so wird ---
 ---- dieser ueberschrieben. ------------------------------------------------*/

	static void WrRecHeader(void)
BEGIN
   Byte b;

   /* assume simple record without relocation info */

   b = (ThisRel = RelSegs) ? FileHeaderRelocRec : FileHeaderDataRec;
   if (fwrite(&b, 1, 1, PrgFile) != 1) ChkIO(10004);
   if (fwrite(&HeaderID, 1, 1, PrgFile) != 1) ChkIO(10004);
   b = ActPC; if (fwrite(&b, 1, 1, PrgFile) != 1) ChkIO(10004);
   b = Grans[ActPC]; if (fwrite(&b, 1, 1, PrgFile) != 1) ChkIO(10004);
   fflush(PrgFile);
END

       	void NewRecord(LargeWord NStart)
BEGIN
   LongInt h;
   LongWord PC;
   Byte Header;

   /* flush remaining code in buffer */

   FlushBuffer();

   /* zero length record which may be deleted ? */
   /* do not write out patches at this place - they
      will be merged with the next record. */

   if (LenSoFar == 0)
    BEGIN
     if (fseek(PrgFile, RecPos, SEEK_SET) != 0) ChkIO(10003);
     WrRecHeader();
     h = NStart;
     if (NOT Write4(PrgFile, &h)) ChkIO(10004);
     LenPos = ftell(PrgFile);
     if (NOT Write2(PrgFile, &LenSoFar)) ChkIO(10004);
    END

   /* otherwise full record */

   else
    BEGIN
     /* store current position (=end of file) */

     h = ftell(PrgFile);

     /* do we have reloc. info? - then change record type */

     if ((PatchList != Nil) OR (ExportList != Nil))
      BEGIN
       fflush(PrgFile);
       if (fseek(PrgFile, RecPos, SEEK_SET) != 0) ChkIO(10003);
       Header = ThisRel ? FileHeaderRRelocRec : FileHeaderRDataRec;
       if (fwrite(&Header, 1, 1, PrgFile) != 1) ChkIO(10004);
      END

     /* fill in length of record */

     fflush(PrgFile);
     if (fseek(PrgFile, LenPos, SEEK_SET) != 0) ChkIO(10003);
     if (NOT Write2(PrgFile, &LenSoFar)) ChkIO(10004);

     /* go back to end of file */

     if (fseek(PrgFile, h, SEEK_SET) != 0) ChkIO(10003);

     /* write out reloc info */

     WrPatches();

     /* store begin of new code record */

     RecPos = ftell(PrgFile);

     LenSoFar = 0;
     WrRecHeader();
     ThisRel = RelSegs;
     PC = NStart;
     if (NOT Write4(PrgFile, &PC)) ChkIO(10004);
     LenPos = ftell(PrgFile);
     if (NOT Write2(PrgFile, &LenSoFar)) ChkIO(10004);
    END
#if 0
   /* put in the hidden symbol for the relocatable segment ? */

   if ((RelSegs) && (strcmp(CurrFileName, "INTERNAL") != 0))
    BEGIN
     sprintf(SectSymbolName, "__%s_%d", NamePart(CurrFileName), (int)(SectSymbolCounter++));
     AddExport(SectSymbolName, ProgCounter());
    END
#endif
END

/*--- Codedatei eroeffnen --------------------------------------------------*/

        void OpenFile(void)
BEGIN
   Word h;

   errno=0;
   PrgFile=fopen(OutName,OPENWRMODE);
   if (PrgFile==Nil) ChkIO(10001);

   errno=0; h=FileMagic;
   if (NOT Write2(PrgFile,&h)) ChkIO(10004);

   CodeBufferFill=0;
   RecPos=ftell(PrgFile); LenSoFar=0;
   NewRecord(PCs[ActPC]);
END

/*---- Codedatei schliessen -------------------------------------------------*/

        void CloseFile(void)
BEGIN
   Byte Head;
   String h;
   LongWord Adr;

   sprintf(h,"AS %s/%s-%s",Version,ARCHPRNAME,ARCHSYSNAME);

   NewRecord(PCs[ActPC]);
   fseek(PrgFile,RecPos,SEEK_SET);

   if (StartAdrPresent)
    BEGIN
     Head=FileHeaderStartAdr;
     if (fwrite(&Head,sizeof(Head),1,PrgFile)!=1) ChkIO(10004);
     Adr=StartAdr;
     if (NOT Write4(PrgFile,&Adr)) ChkIO(10004);
    END

   Head=FileHeaderEnd;
   if (fwrite(&Head,sizeof(Head),1,PrgFile)!=1) ChkIO(10004);
   if (fwrite(h,1,strlen(h),PrgFile)!=strlen(h)) ChkIO(10004);
   fclose(PrgFile); if (Magic!=0) unlink(OutName); 
END

/*--- erzeugten Code einer Zeile in Datei ablegen ---------------------------*/

	void WriteBytes(void)
BEGIN
   Word ErgLen;

   if (CodeLen == 0) return;
   ErgLen = CodeLen * Granularity();
   if ((TurnWords != 0) != (BigEndian != 0)) DreheCodes();
   if (((LongInt)LenSoFar) + ((LongInt)ErgLen) > 0xffff) NewRecord(PCs[ActPC]);
   if (CodeBufferFill + ErgLen < CodeBufferSize)
    BEGIN
     memcpy(CodeBuffer + CodeBufferFill, BAsmCode, ErgLen);
     CodeBufferFill += ErgLen;
    END
   else
    BEGIN
     FlushBuffer();
     if (ErgLen < CodeBufferSize)
      BEGIN
       memcpy(CodeBuffer, BAsmCode, ErgLen); CodeBufferFill = ErgLen;
      END
     else if (fwrite(BAsmCode, 1, ErgLen, PrgFile) != ErgLen) ChkIO(10004);
    END
   LenSoFar += ErgLen;
   if ((TurnWords != 0) != (BigEndian != 0)) DreheCodes();
END

        void RetractWords(Word Cnt)
BEGIN
   Word ErgLen;

   ErgLen=Cnt*Granularity();
   if (LenSoFar<ErgLen)
    BEGIN
     WrError(1950); return;
    END

   if (MakeUseList) DeleteChunk(SegChunks+ActPC,ProgCounter()-Cnt,Cnt);

   PCs[ActPC]-=Cnt;

   if (CodeBufferFill>=ErgLen) CodeBufferFill-=ErgLen;
   else
    BEGIN
     if (fseek(PrgFile,-(ErgLen-CodeBufferFill),SEEK_CUR)==-1)
      ChkIO(10004);
     CodeBufferFill=0;
    END

   LenSoFar-=ErgLen;

   Retracted=True;
END

	void asmcode_init(void)
BEGIN
   PatchList = PatchLast = Nil;
   ExportList = ExportLast = Nil;
   CodeBuffer = (Byte*) malloc(sizeof(Byte) * (CodeBufferSize + 1));
END
