/* toolutils.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Unterroutinen fuer die AS-Tools                                           */
/*                                                                           */
/* Historie: 31. 5.1996 Grundsteinlegung                                     */
/*           27.10.1997 Routinen aus P2... heruebergenommen                  */
/*           27. 3.1999 Granularitaet SC144xx                                */
/*           30. 5.1999 Adresswildcard-Funktion                              */
/*           22. 1.2000 Funktion zum Lesen von RelocInfos                    */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*           26. 6.2000 added reading of export entries                      */
/*            2. 7.2000 updated copyright year                               */
/*            4. 7.2000 ReadRecordHeader transports record type              */
/*           14. 1.2001 silenced warnings about unused parameters            */
/*           30. 9.2001 added workaround for CygWin file pointer bug         */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include "endian.h"
#include <string.h>

#include "strutil.h"
#include "stringlists.h"
#include "cmdarg.h"
#include "stdhandl.h"
#include "ioerrs.h"

#include "nls.h"
#include "nlmessages.h"
#include "tools.rsc"

#include "toolutils.h"

#include "version.h"

/****************************************************************************/

static Boolean DoFilter;
static int FilterCnt;
static Byte FilterBytes[100];

Word FileID = 0x1489;       /* Dateiheader Eingabedateien */
char *OutName = "STDOUT";   /* Pseudoname Output */

static TMsgCat MsgCat;

/****************************************************************************/

void WrCopyRight(const char *Msg)
{
  printf("%s\n%s\n", Msg, InfoMessCopyright);
}

void DelSuffix(char *Name)
{
  char *p,*z,*Part;

  p = NULL;
  for (z = Name; *z != '\0'; z++)
    if (*z == '\\')
      p = z;
  Part = (p != NULL) ? p : Name;
  Part = strchr(Part, '.');
  if (Part != NULL)
    *Part = '\0';
}

void AddSuffix(char *s, char *Suff)
{
  char *p, *z, *Part;

  p = NULL;
  for (z = s; *z != '\0'; z++)
    if (*z == '\\')
      p = z;
  Part = (p != NULL) ? p : s;
  if (strchr(Part, '.') == NULL)
    strmaxcat(s, Suff, 255);
}

void FormatError(const char *Name, const char *Detail)
{
  fprintf(stderr, "%s%s%s (%s)\n",
          catgetmessage(&MsgCat, Num_FormatErr1aMsg),
          Name,
          catgetmessage(&MsgCat, Num_FormatErr1bMsg),
          Detail);
  fprintf(stderr, "%s\n",
          catgetmessage(&MsgCat, Num_FormatErr2Msg));
  exit(3);
}

void ChkIO(const char *Name)
{
  int io;

  io = errno;

  if (io == 0)
    return;

  fprintf(stderr, "%s%s%s\n",
          catgetmessage(&MsgCat, Num_IOErrAHeaderMsg),
          Name,
          catgetmessage(&MsgCat, Num_IOErrBHeaderMsg));

  fprintf(stderr, "%s.\n", GetErrorMsg(io));

  fprintf(stderr, "%s\n",
          catgetmessage(&MsgCat, Num_ErrMsgTerminating));

  exit(2);
}

Word Granularity(Byte Header)
{
  switch (Header)
  {
    case 0x09:
    case 0x76:
    case 0x7d:
      return 4;
    case 0x70:
    case 0x71:
    case 0x72:
    case 0x74:
    case 0x75:
    case 0x77:
    case 0x12:
    case 0x3b:
    case 0x6d:
      return 2;
    default:
      return 1;
  }
}

void ReadRecordHeader(Byte *Header, Byte *CPU, Byte* Segment,
                      Byte *Gran, const char *Name, FILE *f)
{
#ifdef _WIN32
   /* CygWin B20 seems to mix up the file pointer under certain 
      conditions. Difficult to reproduce, so we reposition it. */

  long pos;

  pos = ftell(f);
  fflush(f);
  rewind(f);
  fseek(f, pos, SEEK_SET);
#endif

  if (fread(Header, 1, 1, f) != 1)
    ChkIO(Name);
  if ((*Header != FileHeaderEnd) && (*Header != FileHeaderStartAdr))
  {
    if ((*Header == FileHeaderDataRec) || (*Header == FileHeaderRDataRec) ||
        (*Header == FileHeaderRelocRec) || (*Header == FileHeaderRRelocRec))
    {
      if (fread(CPU, 1, 1, f) != 1)
        ChkIO(Name);
      if (fread(Segment, 1, 1, f) != 1)
        ChkIO(Name);
      if (fread(Gran, 1, 1, f) != 1)
        ChkIO(Name);
    }
    else if (*Header <= 0x7f)
    {
      *CPU = *Header;
      *Header = FileHeaderDataRec;
      *Segment = SegCode;
      *Gran = Granularity(*CPU);
    }
  }
}

void WriteRecordHeader(Byte *Header, Byte *CPU, Byte *Segment,
                       Byte *Gran, const char *Name, FILE *f)
{
  if ((*Header == FileHeaderEnd) || (*Header == FileHeaderStartAdr))
  {
    if (fwrite(Header, 1, 1, f) != 1)
      ChkIO(Name);
  }
  else if ((*Header == FileHeaderDataRec) || (*Header == FileHeaderRDataRec))
  {
    if ((*Segment != SegCode) || (*Gran != Granularity(*CPU)) || (*CPU >= 0x80))
    {
      if (fwrite(Header, 1, 1, f))
        ChkIO(Name);
      if (fwrite(CPU, 1, 1, f))
        ChkIO(Name);
      if (fwrite(Segment, 1, 1, f))
        ChkIO(Name);
      if (fwrite(Gran, 1, 1, f))
        ChkIO(Name);
    }
    else
    {
      if (fwrite(CPU, 1, 1, f))
        ChkIO(Name);
    }
  }
  else
  {
    if (fwrite(CPU, 1, 1, f))
      ChkIO(Name);
  }
}

void SkipRecord(Byte Header, const char *Name, FILE *f)
{
  int Length;
  LongWord Addr, RelocCount, ExportCount, StringLen;
  Word Len;

  switch (Header)
  {
    case FileHeaderStartAdr:
      Length = 4;
      break;
    case FileHeaderEnd:
      Length = 0;
      break;
    case FileHeaderRelocInfo:
      if (!Read4(f, &RelocCount))
        ChkIO(Name);
      if (!Read4(f, &ExportCount))
        ChkIO(Name);
      if (!Read4(f, &StringLen))
        ChkIO(Name);
      Length = (16 * RelocCount) + (16 * ExportCount) + StringLen;
      break;
    default:
      if (!Read4(f, &Addr))
        ChkIO(Name);
      if (!Read2(f, &Len))
        ChkIO(Name);
      Length = Len;
      break;
  }

  if (fseek(f, Length, SEEK_CUR) != 0) ChkIO(Name);
}

PRelocInfo ReadRelocInfo(FILE *f)
{
  PRelocInfo PInfo;
  PRelocEntry PEntry;
  PExportEntry PExp;
  Boolean OK = FALSE;
  LongWord StringLen, StringPos;
  LongInt z;
  
  /* get memory for structure */

  PInfo = (PRelocInfo) malloc(sizeof(TRelocInfo));
  if (PInfo != NULL)
  {
    PInfo->RelocEntries = NULL;
    PInfo->ExportEntries = NULL;
    PInfo->Strings = NULL;

    /* read global numbers */

    if ((Read4(f, &PInfo->RelocCount))
     && (Read4(f, &PInfo->ExportCount))
     && (Read4(f, &StringLen)))
    {
      /* allocate memory */

      PInfo->RelocEntries = (PRelocEntry) malloc(sizeof(TRelocEntry) * PInfo->RelocCount);
      if ((PInfo->RelocCount == 0) || (PInfo->RelocEntries != NULL))
      {
        PInfo->ExportEntries = (PExportEntry) malloc(sizeof(TExportEntry) * PInfo->ExportCount);
        if ((PInfo->ExportCount == 0) || (PInfo->ExportEntries != NULL))
        {
          PInfo->Strings = (char*) malloc(sizeof(char) * StringLen);
          if ((StringLen == 0) || (PInfo->Strings != NULL))
          {
            /* read relocation entries */

            for (z = 0, PEntry = PInfo->RelocEntries; z < PInfo->RelocCount; z++, PEntry++)
            {
              if (!Read8(f, &PEntry->Addr))
                break;
              if (!Read4(f, &StringPos))
                break;
              PEntry->Name = PInfo->Strings + StringPos;
              if (!Read4(f, &PEntry->Type))
                break;
            }

            /* read export entries */

            for (z = 0, PExp = PInfo->ExportEntries; z < PInfo->ExportCount; z++, PExp++)
            {
              if (!Read4(f, &StringPos))
                break;
              PExp->Name = PInfo->Strings + StringPos;
              if (!Read4(f, &PExp->Flags))
                break;
              if (!Read8(f, &PExp->Value))
                break;
            }

            /* read strings */

            if (z == PInfo->ExportCount)
              OK = ((fread(PInfo->Strings, 1, StringLen, f)) == StringLen);
          }
        }
      }
    }
  }

  if (!OK)
  {
    if (PInfo != NULL)
    {
      DestroyRelocInfo(PInfo);
      PInfo = NULL;
    }
  }

  return PInfo;
}

void DestroyRelocInfo(PRelocInfo PInfo)
{
  if (PInfo->Strings != NULL)
  {
    free(PInfo->Strings);
    PInfo->Strings = NULL;
  }
  if ((PInfo->ExportCount > 0) && (PInfo->RelocEntries != NULL))
  {
    free(PInfo->RelocEntries);
    PInfo->RelocEntries = NULL;
  }
  if ((PInfo->RelocCount > 0) && (PInfo->ExportEntries != NULL))
  {
    free(PInfo->ExportEntries);
    PInfo->ExportEntries = NULL;
  }
  free (PInfo);
}

CMDResult CMD_FilterList(Boolean Negate, const char *Arg)
{
  Byte FTemp;
  Boolean err;
  char *p;
  int Search;
  String Copy;

  if (*Arg == '\0')
    return CMDErr;
  strmaxcpy(Copy, Arg, 255);

  do
  {
    p = strchr(Copy,',');
    if (p != NULL)
      *p = '\0';
    FTemp = ConstLongInt(Copy, &err, 10);
    if (!err)
      return CMDErr;

    for (Search = 0; Search < FilterCnt; Search++)
      if (FilterBytes[Search] == FTemp)
        break;

    if ((Negate) && (Search < FilterCnt))
      FilterBytes[Search] = FilterBytes[--FilterCnt];

    else if ((!Negate) && (Search >= FilterCnt))
      FilterBytes[FilterCnt++] = FTemp;

    if (p != NULL)
      strmov(Copy, p + 1);
  }
  while (p != NULL);

  DoFilter = (FilterCnt != 0);

  return CMDArg;
}

Boolean FilterOK(Byte Header)
{
  int z;

  if (DoFilter)
  {
    for (z = 0; z < FilterCnt; z++)
     if (Header == FilterBytes[z])
       return True;
    return False;
  }
  else
    return True;
}

Boolean RemoveOffset(char *Name, LongWord *Offset)
{
  int z, Nest;
  Boolean err;

  *Offset = 0;
  if ((*Name) && (Name[strlen(Name)-1] == ')'))
  {
    z = strlen(Name) - 2;
    Nest = 0;
    while ((z >= 0) && (Nest >= 0))
    {
      switch (Name[z])
      {
        case '(': Nest--; break;
        case ')': Nest++; break;
      }
      if (Nest != -1)
        z--;
    }
    if (Nest != -1)
      return False;
    else
    {
      Name[strlen(Name) - 1] = '\0';
      *Offset = ConstLongInt(Name + z + 1, &err, 10);
      Name[z] = '\0';
      return err;
    }
  }
  else
    return True;
}

void EraseFile(const char *FileName, LongWord Offset)
{
  UNUSED(Offset);
   
  if (unlink(FileName) == -1)
    ChkIO(FileName);
}

void toolutils_init(const char *ProgPath)
{
  version_init();

  opencatalog(&MsgCat, "tools.msg", ProgPath, MsgId1, MsgId2);

  FilterCnt = 0;
  DoFilter = False;
}

Boolean AddressWildcard(const char *addr)
{
  return ((strcmp(addr, "$") == 0) || (strcasecmp(addr, "0x") == 0));
}

#ifdef CKMALLOC
#undef malloc
#undef realloc

void *ckmalloc(size_t s)
{
  void *tmp = malloc(s);
  if (tmp == NULL) 
  {
    fprintf(stderr,"allocation error(malloc): out of memory");
    exit(255);
  }
  return tmp;
}

void *ckrealloc(void *p, size_t s)
{
  void *tmp = realloc(p,s);
  if (tmp == NULL)
  {
    fprintf(stderr,"allocation error(realloc): out of memory");
    exit(255);
  }
  return tmp;
}
#endif

void WrError(Word Num)
{
  UNUSED(Num);
}
