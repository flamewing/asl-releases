/* motpseudo.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Haeufiger benutzte Motorola-Pseudo-Befehle                                */
/*                                                                           */
/*****************************************************************************/
/* $Id: motpseudo.c,v 1.18 2015/10/18 17:26:25 alfred Exp $                   */
/*****************************************************************************
 * $Log: motpseudo.c,v $
 * Revision 1.18  2015/10/18 17:26:25  alfred
 * - allow ? as argument to some BYT/FCB/ADR/FDB
 *
 * Revision 1.17  2014/12/04 13:33:57  alfred
 * - compilable again
 *
 * Revision 1.16  2014/12/04 13:27:19  alfred
 * - rework to current style
 *
 * Revision 1.15  2014/11/16 13:15:08  alfred
 * - remove some superfluous semicolons
 *
 * Revision 1.14  2013/12/21 19:46:51  alfred
 * - dynamically resize code buffer
 *
 * Revision 1.13  2010/08/27 14:52:42  alfred
 * - some more overlapping strcpy() cleanups
 *
 * Revision 1.12  2010/04/17 13:14:24  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.11  2010/03/14 11:40:19  alfred
 * silence compiler warning
 *
 * Revision 1.10  2010/03/14 11:33:21  alfred
 * - allow 64-bit ints halfway on DOS
 *
 * Revision 1.9  2010/03/14 11:04:57  alfred
 * - ADR/RMB accepts string arguments
 *
 * Revision 1.8  2010/03/07 11:16:53  alfred
 * - allow DC.(float) on string operands
 *
 * Revision 1.7  2010/03/07 10:45:22  alfred
 * - generalization of Motorola disposal instructions
 *
 * Revision 1.6  2008/11/23 10:39:17  alfred
 * - allow strings with NUL characters
 *
 * Revision 1.5  2006/06/15 21:17:10  alfred
 * - must patch NUL at correct place
 *
 * Revision 1.4  2005/09/30 09:15:58  alfred
 * - correct Motorola 8-bit pseudo ops on word-wise CPUs
 *
 * Revision 1.3  2004/09/20 18:44:37  alfred
 * - formatting cleanups
 *
 * Revision 1.2  2004/05/29 14:57:56  alfred
 * - added missing include statements
 *
 * Revision 1.1  2004/05/29 12:04:48  alfred
 * - relocated DecodeMot(16)Pseudo into separate module
 *
 *****************************************************************************/

/*****************************************************************************
 * Includes
 *****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "bpemu.h"
#include "endian.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "asmallg.h"
#include "errmsg.h"

#include "motpseudo.h"

/*****************************************************************************
 * Local Variables
 *****************************************************************************/

static Boolean M16Turn = False;

/*****************************************************************************
 * Local Functions
 *****************************************************************************/

static Boolean CutRep(char *pAsc, LongInt *pErg)
{
  if (*pAsc != '[')
  {
    *pErg = 1;
    return True;
  }
  else
  {
    char *pStart, *pEnd;
    Boolean OK;

    pStart = pAsc + 1;
    pEnd = QuotPos(pStart, ']');
    if (!pEnd)
    {
      WrError(1300);
      return False;
    }
    else
    {
      *pEnd = '\0';
      *pErg = EvalIntExpression(pStart, Int32, &OK);
      strmov(pAsc, pEnd + 1);
      return OK;
    }
  }
}

static void PutByte(Byte Value)
{
  if ((ListGran() == 1) || (!(CodeLen & 1)))
    BAsmCode[CodeLen] = Value;
  else if (M16Turn)
    WAsmCode[CodeLen >> 1] = (((Word)BAsmCode[CodeLen -1]) << 8) | Value;
  else
    WAsmCode[CodeLen >> 1] = (((Word)Value) << 8) | BAsmCode[CodeLen -1];
  CodeLen++;
}

static void DecodeBYT(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(1, ArgCntMax))
  {
    ShortInt SpaceFlag = -1;
    int z = 1;
    Boolean OK = True;
    LongInt Rep;

    do
    {
      if (!*ArgStr[z])
      {
        OK = FALSE;
        WrError(2050);
        break;
      }

      KillBlanks(ArgStr[z]);
      OK = CutRep(ArgStr[z], &Rep);
      if (!OK)
        break;

      if (!strcmp(ArgStr[z], "?"))
      {
        if (SpaceFlag == 0)
        {
          WrError(1930);
          OK = FALSE;
        }
        else
        {
          SpaceFlag = 1;
          CodeLen += Rep;
        }
      }
      else if (SpaceFlag == 1)
      {
        WrError(1930);
        OK = FALSE;
      }
      else
      {
        TempResult t;

        SpaceFlag = 0;

        EvalExpression(ArgStr[z], &t);
        switch (t.Typ)
        {
          case TempInt:
            if (!RangeCheck(t.Contents.Int, Int8))
            {
              WrError(1320);
              OK = False;
            }
            else if (SetMaxCodeLen(CodeLen + Rep))
            {
              WrError(1920);
              OK = False;
            }
            else
            {
              LongInt z2;

              for (z2 = 0; z2 < Rep; z2++)
                PutByte(t.Contents.Int);
            }
            break;

          case TempFloat:
            WrError(1135);
            OK = False;
            break;

          case TempString:
          {
            int l;

            l = t.Contents.Ascii.Length;
            TranslateString(t.Contents.Ascii.Contents, l);

            if (SetMaxCodeLen(CodeLen + (Rep * l)))
            {
              WrError(1920);
              OK = False;
            }
            else
            {
              LongInt z2;
              int z3;

              for (z2 = 0; z2 < Rep; z2++)
                for (z3 = 0; z3 < l; z3++)
                  PutByte(t.Contents.Ascii.Contents[z3]);
            }
            break;
          }

          default:
            OK = False;
            break;
        }
      }

      z++;
    }
    while ((z <= ArgCnt) && (OK));

    if (!OK)
      CodeLen = 0;
    else if (SpaceFlag == 1)
      DontPrint = True;
  }
}

static void PutADR(Word Value)
{
  if (ListGran() > 1)
  {
    WAsmCode[CodeLen >> 1] = Value;
    CodeLen += 2;
  }
  else if (M16Turn)
  {
    BAsmCode[CodeLen++] = Hi(Value);
    BAsmCode[CodeLen++] = Lo(Value);
  }
  else
  {
    BAsmCode[CodeLen++] = Lo(Value);
    BAsmCode[CodeLen++] = Hi(Value);
  }
}

static void DecodeADR(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(1, ArgCntMax))
  {
    int z = 1;
    Boolean OK = True;
    LongInt Rep;
    ShortInt SpaceFlag = -1;

    do
    {
      if (!*ArgStr[z])
      {
        OK = FALSE;
        WrError(2050);
        break;
      }

      OK = CutRep(ArgStr[z], &Rep);
      if (!OK)
        break;

      if (!strcmp(ArgStr[z], "?"))
      {
        if (SpaceFlag == 0)
        {
          WrError(1930);
          OK = False;
        }
        else
        {
          SpaceFlag = 1;
          CodeLen += 2 * Rep;
        }
      }
      else if (SpaceFlag == 1)
      {
        WrError(1930);
        OK = False;
      }
      else
      {
        TempResult Res;
        LongInt z2, Cnt;

        SpaceFlag = 0;
        FirstPassUnknown = False;
        EvalExpression(ArgStr[z], &Res);

        switch (Res.Typ)
        {
          case TempInt:
            if (FirstPassUnknown)
              Res.Contents.Int &= 0xffff;
            if (!RangeCheck(Res.Contents.Int, Int16))
            {
              WrError(1320);
              Res.Typ = TempNone;
            }
            Cnt = 1;
            break;
          case TempString:
            Cnt = Res.Contents.Ascii.Length;
            TranslateString(Res.Contents.Ascii.Contents, Res.Contents.Ascii.Length);
            break;
          case TempFloat:
            WrError(1135);
            /* no break */
          default:
            Res.Typ = TempNone;
            Cnt = 0;
            break;
        }
        if (TempNone == Res.Typ)
        {
          OK = False;
          break;
        }

        if (SetMaxCodeLen(CodeLen + ((Cnt * Rep) << 1)))
        {
          WrError(1920);
          OK = False;
          break;
        }

        for (z2 = 0; z2 < Rep; z2++)
          switch (Res.Typ)
          {
            case TempInt:
              PutADR(Res.Contents.Int);
              break;
            case TempString:
            {
              unsigned z3;

              for (z3 = 0; z3 < Res.Contents.Ascii.Length; z3++)
                PutADR(Res.Contents.Ascii.Contents[z3]);
              break;
            }
            default:
              break;
          }
      }
      z++;
    }
    while ((z <= ArgCnt) && (OK));

    if (!OK)
      CodeLen = 0;
    else if (SpaceFlag)
      DontPrint = True;
  }
}

static void DecodeFCC(Word Index)
{
  String SVal;
  Boolean OK;
  int z, z3, l;
  LongInt Rep,z2;
  UNUSED(Index);

  if (ChkArgCnt(1, ArgCntMax))
  {
    z = 1;
    OK = True;

    do
    {
      if (!*ArgStr[z])
      {
        OK = FALSE;
        WrError(2050);
        break;
      }

      OK = CutRep(ArgStr[z], &Rep);
      if (!OK)
        break;

      EvalStringExpression(ArgStr[z], &OK, SVal);
      if (OK)
      {
        if (SetMaxCodeLen(CodeLen + Rep * strlen(SVal)))
        {
          WrError(1920);
          OK = False;
        }
        else
        {
          l = strlen(SVal);
          TranslateString(SVal, l);
          for (z2 = 0; z2 < Rep; z2++)
            for (z3 = 0; z3 < l; z3++)
              PutByte(SVal[z3]);
        }
      }

      z++;
    }
    while ((z <= ArgCnt) && (OK));

    if (!OK)
      CodeLen = 0;
  }
}

static void DecodeDFS(Word Index)
{
  Word HVal16;
  Boolean OK;
  UNUSED(Index);

  if (ChkArgCnt(1, 1))
  {
    FirstPassUnknown = False;
    HVal16 = EvalIntExpression(ArgStr[1], Int16, &OK);
    if (FirstPassUnknown)
      WrError(1820);
    else if (OK)
    {
      DontPrint = True;
      CodeLen = HVal16;
      if (!HVal16)
        WrError(290);
      BookKeeping();
    }
  }
}

/*****************************************************************************
 * Global Functions
 *****************************************************************************/

Boolean DecodeMotoPseudo(Boolean Turn)
{
  static PInstTable InstTable = NULL;

  if (!InstTable)
  {
    InstTable = CreateInstTable(17);
    AddInstTable(InstTable, "BYT", 0, DecodeBYT);
    AddInstTable(InstTable, "FCB", 0, DecodeBYT);
    AddInstTable(InstTable, "ADR", 0, DecodeADR);
    AddInstTable(InstTable, "FDB", 0, DecodeADR);
    AddInstTable(InstTable, "FCC", 0, DecodeFCC);
    AddInstTable(InstTable, "DFS", 0, DecodeDFS);
    AddInstTable(InstTable, "RMB", 0, DecodeDFS);
  }

  M16Turn = Turn;
  return LookupInstTable(InstTable, OpPart.Str);
}

static void DigIns(char Ch, int Pos, Byte *pDest)
{
  int bytepos = Pos >> 1, bitpos = (Pos & 1) << 2;
  Byte dig = Ch - '0';

  pDest[bytepos] |= (dig << bitpos);
}

void ConvertMotoFloatDec(Double F, Byte *pDest, Boolean NeedsBig)
{
  char s[30], Man[30], Exp[30];
  char *pSplit;
  int z, ManLen, ExpLen;
  Byte epos;

  UNUSED(NeedsBig);

  /* convert to ASCII, split mantissa & exponent */

  sprintf(s, "%0.16e", F);
  pSplit = strchr(s, 'e');
  if (!pSplit)
  {
    strcpy(Man, s);
    strcpy(Exp, "+0000");
  }
  else
  {
    *pSplit = '\0';
    strcpy(Man, s);
    strcpy(Exp, pSplit + 1);
  }

  memset(pDest, 0, 12);

  /* handle mantissa sign */

  if (*Man == '-')
  {
    pDest[11] |= 0x80; strmov(Man, Man + 1);
  }
  else if (*Man == '+')
    strmov(Man, Man + 1);

  /* handle exponent sign */

  if (*Exp == '-')
  {
    pDest[11] |= 0x40;
    strmov(Exp, Exp + 1);
  }
  else if (*Exp == '+')
    strmov(Exp, Exp + 1);

  /* integral part of mantissa (one digit) */

  DigIns(*Man, 16, pDest);
  strmov(Man, Man + 2);

  /* truncate mantissa if we have more digits than we can represent */

  if (strlen(Man) > 16)
    Man[16] = '\0';

  /* insert mantissa digits */

  ManLen = strlen(Man);
  for (z = 0; z < ManLen; z++)
    DigIns(Man[z], 15 - z, pDest);

  /* truncate exponent if we have more digits than we can represent - this should
     never occur since an IEEE double is limited to ~1E308 and we have for digits */

  if (strlen(Exp) > 4)
    strmov(Exp, Exp + strlen(Exp) - 4);

  /* insert exponent bits */

  ExpLen = strlen(Exp);
  for (z = ExpLen - 1; z >= 0; z--)
  {
    epos = ExpLen - 1 - z;
    if (epos == 3)
      DigIns(Exp[z], 19, pDest);
    else
      DigIns(Exp[z], epos + 20, pDest);
  }

  if (BigEndian)
    WSwap(pDest, 12);
}

static void EnterByte(LargeWord b)
{
  if (((CodeLen & 1) == 1) && (!BigEndian) && (ListGran() != 1))
  {
    BAsmCode[CodeLen    ] = BAsmCode[CodeLen - 1];
    BAsmCode[CodeLen - 1] = b;
  }
  else
  {
    BAsmCode[CodeLen] = b;
  }
  CodeLen++;
}

static void EnterWord(LargeWord w)
{
  if (ListGran() == 1)
  {
    BAsmCode[CodeLen    ] = Hi(w);
    BAsmCode[CodeLen + 1] = Lo(w);
  }
  else
    WAsmCode[CodeLen >> 1] = w;
  CodeLen += 2;
}

static void EnterLWord(LargeWord l)
{
  if (ListGran() == 1)
  {
    BAsmCode[CodeLen    ] = (l >> 24) & 0xff;
    BAsmCode[CodeLen + 1] = (l >> 16) & 0xff;
    BAsmCode[CodeLen + 2] = (l >>  8) & 0xff;
    BAsmCode[CodeLen + 3] = (l      ) & 0xff;
  }
  else
  {
    WAsmCode[(CodeLen >> 1)    ] = (l >> 16) & 0xffff;
    WAsmCode[(CodeLen >> 1) + 1] = (l      ) & 0xffff;
  }
  CodeLen += 4;
}

static void EnterQWord(LargeWord q)
{
  if (ListGran() == 1)
  {
#ifdef HAS64
    BAsmCode[CodeLen    ] = (q >> 56) & 0xff;
    BAsmCode[CodeLen + 1] = (q >> 48) & 0xff;
    BAsmCode[CodeLen + 2] = (q >> 40) & 0xff;
    BAsmCode[CodeLen + 3] = (q >> 32) & 0xff;
#else
    BAsmCode[CodeLen    ] =
    BAsmCode[CodeLen + 1] =
    BAsmCode[CodeLen + 2] =
    BAsmCode[CodeLen + 3] = 0;
#endif
    BAsmCode[CodeLen + 4] = (q >> 24) & 0xff;
    BAsmCode[CodeLen + 5] = (q >> 16) & 0xff;
    BAsmCode[CodeLen + 6] = (q >>  8) & 0xff;
    BAsmCode[CodeLen + 7] = (q      ) & 0xff;
  }
  else
  {
#ifdef HAS64
    WAsmCode[(CodeLen >> 1)    ] = (q >> 48) & 0xffff;
    WAsmCode[(CodeLen >> 1) + 1] = (q >> 32) & 0xffff;
#else
    WAsmCode[(CodeLen >> 1)    ] =
    WAsmCode[(CodeLen >> 1) + 1] = 0;
#endif
    WAsmCode[(CodeLen >> 1) + 2] = (q >> 16) & 0xffff;
    WAsmCode[(CodeLen >> 1) + 3] = (q      ) & 0xffff;
  }
  CodeLen += 8;
}

static void EnterIEEE4(Word *pField)
{
  if (ListGran() == 1)
  {
     BAsmCode[CodeLen    ] = Hi(pField[1]);
     BAsmCode[CodeLen + 1] = Lo(pField[1]);
     BAsmCode[CodeLen + 2] = Hi(pField[0]);
     BAsmCode[CodeLen + 3] = Lo(pField[0]);
  }
  else
  {
    WAsmCode[(CodeLen >> 1)    ] = pField[1];
    WAsmCode[(CodeLen >> 1) + 1] = pField[0];
  }
  CodeLen += 4;
}

static void EnterIEEE8(Word *pField)
{
  if (ListGran() == 1)
  {
    BAsmCode[CodeLen    ] = Hi(pField[3]);
    BAsmCode[CodeLen + 1] = Lo(pField[3]);
    BAsmCode[CodeLen + 2] = Hi(pField[2]);
    BAsmCode[CodeLen + 3] = Lo(pField[2]);
    BAsmCode[CodeLen + 4] = Hi(pField[1]);
    BAsmCode[CodeLen + 5] = Lo(pField[1]);
    BAsmCode[CodeLen + 6] = Hi(pField[0]);
    BAsmCode[CodeLen + 7] = Lo(pField[0]);
  }
  else
  {
    WAsmCode[(CodeLen >> 1)    ] = pField[3];
    WAsmCode[(CodeLen >> 1) + 1] = pField[2];
    WAsmCode[(CodeLen >> 1) + 2] = pField[1];
    WAsmCode[(CodeLen >> 1) + 3] = pField[0];
  }
  CodeLen += 8;
}

static void EnterIEEE10(Word *pField)
{
  if (ListGran() == 1)
  {
    BAsmCode[CodeLen    ] = Hi(pField[4]);
    BAsmCode[CodeLen + 1] = Lo(pField[4]);
    BAsmCode[CodeLen + 2] = 0;
    BAsmCode[CodeLen + 3] = 0;
    BAsmCode[CodeLen + 4] = Hi(pField[3]);
    BAsmCode[CodeLen + 5] = Lo(pField[3]);
    BAsmCode[CodeLen + 6] = Hi(pField[2]);
    BAsmCode[CodeLen + 7] = Lo(pField[2]);
    BAsmCode[CodeLen + 8] = Hi(pField[1]);
    BAsmCode[CodeLen + 9] = Lo(pField[1]);
    BAsmCode[CodeLen +10] = Hi(pField[0]);
    BAsmCode[CodeLen +11] = Lo(pField[0]);
  }
  else
  {
    WAsmCode[(CodeLen >> 1)    ] = pField[4];
    WAsmCode[(CodeLen >> 1) + 1] = 0;
    WAsmCode[(CodeLen >> 1) + 2] = pField[3];
    WAsmCode[(CodeLen >> 1) + 3] = pField[2];
    WAsmCode[(CodeLen >> 1) + 4] = pField[1];
    WAsmCode[(CodeLen >> 1) + 5] = pField[0];
  }
  CodeLen += 12;
}

static void EnterMotoFloatDec(Word *pField)
{
  if (ListGran() == 1)
  {
    BAsmCode[CodeLen    ] = Hi(pField[5]);
    BAsmCode[CodeLen + 1] = Lo(pField[5]);
    BAsmCode[CodeLen + 2] = Hi(pField[4]);
    BAsmCode[CodeLen + 3] = Lo(pField[4]);
    BAsmCode[CodeLen + 4] = Hi(pField[3]);
    BAsmCode[CodeLen + 5] = Lo(pField[3]);
    BAsmCode[CodeLen + 6] = Hi(pField[2]);
    BAsmCode[CodeLen + 7] = Lo(pField[2]);
    BAsmCode[CodeLen + 8] = Hi(pField[1]);
    BAsmCode[CodeLen + 9] = Lo(pField[1]);
    BAsmCode[CodeLen +10] = Hi(pField[0]);
    BAsmCode[CodeLen +11] = Lo(pField[0]);
  }
  else
  {
    WAsmCode[(CodeLen >> 1)    ] = pField[5];
    WAsmCode[(CodeLen >> 1) + 1] = pField[4];
    WAsmCode[(CodeLen >> 1) + 2] = pField[3];
    WAsmCode[(CodeLen >> 1) + 3] = pField[2];
    WAsmCode[(CodeLen >> 1) + 4] = pField[1];
    WAsmCode[(CodeLen >> 1) + 5] = pField[0];
  }
  CodeLen += 12;
}

void AddMoto16PseudoONOFF(void)
{
  AddONOFF("PADDING", &DoPadding, DoPaddingName, False);
}

Boolean DecodeMoto16Pseudo(ShortInt OpSize, Boolean Turn)
{
  Byte z;
  void (*EnterInt)(LargeWord) = NULL;
  void (*ConvertFloat)(Double, Byte*, Boolean) = NULL;
  void (*EnterFloat)(Word*) = NULL;
  void (*Swap)(void*, int) = NULL;
  IntType IntTypeEnum = UInt1;
  FloatType FloatTypeEnum = Float32;
  Word TurnField[8];
  LongInt z2;
  char *zp;
  LongInt WSize, Rep = 0;
  LongInt NewPC,HVal;
  TempResult t;
  Boolean OK, ValOK;
  ShortInt SpaceFlag;

  UNUSED(Turn);

  if (OpSize < 0)
    OpSize = 1;

  switch (OpSize)
  {
    case 0:
      WSize = 1;
      EnterInt = EnterByte;
      IntTypeEnum = Int8;
      break;
    case 1:
      WSize = 2;
      EnterInt = EnterWord;
      IntTypeEnum = Int16;
      break;
    case 2:
      WSize = 4;
      EnterInt = EnterLWord;
      IntTypeEnum = Int32;
      break;
    case 3:
      WSize = 8;
      EnterInt = EnterQWord;
#ifdef HAS64
      IntTypeEnum = Int64;
#else
      IntTypeEnum = Int32;
#endif
      break;
    case 4:
      WSize = 4;
      ConvertFloat = Double_2_ieee4;
      EnterFloat = EnterIEEE4;
      FloatTypeEnum = Float32;
      Swap = DWSwap;
      break;
    case 5:
      WSize = 8;
      ConvertFloat = Double_2_ieee8;
      EnterFloat = EnterIEEE8;
      FloatTypeEnum = Float64;
      Swap = QWSwap;
      break;

    /* NOTE: Double_2_ieee10() creates 10 bytes, but WSize is set to 12 (two
       padding bytes in binary representation).  This means that WSwap() will
       swap 12 instead of 10 bytes, which doesn't hurt, since TurnField is
       large enough and the two (garbage) bytes at the end will not be used
       by EnterIEEE10() anyway: */

    case 6:
      WSize = 12;
      ConvertFloat = Double_2_ieee10;
      EnterFloat = EnterIEEE10;
      FloatTypeEnum = Float80;
      Swap = WSwap;
      break;
    case 7:
      WSize = 12;
      ConvertFloat = ConvertMotoFloatDec;
      EnterFloat = EnterMotoFloatDec;
      FloatTypeEnum = FloatDec;
      break;
    default:
      WSize = 0;
  }

  if (*OpPart.Str != 'D')
    return False;

  if (Memo("DC"))
  {
    if (ChkArgCnt(1, ArgCntMax))
    {
      OK = True;
      z = 1;
      SpaceFlag = -1;

      while ((z <= ArgCnt) && (OK))
      {
        if (!*ArgStr[z])
        {
          OK = FALSE;
          WrError(2050);
          break;
        }

        FirstPassUnknown = False;
        OK = CutRep(ArgStr[z], &Rep);
        if (!OK)
          break;
        if (FirstPassUnknown)
        {
          OK = FALSE;
          WrError(1820);
          break;
        }

        if (!strcmp(ArgStr[z], "?"))
        {
          if (SpaceFlag == 0)
          {
            WrError(1930);
            OK = FALSE;
          }
          else
          {
            SpaceFlag = 1;
            CodeLen += (Rep * WSize);
          }
        }
        else if (SpaceFlag == 1)
        {
          WrError(1930);
          OK = FALSE;
        }
        else
        {
          SpaceFlag = 0;

          FirstPassUnknown = False;
          EvalExpression(ArgStr[z], &t);

          switch (t.Typ)
          {
            case TempInt:
              if (!EnterInt)
              {
                if (ConvertFloat && EnterFloat)
                {
                  t.Contents.Float = t.Contents.Int;
                  t.Typ = TempFloat;
                  goto HandleFloat;
                }
                else
                {
                  WrError(1135);
                  OK = False;
                }
              }
              else if ((!FirstPassUnknown) && (!RangeCheck(t.Contents.Int, IntTypeEnum)))
              {
                WrError(1320);
                OK = False;
              }
              else if (SetMaxCodeLen(CodeLen + (Rep * WSize)))
              {
                WrError(1920);
                OK = False;
              }
              else
                for (z2 = 0; z2 < Rep; z2++)
                  EnterInt(t.Contents.Int);
              break;
            HandleFloat:
            case TempFloat:
              if ((!ConvertFloat) || (!EnterFloat))
              {
                WrError(1135);
                OK = False;
              }
              else if (!FloatRangeCheck(t.Contents.Float, FloatTypeEnum))
              {
                WrError(1320);
                OK = False;
              }
              else if (SetMaxCodeLen(CodeLen + (Rep * WSize)))
              {
                WrError(1920);
                OK = False;
              }
              else
              {
                ConvertFloat(t.Contents.Float, (Byte *) TurnField, BigEndian);
                if ((BigEndian)  && (Swap))
                  Swap((void*) TurnField, WSize);
                for (z2 = 0; z2 < Rep; z2++)
                  EnterFloat(TurnField);
              }
              break;
            case TempString:
              if (!EnterInt)
              {
                if (ConvertFloat && EnterFloat)
                {
                  if (SetMaxCodeLen(CodeLen + (Rep * WSize * t.Contents.Ascii.Length)))
                  {
                    WrError(1920);
                    OK = False;
                  }
                  else
                  {
                    for (z2 = 0; z2 < Rep; z2++)
                      for (zp = t.Contents.Ascii.Contents; zp < t.Contents.Ascii.Contents + t.Contents.Ascii.Length; zp++)
                      {
                        ConvertFloat(CharTransTable[(usint) (*zp & 0xff)], (Byte *) TurnField, BigEndian);
                        if ((BigEndian)  && (Swap))
                          Swap((void*) TurnField, WSize);
                        EnterFloat(TurnField);
                      }
                  }
                }
                else
                {
                  WrError(1135);
                  OK = False;
                }
              }
              else if (SetMaxCodeLen(CodeLen + Rep * t.Contents.Ascii.Length))
              {
                WrError(1920);
                OK = False;
              }
              else
                for (z2 = 0; z2 < Rep; z2++)
                  for (zp = t.Contents.Ascii.Contents; zp < t.Contents.Ascii.Contents + t.Contents.Ascii.Length; EnterInt(CharTransTable[((usint) *(zp++)) & 0xff]));
              break;
            case TempNone:
              OK = False;
              break;
            default:
              WrError(1135);
              OK = False;
          }

        }

        z++;
      }

      /* purge results if an error occured */

      if (!OK) CodeLen = 0;

      /* just space reservation ? */

      else if (SpaceFlag == 1)
      {
        DontPrint = True;
        if ((DoPadding) && (CodeLen & 1))
          CodeLen++;
      }

      /* otherwise, we actually disposed values */

      else
      {
        if (DoPadding && (CodeLen & 1))
          EnterByte(0);
      }
    }
    return True;
  }

  if (Memo("DS"))
  {
    if (ChkArgCnt(1, 1))
    {
      FirstPassUnknown = False;
      HVal = EvalIntExpression(ArgStr[1], Int32, &ValOK);
      if (FirstPassUnknown)
        WrError(1820);
      if ((ValOK) && (!FirstPassUnknown))
      {
        DontPrint = True;
        if (0 == OpSize)
        {
          if ((HVal & 1) && (DoPadding))
            HVal++;
        }

        /* value of 0 means aligning the PC.  Doesn't make sense
           for bytes, since all adresses are integral numbers :-) */

        if (HVal == 0)
        {
          NewPC = ProgCounter() + WSize - 1;
          NewPC = NewPC-(NewPC % WSize);
          CodeLen = NewPC - ProgCounter();
          if (CodeLen == 0)
          {
            DontPrint = False;
            if (WSize == 1)
              WrError(290);
          }
        }
        else
          CodeLen = HVal * WSize;
        if (DontPrint)
          BookKeeping();
      }
    }
    return True;
  }

  return False;
}

