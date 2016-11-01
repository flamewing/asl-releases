/* tipseudo.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Haeufiger benutzte Texas Instruments Pseudo-Befehle                       */
/*                                                                           */
/*****************************************************************************/
/* $Id: tipseudo.c,v 1.6 2016/09/29 16:43:37 alfred Exp $                    */
/***************************************************************************** 
 * $Log: tipseudo.c,v $
 * Revision 1.6  2016/09/29 16:43:37  alfred
 * - introduce common DecodeDATA/DecodeRES functions
 *
 * Revision 1.5  2016/08/24 12:13:19  alfred
 * - begun with 320C4x support
 *
 * Revision 1.4  2014/11/03 17:36:12  alfred
 * - relocate IsDef() for common TI pseudo instructions
 *
 * Revision 1.3  2008/11/23 10:39:17  alfred
 * - allow strings with NUL characters
 *
 * Revision 1.2  2004/05/29 14:57:56  alfred
 * - added missing include statements
 *
 * Revision 1.1  2004/05/29 12:18:06  alfred
 * - relocated DecodeTIPseudo() to separate module
 *
 *****************************************************************************/

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

#include "fourpseudo.h"
#include "tipseudo.h"

/*****************************************************************************
 * Local Functions
 *****************************************************************************/

static void define_untyped_label(void)
{
  if (LabPart[0])
  {
    PushLocHandle(-1);
    EnterIntSymbol(LabPart, EProgCounter(), SegNone, False);
    PopLocHandle();
  }
}

static void pseudo_qxx(Integer num)
{
  int z;
  Boolean ok = True;
  double res;

  if (!ArgCnt)
  {
    WrError(1110);
    return;
  }

  for (z = 1; z <= ArgCnt; z++)
  {
    if (!*ArgStr[z])
    {
      ok = False;
      break;
    }

    res = ldexp(EvalFloatExpression(ArgStr[z], Float64, &ok), num);
    if (!ok)
      break;

    if ((res > 32767.49) || (res < -32768.49))
    {
      ok = False;
      WrError(1320);
      break;
    }
    WAsmCode[CodeLen++] = res;
  }

  if (!ok)
    CodeLen = 0;
}

static void pseudo_lqxx(Integer num)
{
  int z;
  Boolean ok = True;
  double res;
  LongInt resli;

  if (!ArgCnt)
  {
    WrError(1110);
    return;
  }

  for (z = 1; z <= ArgCnt; z++)
  {
    if (!*ArgStr[z])
    {
      ok = False;
      break;
    }

    res = ldexp(EvalFloatExpression(ArgStr[z], Float64, &ok), num);
    if (!ok) 
      break;

    if ((res > 2147483647.49) || (res < -2147483647.49))
    {
      ok = False;
      WrError(1320);
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
Boolean *, int *, LongInt
#endif
);

static void pseudo_store(tcallback callback)
{
  Boolean ok = True;
  int adr = 0;
  int z;
  TempResult t;

  if (!ArgCnt)
  {
    WrError(1110);
    return;
  }

  define_untyped_label();

  for (z = 1; ok && (z <= ArgCnt); z++)
  {
    if (!*ArgStr[z])
    {
      ok = False;
      break;
    }

    EvalExpression(ArgStr[z], &t);
    switch(t.Typ)
    {
      case TempInt:
        callback(&ok, &adr, t.Contents.Int);
        break;
      case TempFloat:
        WrError(1135);
        return;
      case TempString:
      {
        unsigned char *cp = (unsigned char *)t.Contents.Ascii.Contents,
                    *cend = cp + t.Contents.Ascii.Length;

        while (cp < cend)
          callback(&ok, &adr, CharTransTable[((usint)*cp++)&0xff]);
        break;
      }
      default:
        WrError(1135);
        ok = False;
        break;
    }
  }
   
  if (!ok)
    CodeLen = 0;
}

static void wr_code_byte(Boolean *ok, int *adr, LongInt val)
{
  if ((val < -128) || (val > 0xff))
  {
    WrError(1320);
    *ok = False;
    return;
  }
  WAsmCode[(*adr)++] = val & 0xff;
  CodeLen = *adr;
}

static void wr_code_word(Boolean *ok, int *adr, LongInt val)
{
  if ((val < -32768) || (val > 0xffff))
  {
    WrError(1320);
    *ok = False;
    return;
  }
  WAsmCode[(*adr)++] = val;
  CodeLen = *adr;
}

static void wr_code_long(Boolean *ok, int *adr, LongInt val)
{
  UNUSED(ok);

  WAsmCode[(*adr)++] = val & 0xffff;
  WAsmCode[(*adr)++] = val >> 16;
  CodeLen = *adr;
}

static void wr_code_byte_hilo(Boolean *ok, int *adr, LongInt val)
{
  if ((val < -128) || (val > 0xff))
  {
    WrError(1320);
    *ok = False;
    return;
  }
  if ((*adr) & 1) 
    WAsmCode[((*adr)++) / 2] |= val & 0xff;
  else 
    WAsmCode[((*adr)++) / 2] = val << 8;
  CodeLen = ((*adr) + 1) / 2;
}

static void wr_code_byte_lohi(Boolean *ok, int *adr, LongInt val)
{
  if ((val < -128) || (val > 0xff))
  {
    WrError(1320);
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
  int z, exp;
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
    pseudo_store(wr_code_byte_hilo); 
    return True;
  }
  if (Memo("RSTRING"))
  {
    pseudo_store(wr_code_byte_lohi); 
    return True;
  }
  if (Memo("BYTE"))
  {
    pseudo_store(wr_code_byte); 
    return True;
  }
  if (Memo("WORD"))
  {
    pseudo_store(wr_code_word); 
    return True;
  }
  if (Memo("LONG"))
  {
    pseudo_store(wr_code_long); 
    return True;
  }

  /* Qxx */

  if ((OpPart[0] == 'Q') && (OpPart[1] >= '0') && (OpPart[1] <= '9') &&
     (OpPart[2] >= '0') && (OpPart[2] <= '9') && (OpPart[3] == '\0'))
  {
    pseudo_qxx(10 * (OpPart[1] - '0') + OpPart[2] - '0');
    return True;
  }

  /* LQxx */

  if ((OpPart[0] == 'L') && (OpPart[1] == 'Q') && (OpPart[2] >= '0') && 
     (OpPart[2] <= '9') && (OpPart[3] >= '0') && (OpPart[3] <= '9') && 
     (OpPart[4] == '\0'))
  {
    pseudo_lqxx(10 * (OpPart[2] - '0') + OpPart[3] - '0');
    return True;
  }

  /* Floating point definitions */

  if (Memo("FLOAT"))
  {
    if (!ArgCnt)
    {
      WrError(1110);
      return True;
    }
    define_untyped_label();
    ok = True;
    for (z = 1; (z <= ArgCnt) && ok; z++)
    {
      if (!*ArgStr[z])
      {
        ok = False;
        break;
      }
      flt = EvalFloatExpression(ArgStr[z], Float32, &ok);
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
    if (!ArgCnt)
    {
      WrError(1110);
      return True;
    }
    define_untyped_label();
    ok = True;
    for (z = 1; (z <= ArgCnt) && ok; z++)
    {
      if (!*ArgStr[z])
      {
        ok = False;
        break;
      }
      dbl = EvalFloatExpression(ArgStr[z], Float64, &ok);
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
    if (!ArgCnt)
    {
      WrError(1110);
      return True;
    }
    define_untyped_label();
    ok = True;
    for (z = 1; (z <= ArgCnt) && ok; z++)
    {
      if (!*ArgStr[z])
      {
        ok = False;
        break;
      }
      dbl = EvalFloatExpression(ArgStr[z], Float64, &ok);
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
    if (!ArgCnt)
    {
      WrError(1110);
      return True;
    }
    define_untyped_label();
    ok = True;
    for (z = 1; (z <= ArgCnt) && ok; z++)
    {
      if (!*ArgStr[z])
      {
        ok = False;
        break;
      }
      dbl = EvalFloatExpression(ArgStr[z], Float64, &ok);
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
    if (!ArgCnt)
    {
      WrError(1110);
      return True;
    }
    define_untyped_label();
    ok = True;
    for (z = 1; (z <= ArgCnt) && ok; z++)
    {
      if (!*ArgStr[z])
      {
        ok = False;
        break;
      }
      dbl = EvalFloatExpression(ArgStr[z], Float64, &ok);
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
  int z;
  Boolean OK;

  UNUSED(Code);

  if (ArgCnt == 0) WrError(1110);
  else
  {
    OK = True;
    for (z = 1; z <= ArgCnt; z++)
      if (OK)
      {
        f = EvalFloatExpression(ArgStr[z], Float64, &OK);
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
  int z;
  Boolean OK;

  UNUSED(Code);

  if (ArgCnt == 0) WrError(1110);
  else
  {
    OK = True;
    for (z = 1; z <= ArgCnt; z++)
      if (OK)
      {
        f = EvalFloatExpression(ArgStr[z], Float64, &OK);
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
  int z;

  UNUSED(Code);

  if (ArgCnt == 0) WrError(1110);
  else
  {
    OK = True;
    for (z = 1; z <= ArgCnt; z++)
      if (OK) DAsmCode[CodeLen++] = EvalIntExpression(ArgStr[z], Int32, &OK);
    if (!OK)
      CodeLen = 0;
  }
}

static void DecodeDATA34x(Word Code)
{
  Boolean OK;
  TempResult t;
  int z;

  UNUSED(Code);

  if (ArgCnt == 0) WrError(1110);
  else
  {
    OK = True;
    for (z = 1; z <= ArgCnt; z++)
      if (OK)
      {
        EvalExpression(ArgStr[z], &t);
        switch (t.Typ)
        {
          case TempInt:
#ifdef HAS64
            if (!RangeCheck(t.Contents.Int, Int32))
            {
              OK = False;
              WrError(1320);
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
  LongInt Size;

  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    FirstPassUnknown = False;
    Size = EvalIntExpression(ArgStr[1], UInt24, &OK);
    if (FirstPassUnknown) WrError(1820);
    if ((OK) && (!FirstPassUnknown))
    {
      DontPrint = True;
      if (!Size)
        WrError(290);
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
