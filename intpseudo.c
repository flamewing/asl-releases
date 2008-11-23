/* intpseudo.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Haeufiger benutzte Intel-Pseudo-Befehle                                   */
/*                                                                           */
/*****************************************************************************/
/* $Id: intpseudo.c,v 1.7 2008/11/23 10:39:17 alfred Exp $                   */
/***************************************************************************** 
 * $Log: intpseudo.c,v $
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

static enum{ DSNone, DSConstant, DSSpace} DSFlag;

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
    if (CodeLen + 2 > MaxCodeLen)
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

  FirstPassUnknown = False; EvalExpression(pExpr, &t);
  Result = False;
  switch (t.Typ)
  {
    case TempInt:
      if (FirstPassUnknown) t.Contents.Int &= 0xff;
      if (!RangeCheck(t.Contents.Int, Int8)) WrError(1320);
      else
      {
        BAsmCode[CodeLen++] = t.Contents.Int; *pCnt = 1;
        Result = True;
      }
      break;
    case TempFloat: 
      WrError(1135);
      break;
    case TempString:
      TranslateString(t.Contents.Ascii.Contents, t.Contents.Ascii.Length);
      *pCnt = t.Contents.Ascii.Length;
      if (CodeLen + (*pCnt) > MaxCodeLen) WrError(1920);
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

static Boolean LayoutWord(const char *pExpr, Word *pCnt, Boolean BigEndian)
{
  Boolean Result;
  Word erg;

  if (PreLayoutChk(pExpr, &Result, pCnt, 2))
    return Result;

  erg = EvalIntExpression(pExpr, Int16, &Result);
  if (Result)
  {
    if (BigEndian)
    {
      BAsmCode[CodeLen    ] = Hi(erg);
      BAsmCode[CodeLen + 1] = Lo(erg);
    }
    else
    {
      BAsmCode[CodeLen    ] = Lo(erg);
      BAsmCode[CodeLen + 1] = Hi(erg);
    }
    *pCnt = 2; CodeLen += 2;
  }
  return Result;
}

/*****************************************************************************
 * Function:    LayoutDoubleWord
 * Purpose:     parse argument, interprete as 32-bit word or 
                single precision float, and put into result buffer
 * Result:      TRUE if no errors occured
 *****************************************************************************/

static Boolean LayoutDoubleWord(const char *pExpr, Word *pCnt, Boolean BigEndian)
{
  TempResult erg;
  Boolean Result = False;
  String Copy;

  if (PreLayoutChk(pExpr, &Result, pCnt, 4))
   return Result;

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
       BAsmCode[CodeLen    ] = ((erg.Contents.Int      ) & 0xff);
       BAsmCode[CodeLen + 1] = ((erg.Contents.Int >>  8) & 0xff);
       BAsmCode[CodeLen + 2] = ((erg.Contents.Int >> 16) & 0xff);
       BAsmCode[CodeLen + 3] = ((erg.Contents.Int >> 24) & 0xff);
       Result = True;
     }
     break;
    case TempFloat:
     if (!FloatRangeCheck(erg.Contents.Float, Float32)) WrError(1320);
     else
     {
       Double_2_ieee4(erg.Contents.Float, BAsmCode+CodeLen, False);
       Result = True;
     }
     break;
    case TempString:
    case TempAll:
      WrError(1135);
  }

  if (Result)
  {
    *pCnt = 4;
    if (BigEndian)
      DSwap(BAsmCode + CodeLen, 4);
    CodeLen += 4;
  }

  return Result;
}


/*****************************************************************************
 * Function:    LayoutQuadWord
 * Purpose:     parse argument, interprete as 64-bit word or 
                double precision float, and put into result buffer
 * Result:      TRUE if no errors occured
 *****************************************************************************/

static Boolean LayoutQuadWord(const char *pExpr, Word *pCnt, Boolean BigEndian)
{
  Boolean Result;
  TempResult erg;
#ifndef HAS64
  int z;
#endif
  String Copy;

  if (PreLayoutChk(pExpr, &Result, pCnt, 8))
    return Result;

  CopyNoBlanks(Copy, pExpr, STRINGSIZE);
  EvalExpression(Copy, &erg);
  Result = False;
  switch(erg.Typ)
  {
    case TempNone:
      break;
    case TempInt:
      BAsmCode[CodeLen    ] = ((erg.Contents.Int      ) & 0xff);
      BAsmCode[CodeLen + 1] = ((erg.Contents.Int >>  8) & 0xff);
      BAsmCode[CodeLen + 2] = ((erg.Contents.Int >> 16) & 0xff);
      BAsmCode[CodeLen + 3] = ((erg.Contents.Int >> 24) & 0xff);
#ifndef HAS64
      for (z = 4; z < 8; z++)
        BAsmCode[CodeLen + z] = (BAsmCode[CodeLen + 3] & 0x80) ? 0xff : 0x00;
#endif
      Result = True;
      break;
    case TempFloat:
      Double_2_ieee8(erg.Contents.Float, BAsmCode + CodeLen, False);
      Result = True;
      break;
    case TempString:
    case TempAll:
      WrError(1135);
      return Result;
  }

  if (Result)
  {
    *pCnt = 8;
    if (BigEndian)
      QSwap(BAsmCode + CodeLen, 8);
    CodeLen += 8;
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
  Double erg;

  if (PreLayoutChk(pExpr, &Result, pCnt, 10))
    return Result;

  erg = EvalFloatExpression(pExpr, Float64, &Result);
  if (Result)
  {
    Double_2_ieee10(erg, BAsmCode + CodeLen, False);
    *pCnt = 10;
    if (BigEndian)
      TSwap(BAsmCode + CodeLen, 10);
    CodeLen += 10;
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
      if (CodeLen + SumCnt * (DupCnt - 1) > MaxCodeLen)
      {
        WrError(1920); return False;
      }
      for (z = 1; z <= DupCnt - 1; z++)
      {
        if (CodeLen + SumCnt > MaxCodeLen)
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
  TLayoutFunc LayoutFunc=Nil;
  Boolean OK;
  LongInt HVal;
  char Ident;

  if ((strlen(OpPart )!= 2) OR (*OpPart != 'D'))
    return False;
  Ident = OpPart[1];

  if ((Ident == 'B') OR (Ident == 'W') OR (Ident == 'D') OR (Ident == 'Q') OR (Ident == 'T'))
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
      if (NOT OK)
        CodeLen = 0;
      z++;
    }
    while ((OK) AND (z <= ArgCnt));

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
    if (ArgCnt != 1) WrError(1110);
    else
    {
      FirstPassUnknown = False;
      HVal = EvalIntExpression(ArgStr[1], Int32, &OK);
      if (FirstPassUnknown) WrError(1820);
      else if (OK)
      {
        DontPrint = True; CodeLen = HVal;
        if (!HVal) WrError(290);
        BookKeeping();
      }
    }
    return True;
  }

  return False;
}
