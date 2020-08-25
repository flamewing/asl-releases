/* tipseudo.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* Commonly Used TI-Style Pseudo Instructionso-Befehle                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
 * Includes
 *****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "endian.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "errmsg.h"

#include "fourpseudo.h"
#include "tipseudo.h"

/*****************************************************************************
 * Local Functions
 *****************************************************************************/

static void define_untyped_label(void)
{
  if (LabPart.Str[0])
  {
    PushLocHandle(-1);
    EnterIntSymbol(&LabPart, EProgCounter(), SegNone, False);
    PopLocHandle();
  }
}

static void pseudo_qxx(Integer num)
{
  tStrComp *pArg;
  Boolean ok = True;
  double res;

  if (!ChkArgCnt(1, ArgCntMax))
    return;

  forallargs (pArg, True)
  {
    if (!*pArg->Str)
    {
      ok = False;
      break;
    }

    res = ldexp(EvalStrFloatExpression(pArg, Float64, &ok), num);
    if (!ok)
      break;

    if ((res > 32767.49) || (res < -32768.49))
    {
      ok = False;
      WrError(ErrNum_OverRange);
      break;
    }
    WAsmCode[CodeLen++] = res;
  }

  if (!ok)
    CodeLen = 0;
}

static void pseudo_lqxx(Integer num)
{
  tStrComp *pArg;
  Boolean ok = True;
  double res;
  LongInt resli;

  if (!ChkArgCnt(1, ArgCntMax))
    return;

  forallargs (pArg, True)
  {
    if (!*pArg->Str)
    {
      ok = False;
      break;
    }

    res = ldexp(EvalStrFloatExpression(pArg, Float64, &ok), num);
    if (!ok) 
      break;

    if ((res > 2147483647.49) || (res < -2147483647.49))
    {
      ok = False;
      WrError(ErrNum_OverRange);
      break;
    }
    resli = res;
    WAsmCode[CodeLen++] = resli & 0xffff;
    WAsmCode[CodeLen++] = resli >> 16;
  }
   
  if (!ok)
    CodeLen = 0;
}

typedef void (*tcallback)(
#ifdef __PROTOS__
Boolean *, int *, LongInt, tSymbolFlags
#endif
);

static void pseudo_store(tcallback callback, Word MaxMultCharLen)
{
  Boolean ok = True;
  int adr = 0;
  tStrComp *pArg;
  TempResult t;

  if (!ChkArgCnt(1, ArgCntMax))
    return;

  define_untyped_label();

  forallargs (pArg, ok)
  {
    if (!*pArg->Str)
    {
      ok = False;
      break;
    }

    EvalStrExpression(pArg, &t);
    switch (t.Typ)
    {
      case TempFloat:
        WrStrErrorPos(ErrNum_StringOrIntButFloat, pArg);
        return;
      case TempString:
      {
        unsigned char *cp = (unsigned char *)t.Contents.Ascii.Contents,
                    *cend = cp + t.Contents.Ascii.Length;

        if (MultiCharToInt(&t, MaxMultCharLen))
          goto ToInt;

        while (cp < cend)
          callback(&ok, &adr, CharTransTable[((usint)*cp++) & 0xff], t.Flags);
        break;
      }
      case TempInt:
      ToInt:
        callback(&ok, &adr, t.Contents.Int, t.Flags);
        break;
      default:
        ok = False;
        break;
    }
  }
   
  if (!ok)
    CodeLen = 0;
}

static void wr_code_byte(Boolean *ok, int *adr, LongInt val, tSymbolFlags Flags)
{
  if (!mFirstPassUnknownOrQuestionable(Flags) && !RangeCheck(val, Int8))
  {
    WrError(ErrNum_OverRange);
    *ok = False;
    return;
  }
  WAsmCode[(*adr)++] = val & 0xff;
  CodeLen = *adr;
}

static void wr_code_word(Boolean *ok, int *adr, LongInt val, tSymbolFlags Flags)
{
  if (!mFirstPassUnknownOrQuestionable(Flags) && !RangeCheck(val, Int16))
  {
    WrError(ErrNum_OverRange);
    *ok = False;
    return;
  }
  WAsmCode[(*adr)++] = val;
  CodeLen = *adr;
}

static void wr_code_long(Boolean *ok, int *adr, LongInt val, tSymbolFlags Flags)
{
  UNUSED(ok);
  UNUSED(Flags);

  WAsmCode[(*adr)++] = val & 0xffff;
  WAsmCode[(*adr)++] = val >> 16;
  CodeLen = *adr;
}

static void wr_code_byte_hilo(Boolean *ok, int *adr, LongInt val, tSymbolFlags Flags)
{
  if (!mFirstPassUnknownOrQuestionable(Flags) && !RangeCheck(val, Int8))
  {
    WrError(ErrNum_OverRange);
    *ok = False;
    return;
  }
  if ((*adr) & 1) 
    WAsmCode[((*adr)++) / 2] |= val & 0xff;
  else 
    WAsmCode[((*adr)++) / 2] = val << 8;
  CodeLen = ((*adr) + 1) / 2;
}

static void wr_code_byte_lohi(Boolean *ok, int *adr, LongInt val, tSymbolFlags Flags)
{
  if (!mFirstPassUnknownOrQuestionable(Flags) && !RangeCheck(val, Int8))
  {
    WrError(ErrNum_OverRange);
    *ok = False;
    return;
  }
  if ((*adr) & 1) 
    WAsmCode[((*adr)++) / 2] |= val << 8;
  else 
    WAsmCode[((*adr)++) / 2] = val & 0xff;
  CodeLen = ((*adr) + 1) / 2;
}

/*****************************************************************************
 * Global Functions
 *****************************************************************************/

Boolean DecodeTIPseudo(void)
{
  Boolean ok;
  tStrComp *pArg;
  int exp;
  double dbl, mant;
  float flt;
  long lmant;
  Word w;

  if (Memo("RES") || Memo("BSS"))
  {
    if (Memo("BSS"))
      define_untyped_label();
    DecodeRES(0);
    return True;
  }

  if (Memo("DATA"))
  {
    DecodeDATA(Int16, Int16);
    return True;
  }

  if (Memo("STRING"))
  {
    pseudo_store(wr_code_byte_hilo, 1); 
    return True;
  }
  if (Memo("RSTRING"))
  {
    pseudo_store(wr_code_byte_lohi, 1); 
    return True;
  }
  if (Memo("BYTE"))
  {
    pseudo_store(wr_code_byte, 1); 
    return True;
  }
  if (Memo("WORD"))
  {
    pseudo_store(wr_code_word, 2); 
    return True;
  }
  if (Memo("LONG"))
  {
    pseudo_store(wr_code_long, 4); 
    return True;
  }

  /* Qxx */

  if ((OpPart.Str[0] == 'Q') && (OpPart.Str[1] >= '0') && (OpPart.Str[1] <= '9') &&
     (OpPart.Str[2] >= '0') && (OpPart.Str[2] <= '9') && (OpPart.Str[3] == '\0'))
  {
    pseudo_qxx(10 * (OpPart.Str[1] - '0') + OpPart.Str[2] - '0');
    return True;
  }

  /* LQxx */

  if ((OpPart.Str[0] == 'L') && (OpPart.Str[1] == 'Q') && (OpPart.Str[2] >= '0') && 
     (OpPart.Str[2] <= '9') && (OpPart.Str[3] >= '0') && (OpPart.Str[3] <= '9') && 
     (OpPart.Str[4] == '\0'))
  {
    pseudo_lqxx(10 * (OpPart.Str[2] - '0') + OpPart.Str[3] - '0');
    return True;
  }

  /* Floating point definitions */

  if (Memo("FLOAT"))
  {
    if (!ChkArgCnt(1, ArgCntMax))
      return True;
    define_untyped_label();
    ok = True;
    forallargs (pArg, ok)
    {
      if (!*pArg->Str)
      {
        ok = False;
        break;
      }
      flt = EvalStrFloatExpression(pArg, Float32, &ok);
      memcpy(WAsmCode+CodeLen, &flt, sizeof(float));
      if (BigEndian)
      {
        w = WAsmCode[CodeLen];
        WAsmCode[CodeLen] = WAsmCode[CodeLen+1];
        WAsmCode[CodeLen+1] = w;
      }
      CodeLen += sizeof(float)/2;
    }
    if (!ok)
      CodeLen = 0;
    return True;
  }

  if (Memo("DOUBLE"))
  {
    if (!ChkArgCnt(1, ArgCntMax))
      return True;
    define_untyped_label();
    ok = True;
    forallargs (pArg, ok)
    {
      if (!*pArg->Str)
      {
        ok = False;
        break;
      }
      dbl = EvalStrFloatExpression(pArg, Float64, &ok);
      memcpy(WAsmCode+CodeLen, &dbl, sizeof(dbl));
      if (BigEndian)
      {
        w = WAsmCode[CodeLen];
        WAsmCode[CodeLen] = WAsmCode[CodeLen+3];
        WAsmCode[CodeLen+3] = w;
        w = WAsmCode[CodeLen+1];
        WAsmCode[CodeLen+1] = WAsmCode[CodeLen+2];
        WAsmCode[CodeLen+2] = w;
      }
      CodeLen += sizeof(dbl)/2;
    }
    if (!ok)
      CodeLen = 0;
    return True;
  }

  if (Memo("EFLOAT"))
  {
    if (!ChkArgCnt(1, ArgCntMax))
      return True;
    define_untyped_label();
    ok = True;
    forallargs (pArg, ok)
    {
      if (!*pArg->Str)
      {
        ok = False;
        break;
      }
      dbl = EvalStrFloatExpression(pArg, Float64, &ok);
      mant = frexp(dbl, &exp);
      WAsmCode[CodeLen++] = ldexp(mant, 15);
      WAsmCode[CodeLen++] = exp-1;
    }
    if (!ok)
      CodeLen = 0;
    return True;
  }

  if (Memo("BFLOAT"))
  {
    if (!ChkArgCnt(1, ArgCntMax))
      return True;
    define_untyped_label();
    ok = True;
    forallargs (pArg, ok)
    {
      if (!*pArg->Str)
      {
        ok = False;
        break;
      }
      dbl = EvalStrFloatExpression(pArg, Float64, &ok);
      mant = frexp(dbl, &exp);
      lmant = ldexp(mant, 31);
      WAsmCode[CodeLen++] = (lmant & 0xffff);
      WAsmCode[CodeLen++] = (lmant >> 16);
      WAsmCode[CodeLen++] = exp-1;
    }
    if (!ok)
      CodeLen = 0;
    return True;
  }

  if (Memo("TFLOAT"))
  {
    if (!ChkArgCnt(1, ArgCntMax))
      return True;
    define_untyped_label();
    ok = True;
    forallargs (pArg, ok)
    {
      if (!*pArg->Str)
      {
        ok = False;
        break;
      }
      dbl = EvalStrFloatExpression(pArg, Float64, &ok);
      mant = frexp(dbl, &exp);
      mant = modf(ldexp(mant, 15), &dbl);
      WAsmCode[CodeLen + 3] = dbl;
      mant = modf(ldexp(mant, 16), &dbl);
      WAsmCode[CodeLen + 2] = dbl;
      mant = modf(ldexp(mant, 16), &dbl);
      WAsmCode[CodeLen + 1] = dbl;
      mant = modf(ldexp(mant, 16), &dbl);
      WAsmCode[CodeLen] = dbl;
      CodeLen += 4;
      WAsmCode[CodeLen++] = ((exp - 1) & 0xffff);
      WAsmCode[CodeLen++] = ((exp - 1) >> 16);
    }
    if (!ok)
      CodeLen = 0;
    return True;
  }
  return False;
}

Boolean IsTIDef(void)
{
  static const char *defs[] =
  {
    "BSS", "STRING", "RSTRING", 
    "BYTE", "WORD", "LONG", "FLOAT",
    "DOUBLE", "EFLOAT", "BFLOAT", 
    "TFLOAT", NULL
  }; 
  const char **cp = defs;

  while (*cp)
  {
    if (Memo(*cp))
      return True;
    cp++;
  }
  return False;
}

/*-------------------------------------------------------------------------*/
/* Convert IEEE to C3x/C4x floating point format */

static void SplitExt(Double Inp, LongInt *Expo, LongWord *Mant)
{
  Byte Field[8];
  Boolean Sign;
  int z;

  Double_2_ieee8(Inp, Field, False);
  Sign = (Field[7] > 0x7f);
  *Expo = (((LongWord) Field[7] & 0x7f) << 4) + (Field[6] >> 4);
  *Mant = Field[6] & 0x0f;
  if (*Expo != 0)
    *Mant |= 0x10;
  for (z = 5; z > 2; z--)
    *Mant = ((*Mant) << 8) | Field[z];
  *Mant = ((*Mant) << 3) + (Field[2] >> 5);
  *Expo -= 0x3ff;
  if (Sign)
    *Mant = (0xffffffff - *Mant) + 1;
  *Mant = (*Mant) ^ 0x80000000;
}

Boolean ExtToTIC34xShort(Double Inp, Word *Erg)
{
  LongInt Expo;
  LongWord Mant;

  if (Inp == 0)
    *Erg = 0x8000;
  else
  {
    SplitExt(Inp, &Expo, &Mant);
    if (!ChkRange(Expo, -7, 7))
      return False;
    *Erg = ((Expo << 12) & 0xf000) | ((Mant >> 20) & 0xfff);
  }
  return True;
}

Boolean ExtToTIC34xSingle(Double Inp, LongWord *Erg)
{
  LongInt Expo;
  LongWord Mant;

  if (Inp == 0)
    *Erg = 0x80000000;
  else
  {
    SplitExt(Inp, &Expo, &Mant);
    if (!ChkRange(Expo, -127, 127))
      return False;
    *Erg = ((Expo << 24) & 0xff000000) + (Mant >> 8);
  }
  return True;
}

Boolean ExtToTIC34xExt(Double Inp, LongWord *ErgL, LongWord *ErgH)
{
  LongInt Exp;

  if (Inp == 0)
  {
    *ErgH = 0x80;
    *ErgL = 0x00000000;
  }
  else
  {
    SplitExt(Inp, &Exp, ErgL);
    if (!ChkRange(Exp, -127, 127))
      return False;
    *ErgH = Exp & 0xff;
  }
  return True;
}

/*-------------------------------------------------------------------------*/
/* Pseudo Instructions common to C3x/C4x */

static void DecodeSINGLE(Word Code)
{
  Double f;
  tStrComp *pArg;
  Boolean OK;

  UNUSED(Code);

  if (ChkArgCnt(1, ArgCntMax))
  {
    OK = True;
    forallargs (pArg, True)
      if (OK)
      {
        f = EvalStrFloatExpression(pArg, Float64, &OK);
        if (OK)
          OK = OK && ExtToTIC34xSingle(f, DAsmCode + (CodeLen++));
      }
    if (!OK)
      CodeLen = 0;
  }
}

static void DecodeEXTENDED(Word Code)
{
  Double f;
  tStrComp *pArg;
  Boolean OK;

  UNUSED(Code);

  if (ChkArgCnt(1, ArgCntMax))
  {
    OK = True;
    forallargs (pArg, True)
      if (OK)
      {
        f = EvalStrFloatExpression(pArg, Float64, &OK);
        if (OK)
          OK = OK && ExtToTIC34xExt(f, DAsmCode + CodeLen + 1, DAsmCode + CodeLen);
        CodeLen += 2;
      }
    if (!OK)
      CodeLen = 0;
  }
}

static void DecodeWORD(Word Code)
{
  Boolean OK;
  tStrComp *pArg;

  UNUSED(Code);

  if (ChkArgCnt(1, ArgCntMax))
  {
    OK = True;
    forallargs (pArg, True)
      if (OK) DAsmCode[CodeLen++] = EvalStrIntExpression(pArg, Int32, &OK);
    if (!OK)
      CodeLen = 0;
  }
}

static void DecodeDATA34x(Word Code)
{
  Boolean OK;
  TempResult t;
  tStrComp *pArg;

  UNUSED(Code);

  if (ChkArgCnt(1, ArgCntMax))
  {
    OK = True;
    forallargs (pArg, True)
      if (OK)
      {
        EvalStrExpression(pArg, &t);
        switch (t.Typ)
        {
          case TempInt:
          ToInt:
#ifdef HAS64
            if (!RangeCheck(t.Contents.Int, Int32))
            {
              OK = False;
              WrError(ErrNum_OverRange);
            }
            else
#endif
              DAsmCode[CodeLen++] = t.Contents.Int;
            break;
          case TempFloat:
            if (!ExtToTIC34xSingle(t.Contents.Float, DAsmCode + (CodeLen++)))
              OK = False;
            break;
          case TempString:
          {
            unsigned z2;

            if (MultiCharToInt(&t, 4))
              goto ToInt;

            for (z2 = 0; z2 < t.Contents.Ascii.Length; z2++)
            {
             if (!(z2 & 3))
               DAsmCode[CodeLen++] = 0;
             DAsmCode[CodeLen - 1] |=
                (((LongWord)CharTransTable[((usint)t.Contents.Ascii.Contents[z2]) & 0xff])) << (8 * (3 - (z2 & 3)));
            }
            break;
          }
          default:
            OK = False;
        }
      }
    if (!OK)
      CodeLen = 0;
  }
}

static void DecodeBSS(Word Code)
{
  Boolean OK;
  tSymbolFlags Flags;
  LongInt Size;

  UNUSED(Code);

  if (ChkArgCnt(1, 1))
  {
    Size = EvalStrIntExpressionWithFlags(&ArgStr[1], UInt24, &OK, &Flags);
    if (mFirstPassUnknown(Flags)) WrError(ErrNum_FirstPassCalc);
    if (OK && !mFirstPassUnknown(Flags))
    {
      DontPrint = True;
      if (!Size)
        WrError(ErrNum_NullResMem);
      CodeLen = Size;
      BookKeeping();
    }
  }
}

void AddTI34xPseudo(TInstTable *pInstTable)
{
  AddInstTable(pInstTable, "SINGLE", 0, DecodeSINGLE);
  AddInstTable(pInstTable, "EXTENDED", 0, DecodeEXTENDED);
  AddInstTable(pInstTable, "WORD", 0, DecodeWORD);
  AddInstTable(pInstTable, "DATA", 0, DecodeDATA34x);
  AddInstTable(pInstTable, "BSS", 0, DecodeBSS);
}
