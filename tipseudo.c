/* tipseudo.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Haeufiger benutzte Texas Instruments Pseudo-Befehle                       */
/*                                                                           */
/*****************************************************************************/
/* $Id: tipseudo.c,v 1.2 2004/05/29 14:57:56 alfred Exp $                    */
/***************************************************************************** 
 * $Log: tipseudo.c,v $
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
  unsigned char *cp;

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
        cp = (unsigned char *)t.Contents.Ascii;
        while (*cp) 
          callback(&ok, &adr, CharTransTable[((usint)*cp++)&0xff]);
        break;
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
  int z, z2, exp;
  double dbl, mant;
  float flt;
  long lmant;
  Word w, size;
  TempResult t;
  unsigned char *cp;

  if (Memo("RES") || Memo("BSS"))
  {
    if (ArgCnt != 1)
    {
      WrError(1110);
      return True;
    }
    if (Memo("BSS"))
      define_untyped_label();
    FirstPassUnknown = False;
    size = EvalIntExpression(ArgStr[1], Int16, &ok);
    if (FirstPassUnknown)
    {
      WrError(1820);
      return True;
    }
    if (!ok) 
      return True;
    DontPrint = True;
    if (!size) WrError(290);
    CodeLen = size;
    BookKeeping();
    return True;
  }

  if (Memo("DATA"))
  {
    if (!ArgCnt)
    {
      WrError(1110);
      return True;
    }

    ok = True;
    for (z = 1; (z <= ArgCnt) && ok; z++)
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
          if (!ChkRange(t.Contents.Int, -32768, 0xffff))
            ok = False;
          else
            WAsmCode[CodeLen++] = t.Contents.Int;
          break;
        case TempFloat:
          WrError(1135); 
          ok = False;
          break;
        case TempString:
          z2 = 0;
          cp = (unsigned char *)t.Contents.Ascii;
          while (*cp)
          {
            if (z2 & 1)
              WAsmCode[CodeLen++] |= (CharTransTable[((usint)*cp++)&0xff] << 8);
            else
              WAsmCode[CodeLen] = CharTransTable[((usint)*cp++)&0xff];
            z2++;
          }
          if (z2 & 1)
            CodeLen++;
          break;
        default:
          break;
      }
    }
    if (!ok)
      CodeLen = 0;
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

