/* p2hex.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Konvertierung von AS-P-Dateien nach Hex                                   */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "version.h"
#include "endian.h"
#include "bpemu.h"
#include "hex.h"
#include "nls.h"
#include "nlmessages.h"
#include "p2hex.rsc"
#include "ioerrs.h"
#include "strutil.h"
#include "chunks.h"
#include "stringlists.h"
#include "cmdarg.h"
#include "intconsts.h"

#include "toolutils.h"
#include "headids.h"

static char *HexSuffix = ".hex";
#define MaxLineLen 254
#define AVRLEN_DEFAULT 3
#define DefaultCFormat "dSEl"

typedef void (*ProcessProc)(
#ifdef __PROTOS__
const char *FileName, LongWord Offset
#endif
);

static CMDProcessed ParUnprocessed;
static int z;
static FILE *TargFile;
static String SrcName, TargName, CFormat;

static LongWord StartAdr, StopAdr, LineLen;
static LongWord StartData, StopData, EntryAdr;
static LargeInt Relocate;
static Boolean StartAuto, StopAuto, AutoErase, EntryAdrPresent;
static Word Seg, Ofs;
static LongWord Dummy;
static Byte IntelMode;
static Byte MultiMode;   /* 0=8M, 1=16, 2=8L, 3=8H */
static Byte MinMoto;
static Boolean Rec5;
static Boolean SepMoto;
static LongWord AVRLen, ValidSegs;

static Boolean RelAdr;

static unsigned FormatOccured;
enum
{
  eMotoOccured = (1 << 0),
  eIntelOccured = (1 << 1),
  eMOSOccured = (1 << 2),
  eDSKOccured = (1 << 3),
  eMico8Occured = (1 << 4),
};
static Byte MaxMoto, MaxIntel;

static tHexFormat DestFormat;

static ChunkList UsedList;

static String CTargName;
static unsigned NumCBlocks;

static void Filename2CName(char *pDest, const char *pSrc)
{
  char Trans;
  const char *pPos;
  Boolean Written = False;

  pPos = strrchr(pSrc, PATHSEP);
#ifdef DRSEP
  if (!pPos)
    pPos = strchr(pSrc, DRSEP);
#endif
  if (pPos)
    pSrc = pPos + 1;

  for (; *pSrc; pSrc++)
  {
    Trans = (*pSrc == '-') ? '_' : *pSrc;
    if (isalnum(*pSrc) || (*pSrc == '_'))
    {
      *pDest++ = Trans;
      Written = True;
    }
    else if (Written)
      break;
  }
  *pDest = '\0';
}

static void GetCBlockName(char *pDest, unsigned Num)
{
  if (Num > 0)
    sprintf(pDest, "_%u", Num + 1);
  else
    *pDest = '\0';
}

static void ParamError(Boolean InEnv, char *Arg)
{
   printf("%s%s\n", getmessage(InEnv ? Num_ErrMsgInvEnvParam : Num_ErrMsgInvParam), Arg);
   printf("%s\n", getmessage(Num_ErrMsgProgTerm));
   exit(1);
}

static void OpenTarget(void)
{
  TargFile = fopen(TargName, "w");
  if (!TargFile)
    ChkIO(TargName);
}

static void CloseTarget(void)
{
  errno = 0;
  fclose(TargFile);
  ChkIO(TargName);
  if (Magic != 0)
    unlink(TargName);
}

static void PrCData(FILE *pTargFile, char Ident, const char *pName,
                    const char *pCTargName, const char *pCBlockName, LongWord Value)
{
  const char *pFormat;

  if (strchr(CFormat, toupper(Ident)))
    pFormat = "ul";
  else if (strchr(CFormat, tolower(Ident)))
    pFormat = "u";
  else
    return;

  errno = 0;
  fprintf(pTargFile, "#define %s%s_%s 0x%s%s\n",
          pCTargName, pCBlockName, pName,
          HexLong(Value), pFormat);
  ChkIO(TargName);
}

static void ProcessFile(const char *FileName, LongWord Offset)
{
  FILE *SrcFile;
  Word TestID;
  Byte InpHeader, InpCPU, InpSegment, InpGran;
  LongWord InpStart, SumLen;
  Word InpLen, TransLen;
  Boolean doit, FirstBank = 0;
  Boolean CDataLower = !!strchr(CFormat, 'd'),
          CDataUpper = !!strchr(CFormat, 'D');
  Byte Buffer[MaxLineLen];
  Word *WBuffer = (Word *) Buffer;
  LongWord ErgStart,
           ErgStop = INTCONST_ffffffff,
           IntOffset = 0, MaxAdr;
  LongInt NextPos;
  Word ErgLen = 0, ChkSum = 0, RecCnt, Gran, HSeg;
  String CBlockName;

  LongInt z;

  Byte MotRecType = 0;

  tHexFormat ActFormat;
  PFamilyDescr FoundDscr;

  SrcFile = fopen(FileName, OPENRDMODE);
  if (!SrcFile) ChkIO(FileName);

  if (!Read2(SrcFile, &TestID))
    ChkIO(FileName);
  if (TestID !=FileMagic)
    FormatError(FileName, getmessage(Num_FormatInvHeaderMsg));

  errno = 0; printf("%s==>>%s", FileName, TargName); ChkIO(OutName);

  SumLen = 0;

  do
  {
    ReadRecordHeader(&InpHeader, &InpCPU, &InpSegment, &InpGran, FileName, SrcFile);

    if (InpHeader == FileHeaderStartAdr)
    {
      if (!Read4(SrcFile, &ErgStart))
        ChkIO(FileName);
      if (!EntryAdrPresent)
      {
        EntryAdr = ErgStart;
        EntryAdrPresent = True;
      }
    }

    else if (InpHeader == FileHeaderDataRec)
    {
      Gran = InpGran;

      if ((ActFormat = DestFormat) == eHexFormatDefault)
      {
        FoundDscr = FindFamilyById(InpCPU);
        if (!FoundDscr)
          FormatError(FileName, getmessage(Num_FormatInvRecordHeaderMsg));
        else
          ActFormat = FoundDscr->HexFormat;
      }

      ValidSegs = (1 << SegCode);
      switch (ActFormat)
      {
        case eHexFormatMotoS:
        case eHexFormatIntel32:
        case eHexFormatC:
          MaxAdr = INTCONST_ffffffff;
          break;
        case eHexFormatIntel16:
          MaxAdr = 0xffff0 + 0xffff;
          break;
        case eHexFormatAtmel:
          MaxAdr = (1 << (AVRLen << 3)) - 1;
          break;
        case eHexFormatMico8:
          MaxAdr = INTCONST_ffffffff;
          break;
        case eHexFormatTiDSK:
          ValidSegs = (1 << SegCode) | (1 << SegData);
          /* no break!!! */
        default:
          MaxAdr = 0xffff;
      }

      if (!Read4(SrcFile, &InpStart))
        ChkIO(FileName);
      if (!Read2(SrcFile, &InpLen))
        ChkIO(FileName);

      NextPos = ftell(SrcFile) + InpLen;
      if (NextPos >= FileSize(SrcFile) - 1)
        FormatError(FileName, getmessage(Num_FormatInvRecordLenMsg));

      doit = (FilterOK(InpCPU)) && (ValidSegs & (1 << InpSegment));

      if (doit)
      {
        InpStart += Offset;
        ErgStart = max(StartAdr, InpStart);
        ErgStop = min(StopAdr, InpStart + (InpLen/Gran) - 1);
        doit = (ErgStop >= ErgStart);
        if (doit)
        {
          ErgLen = (ErgStop + 1 - ErgStart) * Gran;
          if (AddChunk(&UsedList, ErgStart, ErgStop - ErgStart + 1, True))
          {
            errno = 0; printf(" %s\n", getmessage(Num_ErrMsgOverlap)); ChkIO(OutName);
          }
        }
      }

      if (ErgStop > MaxAdr)
      {
        errno = 0; printf(" %s\n", getmessage(Num_ErrMsgAdrOverflow)); ChkIO(OutName);
      }

      if (doit)
      {
        /* an Anfang interessierender Daten */

        if (fseek(SrcFile, (ErgStart - InpStart) * Gran, SEEK_CUR) == -1)
          ChkIO(FileName);

        /* Statistik, Anzahl Datenzeilen ausrechnen */

        RecCnt = ErgLen / LineLen;
        if ((ErgLen % LineLen) !=0)
          RecCnt++;

        /* relative Angaben ? */

        if (RelAdr)
          ErgStart -= StartAdr;

        /* Auf Zieladressbereich verschieben */

        ErgStart += Relocate;

        /* Kopf einer Datenzeilengruppe */

        switch (ActFormat)
        {
          case eHexFormatMotoS:
            if ((!(FormatOccured & eMotoOccured)) || (SepMoto))
            {
              errno = 0; fprintf(TargFile, "S0030000FC\n"); ChkIO(TargName);
            }
            if ((ErgStop >> 24) != 0)
              MotRecType = 2;
            else if ((ErgStop >> 16) !=0)
              MotRecType = 1;
            else
              MotRecType = 0;
            if (MotRecType < (MinMoto - 1))
              MotRecType = (MinMoto - 1);
            if (MaxMoto < MotRecType)
              MaxMoto = MotRecType;
            if (Rec5)
            {
              ChkSum = Lo(RecCnt) + Hi(RecCnt) + 3;
              errno = 0;
              fprintf(TargFile, "S503%s%s\n", HexWord(RecCnt), HexByte(Lo(ChkSum ^ 0xff)));
              ChkIO(TargName);
            }
            FormatOccured |= eMotoOccured;
            break;
          case eHexFormatMOS:
            FormatOccured |= eMOSOccured;
            break;
          case eHexFormatIntel:
            FormatOccured |= eIntelOccured;
            IntOffset = 0;
            break;
          case eHexFormatIntel16:
            FormatOccured |= eIntelOccured;
            IntOffset = (ErgStart * Gran);
            IntOffset &= INTCONST_fffffff0;
            HSeg = IntOffset >> 4;
            ChkSum = 4 + Lo(HSeg) + Hi(HSeg);
            IntOffset /= Gran;
            errno = 0; fprintf(TargFile, ":02000002%s%s\n", HexWord(HSeg), HexByte(0x100 - ChkSum)); ChkIO(TargName);
            if (MaxIntel < 1)
              MaxIntel = 1;
            break;
          case eHexFormatIntel32:
            FormatOccured |= eIntelOccured;
            IntOffset = (ErgStart * Gran);
            IntOffset &= INTCONST_ffffff00;
            HSeg = IntOffset >> 16;
            ChkSum = 6 + Lo(HSeg) + Hi(HSeg);
            IntOffset /= Gran;
            errno = 0; fprintf(TargFile, ":02000004%s%s\n", HexWord(HSeg), HexByte(0x100 - ChkSum)); ChkIO(TargName);
            if (MaxIntel < 2)
              MaxIntel = 2;
            FirstBank = False;
            break;
          case eHexFormatTek:
            break;
          case eHexFormatAtmel:
            break;
          case eHexFormatMico8:
            break;
          case eHexFormatTiDSK:
            if (!(FormatOccured & eDSKOccured))
            {
              FormatOccured |= eDSKOccured;
              errno = 0; fprintf(TargFile, "%s%s\n", getmessage(Num_DSKHeaderLine), TargName); ChkIO(TargName);
            }
            break;
          case eHexFormatC:
            GetCBlockName(CBlockName, NumCBlocks);
            PrCData(TargFile, 's', "start", CTargName, CBlockName, ErgStart);
            PrCData(TargFile, 'l', "len", CTargName, CBlockName, ErgLen);
            PrCData(TargFile, 'e', "end", CTargName, CBlockName, ErgStart + ErgLen - 1);
            if (CDataLower || CDataUpper)
            {
              errno = 0;
              fprintf(TargFile, "static const unsigned char %s%s_data[] =\n{\n",
                      CTargName, CBlockName);
              ChkIO(TargName);
            }
            break;
          default:
            break;
        }

        /* Datenzeilen selber */

        while (ErgLen > 0)
        {
          /* evtl. Folgebank fuer Intel32 ausgeben */

          if ((ActFormat == eHexFormatIntel32) && (FirstBank))
          {
            IntOffset += (0x10000 / Gran);
            HSeg = IntOffset >> 16;
            ChkSum = 6 + Lo(HSeg) + Hi(HSeg);
            errno = 0;
            fprintf(TargFile, ":02000004%s%s\n", HexWord(HSeg), HexByte(0x100 - ChkSum));
            ChkIO(TargName);
            FirstBank = False;
          }

          /* Recordlaenge ausrechnen, fuer Intel32 auf 64K-Grenze begrenzen
             Bei Atmel nur 2 Byte pro Zeile!
             Bei Mico8 nur 4 Byte (davon ei Wort=18 Bit) pro Zeile! */

          TransLen = min(LineLen, ErgLen);
          if ((ActFormat == eHexFormatIntel32) && ((ErgStart & 0xffff) + (TransLen/Gran) >= 0x10000))
          {
            TransLen = Gran * (0x10000 - (ErgStart & 0xffff));
            FirstBank = True;
          }
          else if (ActFormat == eHexFormatAtmel)
            TransLen = min(2, TransLen);
          else if (ActFormat == eHexFormatMico8)
            TransLen = min(4, TransLen);

          /* Start der Datenzeile */

          switch (ActFormat)
          {
            case eHexFormatMotoS:
              errno = 0;
              fprintf(TargFile, "S%c%s", '1' + MotRecType, HexByte(TransLen + 3 + MotRecType));
              ChkIO(TargName);
              ChkSum = TransLen + 3 + MotRecType;
              if (MotRecType >= 2)
              {
                errno = 0; fprintf(TargFile, "%s", HexByte((ErgStart >> 24) & 0xff)); ChkIO(TargName);
                ChkSum += ((ErgStart >> 24) & 0xff);
              }
              if (MotRecType >= 1)
              {
                errno = 0; fprintf(TargFile, "%s", HexByte((ErgStart >> 16) & 0xff)); ChkIO(TargName);
                ChkSum += ((ErgStart >> 16) & 0xff);
              }
              errno = 0; fprintf(TargFile, "%s", HexWord(ErgStart & 0xffff)); ChkIO(TargName);
              ChkSum += Hi(ErgStart) + Lo(ErgStart);
              break;
            case eHexFormatMOS:
              errno = 0; fprintf(TargFile, ";%s%s", HexByte(TransLen), HexWord(ErgStart & 0xffff)); ChkIO(TargName);
              ChkSum += TransLen + Lo(ErgStart) + Hi(ErgStart);
              break;
            case eHexFormatIntel:
            case eHexFormatIntel16:
            case eHexFormatIntel32:
            {
              Word WrTransLen;
              LongWord WrErgStart;

              WrTransLen = (MultiMode < 2) ? TransLen : (TransLen / Gran);
              WrErgStart = (ErgStart - IntOffset) * ((MultiMode < 2) ? Gran : 1);
              errno = 0;
              fprintf(TargFile, ":%s%s00", HexByte(WrTransLen), HexWord(WrErgStart));
              ChkIO(TargName);
              ChkSum = Lo(WrTransLen) + Hi(WrErgStart) + Lo(WrErgStart);

              break;
            }
            case eHexFormatTek:
              errno = 0;
              fprintf(TargFile, "/%s%s%s", HexWord(ErgStart), HexByte(TransLen),
                                         HexByte(Lo(ErgStart) + Hi(ErgStart) + TransLen));
              ChkIO(TargName);
              ChkSum = 0;
              break;
            case eHexFormatTiDSK:
              errno = 0; fprintf(TargFile, "9%s", HexWord(/*Gran**/ErgStart));
              ChkIO(TargName);
              ChkSum = 0;
              break;
            case eHexFormatAtmel:
              for (z = (AVRLen - 1) << 3; z >= 0; z -= 8)
              {
                errno = 0;
                fputs(HexByte((ErgStart >> z) & 0xff), TargFile);
                ChkIO(TargName);
              }
              errno = 0;
              fputc(':', TargFile);
              ChkIO(TargName);
              break;
            case eHexFormatMico8:
              break;
            case eHexFormatC:
              errno = 0;
              fprintf(TargFile, "  ");
              ChkIO(TargName);
              break;
            default:
              break;
          }

          /* Daten selber */

          if (fread(Buffer, 1, TransLen, SrcFile) !=TransLen)
            ChkIO(FileName);
          if (MultiMode == 1)
            switch (Gran)
            {
              case 4:
                DSwap(Buffer, TransLen);
                break;
              case 2:
                WSwap(Buffer, TransLen);
                break;
              case 1:
                break;
            }
          switch (ActFormat)
          {
            case eHexFormatTiDSK:
              if (BigEndian)
                WSwap(WBuffer, TransLen);
              for (z = 0; z < (TransLen / 2); z++)
              {
                errno = 0;
                if (((ErgStart + z >= StartData) && (ErgStart + z <= StopData))
                 || (InpSegment == SegData))
                  fprintf(TargFile, "M%s", HexWord(WBuffer[z]));
                else
                  fprintf(TargFile, "B%s", HexWord(WBuffer[z]));
                ChkIO(TargName);
                ChkSum += WBuffer[z];
                SumLen += Gran;
              }
              break;
            case eHexFormatAtmel:
              if (TransLen >= 2)
              {
                fprintf(TargFile, "%s", HexWord(WBuffer[0]));
                SumLen += 2;
              }
              break;
            case eHexFormatMico8:
              if (TransLen >= 4)
              {
                fprintf(TargFile, "%s", HexNibble(Buffer[1] & 0x0f));
                fprintf(TargFile, "%s", HexByte(Buffer[2]));
                fprintf(TargFile, "%s", HexByte(Buffer[3]));
                SumLen += 4;
              }
              break;
            case eHexFormatC:
              if (CDataLower || CDataUpper)
                for (z = 0; z < (LongInt)TransLen; z++)
                  if ((MultiMode < 2) || (z % Gran == MultiMode - 2))
                  {
                    errno = 0;
                    fprintf(TargFile, CDataLower ? "0x%02x%s" : "0x%02X%s", (unsigned)Buffer[z],
                            (ErgLen - z > 1) ? "," : "");
                    ChkIO(TargName);
                    ChkSum += Buffer[z];
                    SumLen++;
                  }
              break;
            default:
              for (z = 0; z < (LongInt)TransLen; z++)
                if ((MultiMode < 2) || (z % Gran == MultiMode - 2))
                {
                  errno = 0; fprintf(TargFile, "%s", HexByte(Buffer[z])); ChkIO(TargName);
                  ChkSum += Buffer[z];
                  SumLen++;
                }
          }

          /* Ende Datenzeile */

          switch (ActFormat)
          {
            case eHexFormatMotoS:
              errno = 0;
              fprintf(TargFile, "%s\n", HexByte(Lo(ChkSum ^ 0xff)));
              ChkIO(TargName);
              break;
            case eHexFormatMOS:
              errno = 0;
              fprintf(TargFile, "%s\n", HexWord(ChkSum));
              break;
            case eHexFormatIntel:
            case eHexFormatIntel16:
            case eHexFormatIntel32:
              errno = 0;
              fprintf(TargFile, "%s\n", HexByte(Lo(1 + (ChkSum ^ 0xff))));
              ChkIO(TargName);
              break;
            case eHexFormatTek:
              errno = 0;
              fprintf(TargFile, "%s\n", HexByte(Lo(ChkSum)));
              ChkIO(TargName);
              break;
            case eHexFormatTiDSK:
              errno = 0;
              fprintf(TargFile, "7%sF\n", HexWord(ChkSum));
              ChkIO(TargName);
              break;
            case eHexFormatAtmel:
            case eHexFormatMico8:
            case eHexFormatC:
              errno = 0;
              fprintf(TargFile, "\n");
              ChkIO(TargName);
              break;
            default:
              break;
          }

          /* Zaehler rauf */

          ErgLen -= TransLen;
          ErgStart += TransLen/Gran;
        }

        /* Ende der Datenzeilengruppe */

        switch (ActFormat)
        {
          case eHexFormatMotoS:
            if (SepMoto)
            {
              errno = 0;
              fprintf(TargFile, "S%c%s", '9' - MotRecType, HexByte(3 + MotRecType));
              ChkIO(TargName);
              for (z = 1; z <= 2 + MotRecType; z++)
              {
                errno = 0; fprintf(TargFile, "%s", HexByte(0)); ChkIO(TargName);
              }
              errno = 0;
              fprintf(TargFile, "%s\n", HexByte(0xff - 3 - MotRecType));
              ChkIO(TargName);
            }
            break;
          case eHexFormatMOS:
            break;
          case eHexFormatIntel:
          case eHexFormatIntel16:
          case eHexFormatIntel32:
            break;
          case eHexFormatTek:
            break;
          case eHexFormatTiDSK:
            break;
          case eHexFormatAtmel:
            break;
          case eHexFormatMico8:
            break;
          case eHexFormatC:
            if (CDataLower || CDataUpper)
            {
              errno = 0;
              fprintf(TargFile, "};\n\n");
              NumCBlocks++;
              ChkIO(TargName);
            }
            break;
          default:
            break;
        };
      }
      if (fseek(SrcFile, NextPos, SEEK_SET) == -1)
        ChkIO(FileName);
    }
    else
      SkipRecord(InpHeader, FileName, SrcFile);
  }
  while (InpHeader !=0);

  errno = 0; printf("  ("); ChkIO(OutName);
  errno = 0; printf(Integ32Format, SumLen); ChkIO(OutName);
  errno = 0; printf(" %s)\n", getmessage((SumLen == 1) ? Num_Byte : Num_Bytes)); ChkIO(OutName);
  if (!SumLen)
  {
    errno = 0; fputs(getmessage(Num_WarnEmptyFile), stdout); ChkIO(OutName);
  }

  errno = 0;
  fclose(SrcFile);
  ChkIO(FileName);
}

static ProcessProc CurrProcessor;
static LongWord CurrOffset;

static void Callback(char *Name)
{
  CurrProcessor(Name, CurrOffset);
}

static void ProcessGroup(char *GroupName_O, ProcessProc Processor)
{
  String Ext, GroupName;

  CurrProcessor = Processor;
  strmaxcpy(GroupName, GroupName_O, STRINGSIZE);
  strmaxcpy(Ext, GroupName, STRINGSIZE);
  if (!RemoveOffset(GroupName, &CurrOffset))
    ParamError(False, Ext);
  AddSuffix(GroupName, STRINGSIZE, getmessage(Num_Suffix));

  if (!DirScan(GroupName, Callback))
    fprintf(stderr, "%s%s%s\n", getmessage(Num_ErrMsgNullMaskA), GroupName, getmessage(Num_ErrMsgNullMaskB));
}

static void MeasureFile(const char *FileName, LongWord Offset)
{
  FILE *f;
  Byte Header, CPU, Segment, Gran;
  Word Length, TestID;
  LongWord Adr, EndAdr;
  LongInt NextPos;

  f = fopen(FileName, OPENRDMODE);
  if (!f)
    ChkIO(FileName);

  if (!Read2(f, &TestID))
    ChkIO(FileName);
  if (TestID !=FileMagic)
    FormatError(FileName, getmessage(Num_FormatInvHeaderMsg));

  do
  {
    ReadRecordHeader(&Header, &CPU, &Segment, &Gran, FileName, f);

    if (Header == FileHeaderDataRec)
    {
      if (!Read4(f, &Adr))
        ChkIO(FileName);
      if (!Read2(f, &Length))
        ChkIO(FileName);
      NextPos = ftell(f) + Length;
      if (NextPos > FileSize(f))
        FormatError(FileName, getmessage(Num_FormatInvRecordLenMsg));

      if (FilterOK(Header))
      {
        Adr += Offset;
        EndAdr = Adr + (Length/Gran) - 1;
        if (StartAuto)
          if (StartAdr > Adr)
            StartAdr = Adr;
        if (StopAuto)
          if (EndAdr > StopAdr)
            StopAdr = EndAdr;
      }

      fseek(f, NextPos, SEEK_SET);
    }
    else
     SkipRecord(Header, FileName, f);
  }
  while(Header !=0);

  fclose(f);
}

static CMDResult CMD_AdrRange(Boolean Negate, const char *Arg)
{
  char *p, Save;
  Boolean ok;

  if (Negate)
  {
    StartAdr = 0;
    StopAdr = 0x7fff;
    return CMDOK;
  }
  else
  {
    p = strchr(Arg, '-');
    if (!p) return CMDErr;

    Save = (*p); *p = '\0';
    StartAuto = AddressWildcard(Arg);
    if (StartAuto)
      ok = True;
    else
      StartAdr = ConstLongInt(Arg, &ok, 10);
    *p = Save;
    if (!ok)
      return CMDErr;

    StopAuto = AddressWildcard(p + 1);
    if (StopAuto)
      ok = True;
    else
      StopAdr = ConstLongInt(p + 1, &ok, 10);
    if (!ok)
      return CMDErr;

    if ((!StartAuto) && (!StopAuto) && (StartAdr > StopAdr))
      return CMDErr;

    return CMDArg;
  }
}

static CMDResult CMD_RelAdr(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  RelAdr = (!Negate);
  return CMDOK;
}

static CMDResult CMD_AdrRelocate(Boolean Negate, const char *Arg)
{
  Boolean ok;
  UNUSED(Arg);

  if (Negate)
  {
    Relocate = 0;
    return CMDOK;
  }
  else
  {
    Relocate = ConstLongInt(Arg, &ok, 10);
    if (!ok) return CMDErr;

    return CMDArg;
  }
}

static CMDResult CMD_Rec5(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  Rec5 = (!Negate);
  return CMDOK;
}

static CMDResult CMD_SepMoto(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  SepMoto = !Negate;
  return CMDOK;
}

static CMDResult CMD_IntelMode(Boolean Negate, const char *Arg)
{
  int Mode;
  Boolean ok;

  if (*Arg == '\0')
    return CMDErr;
  else
  {
    Mode = ConstLongInt(Arg, &ok, 10);
    if ((!ok) || (Mode < 0) || (Mode > 2))
      return CMDErr;
    else
    {
      if (!Negate)
        IntelMode = Mode;
      else if (IntelMode == Mode)
        IntelMode = 0;
      return CMDArg;
    }
  }
}

static CMDResult CMD_MultiMode(Boolean Negate, const char *Arg)
{
  int Mode;
  Boolean ok;

  if (*Arg == '\0')
    return CMDErr;
  else
  {
    Mode = ConstLongInt(Arg, &ok, 10);
    if ((!ok) || (Mode < 0) || (Mode > 3))
      return CMDErr;
    else
    {
      if (!Negate)
        MultiMode = Mode;
      else if (MultiMode == Mode)
        MultiMode = 0;
      return CMDArg;
    }
  }
}

static CMDResult CMD_DestFormat(Boolean Negate, const char *pArg)
{
#define NameCnt (sizeof(Names) / sizeof(*Names))

  static char *Names[] =
  {
    "DEFAULT", "MOTO", "INTEL", "INTEL16", "INTEL32", "MOS", "TEK", "DSK", "ATMEL", "MICO8", "C"
  };
  static tHexFormat Format[] =
  {
    eHexFormatDefault, eHexFormatMotoS, eHexFormatIntel, eHexFormatIntel16,
    eHexFormatIntel32, eHexFormatMOS, eHexFormatTek, eHexFormatTiDSK,
    eHexFormatAtmel, eHexFormatMico8, eHexFormatC
  };
  unsigned z;
  String Arg;

  strmaxcpy(Arg, pArg, STRINGSIZE);
  NLS_UpString(Arg);

  z = 0;
  while ((z < NameCnt) && (strcmp(Arg, Names[z])))
    z++;
  if (z >= NameCnt)
    return CMDErr;

  if (!Negate)
    DestFormat = Format[z];
  else if (DestFormat == Format[z])
    DestFormat = eHexFormatDefault;

  return CMDArg;
}

static CMDResult CMD_DataAdrRange(Boolean Negate,  const char *Arg)
{
  char *p, Save;
  Boolean ok;

  fputs(getmessage(Num_WarnDOption), stderr);
  fflush(stdout);

  if (Negate)
  {
    StartData = 0;
    StopData = 0x1fff;
    return CMDOK;
  }
  else
  {
    p = strchr(Arg, '-');
    if (!p)
      return CMDErr;

    Save = (*p);
    *p = '\0';
    StartData = ConstLongInt(Arg, &ok, 10);
    *p = Save;
    if (!ok)
      return CMDErr;

    StopData = ConstLongInt(p + 1, &ok, 10);
    if (!ok)
      return CMDErr;

    if (StartData > StopData)
      return CMDErr;

    return CMDArg;
  }
}

static CMDResult CMD_EntryAdr(Boolean Negate, const char *Arg)
{
  Boolean ok;

  if (Negate)
  {
    EntryAdrPresent = False;
    return CMDOK;
  }
  else
  {
    EntryAdr = ConstLongInt(Arg, &ok, 10);
    if ((!ok) || (EntryAdr > 0xffff))
      return CMDErr;
    EntryAdrPresent = True;
    return CMDArg;
  }
}

static CMDResult CMD_LineLen(Boolean Negate, const char *Arg)
{
  Boolean ok;

  if (Negate)
  {
    if (*Arg !='\0')
      return CMDErr;
    else
    {
      LineLen = 16;
      return CMDOK;
    }
  }
  else if (*Arg == '\0')
    return CMDErr;
  else
  {
    LineLen = ConstLongInt(Arg, &ok, 10);
    if ((!ok) || (LineLen < 1) || (LineLen > MaxLineLen))
      return CMDErr;
    else
    {
      LineLen += LineLen & 1;
      return CMDArg;
    }
  }
}

static CMDResult CMD_MinMoto(Boolean Negate, const char *Arg)
{
  Boolean ok;

  if (Negate)
  {
    if (*Arg != '\0')
      return CMDErr;
    else
    {
      MinMoto = 0;
      return CMDOK;
    }
  }
  else if (*Arg == '\0')
    return CMDErr;
  else
  {
    MinMoto = ConstLongInt(Arg, &ok, 10);
    if ((!ok) || (MinMoto < 1) || (MinMoto > 3))
      return CMDErr;
    else
      return CMDArg;
  }
}

static CMDResult CMD_AutoErase(Boolean Negate, const char *Arg)
{
  UNUSED(Arg);

  AutoErase = !Negate;
  return CMDOK;
}

static CMDResult CMD_AVRLen(Boolean Negate, const char *Arg)
{
  Word Temp;
  Boolean ok;

  if (Negate)
  {
    AVRLen = AVRLEN_DEFAULT;
    return CMDOK;
  }
  else
  {
    Temp = ConstLongInt(Arg, &ok, 10);
    if ((!ok) || (Temp < 2) || (Temp > 3))
      return CMDErr;
    else
    {
      AVRLen = Temp;
      return CMDArg;
    }
  }
}

static CMDResult CMD_CFormat(Boolean Negate, const char *pArg)
{
  if (Negate)
  {
    strcpy(CFormat, DefaultCFormat);
    return CMDOK;
  }
  else
  {
    int NumData = 0, NumStart = 0, NumLen = 0, NumEnd = 0;
    const char *pFormat;

    for (pFormat = pArg; *pFormat; pFormat++)
      switch (toupper(*pFormat))
      {
        case 'S': NumStart++; break;
        case 'D': NumData++; break;
        case 'L': NumLen++; break;
        case 'E': NumEnd++; break;
        default: return CMDErr;
      }
    if ((NumData > 1) || (NumStart > 1) || (NumLen > 1) || (NumEnd > 1))
      return CMDErr;
    strcpy(CFormat, pArg);
    return CMDArg;
  }
}

#define P2HEXParamCnt (sizeof(P2HEXParams) / sizeof(*P2HEXParams))
static CMDRec P2HEXParams[] =
{
  { "f", CMD_FilterList },
  { "r", CMD_AdrRange },
  { "R", CMD_AdrRelocate },
  { "a", CMD_RelAdr },
  { "i", CMD_IntelMode },
  { "m", CMD_MultiMode },
  { "F", CMD_DestFormat },
  { "5", CMD_Rec5 },
  { "s", CMD_SepMoto },
  { "d", CMD_DataAdrRange },
  { "e", CMD_EntryAdr },
  { "l", CMD_LineLen },
  { "k", CMD_AutoErase },
  { "M", CMD_MinMoto },
  { "AVRLEN", CMD_AVRLen },
  { "CFORMAT", CMD_CFormat },
};

static Word ChkSum;

int main(int argc, char **argv)
{
  char *ph1, *ph2;
  String Ver;

  ParamCount = argc - 1;
  ParamStr = argv;

  nls_init();
  NLS_Initialize();

  hex_init();
  endian_init();
  bpemu_init();
  hex_init();
  chunks_init();
  cmdarg_init(*argv);
  toolutils_init(*argv);
  nlmessages_init("p2hex.msg", *argv, MsgId1, MsgId2);
  ioerrs_init(*argv);

  sprintf(Ver, "P2HEX/C V%s", Version);
  WrCopyRight(Ver);

  InitChunk(&UsedList);

  if (ParamCount == 0)
  {
    errno = 0; printf("%s%s%s\n", getmessage(Num_InfoMessHead1), GetEXEName(), getmessage(Num_InfoMessHead2)); ChkIO(OutName);
    for (ph1 = getmessage(Num_InfoMessHelp), ph2 = strchr(ph1, '\n'); ph2; ph1 = ph2 + 1, ph2 = strchr(ph1, '\n'))
    {
      *ph2 = '\0';
      printf("%s\n", ph1);
      *ph2 = '\n';
    }
    exit(1);
  }

  StartAdr = 0;
  StopAdr = 0x7fff;
  StartAuto = True;
  StopAuto = True;
  StartData = 0;
  StopData = 0x1fff;
  EntryAdr = -1;
  EntryAdrPresent = False;
  AutoErase = False;
  RelAdr = False;
  Rec5 = True;
  LineLen = 16;
  AVRLen = AVRLEN_DEFAULT;
  IntelMode = 0;
  MultiMode = 0;
  DestFormat = eHexFormatDefault;
  MinMoto = 1;
  *TargName = '\0';
  Relocate = 0;
  strcpy(CFormat, DefaultCFormat);
  ProcessCMD(P2HEXParams, P2HEXParamCnt, ParUnprocessed, "P2HEXCMD", ParamError);

  if (ProcessedEmpty(ParUnprocessed))
  {
    errno = 0; printf("%s\n", getmessage(Num_ErrMsgTargMissing)); ChkIO(OutName);
    exit(1);
  }

  z = ParamCount;
  while ((z > 0) && (!ParUnprocessed[z]))
    z--;
  strmaxcpy(TargName, ParamStr[z], STRINGSIZE);
  if (!RemoveOffset(TargName, &Dummy))
    ParamError(False, ParamStr[z]);
  ParUnprocessed[z] = False;
  if (ProcessedEmpty(ParUnprocessed))
  {
    strmaxcpy(SrcName, ParamStr[z], STRINGSIZE);
    DelSuffix(TargName);
  }
  AddSuffix(TargName, STRINGSIZE, HexSuffix);
  Filename2CName(CTargName, TargName);
  NumCBlocks = 0;

  if (StartAuto || StopAuto)
  {
    if (StartAuto)
      StartAdr = INTCONST_ffffffff;
    if (StopAuto)
      StopAdr = 0;
    if (ProcessedEmpty(ParUnprocessed))
      ProcessGroup(SrcName, MeasureFile);
    else
      for (z = 1; z <= ParamCount; z++)
        if (ParUnprocessed[z])
          ProcessGroup(ParamStr[z], MeasureFile);
    if (StartAdr > StopAdr)
    {
      errno = 0;
      printf("%s\n", getmessage(Num_ErrMsgAutoFailed));
      ChkIO(OutName);
      exit(1);
    }
    printf("%s: 0x%s-", getmessage(Num_InfoMessDeducedRange), HexLong(StartAdr));
    printf("0x%s\n", HexLong(StopAdr));
  }

  OpenTarget();
  FormatOccured = 0;
  MaxMoto = 0;
  MaxIntel = 0;

  if (DestFormat == eHexFormatC)
  {
    errno = 0;
    fprintf(TargFile, "#ifndef _%s_H\n#define _%s_H\n\n", CTargName, CTargName);
    ChkIO(TargName);
    NumCBlocks = 0;
  }

  if (ProcessedEmpty(ParUnprocessed))
    ProcessGroup(SrcName, ProcessFile);
  else
    for (z = 1; z <= ParamCount; z++)
      if (ParUnprocessed[z])
        ProcessGroup(ParamStr[z], ProcessFile);


  if ((FormatOccured & eMotoOccured) && (!SepMoto))
  {
    errno = 0; fprintf(TargFile, "S%c%s", '9' - MaxMoto, HexByte(3 + MaxMoto)); ChkIO(TargName);
    ChkSum = 3 + MaxMoto;
    if (!EntryAdrPresent)
      EntryAdr = 0;
    if (MaxMoto >= 2)
    {
      errno = 0; fputs(HexByte((EntryAdr >> 24) & 0xff), TargFile); ChkIO(TargName);
      ChkSum += (EntryAdr >> 24) & 0xff;
    }
    if (MaxMoto >= 1)
    {
      errno = 0; fputs(HexByte((EntryAdr >> 16) & 0xff), TargFile); ChkIO(TargName);
      ChkSum += (EntryAdr >> 16) & 0xff;
    }
    errno = 0; fprintf(TargFile, "%s", HexWord(EntryAdr & 0xffff)); ChkIO(TargName);
    ChkSum += (EntryAdr >> 8) & 0xff;
    ChkSum += EntryAdr & 0xff;
    errno = 0; fprintf(TargFile, "%s\n", HexByte(0xff - (ChkSum & 0xff))); ChkIO(TargName);
  }

  if (FormatOccured & eIntelOccured)
  {
    Word EndRecAddr = 0;

    if (EntryAdrPresent)
    {
      switch (MaxIntel)
      {
        case 2:
          errno = 0; fprintf(TargFile, ":04000005"); ChkIO(TargName);
          ChkSum = 4 + 5;
          errno = 0; fprintf(TargFile, "%s", HexLong(EntryAdr)); ChkIO(TargName);
          ChkSum += ((EntryAdr >> 24) & 0xff) +
                    ((EntryAdr >> 16) & 0xff) +
                    ((EntryAdr >>  8) & 0xff) +
                    ( EntryAdr        & 0xff);
          goto WrChkSum;

        case 1:
          Seg = (EntryAdr >> 4) & 0xffff;
          Ofs = EntryAdr & 0x000f;
          errno = 0; fprintf(TargFile, ":04000003%s%s", HexWord(Seg), HexWord(Ofs)); ChkIO(TargName);
          ChkSum = 4 + 3 + Lo(Seg) + Hi(Seg) + Ofs;
          goto WrChkSum;

        default: /* == 0 */
          EndRecAddr = EntryAdr & 0xffff;
          break;

        WrChkSum:
          errno = 0; fprintf(TargFile, "%s\n", HexByte(0x100 - ChkSum)); ChkIO(TargName);
      }
    }
    errno = 0;
    switch (IntelMode)
    {
      case 0:
      {
        ChkSum = 1 + Hi(EndRecAddr) + Lo(EndRecAddr);
        fprintf(TargFile, ":00%s01%s\n", HexWord(EndRecAddr), HexByte(0x100 - ChkSum));
        break;
      }
      case 1:
        fprintf(TargFile, ":00000001\n");
        break;
      case 2:
        fprintf(TargFile, ":0000000000\n");
        break;
    }
    ChkIO(TargName);
  }

  if (FormatOccured & eMOSOccured)
  {
    errno = 0; fprintf(TargFile, ";0000040004\n"); ChkIO(TargName);
  }

  if (FormatOccured & eDSKOccured)
  {
    if (EntryAdrPresent)
    {
      errno = 0;
      fprintf(TargFile, "1%s7%sF\n", HexWord(EntryAdr), HexWord(EntryAdr));
      ChkIO(TargName);
    }
    errno = 0; fprintf(TargFile, ":\n"); ChkIO(TargName);
  }

  if (DestFormat == eHexFormatC)
  {
    unsigned ThisCBlock;
    String CBlockName;
    const char *pFormat;

    errno = 0;
    fprintf(TargFile,
            "typedef struct\n"
            "{\n");
    for (pFormat = CFormat; *pFormat; pFormat++)
      switch (*pFormat)
      {
        case 'd':
        case 'D':
          fprintf(TargFile, "  const char *data;\n"); break;
        case 's':
          fprintf(TargFile, "  unsigned start;\n"); break;
        case 'S':
          fprintf(TargFile, "  unsigned long start;\n"); break;
        case 'l':
          fprintf(TargFile, "  unsigned len;\n"); break;
        case 'L':
          fprintf(TargFile, "  unsigned long len;\n"); break;
        case 'e':
          fprintf(TargFile, "  unsigned end;\n"); break;
        case 'E':
          fprintf(TargFile, "  unsigned long end;\n"); break;
        default:
          break;
      }
    fprintf(TargFile,
            "} %s_blk;\n"
            "static const %s_blk %s_blks[] =\n"
            "{\n",
            CTargName, CTargName, CTargName);
    for (ThisCBlock = 0; ThisCBlock < NumCBlocks; ThisCBlock++)
    {
      GetCBlockName(CBlockName, ThisCBlock);
      fprintf(TargFile, "  {");
      for (pFormat = CFormat; *pFormat; pFormat++)
      {
        fprintf(TargFile, (pFormat != CFormat) ? ", " : " ");
        switch (toupper(*pFormat))
        {
          case 'D': fprintf(TargFile, "%s%s_data", CTargName, CBlockName); break;
          case 'S': fprintf(TargFile, "%s%s_start", CTargName, CBlockName); break;
          case 'L': fprintf(TargFile, "%s%s_len", CTargName, CBlockName); break;
          case 'E': fprintf(TargFile, "%s%s_end", CTargName, CBlockName); break;
          default: break;
        }
      }
      fprintf(TargFile, " },\n");
    }
    fprintf(TargFile, "  {");
    for (pFormat = CFormat; *pFormat; pFormat++)
      fprintf(TargFile, (pFormat != CFormat) ? ", 0" : " 0");
    fprintf(TargFile, " }\n"
                      "};\n\n");

    if (EntryAdrPresent)
      fprintf(TargFile, "#define %s_entry 0x%sul\n\n",
              CTargName, HexLong(EntryAdr));
    fprintf(TargFile,
            "#endif /* _%s_H */\n",
            CTargName);
    ChkIO(TargName);
  }
  CloseTarget();

  if (AutoErase)
  {
    if (ProcessedEmpty(ParUnprocessed)) ProcessGroup(SrcName, EraseFile);
    else
      for (z = 1; z <= ParamCount; z++)
        if (ParUnprocessed[z])
          ProcessGroup(ParamStr[z], EraseFile);
  }

  return 0;
}
