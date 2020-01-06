/* codeavr.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Atmel AVR                                                   */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <ctype.h>
#include <string.h>

#include "bpemu.h"
#include "nls.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "codevars.h"
#include "errmsg.h"

#include "codeavr.h"

#define FixedOrderCnt 27
#define Reg1OrderCnt 10
#define Reg2OrderCnt 12

#define RegBankSize 32
#define IOAreaStdSize 64
#define IOAreaExtSize (IOAreaStdSize + 160)
#define IOAreaExt2Size (IOAreaExtSize + 256)

#define BitFlag_Data 0x800000ul
#define BitFlag_IO 0x400000ul

typedef enum
{
  eCoreNone,
  eCoreMinTiny,
  eCore90S1200,
  eCoreClassic, /* AT90Sxxxx */
  eCoreTiny, /* ATtiny up to 8KB flash */
  eCoreTiny16K,
  eCoreMega
} tCPUCore;

#define MinCoreMask(c) ((Word)(0xffffu << (c)))

typedef struct
{
  Word Code;
  Word CoreMask;
} FixedOrder;

typedef struct
{
  const char *pName;
  LongWord FlashEnd;
  Word RAMSize, IOAreaSize;
  Boolean RegistersMapped;
  tCPUCore Core;
} tCPUProps;

static FixedOrder *FixedOrders, *Reg1Orders, *Reg2Orders;

static Boolean WrapFlag;
static LongInt ORMask, SignMask;
static const tCPUProps *pCurrCPUProps;

static IntType CodeAdrIntType,
               DataAdrIntType;

static const char *WrapFlagName = "WRAPMODE";

/*---------------------------------------------------------------------------*/

static LongInt CutAdr(LongInt Adr)
{
  if ((Adr & SignMask) != 0)
    return (Adr | ORMask);
  else
    return (Adr & SegLimits[SegCode]);
}

static Boolean ChkMinCore(tCPUCore MinCore)
{
  if (pCurrCPUProps->Core < MinCore)
  {
    WrError(ErrNum_InstructionNotSupported);
    return False;
  }
  return True;
}

static Boolean ChkCoreMask(Word CoreMask)
{
  if ((1 << pCurrCPUProps->Core) & CoreMask)
    return True;
  WrError(ErrNum_InstructionNotSupported);
  return False;
}

static void DissectBit_AVR(char *pDest, int DestSize, LargeWord Inp)
{
  LongWord BitSpec = Inp;

  as_snprintf(pDest, DestSize, "0x%0*x(%c).%d",
              (BitSpec & BitFlag_IO) ? 2 : 3,
              (unsigned)((BitSpec >> 3) & 0xffff),
              (BitSpec & BitFlag_Data) ? SegShorts[SegData]
                                   : ((BitSpec & BitFlag_IO) ? SegShorts[SegIO] : SegShorts[SegNone]),
              (int)(BitSpec & 7));
}

/*---------------------------------------------------------------------------*/
/* argument decoders                                                         */

static Boolean DecodeReg(char *Asc, Word *Erg)
{
  Boolean io;
  char *s;
  int l;

  if (FindRegDef(Asc, &s))
    Asc = s;
  l = strlen(Asc);

  if ((l < 2) || (l > 3) || (mytoupper(*Asc) != 'R')) return False;
  else
  {
    *Erg = ConstLongInt(Asc + 1, &io, 10);
    return ((io)
         && ((*Erg >= 16) || (pCurrCPUProps->Core != eCoreMinTiny))
         && (*Erg < 32));
  }
}

static Boolean DecodeMem(char * Asc, Word *Erg)
{
  if (as_strcasecmp(Asc, "X") == 0) *Erg = 0x1c;
  else if (as_strcasecmp(Asc, "X+") == 0) *Erg = 0x1d;
  else if (as_strcasecmp(Asc, "-X") == 0) *Erg = 0x1e;
  else if (as_strcasecmp(Asc, "Y" ) == 0) *Erg = 0x08;
  else if (as_strcasecmp(Asc, "Y+") == 0) *Erg = 0x19;
  else if (as_strcasecmp(Asc, "-Y") == 0) *Erg = 0x1a;
  else if (as_strcasecmp(Asc, "Z" ) == 0) *Erg = 0x00;
  else if (as_strcasecmp(Asc, "Z+") == 0) *Erg = 0x11;
  else if (as_strcasecmp(Asc, "-Z") == 0) *Erg = 0x12;
  else return False;
  return True;
}

static Boolean DecodeBitArg2(const tStrComp *pRegArg, const tStrComp *pBitArg, LongWord *pResult)
{
  Boolean OK;
  LongWord Addr;

  *pResult = EvalStrIntExpression(pBitArg, UInt3, &OK);
  if (!OK)
    return False;

  FirstPassUnknown = False;
  Addr = EvalStrIntExpression(pRegArg, DataAdrIntType, &OK);
  if (!OK)
    return False;

  if (TypeFlag & (1 << SegIO))
  {
    if (!FirstPassUnknown && !ChkRange(Addr, 0, IOAreaStdSize - 1))
      return False;
    *pResult |= BitFlag_IO | (Addr & 0x3f) << 3;
    return True;
  }
  else
  {
    ChkSpace(SegData);

    if (!FirstPassUnknown && !ChkRange(Addr, 0, SegLimits[SegData]))
      return False;
    *pResult |= ((TypeFlag & (1 << SegData)) ? BitFlag_Data : 0) | (Addr & 0x1ff) << 3;
    return True;
  }
}

static Boolean DecodeBitArg(int Start, int Stop, LongWord *pResult)
{
  if (Start == Stop)
  {
    char *pPos = QuotPos(ArgStr[Start].Str, '.');
    Boolean OK;

    if (pPos)
    {
      tStrComp RegArg, BitArg;

      StrCompSplitRef(&RegArg, &BitArg, &ArgStr[Start], pPos);
      return DecodeBitArg2(&RegArg, &BitArg, pResult);
    }
    *pResult = EvalStrIntExpression(&ArgStr[Start], UInt16, &OK);
    if (OK)
      ChkSpace(SegBData);
    return OK;
  }
  else if (Stop == Start + 1)
    return DecodeBitArg2(&ArgStr[Start], &ArgStr[Stop], pResult);
  else
  {
    WrError(ErrNum_WrongArgCnt);
    return False;
  }
}

static const LongWord
       AllRegMask = 0xfffffffful,
       UpperHalfRegMask = 0xffff0000ul,
       Reg16_23Mask = 0x00ff0000ul,
       EvenRegMask = 0x55555555ul,
       UpperEightEvenRegMask = 0x55000000ul;

static Boolean DecodeArgReg(unsigned ArgIndex, Word *pReg, LongWord RegMask)
{
  Boolean Result;

  Result = DecodeReg(ArgStr[ArgIndex].Str, pReg)
        && ((RegMask >> *pReg) & 1);
  if (!Result)
    WrStrErrorPos(ErrNum_InvReg, &ArgStr[ArgIndex]);
  return Result;
}

/*---------------------------------------------------------------------------*/
/* individual decoders                                                       */

/* pseudo instructions */

static void DecodePORT(Word Index)
{
  UNUSED(Index);

  CodeEquate(SegIO, 0, 0x3f);
}

static void DecodeSFR(Word Index)
{
  LargeWord Start = (pCurrCPUProps->RegistersMapped ? RegBankSize : 0);
  UNUSED(Index);

  CodeEquate(SegData, Start, Start + pCurrCPUProps->IOAreaSize);
}

/* no argument */

static void DecodeFixed(Word Index)
{
  const FixedOrder *pOrder = FixedOrders + Index;

  if (ChkArgCnt(0, 0) && ChkCoreMask(pOrder->CoreMask))
  {
    WAsmCode[0] = pOrder->Code;
    CodeLen = 1;
  }
}

static void DecodeRES(Word Index)
{
  Boolean OK;
  Integer Size;

  UNUSED(Index);

  FirstPassUnknown = False;
  Size = EvalStrIntExpression(&ArgStr[1], Int16, &OK);
  if (FirstPassUnknown) WrError(ErrNum_FirstPassCalc);
  if ((OK) && (!FirstPassUnknown))
  {
    DontPrint = True;
    if (!Size) WrError(ErrNum_NullResMem);
    CodeLen = Size;
    BookKeeping();
  }
}

static Boolean AccFull;

static void PlaceByte(Word Value, Boolean Pack)
{
  if (ActPC == SegCode)
  {
    if (Pack)
    {
      Value &= 0xff;
      if (AccFull)
        WAsmCode[CodeLen - 1] |= (Value << 8);
      else
       WAsmCode[CodeLen++] = Value;
      AccFull = !AccFull;
    }
    else
    {
      WAsmCode[CodeLen++] = Value;
      AccFull = FALSE;
    }
  }
  else
    BAsmCode[CodeLen++] = Value;
}

static void DecodeDATA_AVR(Word Index)
{
  Integer Trans;
  int z, z2;
  Boolean OK;
  TempResult t;
  LongInt MinV, MaxV;

  UNUSED(Index);

  MaxV = ((ActPC == SegCode) && (!Packing)) ? 65535 : 255;
  MinV = (-((MaxV + 1) >> 1));
  AccFull = FALSE;
  if (ChkArgCnt(1, ArgCntMax))
  {
    OK = True;
    for (z = 1; z <= ArgCnt; z++)
     if (OK)
     {
       EvalStrExpression(&ArgStr[z], &t);
       if ((FirstPassUnknown) && (t.Typ == TempInt)) t.Contents.Int &= MaxV;
       switch (t.Typ)
       {
         case TempInt:
           if (ChkRange(t.Contents.Int, MinV, MaxV))
             PlaceByte(t.Contents.Int, Packing);
           break;
         case TempString:
           for (z2 = 0; z2 < (int)t.Contents.Ascii.Length; z2++)
           {
             Trans = CharTransTable[((usint) t.Contents.Ascii.Contents[z2]) & 0xff];
             PlaceByte(Trans, TRUE);
           }
           break;
         case TempFloat:
           WrStrErrorPos(ErrNum_StringOrIntButFloat, &ArgStr[z]);
           /* fall-through */
         default:
           OK = False;
       }
     }
    if (!OK)
       CodeLen = 0;
  }
}

static void DecodeREG(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(1, 1))
    AddRegDef(LabPart.Str,ArgStr[1].Str);
}

/* one register 0..31 */

static void DecodeReg1(Word Index)
{
  const FixedOrder *pOrder = Reg1Orders + Index;
  Word Reg;

  if (ChkArgCnt(1, 1)
   && ChkCoreMask(pOrder->CoreMask)
   && DecodeArgReg(1, &Reg, AllRegMask))
  {
    WAsmCode[0] = pOrder->Code | (Reg << 4);
    CodeLen = 1;
  }
}

/* two registers 0..31 */

static void DecodeReg2(Word Index)
{
  const FixedOrder *pOrder = Reg2Orders + Index;
  Word Reg1, Reg2;

  if (ChkArgCnt(2, 2)
   && ChkCoreMask(pOrder->CoreMask)
   && DecodeArgReg(1, &Reg1, AllRegMask)
   && DecodeArgReg(2, &Reg2, AllRegMask))
  {
    WAsmCode[0] = pOrder->Code | (Reg2 & 15) | (Reg1 << 4) | ((Reg2 & 16) << 5);
    CodeLen = 1;
  }
}

/* one register 0..31 with itself */

static void DecodeReg3(Word Code)
{
  Word Reg;

  if (ChkArgCnt(1, 1) && DecodeArgReg(1, &Reg, AllRegMask))
  {
    WAsmCode[0] = Code | (Reg & 15) | (Reg << 4) | ((Reg & 16) << 5);
    CodeLen = 1;
  }
}

/* immediate with register */

static void DecodeImm(Word Code)
{
  Word Reg, Const;
  Boolean OK;

  if (ChkArgCnt(2, 2) && DecodeArgReg(1, &Reg, UpperHalfRegMask))
  {
    Const = EvalStrIntExpression(&ArgStr[2], Int8, &OK);
    if (OK)
    {
      WAsmCode[0] = Code | ((Const & 0xf0) << 4) | (Const & 0x0f) | ((Reg & 0x0f) << 4);
      CodeLen = 1;
    }
  }
}

static void DecodeADIW(Word Index)
{
  Word Reg, Const;
  Boolean OK;

  if (ChkArgCnt(2, 2)
   && ChkMinCore(eCoreClassic)
   && DecodeArgReg(1, &Reg, UpperEightEvenRegMask))
  {
    Const = EvalStrIntExpression(&ArgStr[2], UInt6, &OK);
    if (OK)
    {
      WAsmCode[0] = 0x9600 | Index | ((Reg & 6) << 3) | (Const & 15) | ((Const & 0x30) << 2);
      CodeLen = 1;
    }
  }
}

/* transfer operations */

static void DecodeLDST(Word Index)
{
  int RegI, MemI;
  Word Reg, Mem;

  if (ChkArgCnt(2, 2))
  {
    RegI = Index ? 2 : 1; /* ST */
    MemI = 3 - RegI;
    if (!DecodeArgReg(RegI, &Reg, AllRegMask));
    else if (!DecodeMem(ArgStr[MemI].Str, &Mem)) WrError(ErrNum_InvAddrMode);
    else if ((pCurrCPUProps->Core == eCore90S1200) && (Mem != 0)) WrError(ErrNum_MustBeEven);
    else
    {
      WAsmCode[0] = 0x8000 | Index | (Reg << 4) | (Mem & 0x0f) | ((Mem & 0x10) << 8);
      CodeLen = 1;
      if (((Mem >= 0x1d) && (Mem <= 0x1e) && (Reg >= 26) && (Reg <= 27))  /* X+/-X with X */
       || ((Mem >= 0x19) && (Mem <= 0x1a) && (Reg >= 28) && (Reg <= 29))  /* Y+/-Y with Y */
       || ((Mem >= 0x11) && (Mem <= 0x12) && (Reg >= 30) && (Reg <= 31))) /* Z+/-Z with Z */
        WrError(ErrNum_Unpredictable);
    }
  }
}

static void DecodeLDDSTD(Word Index)
{
  int RegI, MemI;
  Word Reg, Disp;
  Boolean OK;

  if (ChkArgCnt(2, 2)
   && ChkMinCore(eCoreClassic))
  {
    char RegChar;

    RegI = Index ? 2 : 1; /* STD */
    MemI = 3 - RegI;
    RegChar = *ArgStr[MemI].Str;
    OK = True;
    if (mytoupper(RegChar) == 'Y') Index += 8;
    else if (mytoupper(RegChar) == 'Z');
    else OK = False;
    if (!OK) WrError(ErrNum_InvAddrMode);
    else if (DecodeArgReg(RegI, &Reg, AllRegMask))
    {
      *ArgStr[MemI].Str = '0';
      Disp = EvalStrIntExpression(&ArgStr[MemI], UInt6, &OK);
      *ArgStr[MemI].Str = RegChar;
      if (OK)
      {
        WAsmCode[0] = 0x8000 | Index | (Reg << 4) | (Disp & 7) | ((Disp & 0x18) << 7) | ((Disp & 0x20) << 8);
        CodeLen = 1;
      }
    }
  }
}

static void DecodeINOUT(Word Index)
{
  int RegI, MemI;
  Word Reg, Mem;
  Boolean OK;

  if (ChkArgCnt(2, 2))
  {
    RegI = Index ? 2 : 1; /* OUT */
    MemI = 3 - RegI;
    if (DecodeArgReg(RegI, &Reg, AllRegMask))
    {
      Mem = EvalStrIntExpression(&ArgStr[MemI], UInt6, &OK);
      if (OK)
      {
        ChkSpace(SegIO);
        WAsmCode[0] = 0xb000 | Index | (Reg << 4) | (Mem & 0x0f) | ((Mem & 0xf0) << 5);
        CodeLen = 1;
      }
    }
  }
}

static void DecodeLDSSTS(Word Index)
{
  int RegI, MemI;
  Word Reg;
  Boolean OK;

  if (ChkArgCnt(2, 2)
   && ChkCoreMask(MinCoreMask(eCoreClassic) | (1 << eCoreMinTiny)))
  {
    RegI = Index ? 2 : 1; /* STS */
    MemI = 3 - RegI;
    if (DecodeArgReg(RegI, &Reg, AllRegMask))
    {
      WAsmCode[1] = EvalStrIntExpression(&ArgStr[MemI], UInt16, &OK);
      if (OK)
      {
        ChkSpace(SegData);
        WAsmCode[0] = 0x9000 | Index | (Reg << 4);
        CodeLen = 2;
      }
    }
  }
}

/* bit operations */

static void DecodeBCLRSET(Word Index)
{
  Word Bit;
  Boolean OK;

  if (ChkArgCnt(1, 1))
  {
    Bit = EvalStrIntExpression(&ArgStr[1], UInt3, &OK);
    if (OK)
    {
      WAsmCode[0] = 0x9408 | (Bit << 4) | Index;
      CodeLen = 1;
    }
  }
}

static void DecodeBit(Word Code)
{
  Word Reg, Bit;
  Boolean OK;

  if (!ChkArgCnt(2, 2));
  else if (DecodeArgReg(1, &Reg, AllRegMask))
  {
    Bit = EvalStrIntExpression(&ArgStr[2], UInt3, &OK);
    if (OK)
    {
      WAsmCode[0] = Code | (Reg << 4) | Bit;
      CodeLen = 1;
    }
  }
}

static void DecodeCBR(Word Index)
{
  Word Reg, Mask;
  Boolean OK;

  UNUSED(Index);

  if (ChkArgCnt(2, 2) && DecodeArgReg(1, &Reg, UpperHalfRegMask))
  {
    Mask = EvalStrIntExpression(&ArgStr[2], Int8, &OK) ^ 0xff;
    if (OK)
    {
      WAsmCode[0] = 0x7000 | ((Mask & 0xf0) << 4) | (Mask & 0x0f) | ((Reg & 0x0f) << 4);
      CodeLen = 1;
    }
  }
}

static void DecodeSER(Word Index)
{
  Word Reg;

  UNUSED(Index);

  if (ChkArgCnt(1, 1) && DecodeArgReg(1, &Reg, UpperHalfRegMask))
  {
    WAsmCode[0] = 0xef0f | ((Reg & 0x0f) << 4);
    CodeLen = 1;
  }
}

static void DecodePBit(Word Code)
{
  LongWord BitSpec;

  if (DecodeBitArg(1, ArgCnt, &BitSpec))
  {
    Word Bit = BitSpec & 7,
         Adr = (BitSpec >> 3) & 0xffff;

    if (BitSpec & BitFlag_Data) WrError(ErrNum_WrongSegment);
    if (ChkRange(Adr, 0, 31))
    {
      WAsmCode[0] = Code | Bit | (Adr << 3);
      CodeLen = 1;
    }
  }
}

/* branches */

static void DecodeRel(Word Code)
{
  LongInt AdrInt;
  Boolean OK;

  if (ChkArgCnt(1, 1))
  {
    AdrInt = EvalStrIntExpression(&ArgStr[1], CodeAdrIntType, &OK) - (EProgCounter() + 1);
    if (OK)
    {
      if (WrapFlag) AdrInt = CutAdr(AdrInt);
      if ((!SymbolQuestionable) && ((AdrInt < -64) || (AdrInt > 63))) WrError(ErrNum_JmpDistTooBig);
      else
      {
        ChkSpace(SegCode);
        WAsmCode[0] = Code | ((AdrInt & 0x7f) << 3);
        CodeLen = 1;
      }
    }
  }
}

static void DecodeBRBSBC(Word Index)
{
  Word Bit;
  LongInt AdrInt;
  Boolean OK;

  if (ChkArgCnt(2, 2))
  {
    Bit = EvalStrIntExpression(&ArgStr[1], UInt3, &OK);
    if (OK)
    {
      AdrInt = EvalStrIntExpression(&ArgStr[2], CodeAdrIntType, &OK) - (EProgCounter() + 1);
      if (OK)
      {
        if (WrapFlag) AdrInt = CutAdr(AdrInt);
        if ((!SymbolQuestionable) && ((AdrInt < -64) || (AdrInt > 63))) WrError(ErrNum_JmpDistTooBig);
        else
        {
          ChkSpace(SegCode);
          WAsmCode[0] = 0xf000 | Index | ((AdrInt & 0x7f) << 3) | Bit;
          CodeLen = 1;
        }
      }
    }
  }
}

static void DecodeJMPCALL(Word Index)
{
  LongInt AdrInt;
  Boolean OK;

  if (ChkArgCnt(1, 1)
   && ChkMinCore(eCoreTiny16K))
  {
    AdrInt = EvalStrIntExpression(&ArgStr[1], UInt22, &OK);
    if (OK)
    {
      ChkSpace(SegCode);
      WAsmCode[0] = 0x940c | Index | ((AdrInt & 0x3e0000) >> 13) | ((AdrInt & 0x10000) >> 16);
      WAsmCode[1] = AdrInt & 0xffff;
      CodeLen = 2;
    }
  }
}

static void DecodeRJMPCALL(Word Index)
{
  LongInt AdrInt;
  Boolean OK;

  if (ChkArgCnt(1, 1))
  {
    AdrInt = EvalStrIntExpression(&ArgStr[1], UInt22, &OK) - (EProgCounter() + 1);
    if (OK)
    {
      if (WrapFlag) AdrInt = CutAdr(AdrInt);
      if ((!SymbolQuestionable) && ((AdrInt < -2048) || (AdrInt > 2047))) WrError(ErrNum_JmpDistTooBig);
      else
      {
        ChkSpace(SegCode);
        WAsmCode[0] = 0xc000 | Index | (AdrInt & 0xfff);
        CodeLen = 1;
      }
    }
  }
}

static void DecodeMULS(Word Index)
{
  Word Reg1, Reg2;

  UNUSED(Index);

  if (ChkArgCnt(2, 2)
   && ChkMinCore(eCoreMega)
   && DecodeArgReg(1, &Reg1, UpperHalfRegMask)
   && DecodeArgReg(2, &Reg2, UpperHalfRegMask))
  {
    WAsmCode[0] = 0x0200 | ((Reg1 & 15) << 4) | (Reg2 & 15);
    CodeLen = 1;
  }
}

static void DecodeMegaMUL(Word Index)
{
  Word Reg1, Reg2;

  if (ChkArgCnt(2, 2)
   && ChkMinCore(eCoreMega)
   && DecodeArgReg(1, &Reg1, Reg16_23Mask)
   && DecodeArgReg(2, &Reg2, Reg16_23Mask))
  {
    WAsmCode[0] = Index | ((Reg1 & 7) << 4) | (Reg2 & 7);
    CodeLen = 1;
  }
}

static void DecodeMOVW(Word Index)
{
  Word Reg1, Reg2;

  UNUSED(Index);

  if (ChkArgCnt(2, 2)
   && ChkMinCore(eCoreTiny)
   && DecodeArgReg(1, &Reg1, EvenRegMask)
   && DecodeArgReg(2, &Reg2, EvenRegMask))
  {
    WAsmCode[0] = 0x0100 | ((Reg1 >> 1) << 4) | (Reg2 >> 1);
    CodeLen = 1;
  }
}

static void DecodeLPM(Word Index)
{
  Word Reg, Adr;

  UNUSED(Index);

  if (!ArgCnt)
  {
    if (ChkMinCore(eCoreClassic))
    {
      WAsmCode[0] = 0x95c8;
      CodeLen = 1;
    }
  }
  else if (ArgCnt == 2)
  {
    if (!ChkMinCore(eCoreTiny));
    else if (!DecodeArgReg(1, &Reg, AllRegMask));
    else if (!DecodeMem(ArgStr[2].Str, &Adr)) WrError(ErrNum_InvAddrMode);
    else if ((Adr != 0x00) && (Adr != 0x11)) WrError(ErrNum_InvAddrMode);
    else
    {
      if (((Reg == 30) || (Reg == 31)) && (Adr == 0x11)) WrError(ErrNum_Unpredictable);
      WAsmCode[0] = 0x9004 | (Reg << 4) | (Adr & 1);
      CodeLen = 1;
    }
  }
  else
    (void)ChkArgCnt(2, 2);
}

static void DecodeELPM(Word Index)
{
  Word Reg, Adr;

  UNUSED(Index);

  if (!ChkMinCore(eCoreMega));
  else if (!ArgCnt)
  {
    WAsmCode[0] = 0x95d8;
    CodeLen = 1;
  }
  else if (!ChkArgCnt(2, 2));
  else if (!DecodeArgReg(1, &Reg, AllRegMask));
  else if (!DecodeMem(ArgStr[2].Str, &Adr)) WrError(ErrNum_InvAddrMode);
  else if ((Adr != 0x00) && (Adr != 0x11)) WrError(ErrNum_InvAddrMode);
  else
  {
    if (((Reg == 30) || (Reg == 31)) && (Adr == 0x11)) WrError(ErrNum_Unpredictable);
    WAsmCode[0] = 0x9006 | (Reg << 4) | (Adr & 1);
    CodeLen = 1;
  }
}

static void DecodeBIT(Word Code)
{
  LongWord BitSpec;

  UNUSED(Code);

  if (DecodeBitArg(1, ArgCnt, &BitSpec))
  {
    *ListLine = '=';
    DissectBit_AVR(ListLine + 1, STRINGSIZE - 3, BitSpec);
    PushLocHandle(-1);
    EnterIntSymbol(&LabPart, BitSpec, SegBData, False);
    PopLocHandle();
    if (MakeUseList)
    {
      if (AddChunk(SegChunks + SegBData, BitSpec, 1, False))
        WrError(ErrNum_Overlap);
    }
  }
}

/*---------------------------------------------------------------------------*/
/* dynamic code table handling                                               */

static void AddFixed(char *NName, Word NMin, Word NCode)
{
  if (InstrZ >= FixedOrderCnt) exit(255);
  FixedOrders[InstrZ].Code = NCode;
  FixedOrders[InstrZ].CoreMask = NMin;
  AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
}

static void AddReg1(char *NName, Word NMin, Word NCode)
{
  if (InstrZ >= Reg1OrderCnt) exit(255);
  Reg1Orders[InstrZ].Code = NCode;
  Reg1Orders[InstrZ].CoreMask = NMin;
  AddInstTable(InstTable, NName, InstrZ++, DecodeReg1);
}

static void AddReg2(char *NName, Word NMin, Word NCode)
{
  if (InstrZ >= Reg2OrderCnt) exit(255);
  Reg2Orders[InstrZ].Code = NCode;
  Reg2Orders[InstrZ].CoreMask = NMin;
  AddInstTable(InstTable, NName, InstrZ++, DecodeReg2);
}

static void AddReg3(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeReg3);
}

static void AddImm(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeImm);
}

static void AddRel(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeRel);
}

static void AddBit(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeBit);
}

static void AddPBit(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodePBit);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(203);

  FixedOrders = (FixedOrder*)malloc(sizeof(*FixedOrders) * FixedOrderCnt); InstrZ = 0;
  AddFixed("IJMP" , MinCoreMask(eCoreClassic) | (1 << eCoreMinTiny), 0x9409);
  AddFixed("ICALL", MinCoreMask(eCoreClassic) | (1 << eCoreMinTiny), 0x9509);
  AddFixed("RET"  , MinCoreMask(eCoreMinTiny), 0x9508); AddFixed("RETI"  , MinCoreMask(eCoreMinTiny), 0x9518);
  AddFixed("SEC"  , MinCoreMask(eCoreMinTiny), 0x9408);
  AddFixed("CLC"  , MinCoreMask(eCoreMinTiny), 0x9488); AddFixed("SEN"   , MinCoreMask(eCoreMinTiny), 0x9428);
  AddFixed("CLN"  , MinCoreMask(eCoreMinTiny), 0x94a8); AddFixed("SEZ"   , MinCoreMask(eCoreMinTiny), 0x9418);
  AddFixed("CLZ"  , MinCoreMask(eCoreMinTiny), 0x9498); AddFixed("SEI"   , MinCoreMask(eCoreMinTiny), 0x9478);
  AddFixed("CLI"  , MinCoreMask(eCoreMinTiny), 0x94f8); AddFixed("SES"   , MinCoreMask(eCoreMinTiny), 0x9448);
  AddFixed("CLS"  , MinCoreMask(eCoreMinTiny), 0x94c8); AddFixed("SEV"   , MinCoreMask(eCoreMinTiny), 0x9438);
  AddFixed("CLV"  , MinCoreMask(eCoreMinTiny), 0x94b8); AddFixed("SET"   , MinCoreMask(eCoreMinTiny), 0x9468);
  AddFixed("CLT"  , MinCoreMask(eCoreMinTiny), 0x94e8); AddFixed("SEH"   , MinCoreMask(eCoreMinTiny), 0x9458);
  AddFixed("CLH"  , MinCoreMask(eCoreMinTiny), 0x94d8); AddFixed("NOP"   , MinCoreMask(eCoreMinTiny), 0x0000);
  AddFixed("SLEEP", MinCoreMask(eCoreMinTiny), 0x9588); AddFixed("WDR"   , MinCoreMask(eCoreMinTiny), 0x95a8);
  AddFixed("EIJMP", MinCoreMask(eCoreMega   ), 0x9419); AddFixed("EICALL", MinCoreMask(eCoreMega   ), 0x9519);
  AddFixed("SPM"  , MinCoreMask(eCoreTiny   ), 0x95e8);
  AddFixed("BREAK" , MinCoreMask(eCoreTiny   ) | (1 << eCoreMinTiny), 0x9598);

  Reg1Orders = (FixedOrder*)malloc(sizeof(*Reg1Orders) * Reg1OrderCnt); InstrZ = 0;
  AddReg1("COM"  , MinCoreMask(eCoreMinTiny), 0x9400); AddReg1("NEG"  , MinCoreMask(eCoreMinTiny), 0x9401);
  AddReg1("INC"  , MinCoreMask(eCoreMinTiny), 0x9403); AddReg1("DEC"  , MinCoreMask(eCoreMinTiny), 0x940a);
  AddReg1("PUSH" , MinCoreMask(eCoreClassic) | (1 << eCoreMinTiny), 0x920f);
  AddReg1("POP"  , MinCoreMask(eCoreClassic) | (1 << eCoreMinTiny), 0x900f);
  AddReg1("LSR"  , MinCoreMask(eCoreMinTiny), 0x9406); AddReg1("ROR"  , MinCoreMask(eCoreMinTiny), 0x9407);
  AddReg1("ASR"  , MinCoreMask(eCoreMinTiny), 0x9405); AddReg1("SWAP" , MinCoreMask(eCoreMinTiny), 0x9402);

  Reg2Orders = (FixedOrder*)malloc(sizeof(*Reg2Orders) * Reg2OrderCnt); InstrZ = 0;
  AddReg2("ADD"  , MinCoreMask(eCoreMinTiny), 0x0c00); AddReg2("ADC"  , MinCoreMask(eCoreMinTiny), 0x1c00);
  AddReg2("SUB"  , MinCoreMask(eCoreMinTiny), 0x1800); AddReg2("SBC"  , MinCoreMask(eCoreMinTiny), 0x0800);
  AddReg2("AND"  , MinCoreMask(eCoreMinTiny), 0x2000); AddReg2("OR"   , MinCoreMask(eCoreMinTiny), 0x2800);
  AddReg2("EOR"  , MinCoreMask(eCoreMinTiny), 0x2400); AddReg2("CPSE" , MinCoreMask(eCoreMinTiny), 0x1000);
  AddReg2("CP"   , MinCoreMask(eCoreMinTiny), 0x1400); AddReg2("CPC"  , MinCoreMask(eCoreMinTiny), 0x0400);
  AddReg2("MOV"  , MinCoreMask(eCoreMinTiny), 0x2c00); AddReg2("MUL"  , MinCoreMask(eCoreMega   ), 0x9c00);

  AddReg3("CLR"  , 0x2400); AddReg3("TST"  , 0x2000); AddReg3("LSL"  , 0x0c00);
  AddReg3("ROL"  , 0x1c00);

  AddImm("SUBI" , 0x5000); AddImm("SBCI" , 0x4000); AddImm("ANDI" , 0x7000);
  AddImm("ORI"  , 0x6000); AddImm("SBR"  , 0x6000); AddImm("CPI"  , 0x3000);
  AddImm("LDI"  , 0xe000);

  AddRel("BRCC" , 0xf400); AddRel("BRCS" , 0xf000); AddRel("BREQ" , 0xf001);
  AddRel("BRGE" , 0xf404); AddRel("BRSH" , 0xf400); AddRel("BRID" , 0xf407);
  AddRel("BRIE" , 0xf007); AddRel("BRLO" , 0xf000); AddRel("BRLT" , 0xf004);
  AddRel("BRMI" , 0xf002); AddRel("BRNE" , 0xf401); AddRel("BRHC" , 0xf405);
  AddRel("BRHS" , 0xf005); AddRel("BRPL" , 0xf402); AddRel("BRTC" , 0xf406);
  AddRel("BRTS" , 0xf006); AddRel("BRVC" , 0xf403); AddRel("BRVS" , 0xf003);

  AddBit("BLD"  , 0xf800); AddBit("BST"  , 0xfa00);
  AddBit("SBRC" , 0xfc00); AddBit("SBRS" , 0xfe00);

  AddPBit("CBI" , 0x9800); AddPBit("SBI" , 0x9a00);
  AddPBit("SBIC", 0x9900); AddPBit("SBIS", 0x9b00);

  AddInstTable(InstTable, "ADIW", 0x0000, DecodeADIW);
  AddInstTable(InstTable, "SBIW", 0x0100, DecodeADIW);

  AddInstTable(InstTable, "LD", 0x0000, DecodeLDST);
  AddInstTable(InstTable, "ST", 0x0200, DecodeLDST);

  AddInstTable(InstTable, "LDD", 0x0000, DecodeLDDSTD);
  AddInstTable(InstTable, "STD", 0x0200, DecodeLDDSTD);

  AddInstTable(InstTable, "IN" , 0x0000, DecodeINOUT);
  AddInstTable(InstTable, "OUT", 0x0800, DecodeINOUT);

  AddInstTable(InstTable, "LDS", 0x0000, DecodeLDSSTS);
  AddInstTable(InstTable, "STS", 0x0200, DecodeLDSSTS);

  AddInstTable(InstTable, "BCLR", 0x0080, DecodeBCLRSET);
  AddInstTable(InstTable, "BSET", 0x0000, DecodeBCLRSET);

  AddInstTable(InstTable, "CBR", 0, DecodeCBR);
  AddInstTable(InstTable, "SER", 0, DecodeSER);

  AddInstTable(InstTable, "BRBC", 0x0400, DecodeBRBSBC);
  AddInstTable(InstTable, "BRBS", 0x0000, DecodeBRBSBC);

  AddInstTable(InstTable, "JMP" , 0, DecodeJMPCALL);
  AddInstTable(InstTable, "CALL", 2, DecodeJMPCALL);

  AddInstTable(InstTable, "RJMP" , 0x0000, DecodeRJMPCALL);
  AddInstTable(InstTable, "RCALL", 0x1000, DecodeRJMPCALL);

  AddInstTable(InstTable, "PORT", 0, DecodePORT);
  AddInstTable(InstTable, "SFR" , 0, DecodeSFR);
  AddInstTable(InstTable, "RES" , 0, DecodeRES);
  AddInstTable(InstTable, "DATA", 0, DecodeDATA_AVR);
  AddInstTable(InstTable, "REG" , 0, DecodeREG);
  AddInstTable(InstTable, "BIT" , 0, DecodeBIT);

  AddInstTable(InstTable, "MULS", 0, DecodeMULS);

  AddInstTable(InstTable, "MULSU" , 0x0300, DecodeMegaMUL);
  AddInstTable(InstTable, "FMUL"  , 0x0308, DecodeMegaMUL);
  AddInstTable(InstTable, "FMULS" , 0x0380, DecodeMegaMUL);
  AddInstTable(InstTable, "FMULSU", 0x0388, DecodeMegaMUL);

  AddInstTable(InstTable, "MOVW", 0, DecodeMOVW);

  AddInstTable(InstTable, "LPM" , 0, DecodeLPM);
  AddInstTable(InstTable, "ELPM", 0, DecodeELPM);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
  free(FixedOrders);
  free(Reg1Orders);
  free(Reg2Orders);
}

/*---------------------------------------------------------------------------*/

static void MakeCode_AVR(void)
{
  CodeLen = 0; DontPrint = False;

  /* zu ignorierendes */

  if (Memo("")) return;

  /* all done via table :-) */

  if (!LookupInstTable(InstTable, OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
}

static void InitCode_AVR(void)
{
  SetFlag(&Packing, PackingName, False);
}

static Boolean IsDef_AVR(void)
{
  return (Memo("PORT")
       || Memo("REG")
       || Memo("SFR")
       || Memo("BIT"));
}

static void SwitchFrom_AVR(void)
{
  DeinitFields();
  ClearONOFF();
}

static Boolean ChkZeroArg(void)
{
  return (0 == ArgCnt);
}

static void SwitchTo_AVR(void *pUser)
{
  TurnWords = False;
  ConstMode = ConstModeC;
  SetIsOccupiedFnc = ChkZeroArg;

  PCSymbol = "*";
  HeaderID = 0x3b;
  NOPCode = 0x0000;
  DivideChars = ",";
  HasAttrs = False;

  ValidSegs = (1 << SegCode) | (1 << SegData) | (1 << SegIO);
  Grans[SegCode] = 2; ListGrans[SegCode] = 2; SegInits[SegCode] = 0;
  Grans[SegData] = 1; ListGrans[SegData] = 1; SegInits[SegData] = 32;
  Grans[SegIO  ] = 1; ListGrans[SegIO  ] = 1; SegInits[SegIO  ] = 0;  SegLimits[SegIO] = 0x3f;

  pCurrCPUProps = (const tCPUProps*)pUser;

  SegLimits[SegCode] = pCurrCPUProps->FlashEnd;
  SegLimits[SegData] = (pCurrCPUProps->RegistersMapped ? RegBankSize : 0)
                     + pCurrCPUProps->IOAreaSize
                     + pCurrCPUProps->RAMSize
                     - 1;

  CodeAdrIntType = GetSmallestUIntType(SegLimits[SegCode]);
  DataAdrIntType = GetSmallestUIntType(SegLimits[SegData]);

  SignMask = (SegLimits[SegCode] + 1) >> 1;
  ORMask = ((LongInt) - 1) - SegLimits[SegCode];

  AddONOFF("WRAPMODE", &WrapFlag, WrapFlagName, False);
  AddONOFF("PACKING", &Packing, PackingName, False);
  SetFlag(&WrapFlag, WrapFlagName, False);

  MakeCode = MakeCode_AVR;
  IsDef = IsDef_AVR;
  SwitchFrom = SwitchFrom_AVR;
  DissectBit = DissectBit_AVR;
  InitFields();
}

static const tCPUProps CPUProps[] =
{
  { "AT90S1200"      ,  0x01ff, 0x0000, IOAreaStdSize , True , eCore90S1200   },
  { "AT90S2313"      ,  0x03ff, 0x0080, IOAreaStdSize , True , eCoreClassic   },
  { "AT90S2323"      ,  0x03ff, 0x0080, IOAreaStdSize , True , eCoreClassic   },
  { "AT90S2333"      ,  0x03ff, 0x0080, IOAreaStdSize , True , eCoreClassic   }, /* == ATtiny22 */
  { "AT90S2343"      ,  0x03ff, 0x0080, IOAreaStdSize , True , eCoreClassic   },
  { "AT90S4414"      ,  0x07ff, 0x0100, IOAreaStdSize , True , eCoreClassic   },
  { "AT90S4433"      ,  0x07ff, 0x0080, IOAreaStdSize , True , eCoreClassic   },
  { "AT90S4434"      ,  0x07ff, 0x0100, IOAreaStdSize , True , eCoreClassic   },
  { "AT90S8515"      ,  0x0fff, 0x0200, IOAreaStdSize , True , eCoreClassic   },
  { "AT90C8534"      ,  0x0fff, 0x0100, IOAreaStdSize , True , eCoreClassic   },
  { "AT90S8535"      ,  0x0fff, 0x0200, IOAreaStdSize , True , eCoreClassic   },
  { "AT90USB646"     ,  0x7fff, 0x1000, IOAreaExtSize , True , eCoreMega      },
  { "AT90USB647"     ,  0x7fff, 0x1000, IOAreaExtSize , True , eCoreMega      },
  { "AT90USB1286"    ,  0xffff, 0x2000, IOAreaExtSize , True , eCoreMega      },
  { "AT90USB1287"    ,  0xffff, 0x2000, IOAreaExtSize , True , eCoreMega      },

  { "AT43USB355"     ,  0x2fff, 0x0400, 0x2000-RegBankSize, True , eCoreClassic   }, /* allow USB registers @ 0x1fxx */

  { "ATTINY4"        ,  0x00ff, 0x0020, IOAreaStdSize , False, eCoreMinTiny   },
  { "ATTINY5"        ,  0x00ff, 0x0020, IOAreaStdSize , False, eCoreMinTiny   },
  { "ATTINY9"        ,  0x01ff, 0x0020, IOAreaStdSize , False, eCoreMinTiny   },
  { "ATTINY10"       ,  0x01ff, 0x0020, IOAreaStdSize , False, eCoreMinTiny   },
  { "ATTINY11"       ,  0x01ff, 0x0000, IOAreaStdSize , True , eCore90S1200   },
  { "ATTINY12"       ,  0x01ff, 0x0000, IOAreaStdSize , True , eCore90S1200   },
  { "ATTINY13"       ,  0x01ff, 0x0040, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY13A"      ,  0x01ff, 0x0040, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY15"       ,  0x01ff, 0x0000, IOAreaStdSize , True , eCore90S1200   },
  { "ATTINY20"       ,  0x03ff, 0x0080, IOAreaStdSize , False, eCoreMinTiny   },
  { "ATTINY24"       ,  0x03ff, 0x0080, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY24A"      ,  0x03ff, 0x0080, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY25"       ,  0x03ff, 0x0080, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY26"       ,  0x03ff, 0x0080, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY28"       ,  0x03ff, 0x0000, IOAreaStdSize , True , eCore90S1200   },
  { "ATTINY40"       ,  0x07ff, 0x0100, IOAreaStdSize , False, eCoreMinTiny   },
  { "ATTINY44"       ,  0x07ff, 0x0100, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY44A"      ,  0x07ff, 0x0100, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY45"       ,  0x07ff, 0x0100, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY48"       ,  0x07ff, 0x0100, IOAreaExtSize , True , eCoreTiny      },
  { "ATTINY84"       ,  0x0fff, 0x0200, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY84A"      ,  0x0fff, 0x0200, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY85"       ,  0x0fff, 0x0200, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY87"       ,  0x0fff, 0x0200, IOAreaExtSize , True , eCoreTiny16K   },
  { "ATTINY88"       ,  0x0fff, 0x0200, IOAreaExtSize , True , eCoreTiny      },
  { "ATTINY102"      ,  0x01ff, 0x0020, IOAreaStdSize , False, eCoreMinTiny   },
  { "ATTINY104"      ,  0x01ff, 0x0020, IOAreaStdSize , False, eCoreMinTiny   },
  { "ATTINY167"      ,  0x1fff, 0x0200, IOAreaExtSize , True , eCoreTiny16K   },
  { "ATTINY261"      ,  0x03ff, 0x0080, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY261A"     ,  0x03ff, 0x0080, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY43U"      ,  0x07ff, 0x0100, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY441"      ,  0x07ff, 0x0100, IOAreaExtSize , True , eCoreTiny      },
  { "ATTINY461"      ,  0x07ff, 0x0100, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY461A"     ,  0x07ff, 0x0100, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY828"      ,  0x0fff, 0x0200, IOAreaExtSize , True , eCoreTiny      },
  { "ATTINY841"      ,  0x0fff, 0x0200, IOAreaExtSize , True , eCoreTiny      },
  { "ATTINY861"      ,  0x0fff, 0x0200, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY861A"     ,  0x0fff, 0x0200, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY1634"     ,  0x1fff, 0x0400, IOAreaExtSize , True , eCoreTiny16K   },
  { "ATTINY2313"     ,  0x03ff, 0x0080, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY2313A"    ,  0x03ff, 0x0080, IOAreaStdSize , True , eCoreTiny      },
  { "ATTINY4313"     ,  0x07ff, 0x0100, IOAreaStdSize , True , eCoreTiny      },

  { "ATMEGA48"       ,  0x07ff, 0x0200, IOAreaExtSize , True , eCoreMega      },

  { "ATMEGA8"        ,  0x0fff, 0x0400, IOAreaStdSize , True , eCoreMega      },
  { "ATMEGA8515"     ,  0x0fff, 0x0200, IOAreaStdSize , True , eCoreMega      },
  { "ATMEGA8535"     ,  0x0fff, 0x0200, IOAreaStdSize , True , eCoreMega      },
  { "ATMEGA88"       ,  0x0fff, 0x0200, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA8U2"      ,  0x0fff, 0x0200, IOAreaExtSize , True , eCoreMega      },

  { "ATMEGA16"       ,  0x1fff, 0x0400, IOAreaStdSize , True , eCoreMega      },
  { "ATMEGA161"      ,  0x1fff, 0x0400, IOAreaStdSize , True , eCoreMega      },
  { "ATMEGA162"      ,  0x1fff, 0x0400, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA163"      ,  0x1fff, 0x0400, IOAreaStdSize , True , eCoreMega      },
  { "ATMEGA164"      ,  0x1fff, 0x0400, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA165"      ,  0x1fff, 0x0200, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA168"      ,  0x1fff, 0x0400, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA169"      ,  0x1fff, 0x0400, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA16U2"     ,  0x1fff, 0x0200, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA16U4"     ,  0x1fff, 0x0500, IOAreaExtSize , True , eCoreMega      },

  { "ATMEGA32"       ,  0x3fff, 0x0800, IOAreaStdSize , True , eCoreMega      },
  { "ATMEGA323"      ,  0x3fff, 0x0800, IOAreaStdSize , True , eCoreMega      },
  { "ATMEGA324"      ,  0x3fff, 0x0800, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA325"      ,  0x3fff, 0x0800, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA3250"     ,  0x3fff, 0x0800, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA328"      ,  0x3fff, 0x0800, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA329"      ,  0x3fff, 0x0800, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA3290"     ,  0x3fff, 0x0800, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA32U2"     ,  0x3fff, 0x0400, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA32U4"     ,  0x3fff, 0x0a00, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA32U6"     ,  0x3fff, 0x0a00, IOAreaExtSize , True , eCoreMega      },

  { "ATMEGA406"      ,  0x4fff, 0x0800, IOAreaExtSize , True , eCoreMega      },

  { "ATMEGA64"       ,  0x7fff, 0x1000, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA640"      ,  0x7fff, 0x2000, IOAreaExt2Size, True , eCoreMega      },
  { "ATMEGA644"      ,  0x7fff, 0x1000, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA644RFR2"  ,  0x7fff, 0x2000, IOAreaExt2Size, True , eCoreMega      },
  { "ATMEGA645"      ,  0x7fff, 0x1000, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA6450"     ,  0x7fff, 0x1000, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA649"      ,  0x7fff, 0x1000, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA6490"     ,  0x7fff, 0x1000, IOAreaExtSize , True , eCoreMega      },

  { "ATMEGA103"      ,  0xffff, 0x1000, IOAreaStdSize , True , eCoreMega      },
  { "ATMEGA128"      ,  0xffff, 0x1000, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA1280"     ,  0xffff, 0x2000, IOAreaExt2Size, True , eCoreMega      },
  { "ATMEGA1281"     ,  0xffff, 0x2000, IOAreaExt2Size, True , eCoreMega      },
  { "ATMEGA1284"     ,  0xffff, 0x4000, IOAreaExtSize , True , eCoreMega      },
  { "ATMEGA1284RFR2" ,  0xffff, 0x4000, IOAreaExt2Size, True , eCoreMega      },

  { "ATMEGA2560"     , 0x1ffff, 0x2000, IOAreaExt2Size, True , eCoreMega      },
  { "ATMEGA2561"     , 0x1ffff, 0x2000, IOAreaExt2Size, True , eCoreMega      },
  { "ATMEGA2564RFR2" , 0x1ffff, 0x8000, IOAreaExt2Size, True , eCoreMega      },
  { NULL             ,     0x0, 0     , 0             , False, eCoreNone      },
};

void codeavr_init(void)
{
  const tCPUProps *pProp;

  for (pProp = CPUProps; pProp->pName; pProp++)
    (void)AddCPUUser(pProp->pName, SwitchTo_AVR, (void*)pProp, NULL);

   AddInitPassProc(InitCode_AVR);
}

