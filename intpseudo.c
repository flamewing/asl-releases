/* intpseudo.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Haeufiger benutzte Intel-Pseudo-Befehle                                   */
/*                                                                           */
/*****************************************************************************/
/* $Id: intpseudo.c,v 1.11 2014/12/07 19:14:02 alfred Exp $                   */
/***************************************************************************** 
 * $Log: intpseudo.c,v $
 * Revision 1.11  2014/12/07 19:14:02  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.10  2014/12/05 11:09:11  alfred
 * - eliminate Nil
 *
 * Revision 1.9  2013/12/21 19:46:51  alfred
 * - dynamically resize code buffer
 *
 * Revision 1.8  2010/03/14 10:45:15  alfred
 * - allow string arguments for DW/DD/DQ/DT
 *
 * Revision 1.7  2008/11/23 10:39:17  alfred
 * - allow strings with NUL characters
 *
 * Revision 1.6  2007/11/24 22:48:08  alfred
 * - some NetBSD changes
 *
 * Revision 1.5  2005/11/04 19:38:00  alfred
 * - ignore case on DUP search
 *
 * Revision 1.4  2004/05/31 12:47:28  alfred
 * - use CopyNoBlanks()
 *
 * Revision 1.3  2004/05/29 14:57:56  alfred
 * - added missing include statements
 *
 * Revision 1.2  2004/05/29 12:04:48  alfred
 * - relocated DecodeMot(16)Pseudo into separate module
 *
 * Revision 1.1  2004/05/29 11:33:04  alfred
 * - relocated DecodeIntelPseudo() into own module
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
#include "errmsg.h"

#include "intpseudo.h"

/*****************************************************************************
 * Local Types
 *****************************************************************************/

typedef Boolean (*TLayoutFunc)(
#ifdef __PROTOS__
                               const char *Asc, Word *Cnt, Boolean Turn
#endif
                               );

/*****************************************************************************
 * Local Variables
 *****************************************************************************/

static enum
{
  DSNone, DSConstant, DSSpace
} DSFlag;

/*****************************************************************************
 * Local Functions
 *****************************************************************************/

/*****************************************************************************
 * Function:    PreLayoutChk
 * Purpose:     common pre-handling for Layout...
 * Result:      TRUE if no further processing needed/possible
 *****************************************************************************/

static Boolean PreLayoutChk(const char *pExpr, Boolean *pResult, Word *pCnt, int BinLen)
{
  /* only reserve storage? */

  if (!strcmp(pExpr, "?"))
  {
    /* yes -> check for illegal argument mix */

    if (DSFlag == DSConstant)
    {
      WrError(1930);
      *pResult = False;
    }
    else
    {
      *pResult = True;
      DSFlag = DSSpace;
      *pCnt = BinLen;
      CodeLen += BinLen;
    }
    return True;
  }

  /* OK, we dispose a value. Check for illegal argument mix */

  else if (DSFlag == DSSpace)
  {
    WrError(1930);
    *pResult = False;
    return True;
  }

  /* OK, type is fine so far.  See if we still have space left */

  else
  {
    DSFlag = DSConstant;
    if (SetMaxCodeLen(CodeLen + 2))
    {
      WrError(1920);
      *pResult = False;
      return True;
    }
    else
      return False;
  }
}

/*****************************************************************************
 * Function:    LayoutByte
 * Purpose:     parse argument, interprete as byte,
 *              and put into result buffer
 * Result:      TRUE if no errors occured
 *****************************************************************************/

static Boolean LayoutByte(const char *pExpr, Word *pCnt, Boolean BigEndian)
{
  Boolean Result;
  TempResult t;

  UNUSED(BigEndian);

  if (PreLayoutChk(pExpr, &Result, pCnt, 1))
    return Result;

  /* PreLayoutChk() has checked for at least one byte free space
     in output buffer, so we only need to check again in case of
     a string argument: */

  FirstPassUnknown = False;
  EvalExpression(pExpr, &t);
  Result = False;
  switch (t.Typ)
  {
    case TempInt:
      if (FirstPassUnknown) t.Contents.Int &= 0xff;
      if (!RangeCheck(t.Contents.Int, Int8)) WrError(1320);
      else
      {
        BAsmCode[CodeLen++] = t.Contents.Int;
        *pCnt = 1;
        Result = True;
      }
      break;
    case TempFloat: 
      WrError(1135);
      break;
    case TempString:
      TranslateString(t.Contents.Ascii.Contents, t.Contents.Ascii.Length);
      *pCnt = t.Contents.Ascii.Length;
      if (SetMaxCodeLen(CodeLen + (*pCnt))) WrError(1920);
      else
      {
        memcpy(BAsmCode + CodeLen, t.Contents.Ascii.Contents, *pCnt);
        CodeLen += (*pCnt);
        Result = True;
      }
      break;
    default:
      break;
  }

  return Result;
}

/*****************************************************************************
 * Function:    LayoutWord
 * Purpose:     parse argument, interprete as 16-bit word,
 *              and put into result buffer
 * Result:      TRUE if no errors occured
 *****************************************************************************/

static void PutWord(Word w, Boolean BigEndian)
{
  if (BigEndian)
  {
    BAsmCode[CodeLen    ] = Hi(w);
    BAsmCode[CodeLen + 1] = Lo(w);
  }
  else
  {
    BAsmCode[CodeLen    ] = Lo(w);
    BAsmCode[CodeLen + 1] = Hi(w);
  }
  CodeLen += 2;
}

static Boolean LayoutWord(const char *pExpr, Word *pCnt, Boolean BigEndian)
{
  Boolean Result;
  TempResult t;

  if (PreLayoutChk(pExpr, &Result, pCnt, 2))
    return Result;

  /* PreLayoutChk() has checked for at least two bytes free space
     in output buffer, so we only need to check again in case of
     a string argument: */

  FirstPassUnknown = False;
  EvalExpression(pExpr, &t);
  Result = True; *pCnt = 0;
  switch (t.Typ)
  {
    case TempInt:
      if (FirstPassUnknown)
        t.Contents.Int &= 0xffff;
      if (!RangeCheck(t.Contents.Int, Int16)) WrError(1320);
      else
      {
        PutWord(t.Contents.Int, BigEndian);
        *pCnt = 2;
        Result = True;
      }
      break;
    case TempFloat:
      WrError(1135);
      break;
    case TempString:
      TranslateString(t.Contents.Ascii.Contents, t.Contents.Ascii.Length);
      *pCnt = t.Contents.Ascii.Length * 2;
      if (SetMaxCodeLen(CodeLen + (*pCnt))) WrError(1920);
      else
      {
        unsigned z;

        for (z = 0; z < t.Contents.Ascii.Length; z++)
          PutWord(t.Contents.Ascii.Contents[z], BigEndian);
        Result = True;
      }
      break;
    default:
      break;
  }

  return Result;
}

/*****************************************************************************
 * Function:    LayoutDoubleWord
 * Purpose:     parse argument, interprete as 32-bit word or 
                single precision float, and put into result buffer
 * Result:      TRUE if no errors occured
 *****************************************************************************/

static void PutDWord(LongWord l, Boolean BigEndian)
{
  if (BigEndian)   
  {
    BAsmCode[CodeLen    ] = (l >> 24) & 0xff;
    BAsmCode[CodeLen + 1] = (l >> 16) & 0xff;
    BAsmCode[CodeLen + 2] = (l >>  8) & 0xff;
    BAsmCode[CodeLen + 3] = (l      ) & 0xff;
  }
  else
  {
    BAsmCode[CodeLen    ] = (l      ) & 0xff;
    BAsmCode[CodeLen + 1] = (l >>  8) & 0xff;
    BAsmCode[CodeLen + 2] = (l >> 16) & 0xff;
    BAsmCode[CodeLen + 3] = (l >> 24) & 0xff;
  }
  CodeLen += 4;
}

static Boolean LayoutDoubleWord(const char *pExpr, Word *pCnt, Boolean BigEndian)
{
  TempResult erg;
  Boolean Result = False;
  String Copy;

  if (PreLayoutChk(pExpr, &Result, pCnt, 4))
    return Result;

  /* PreLayoutChk() has checked for at least four bytes free space
     in output buffer, so we only need to check again in case of
     a string argument: */

  CopyNoBlanks(Copy, pExpr, STRINGSIZE);
  EvalExpression(Copy, &erg);
  Result = False;
  switch (erg.Typ)
  {
    case TempNone:
      break;
    case TempInt:
      if (!RangeCheck(erg.Contents.Int, Int32)) WrError(1320);
      else
      {
        PutDWord(erg.Contents.Int, BigEndian);
        *pCnt = 4;
        Result = True;
      }
      break;
    case TempFloat:
      if (!FloatRangeCheck(erg.Contents.Float, Float32)) WrError(1320);
      else
      {
        Double_2_ieee4(erg.Contents.Float, BAsmCode + CodeLen, False);
        *pCnt = 4;
        CodeLen += 4;
        Result = True;
      }
      break;
    case TempString:
      TranslateString(erg.Contents.Ascii.Contents, erg.Contents.Ascii.Length);
      *pCnt = erg.Contents.Ascii.Length * 4;
      if (SetMaxCodeLen(CodeLen + (*pCnt))) WrError(1920);
      else
      {   
        unsigned z;

        for (z = 0; z < erg.Contents.Ascii.Length; z++)
          PutDWord(erg.Contents.Ascii.Contents[z], BigEndian);
        Result = True;
      }
      break;
    case TempAll:
      WrError(1135);
  }

  if (Result)
  {
    if (BigEndian)
      DSwap(BAsmCode + CodeLen - *pCnt, *pCnt);
  }

  return Result;
}


/*****************************************************************************
 * Function:    LayoutQuadWord
 * Purpose:     parse argument, interprete as 64-bit word or 
                double precision float, and put into result buffer
 * Result:      TRUE if no errors occured
 *****************************************************************************/

static void PutQWord(LargeWord l, Boolean BigEndian)
{
  if (BigEndian)
  {
    BAsmCode[CodeLen + 7] = (l      ) & 0xff;
    BAsmCode[CodeLen + 6] = (l >>  8) & 0xff;
    BAsmCode[CodeLen + 5] = (l >> 16) & 0xff;
    BAsmCode[CodeLen + 4] = (l >> 24) & 0xff;
#ifdef HAS64
    BAsmCode[CodeLen + 3] = (l >> 32) & 0xff;
    BAsmCode[CodeLen + 2] = (l >> 40) & 0xff;
    BAsmCode[CodeLen + 1] = (l >> 48) & 0xff;
    BAsmCode[CodeLen + 0] = (l >> 56) & 0xff;
#else
    BAsmCode[CodeLen + 3] =
    BAsmCode[CodeLen + 2] =
    BAsmCode[CodeLen + 1] =
    BAsmCode[CodeLen    ] = 0;
#endif
  }
  else
  {
    BAsmCode[CodeLen    ] = (l      ) & 0xff;
    BAsmCode[CodeLen + 1] = (l >>  8) & 0xff;
    BAsmCode[CodeLen + 2] = (l >> 16) & 0xff;
    BAsmCode[CodeLen + 3] = (l >> 24) & 0xff;
#ifdef HAS64
    BAsmCode[CodeLen + 4] = (l >> 32) & 0xff;
    BAsmCode[CodeLen + 5] = (l >> 40) & 0xff;
    BAsmCode[CodeLen + 6] = (l >> 48) & 0xff;
    BAsmCode[CodeLen + 7] = (l >> 56) & 0xff;
#else
    BAsmCode[CodeLen + 4] =
    BAsmCode[CodeLen + 5] =
    BAsmCode[CodeLen + 6] =
    BAsmCode[CodeLen + 7] = 0;
#endif
  }
  CodeLen += 8;
}

static Boolean LayoutQuadWord(const char *pExpr, Word *pCnt, Boolean BigEndian)
{
  Boolean Result;
  TempResult erg;
  String Copy;

  if (PreLayoutChk(pExpr, &Result, pCnt, 8))
    return Result;

  /* PreLayoutChk() has checked for at least eight bytes free space
     in output buffer, so we only need to check again in case of
     a string argument: */

  CopyNoBlanks(Copy, pExpr, STRINGSIZE);
  EvalExpression(Copy, &erg);
  Result = False;
  switch(erg.Typ)
  {
    case TempNone:
      break;
    case TempInt:
      PutQWord(erg.Contents.Int, BigEndian);
      *pCnt = 8;
      Result = True;
      break;
    case TempFloat:
      Double_2_ieee8(erg.Contents.Float, BAsmCode + CodeLen, False);
      *pCnt = 8;
      CodeLen += 8;
      Result = True;
      break;
    case TempString:
      TranslateString(erg.Contents.Ascii.Contents, erg.Contents.Ascii.Length);
      *pCnt = erg.Contents.Ascii.Length * 8; 
      if (SetMaxCodeLen(CodeLen + (*pCnt))) WrError(1920);
      else  
      {   
        unsigned z;

        for (z = 0; z < erg.Contents.Ascii.Length; z++)
          PutQWord(erg.Contents.Ascii.Contents[z], BigEndian);
        Result = True;
      }
      break;
    case TempAll:
      WrError(1135);
      return Result;
  }

  if (Result)
  {
    if (BigEndian)
      QSwap(BAsmCode + CodeLen - *pCnt, *pCnt);
  }
  return Result;
}

/*****************************************************************************
 * Function:    LayoutTenBytes
 * Purpose:     parse argument, interprete extended precision float,
 *              and put into result buffer
 * Result:      TRUE if no errors occured
 *****************************************************************************/

static Boolean LayoutTenBytes(const char *pExpr, Word *pCnt, Boolean BigEndian)
{
  Boolean Result;
  TempResult erg;
  String Copy;

  if (PreLayoutChk(pExpr, &Result, pCnt, 10))
    return Result;

  /* PreLayoutChk() has checked for at least eight bytes free space
     in output buffer, so we only need to check again in case of
     a string argument: */

  CopyNoBlanks(Copy, pExpr, STRINGSIZE);
  EvalExpression(Copy, &erg);
  Result = False;
  switch(erg.Typ)
  {
    case TempNone:
      break;
    case TempInt:
      erg.Contents.Float = erg.Contents.Int;
      erg.Typ = TempFloat;
      /* no break! */
    case TempFloat:
      Double_2_ieee10(erg.Contents.Float, BAsmCode + CodeLen, False);
      *pCnt = 10;
      CodeLen += 10;
      Result = True;
      break;
    case TempString:
      TranslateString(erg.Contents.Ascii.Contents, erg.Contents.Ascii.Length);
      *pCnt = erg.Contents.Ascii.Length * 10;
      if (SetMaxCodeLen(CodeLen + (*pCnt))) WrError(1920);
      else
      {
        unsigned z;

        for (z = 0; z < erg.Contents.Ascii.Length; z++)
        {
          Double_2_ieee10(erg.Contents.Ascii.Contents[z], BAsmCode + CodeLen, False);
          CodeLen += 10;
        }
        Result = True;
      }
      break;
    case TempAll:
      WrError(1135);
      return Result;
  }

  if (Result)
  {
    if (BigEndian)
      TSwap(BAsmCode + CodeLen - *pCnt, *pCnt);
  }
  return Result;
}

/***************************************************************************** 
 * Global Functions
 *****************************************************************************/
 
/*****************************************************************************
 * Function:    DecodeIntelPseudo
 * Purpose:     handle Intel-style pseudo instructions
 * Result:      TRUE if mnemonic was handled
 *****************************************************************************/
   
static Boolean DecodeIntelPseudo_ValidSymChar(char ch)
{
  ch = mytoupper(ch);

  return (((ch >= 'A') && (ch <= 'Z'))
       || ((ch >= '0') && (ch <= '9'))
       || (ch == '_')
       || (ch == '.'));
}

static void DecodeIntelPseudo_HandleQuote(int *pDepth, Byte *pQuote, char Ch)
{
  switch (Ch)
  {
    case '(':
      if (!(*pQuote))
        (*pDepth)++;
      break;
    case ')':
      if (!(*pQuote))
        (*pDepth)--;
      break;
    case '\'':
      if (!((*pQuote) & 2))
        (*pQuote) ^= 1;
      break;
    case '"':
      if (!((*pQuote) & 1))
        (*pQuote) ^= 2;
      break;
  }
}

static Boolean DecodeIntelPseudo_LayoutMult(const char *pArg, Word *Cnt,
                                            TLayoutFunc LayoutFunc,
                                            Boolean Turn)
{
  int z, Depth, Len;
  Boolean OK, LastValid;
  Byte Quote;
  const char *pDupFnd, *pRun;

  /* search for DUP: exclude parts in parentheses,
     and parts in quotation marks */

  Depth = Quote = 0;
  LastValid = FALSE;
  pDupFnd = NULL; Len = strlen(pArg);
  for (pRun = pArg; pRun < pArg + Len - 2; pRun++)
  {
    DecodeIntelPseudo_HandleQuote(&Depth, &Quote, *pRun);
    if ((!Depth) && (!Quote))
    {
      if ((!LastValid)
      &&  (!DecodeIntelPseudo_ValidSymChar(pRun[3]))
      &&  (!strncasecmp(pRun, "DUP", 3)))
      {
        pDupFnd = pRun;
        break;
      }
    }
    LastValid = DecodeIntelPseudo_ValidSymChar(*pRun);
  }

  /* found DUP: */

  if (pDupFnd)
  {
    LongInt DupCnt;
    char *pDupPart, *pPart, *pSep, *pRun;
    Word SumCnt, ECnt, SInd;
    String Copy;

    /* operate on copy */

    strmaxcpy(Copy, pArg, 255);
    pSep = Copy + (pDupFnd - pArg);

    /* evaluate count */

    FirstPassUnknown = False;
    *pSep = '\0';
    DupCnt = EvalIntExpression(Copy, Int32, &OK);
    if (FirstPassUnknown)
    {
      WrError(1820); return False;
    }
    if (!OK)
      return False;

    /* catch invalid counts */

    if (DupCnt <= 0)
    {
      if (DupCnt < 0)
        WrError(270);
      return True;
    }

    /* split into parts and evaluate */

    pDupPart = pSep + 3;
    while (myisspace(*pDupPart))
      pDupPart++;
    SumCnt = 0;
    Len = strlen(pDupPart); 
    if ((strlen(pDupPart) >= 2) && (*pDupPart == '(') && (pDupPart[Len - 1] == ')'))
    {
      pDupPart++;
      pDupPart[Len - 2] = '\0';
    }
    while (*pDupPart)
    {
      pSep = NULL; Quote = Depth = 0;
      for (pRun = pDupPart; *pRun; pRun++)
      {
        DecodeIntelPseudo_HandleQuote(&Depth, &Quote, *pRun);
        if ((!Depth) && (!Quote) && (*pRun == ','))
        {
          pSep = pRun;
          break;
        }
      }
      pPart = pDupPart; 
      if (!pSep)
        pDupPart = "";
      else
      {
        *pSep = '\0';
        pDupPart = pSep + 1;
      }
      if (!DecodeIntelPseudo_LayoutMult(pPart, &ECnt, LayoutFunc, Turn))
        return False; 
      SumCnt += ECnt;
    }

    /* replicate result */

    if (DSFlag == DSConstant)
    {
      SInd = CodeLen - SumCnt;
      if (SetMaxCodeLen(CodeLen + SumCnt * (DupCnt - 1)))
      {
        WrError(1920); return False;
      }
      for (z = 1; z <= DupCnt - 1; z++)
      {
        if (SetMaxCodeLen(CodeLen + SumCnt))
          return False;
        memcpy(BAsmCode + CodeLen, BAsmCode + SInd, SumCnt);
        CodeLen += SumCnt;
      }
    }
    else
      CodeLen += SumCnt * (DupCnt - 1);
    *Cnt = SumCnt * DupCnt;
    return True;
  }

  /* no DUP: simple expression */

  else 
    return LayoutFunc(pArg, Cnt, Turn);
}

Boolean DecodeIntelPseudo(Boolean Turn)
{
  Word Dummy;
  int z;
  TLayoutFunc LayoutFunc = NULL;
  Boolean OK;
  LongInt HVal;
  char Ident;

  if ((strlen(OpPart )!= 2) || (*OpPart != 'D'))
    return False;
  Ident = OpPart[1];

  if ((Ident == 'B') || (Ident == 'W') || (Ident == 'D') || (Ident == 'Q') || (Ident == 'T'))
  {
    DSFlag = DSNone;
    switch (Ident)
    {
      case 'B':
        LayoutFunc = LayoutByte;
        if (*LabPart != '\0')
          SetSymbolSize(LabPart, 0);
        break;
      case 'W':
        LayoutFunc = LayoutWord;
        if (*LabPart != '\0')
          SetSymbolSize(LabPart, 1);
        break;
      case 'D':
        LayoutFunc = LayoutDoubleWord;
        if (*LabPart != '\0')
          SetSymbolSize(LabPart, 2);
        break;
      case 'Q':
        LayoutFunc = LayoutQuadWord;
        if (*LabPart != '\0')
          SetSymbolSize(LabPart, 3);
        break;
      case 'T':
        LayoutFunc = LayoutTenBytes;
        if (*LabPart != '\0')
          SetSymbolSize(LabPart, 4);
        break;
    }

    z = 1;
    do
    {
      if (!*ArgStr[z])
      {
        OK = FALSE;
        WrError(2050);
      }
      else
        OK = DecodeIntelPseudo_LayoutMult(ArgStr[z], &Dummy, LayoutFunc, Turn);
      if (!OK)
        CodeLen = 0;
      z++;
    }
    while ((OK) && (z <= ArgCnt));

    DontPrint = (DSFlag == DSSpace);
    if (DontPrint)
    {
      BookKeeping();
      if (!CodeLen) WrError(290);
    }
    if (OK)
      ActListGran = 1;
    return True;
  }

  if (Ident == 'S')
  {
    if (ChkArgCnt(1, 1))
    {
      FirstPassUnknown = False;
      HVal = EvalIntExpression(ArgStr[1], Int32, &OK);
      if (FirstPassUnknown) WrError(1820);
      else if (OK)
      {
        DontPrint = True;
        CodeLen = HVal;
        if (!HVal)
          WrError(290);
        BookKeeping();
      }
    }
    return True;
  }

  return False;
}
