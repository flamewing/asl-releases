/* intpseudo.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* Commonly Used Intel-Style Pseudo Instructions                             */
/*                                                                           */
/*****************************************************************************/

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
                               const tStrComp *pArg, Word *Cnt, Boolean Turn
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
      WrError(ErrNum_MixDBDS);
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
    WrError(ErrNum_MixDBDS);
    *pResult = False;
    return True;
  }

  /* OK, type is fine so far.  See if we still have space left */

  else
  {
    DSFlag = DSConstant;
    if (SetMaxCodeLen(CodeLen + 2))
    {
      WrError(ErrNum_CodeOverflow);
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

static Boolean LayoutByte(const tStrComp *pExpr, Word *pCnt, Boolean BigEndian)
{
  Boolean Result;
  TempResult t;

  UNUSED(BigEndian);

  if (PreLayoutChk(pExpr->Str, &Result, pCnt, 1))
    return Result;

  /* PreLayoutChk() has checked for at least one byte free space
     in output buffer, so we only need to check again in case of
     a string argument: */

  FirstPassUnknown = False;
  EvalStrExpression(pExpr, &t);
  Result = False;
  switch (t.Typ)
  {
    case TempInt:
      if (FirstPassUnknown) t.Contents.Int &= 0xff;
      if (!SymbolQuestionable && !RangeCheck(t.Contents.Int, Int8)) WrError(ErrNum_OverRange);
      else
      {
        BAsmCode[CodeLen++] = t.Contents.Int;
        *pCnt = 1;
        Result = True;
      }
      break;
    case TempFloat: 
      WrError(ErrNum_InvOpType);
      break;
    case TempString:
      TranslateString(t.Contents.Ascii.Contents, t.Contents.Ascii.Length);
      *pCnt = t.Contents.Ascii.Length;
      if (SetMaxCodeLen(CodeLen + (*pCnt))) WrError(ErrNum_CodeOverflow);
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

static Boolean LayoutWord(const tStrComp *pExpr, Word *pCnt, Boolean BigEndian)
{
  Boolean Result;
  TempResult t;

  if (PreLayoutChk(pExpr->Str, &Result, pCnt, 2))
    return Result;

  /* PreLayoutChk() has checked for at least two bytes free space
     in output buffer, so we only need to check again in case of
     a string argument: */

  FirstPassUnknown = False;
  EvalStrExpression(pExpr, &t);
  Result = True; *pCnt = 0;
  switch (t.Typ)
  {
    case TempInt:
      if (FirstPassUnknown)
        t.Contents.Int &= 0xffff;
      if (!SymbolQuestionable && !RangeCheck(t.Contents.Int, Int16)) WrError(ErrNum_OverRange);
      else
      {
        PutWord(t.Contents.Int, BigEndian);
        *pCnt = 2;
        Result = True;
      }
      break;
    case TempFloat:
      WrError(ErrNum_InvOpType);
      break;
    case TempString:
      TranslateString(t.Contents.Ascii.Contents, t.Contents.Ascii.Length);
      *pCnt = t.Contents.Ascii.Length * 2;
      if (SetMaxCodeLen(CodeLen + (*pCnt))) WrError(ErrNum_CodeOverflow);
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

static Boolean LayoutDoubleWord(const tStrComp *pExpr, Word *pCnt, Boolean BigEndian)
{
  TempResult erg;
  Boolean Result = False;

  if (PreLayoutChk(pExpr->Str, &Result, pCnt, 4))
    return Result;

  /* PreLayoutChk() has checked for at least four bytes free space
     in output buffer, so we only need to check again in case of
     a string argument: */

  FirstPassUnknown = False;
  EvalStrExpression(pExpr, &erg);
  Result = False;
  switch (erg.Typ)
  {
    case TempNone:
      break;
    case TempInt:
      if (FirstPassUnknown)
        erg.Contents.Int &= 0xfffffffful;
      if (!SymbolQuestionable && !RangeCheck(erg.Contents.Int, Int32)) WrError(ErrNum_OverRange);
      else
      {
        PutDWord(erg.Contents.Int, BigEndian);
        *pCnt = 4;
        Result = True;
      }
      break;
    case TempFloat:
      if (!FloatRangeCheck(erg.Contents.Float, Float32)) WrError(ErrNum_OverRange);
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
      if (SetMaxCodeLen(CodeLen + (*pCnt))) WrError(ErrNum_CodeOverflow);
      else
      {   
        unsigned z;

        for (z = 0; z < erg.Contents.Ascii.Length; z++)
          PutDWord(erg.Contents.Ascii.Contents[z], BigEndian);
        Result = True;
      }
      break;
    case TempAll:
      WrError(ErrNum_InvOpType);
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

static Boolean LayoutQuadWord(const tStrComp *pExpr, Word *pCnt, Boolean BigEndian)
{
  Boolean Result;
  TempResult erg;

  if (PreLayoutChk(pExpr->Str, &Result, pCnt, 8))
    return Result;

  /* PreLayoutChk() has checked for at least eight bytes free space
     in output buffer, so we only need to check again in case of
     a string argument: */

  EvalStrExpression(pExpr, &erg);
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
      if (SetMaxCodeLen(CodeLen + (*pCnt))) WrError(ErrNum_CodeOverflow);
      else  
      {   
        unsigned z;

        for (z = 0; z < erg.Contents.Ascii.Length; z++)
          PutQWord(erg.Contents.Ascii.Contents[z], BigEndian);
        Result = True;
      }
      break;
    case TempAll:
      WrError(ErrNum_InvOpType);
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

static Boolean LayoutTenBytes(const tStrComp *pExpr, Word *pCnt, Boolean BigEndian)
{
  Boolean Result;
  TempResult erg;

  if (PreLayoutChk(pExpr->Str, &Result, pCnt, 10))
    return Result;

  /* PreLayoutChk() has checked for at least eight bytes free space
     in output buffer, so we only need to check again in case of
     a string argument: */

  EvalStrExpression(pExpr, &erg);
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
      if (SetMaxCodeLen(CodeLen + (*pCnt))) WrError(ErrNum_CodeOverflow);
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
      WrError(ErrNum_InvOpType);
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

static Boolean DecodeIntelPseudo_LayoutMult(const tStrComp *pArg, Word *Cnt,
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
  pDupFnd = NULL; Len = strlen(pArg->Str);
  for (pRun = pArg->Str; pRun < pArg->Str + Len - 2; pRun++)
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
    char *pSep, *pRun;
    Word SumCnt, ECnt, SInd;
    String CopyStr;
    tStrComp Copy, DupArg, RemArg, ThisRemArg;

    /* operate on copy */

    StrCompMkTemp(&Copy, CopyStr);
    StrCompCopy(&Copy, pArg);
    pSep = Copy.Str + (pDupFnd - pArg->Str);

    /* evaluate count */

    FirstPassUnknown = False;
    StrCompSplitRef(&DupArg, &RemArg, &Copy, pSep);
    DupCnt = EvalStrIntExpression(&DupArg, Int32, &OK);
    if (FirstPassUnknown)
    {
      WrError(ErrNum_FirstPassCalc); return False;
    }
    if (!OK)
      return False;

    /* catch invalid counts */

    if (DupCnt <= 0)
    {
      if (DupCnt < 0)
        WrStrErrorPos(ErrNum_NegDUP, &DupArg);
      return True;
    }

    /* split into parts and evaluate */

    StrCompIncRefLeft(&RemArg, 2);
    KillPrefBlanksStrCompRef(&RemArg);
    SumCnt = 0;
    Len = strlen(RemArg.Str); 
    if ((Len >= 2) && (*RemArg.Str == '(') && (RemArg.Str[Len - 1] == ')'))
    {
      StrCompIncRefLeft(&RemArg, 1);
      StrCompShorten(&RemArg, 1);
      Len -= 2;
    }
    do
    {
      pSep = NULL; Quote = Depth = 0;
      for (pRun = RemArg.Str; *pRun; pRun++)
      {
        DecodeIntelPseudo_HandleQuote(&Depth, &Quote, *pRun);
        if ((!Depth) && (!Quote) && (*pRun == ','))
        {
          pSep = pRun;
          break;
        }
      }
      if (pSep)
        StrCompSplitRef(&RemArg, &ThisRemArg, &RemArg, pSep);
      if (!DecodeIntelPseudo_LayoutMult(&RemArg, &ECnt, LayoutFunc, Turn))
        return False;
      SumCnt += ECnt;
      if (pSep)
        RemArg = ThisRemArg;
    }
    while (pSep);

    /* replicate result */

    if (DSFlag == DSConstant)
    {
      SInd = CodeLen - SumCnt;
      if (SetMaxCodeLen(CodeLen + SumCnt * (DupCnt - 1)))
      {
        WrError(ErrNum_CodeOverflow); return False;
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
  tStrComp *pArg;
  TLayoutFunc LayoutFunc = NULL;
  Boolean OK;
  LongInt HVal;
  char Ident;

  if ((strlen(OpPart.Str) != 2) || (*OpPart.Str != 'D'))
    return False;
  Ident = OpPart.Str[1];

  if ((Ident == 'B') || (Ident == 'W') || (Ident == 'D') || (Ident == 'Q') || (Ident == 'T'))
  {
    DSFlag = DSNone;
    switch (Ident)
    {
      case 'B':
        LayoutFunc = LayoutByte;
        if (*LabPart.Str)
          SetSymbolOrStructElemSize(&LabPart, eSymbolSize8Bit);
        break;
      case 'W':
        LayoutFunc = LayoutWord;
        if (*LabPart.Str)
          SetSymbolOrStructElemSize(&LabPart, eSymbolSize16Bit);
        break;
      case 'D':
        LayoutFunc = LayoutDoubleWord;
        if (*LabPart.Str)
          SetSymbolOrStructElemSize(&LabPart, eSymbolSize32Bit);
        break;
      case 'Q':
        LayoutFunc = LayoutQuadWord;
        if (*LabPart.Str)
          SetSymbolOrStructElemSize(&LabPart, eSymbolSize64Bit);
        break;
      case 'T':
        LayoutFunc = LayoutTenBytes;
        if (*LabPart.Str)
          SetSymbolOrStructElemSize(&LabPart, eSymbolSize80Bit);
        break;
    }

    OK = True;
    forallargs(pArg, OK)
    {
      if (!*pArg->Str)
      {
        OK = FALSE;
        WrError(ErrNum_EmptyArgument);
      }
      else
        OK = DecodeIntelPseudo_LayoutMult(pArg, &Dummy, LayoutFunc, Turn);
      if (!OK)
        CodeLen = 0;
    }

    DontPrint = (DSFlag == DSSpace);
    if (DontPrint)
    {
      BookKeeping();
      if (!CodeLen) WrError(ErrNum_NullResMem);
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
      HVal = EvalStrIntExpression(&ArgStr[1], Int32, &OK);
      if (FirstPassUnknown) WrError(ErrNum_FirstPassCalc);
      else if (OK)
      {
        DontPrint = True;
        CodeLen = HVal;
        if (!HVal)
          WrError(ErrNum_NullResMem);
        BookKeeping();
      }
    }
    return True;
  }

  return False;
}
