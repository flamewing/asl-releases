/* code16c8x.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* AS-Codegenerator PIC16C8x                                                 */
/*                                                                           */
/* Historie: 21.8.1996 Grundsteinlegung                                      */
/*            7. 7.1998 Fix Zugriffe auf CharTransTable wg. signed chars     */
/*           18. 8.1998 Bookkeeping-Aufruf bei RES                           */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*                      Sonderadressbereich PIC16C84                         */
/*           2. 10.1999 ChkPC wurde nicht angebunden...                      */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: code16c8x.c,v 1.8 2014/10/03 12:03:45 alfred Exp $                   */
/*****************************************************************************
 * $Log: code16c8x.c,v $
 * Revision 1.8  2014/10/03 12:03:45  alfred
 * - rework to current style
 *
 * Revision 1.7  2013/12/21 19:46:51  alfred
 * - dynamically resize code buffer
 *
 * Revision 1.6  2008/11/23 10:39:16  alfred
 * - allow strings with NUL characters
 *
 * Revision 1.5  2007/04/29 21:34:26  alfred
 * - add BANKSEL pseudo-instruction
 *
 * Revision 1.4  2006/08/05 18:07:55  alfred
 * - silence some warnings
 *
 * Revision 1.3  2005/09/08 16:53:39  alfred
 * - use common PInstTable
 *
 * Revision 1.2  2005/05/16 09:38:19  alfred
 * - major code cleanup, added types with 2K program space
 *
 * Revision 1.1  2003/11/06 02:49:19  alfred
 * - recreated
 *
 * Revision 1.2  2002/08/14 18:43:48  alfred
 * - warn null allocation, remove some warnings
 *
 *****************************************************************************/

#include "stdinc.h"

#include <string.h>

#include "chunks.h"
#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "headids.h"
#include "codepseudo.h"
#include "codevars.h"

/*---------------------------------------------------------------------------*/

#define AddCodeSpace 0x300

static CPUVar CPU16C64, CPU16C84, CPU16C873, CPU16C874, CPU16C876, CPU16C877;

/*--------------------------------------------------------------------------*/
/* helper functions */

static Word EvalFExpression(char *pAsc, Boolean *pOK)
{
  LongInt h;

  h = EvalIntExpression(pAsc, UInt9, pOK);
  if (*pOK)
  {
    ChkSpace(SegData);
    return (h & 0x7f);
  }
  else
    return 0;
}

/*--------------------------------------------------------------------------*/
/* instruction decoders */

static void DecodeFixed(Word Code)
{
  if (ArgCnt != 0) WrError(1110);
  else
  {
    WAsmCode[CodeLen++] = Code;
    if (Memo("OPTION"))
      WrError(130);
  }
}

static void DecodeLit(Word Code)
{
  Word AdrWord;
  Boolean OK;

  if (ArgCnt != 1) WrError(1110);
  else
  {
    AdrWord = EvalIntExpression(ArgStr[1], Int8, &OK);
    if (OK)
      WAsmCode[CodeLen++] = Code | Lo(AdrWord);
  }
}

static void DecodeAri(Word Code)
{
  Word DefaultDir = (Code >> 8) & 0x80;
  Boolean OK;
  Word AdrWord;

  Code &= 0x7fff;

  if ((ArgCnt == 0) || (ArgCnt > 2)) WrError(1110);
  else
  {
    AdrWord = EvalFExpression(ArgStr[1], &OK);
    if (OK)
    {
      WAsmCode[0] = Code | AdrWord;
      if (1 == ArgCnt)
      {
        WAsmCode[0] |= DefaultDir;
        CodeLen = 1;
      }
      else if (!strcasecmp(ArgStr[2], "W"))
        CodeLen = 1;
      else if (!strcasecmp(ArgStr[2], "F"))
      {
         WAsmCode[0] |= 0x80;
        CodeLen = 1;
      }
      else
      {
        AdrWord = EvalIntExpression(ArgStr[2], UInt1, &OK);
        if (OK)
        {
          WAsmCode[0] |= AdrWord << 7;
          CodeLen = 1;
        }
      }
    }
  }
}

static void DecodeBit(Word Code)
{
  Word AdrWord;
  Boolean OK;

  if (ArgCnt != 2) WrError(1110);
  else
  {
    AdrWord = EvalIntExpression(ArgStr[2], UInt3, &OK);
    if (OK)
    {
      WAsmCode[0] = EvalFExpression(ArgStr[1], &OK);
      if (OK)
      {
        WAsmCode[0] |= Code | (AdrWord << 7);
        CodeLen = 1;
      }
    }
  }
}

static void DecodeF(Word Code)
{
  Word AdrWord;
  Boolean OK;

  if (ArgCnt != 1) WrError(1110);
  else
  {
    AdrWord = EvalFExpression(ArgStr[1], &OK);
    if (OK)
      WAsmCode[CodeLen++] = Code | AdrWord;
  }
}

static void DecodeTRIS(Word Index)
{
  Word AdrWord;
  Boolean OK;
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    FirstPassUnknown = False;
    AdrWord=EvalIntExpression(ArgStr[1], UInt3, &OK);
    if (FirstPassUnknown)
      AdrWord = 5;
    if (OK)
      if (ChkRange(AdrWord, 5, 6))
      {
        WAsmCode[CodeLen++] = 0x0060 | AdrWord;
        ChkSpace(SegData); WrError(130);
      }
  }
}

static void DecodeJump(Word Index)
{
  Word AdrWord;
  Boolean OK;

  if (ArgCnt != 1) WrError(1110);
  else
  {
    AdrWord = EvalIntExpression(ArgStr[1], Int16, &OK);
    if (OK)
    {
      if (AdrWord > (SegLimits[SegCode] - AddCodeSpace)) WrError(1320);
      else
      {
        Word XORVal, Mask, RegBit;

        ChkSpace(SegCode);

        XORVal = (ProgCounter() ^ AdrWord) & ~0x7ff;

        /* add BCF/BSF instruction for non-matching upper address bits 
           - we might need to extend this for the PICs with more than
             8K of program space */

        for (RegBit = 3, Mask = 0x800; RegBit <= 4; RegBit++, Mask <<= 1)
          if (XORVal & Mask)
            WAsmCode[CodeLen++] = 0x100a
                                | (RegBit << 7)
                                | ((AdrWord & Mask) >> (RegBit - 2));

        WAsmCode[CodeLen++] = Index | (AdrWord & 0x7ff);
      }
    }
  }
}

static void DecodeSFR(Word Index)
{
  UNUSED(Index);

  CodeEquate(SegData, 0, 511);
}

static void DecodeRES(Word Index)
{
  Word Size;
  Boolean ValOK;

  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    FirstPassUnknown = False;
    Size = EvalIntExpression(ArgStr[1], Int16, &ValOK);
    if (FirstPassUnknown) WrError(1820);
    if ((ValOK) && (!FirstPassUnknown))
    {
      DontPrint = True;
      if (!Size) WrError(290);
      CodeLen = Size;
      BookKeeping();
    }
  }
}

static void DecodeDATA(Word Index)
{
  const LongInt MaxV = (ActPC == SegCode) ? 16383 : 255,
                MinV = (-((MaxV + 1) >> 1));
  Boolean ValOK;
  int z;
  TempResult t;

  UNUSED(Index);

  if (ArgCnt == 0) WrError(1110);
  else
  {
    ValOK = True;
    for (z = 1; z <= ArgCnt; z++)
    {
      FirstPassUnknown = False;
      EvalExpression(ArgStr[z], &t);
      if ((FirstPassUnknown) && (t.Typ==TempInt))
        t.Contents.Int &= MaxV;
      switch (t.Typ)
      {
        case TempInt:
          if (ChkRange(t.Contents.Int, MinV, MaxV))
          {
            if (ActPC == SegCode)
              WAsmCode[CodeLen++] = t.Contents.Int & MaxV;
            else
              BAsmCode[CodeLen++] = t.Contents.Int & MaxV;
          }
          break;
        case TempFloat:
          WrError(1135);
          ValOK = False; 
          break;
        case TempString:
        {
          unsigned z;

          for (z = 0; z < t.Contents.Ascii.Length; z++)
            if (ActPC == SegCode)
              WAsmCode[CodeLen++] = CharTransTable[((usint) t.Contents.Ascii.Contents[z]) & 0xff];
            else
              BAsmCode[CodeLen++] = CharTransTable[((usint) t.Contents.Ascii.Contents[z]) & 0xff];
          break;
        }
        default:
          ValOK = False;
      }

      if (!ValOK)
        break;
    }
    if (!ValOK)
      CodeLen = 0;
  }
}

static void DecodeZERO(Word Index)
{
  Word Size, Shift = (ActPC == SegCode) ? 1 : 0;
  Boolean ValOK;

  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    FirstPassUnknown = False;
    Size = EvalIntExpression(ArgStr[1], Int16, &ValOK);
    if (FirstPassUnknown) WrError(1820);
    if ((ValOK) && (!FirstPassUnknown))
    {
      if (SetMaxCodeLen(Size << Shift)) WrError(1920);
      else
      {
        CodeLen = Size;
        memset(WAsmCode, 0, Size << Shift);
      }
    }
  }
}

static void DecodeBANKSEL(Word Index)
{
  Word Adr;
  Boolean ValOK;

  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    Adr = EvalIntExpression(ArgStr[1], UInt9, &ValOK);
    if (ValOK)
    {
      WAsmCode[0] = 0x1283 | ((Adr &  0x80) << 3); /* BxF Status, 5 */
      WAsmCode[1] = 0x1303 | ((Adr & 0x100) << 2); /* BxF Status, 6 */
      CodeLen = 2;
    }
  }
}

/*--------------------------------------------------------------------------*/
/* dynamic code table handling */

static void AddFixed(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeFixed);
}

static void AddLit(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeLit);
}

static void AddAri(char *NName, Word NCode, Word NDir)
{
  AddInstTable(InstTable, NName, NCode | (NDir << 15), DecodeAri);
}

static void AddBit(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeBit);
}

static void AddF(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeF);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(201);

  AddFixed("CLRW"  , 0x0100);
  AddFixed("NOP"   , 0x0000);
  AddFixed("CLRWDT", 0x0064);
  AddFixed("OPTION", 0x0062);
  AddFixed("SLEEP" , 0x0063);
  AddFixed("RETFIE", 0x0009);
  AddFixed("RETURN", 0x0008);
 
  AddLit("ADDLW", 0x3e00);
  AddLit("ANDLW", 0x3900);
  AddLit("IORLW", 0x3800);
  AddLit("MOVLW", 0x3000);
  AddLit("RETLW", 0x3400);
  AddLit("SUBLW", 0x3c00);
  AddLit("XORLW", 0x3a00);

  AddAri("ADDWF" , 0x0700, 0);
  AddAri("ANDWF" , 0x0500, 0);
  AddAri("COMF"  , 0x0900, 1);
  AddAri("DECF"  , 0x0300, 1);
  AddAri("DECFSZ", 0x0b00, 1);
  AddAri("INCF"  , 0x0a00, 1);
  AddAri("INCFSZ", 0x0f00, 1);
  AddAri("IORWF" , 0x0400, 0);
  AddAri("MOVF"  , 0x0800, 0);
  AddAri("RLF"   , 0x0d00, 1);
  AddAri("RRF"   , 0x0c00, 1);
  AddAri("SUBWF" , 0x0200, 0);
  AddAri("SWAPF" , 0x0e00, 1);
  AddAri("XORWF" , 0x0600, 0);

  AddBit("BCF"  , 0x1000);
  AddBit("BSF"  , 0x1400);
  AddBit("BTFSC", 0x1800);
  AddBit("BTFSS", 0x1c00);

  AddF("CLRF" , 0x0180);
  AddF("MOVWF", 0x0080);

  AddInstTable(InstTable, "TRIS", 0, DecodeTRIS);
  AddInstTable(InstTable, "GOTO", 0x2800, DecodeJump);
  AddInstTable(InstTable, "CALL", 0x2000, DecodeJump);
  AddInstTable(InstTable, "SFR" , 0, DecodeSFR);
  AddInstTable(InstTable, "RES" , 0, DecodeRES);
  AddInstTable(InstTable, "DATA", 0, DecodeDATA);
  AddInstTable(InstTable, "ZERO", 0, DecodeZERO);

  AddInstTable(InstTable, "BANKSEL", 0, DecodeBANKSEL);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*--------------------------------------------------------------------------*/

static void MakeCode_16c8x(void)
{
  CodeLen = 0; DontPrint = False;

  /* zu ignorierendes */

  if (Memo("")) return;

  /* seek instruction */

  if (!LookupInstTable(InstTable, OpPart))
    WrXError(1200,OpPart);
}

static Boolean IsDef_16c8x(void)
{
  return (Memo("SFR"));
}

static Boolean ChkPC_16c8x(LargeWord Addr)
{
  if ((ActPC == SegCode) && (Addr > (LargeWord)SegLimits[SegCode]))
    return ((Addr >= 0x2000) && (Addr <= 0x2007));
  else
    return (Addr <= (LargeWord)SegLimits[ActPC]);
}

static void SwitchFrom_16c8x(void)
{
   DeinitFields();
}

static void SwitchTo_16c8x(void)
{
  PFamilyDescr pDescr;

  TurnWords = False;
  ConstMode = ConstModeMoto;
  SetIsOccupied = False;

  pDescr = FindFamilyByName("16C8x");
  PCSymbol = "*";
  HeaderID = pDescr->Id;
  NOPCode = 0x0000;
  DivideChars = ",";
  HasAttrs = False;

  ValidSegs = (1 << SegCode) | (1 << SegData);
  Grans[SegCode] = 2; ListGrans[SegCode] = 2; SegInits[SegCode] = 0;
  if (MomCPU == CPU16C64)
    SegLimits[SegCode] = 0x7ff;
  else if (MomCPU == CPU16C873)
    SegLimits[SegCode] = 0x0fff;
  else if (MomCPU == CPU16C874)
    SegLimits[SegCode] = 0x0fff;
  else if (MomCPU == CPU16C876)
    SegLimits[SegCode] = 0x1fff;
  else if (MomCPU == CPU16C877)
    SegLimits[SegCode] = 0x1fff;
  else
    SegLimits[SegCode] = 0x3ff;

  SegLimits[SegCode] += AddCodeSpace;
  Grans[SegData] = 1; ListGrans[SegData] = 1; SegInits[SegData] = 0;
  SegLimits[SegData] = 0x1ff;
  ChkPC = ChkPC_16c8x;

  MakeCode = MakeCode_16c8x;
  IsDef = IsDef_16c8x;
  SwitchFrom = SwitchFrom_16c8x;
  InitFields();
}

void code16c8x_init(void)
{
  CPU16C64  = AddCPU("16C64",  SwitchTo_16c8x);
  CPU16C84  = AddCPU("16C84",  SwitchTo_16c8x);
  CPU16C873 = AddCPU("16C873", SwitchTo_16c8x);
  CPU16C874 = AddCPU("16C874", SwitchTo_16c8x);
  CPU16C876 = AddCPU("16C876", SwitchTo_16c8x);
  CPU16C877 = AddCPU("16C877", SwitchTo_16c8x);
}
