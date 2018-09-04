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
#include "errmsg.h"
#include "asmsub.h"
#include "asmpars.h"
#include "intconsts.h"
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
{
  if (CodeBufferFill > 0)
  {
    if (fwrite(CodeBuffer, 1, CodeBufferFill, PrgFile) != CodeBufferFill)
      ChkIO(10004);
    CodeBufferFill = 0;
  }
}

void DreheCodes(void)
{
  int z;
  LongInt l = CodeLen * Granularity();

  switch (ActListGran)
  {
    case 2:
      for (z = 0; z < l >> 1; z++)
        WAsmCode[z] = ((WAsmCode[z] & 0xff) << 8) + ((WAsmCode[z] & 0xff00) >> 8);
      break;
    case 4:
      for (z = 0; z < l >> 2; z++)
      {
        DAsmCode[z] = ((DAsmCode[z] & INTCONST_ff000000) >> 24)
                    + ((DAsmCode[z] & INTCONST_00ff0000) >> 8)
                    + ((DAsmCode[z] & INTCONST_0000ff00) << 8)
		    + ((DAsmCode[z] & INTCONST_000000ff) << 24);
      }
      break;
  }
}

static void WrPatches(void)
{
  LongWord Cnt, ExportCnt, StrLen;
  Byte T8;

  if (PatchList || ExportList)
  {
    /* find out length of string field */

    Cnt = StrLen = 0;
    for (PatchLast = PatchList; PatchLast; PatchLast = PatchLast->Next)
    {
      Cnt++;
      StrLen += (PatchLast->len = strlen(PatchLast->Ref) + 1);
    }
    ExportCnt = 0;
    for (ExportLast = ExportList; ExportLast; ExportLast = ExportLast->Next)
    {
      ExportCnt++;
      StrLen += (ExportLast->len = strlen(ExportLast->Name) + 1);
    }

    /* write header */

    T8 = FileHeaderRelocInfo;
    if (fwrite(&T8, 1, 1, PrgFile) != 1) ChkIO(10004);
    if (!Write4(PrgFile, &Cnt)) ChkIO(10004);
    if (!Write4(PrgFile, &ExportCnt)) ChkIO(10004);
    if (!Write4(PrgFile, &StrLen)) ChkIO(10004);

    /* write patch entries */

    StrLen = 0;
    for (PatchLast = PatchList; PatchLast; PatchLast = PatchLast->Next)
    {
      if (!Write8(PrgFile, &(PatchLast->Address))) ChkIO(10004);
      if (!Write4(PrgFile, &StrLen)) ChkIO(10004);
      if (!Write4(PrgFile, &(PatchLast->RelocType))) ChkIO(10004);
      StrLen += PatchLast->len;
    }

    /* write export entries */

    for (ExportLast = ExportList; ExportLast; ExportLast = ExportLast->Next)
    {
      if (!Write4(PrgFile, &StrLen)) ChkIO(10004);
      if (!Write4(PrgFile, &(ExportLast->Flags))) ChkIO(10004);
      if (!Write8(PrgFile, &(ExportLast->Value))) ChkIO(10004);
      StrLen += ExportLast->len;
    }

    /* write string table, free structures */

    while (PatchList)
    {
      PatchLast = PatchList;
      if (fwrite(PatchLast->Ref, 1, PatchLast->len, PrgFile) != PatchLast->len) ChkIO(10004);
      free(PatchLast->Ref);
      PatchList = PatchLast->Next;
      free(PatchLast);
    }
    PatchLast = NULL;

    while (ExportList)
    {
      ExportLast = ExportList;
      if (fwrite(ExportLast->Name, 1, ExportLast->len, PrgFile) != ExportLast->len) ChkIO(10004);
      free(ExportLast->Name);
      ExportList = ExportLast->Next;
      free(ExportLast);
    }
    ExportLast = NULL;
  }
}

/*--- neuen Record in Codedatei anlegen.  War der bisherige leer, so wird ---
 ---- dieser ueberschrieben. ------------------------------------------------*/

static void WrRecHeader(void)
{
  Byte b;

  /* assume simple record without relocation info */

  ThisRel = RelSegs;
  b = ThisRel ? FileHeaderRelocRec : FileHeaderDataRec;
  if (fwrite(&b, 1, 1, PrgFile) != 1) ChkIO(10004);
  if (fwrite(&HeaderID, 1, 1, PrgFile) != 1) ChkIO(10004);
  b = ActPC; if (fwrite(&b, 1, 1, PrgFile) != 1) ChkIO(10004);
  b = Grans[ActPC]; if (fwrite(&b, 1, 1, PrgFile) != 1) ChkIO(10004);
  fflush(PrgFile);
}

void NewRecord(LargeWord NStart)
{
  LongInt h;
  LongWord PC;
  Byte Header;

  /* flush remaining code in buffer */

  FlushBuffer();

  /* zero length record which may be deleted ? */
  /* do not write out patches at this place - they
     will be merged with the next record. */

  if (LenSoFar == 0)
  {
    if (fseek(PrgFile, RecPos, SEEK_SET) != 0) ChkIO(10003);
    WrRecHeader();
    h = NStart;
    if (!Write4(PrgFile, &h)) ChkIO(10004);
    LenPos = ftell(PrgFile);
    if (!Write2(PrgFile, &LenSoFar)) ChkIO(10004);
  }

  /* otherwise full record */

  else
  {
    /* store current position (=end of file) */

    h = ftell(PrgFile);

    /* do we have reloc. info? - then change record type */

    if (PatchList || ExportList)
    {
      fflush(PrgFile);
      if (fseek(PrgFile, RecPos, SEEK_SET) != 0) ChkIO(10003);
      Header = ThisRel ? FileHeaderRRelocRec : FileHeaderRDataRec;
      if (fwrite(&Header, 1, 1, PrgFile) != 1) ChkIO(10004);
    }

    /* fill in length of record */

    fflush(PrgFile);
    if (fseek(PrgFile, LenPos, SEEK_SET) != 0) ChkIO(10003);
    if (!Write2(PrgFile, &LenSoFar)) ChkIO(10004);

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
    if (!Write4(PrgFile, &PC)) ChkIO(10004);
    LenPos = ftell(PrgFile);
    if (!Write2(PrgFile, &LenSoFar)) ChkIO(10004);
  }
#if 0
  /* put in the hidden symbol for the relocatable segment ? */

  if ((RelSegs) && (strcmp(CurrFileName, "INTERNAL")))
  {
    sprintf(SectSymbolName, "__%s_%d", NamePart(CurrFileName), (int)(SectSymbolCounter++));
    AddExport(SectSymbolName, ProgCounter());
  }
#endif
}

/*--- Codedatei eroeffnen --------------------------------------------------*/

void OpenFile(void)
{
  Word h;

  errno = 0;
  PrgFile = fopen(OutName, OPENWRMODE);
  if (!PrgFile) ChkIO(10001);

  errno = 0;
  h = FileMagic;
  if (!Write2(PrgFile,&h)) ChkIO(10004);

  CodeBufferFill = 0;
  RecPos = ftell(PrgFile);
  LenSoFar = 0;
  NewRecord(PCs[ActPC]);
}

/*---- Codedatei schliessen -------------------------------------------------*/

void CloseFile(void)
{
  Byte Head;
  String h;
  LongWord Adr;

  sprintf(h, "AS %s/%s-%s", Version, ARCHPRNAME, ARCHSYSNAME);

  NewRecord(PCs[ActPC]);
  fseek(PrgFile, RecPos, SEEK_SET);

  if (StartAdrPresent)
  {
    Head = FileHeaderStartAdr;
    if (fwrite(&Head,sizeof(Head), 1, PrgFile) != 1) ChkIO(10004);
    Adr = StartAdr;
    if (!Write4(PrgFile,&Adr)) ChkIO(10004);
  }

  Head = FileHeaderEnd;
  if (fwrite(&Head,sizeof(Head), 1, PrgFile) != 1) ChkIO(10004);
  if (fwrite(h, 1, strlen(h), PrgFile) != strlen(h)) ChkIO(10004);
  fclose(PrgFile);
  if (Magic)
    unlink(OutName);
}

/*--- erzeugten Code einer Zeile in Datei ablegen ---------------------------*/

void WriteBytes(void)
{
  Word ErgLen;

  if (CodeLen == 0)
    return;
  ErgLen = CodeLen * Granularity();
  if ((TurnWords != 0) != (BigEndian != 0))
    DreheCodes();
  if (((LongInt)LenSoFar) + ((LongInt)ErgLen) > 0xffff)
    NewRecord(PCs[ActPC]);
  if (CodeBufferFill + ErgLen < CodeBufferSize)
  {
    memcpy(CodeBuffer + CodeBufferFill, BAsmCode, ErgLen);
    CodeBufferFill += ErgLen;
  }
  else
  {
    FlushBuffer();
    if (ErgLen < CodeBufferSize)
    {
      memcpy(CodeBuffer, BAsmCode, ErgLen);
      CodeBufferFill = ErgLen;
    }
    else if (fwrite(BAsmCode, 1, ErgLen, PrgFile) != ErgLen)
      ChkIO(10004);
  }
  LenSoFar += ErgLen;
  if ((TurnWords != 0) != (BigEndian != 0))
    DreheCodes();
}

void RetractWords(Word Cnt)
{
  Word ErgLen;

  ErgLen = Cnt * Granularity();
  if (LenSoFar < ErgLen)
  {
    WrError(ErrNum_ParNotPossible);
    return;
  }

  if (MakeUseList)
    DeleteChunk(SegChunks + ActPC, ProgCounter() - Cnt, Cnt);

  PCs[ActPC] -= Cnt;

  if (CodeBufferFill >= ErgLen)
    CodeBufferFill -= ErgLen;
  else
  {
    if (fseek(PrgFile, -(ErgLen - CodeBufferFill), SEEK_CUR) == -1)
      ChkIO(10004);
    CodeBufferFill = 0;
  }

  LenSoFar -= ErgLen;

  Retracted = True;
}

void asmcode_init(void)
{
  PatchList = PatchLast = NULL;
  ExportList = ExportLast = NULL;
  CodeBuffer = (Byte*) malloc(sizeof(Byte) * (CodeBufferSize + 1));
}
