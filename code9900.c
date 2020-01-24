/* code9900.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator TMS99xx(x)                                                  */
/*                                                                           */
/*****************************************************************************/

/*
 * TODO:
 *
 * - implement remaining TI990/12 instructions
 * - add remaining TI990/xx machines
 * - regard machines with more than 64K of memory
 *
 */

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "strutil.h"
#include "endian.h"
#include "bpemu.h"
#include "nls.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "codevars.h"
#include "ibmfloat.h"
#include "errmsg.h"

#include "code9900.h"

#define TwoExtOrderCnt 4
#define SingOrderCnt 27
#define ImmOrderCnt 6
#define FixedOrderCnt 10
#define RegOrderCnt 4

enum
{
  eCoreNone = 0,
  eCore990_10  = 1 << 0,
  eCore990_12  = 1 << 1,
  eCore9900    = 1 << 2,
  eCore9940    = 1 << 3,
  eCore9995    = 1 << 4,
  eCore99105   = 1 << 5,
  eCore99110   = 1 << 6,
  eCoreAll     = eCore990_10 | eCore990_12 | eCore9900 | eCore9940 | eCore9995 | eCore99105 | eCore99110,

  eCoreFlagSupMode = 1 << 7
};

typedef struct
{
  const char *pName;
  Byte CoreFlags;
} tCPUProps;

typedef struct
{
  Word Code;
  Byte Flags;
} tOrder;

typedef struct
{
  Word Code1, Code2;
  Byte Flags;
} tExtOrder;

static const tCPUProps *pCurrCPUProps;
static tOrder *SingOrders, *ImmOrders, *FixedOrders, *RegOrders;
static tExtOrder *TwoExtOrders;
static Boolean IsWord;
static Word AdrVal, AdrPart;

/*-------------------------------------------------------------------------*/
/* Adressparser */

static Boolean DecodeReg(const tStrComp *pArg, Word *Erg)
{
  Boolean OK;
  *Erg = EvalStrIntExpression(pArg, UInt4, &OK);
  return OK;
}

static char *HasDisp(char *Asc)
{
  char *p;
  int Lev, Len = strlen(Asc);

  if ((Len >= 2) && (Asc[Len - 1] == ')'))
  {
    p = Asc + Len - 2; Lev = 0;
    while ((p >= Asc) && (Lev != -1))
    {
      switch (*p)
      {
        case '(': Lev--; break;
        case ')': Lev++; break;
      }
      if (Lev != -1) p--;
    }
    if (Lev != -1)
    {
      WrError(ErrNum_BrackErr);
      p = NULL;
    }
  }
  else
    p = NULL;

  return p;
}

static Boolean DecodeAdr(const tStrComp *pArg)
{
  Boolean IncFlag;
  Boolean OK;
  char *p;

  AdrCnt = 0;

  if (*pArg->Str == '*')
  {
    tStrComp IArg;

    StrCompRefRight(&IArg, pArg, 1);
    if (IArg.Str[strlen(IArg.Str) - 1] == '+')
    {
      IncFlag = True;
      StrCompShorten(&IArg, 1);
    }
    else
      IncFlag = False;
    if (DecodeReg(&IArg, &AdrPart))
    {
      AdrPart += 0x10 + (Ord(IncFlag) << 5);
      return True;
    }
    return False;
  }

  if (*pArg->Str == '@')
  {
    tStrComp IArg;

    StrCompRefRight(&IArg, pArg, 1);
    p = HasDisp(IArg.Str);
    if (!p)
    {
      FirstPassUnknown = False;
      AdrVal = EvalStrIntExpression(&IArg, UInt16, &OK);
      if (OK)
      {
        AdrPart = 0x20;
        AdrCnt = 1;
        if ((!FirstPassUnknown) && (IsWord) && (Odd(AdrVal)))
          WrError(ErrNum_AddrNotAligned);
        return True;
      }
    }
    else
    {
      tStrComp Disp, Reg;

      StrCompSplitRef(&Disp, &Reg, &IArg, p);
      StrCompShorten(&Reg, 1);
      if (DecodeReg(&Reg, &AdrPart))
      {
        if (AdrPart == 0) WrStrErrorPos(ErrNum_InvReg, &Reg);
        else
        {
          AdrVal = EvalStrIntExpression(&Disp, Int16, &OK);
          if (OK)
          {
            AdrPart += 0x20;
            AdrCnt = 1;
            return True;
          }
        }
      }
    }
    return False;
  }  

  if (DecodeReg(pArg, &AdrPart))
    return True;
  else
  {
    WrError(ErrNum_InvAddrMode);
    return False;
  }
}

static void PutByte(Byte Value)
{
  if ((CodeLen & 1) && (!BigEndian))
  {
    BAsmCode[CodeLen] = BAsmCode[CodeLen - 1];
    BAsmCode[CodeLen - 1] = Value;
  }
  else
  {
    BAsmCode[CodeLen] = Value;
  }
  CodeLen++;
}

/*-------------------------------------------------------------------------*/
/* Code Generators */

static void CheckSupMode(void)
{
  if (!SupAllowed && (pCurrCPUProps->CoreFlags & eCoreFlagSupMode))
    WrError(ErrNum_PrivOrder);
}
 
static Boolean CheckCore(Byte CoreReqFlags)
{
  if (!(CoreReqFlags & pCurrCPUProps->CoreFlags))
  {
    WrStrErrorPos(ErrNum_InstructionNotSupported, &OpPart);
    return False;
  }
  else
    return True;
}

static Boolean CheckNotMode3(const tStrComp *pArg)
{
  if ((AdrPart & 0x30) == 0x30)
  {
    WrStrErrorPos(ErrNum_InvAddrMode, pArg);
    return False;
  }
  else
    return True;
}

static void DecodeTwo(Word Code)
{
  Word HPart;

  if (ChkArgCnt(2, 2)
   && DecodeAdr(&ArgStr[1]))
  {
    WAsmCode[0] = AdrPart;
    WAsmCode[1] = AdrVal;
    HPart = AdrCnt;
    if (DecodeAdr(&ArgStr[2]))
    {
      WAsmCode[0] += AdrPart << 6;
      WAsmCode[1 + HPart] = AdrVal;
      CodeLen = (1 + HPart + AdrCnt) << 1;
      WAsmCode[0] += Code;
    }
  }
}

static void DecodeTwoExt(Word Index)
{
  const tExtOrder *pOrder = &TwoExtOrders[Index];
  Word AdrCnt1;

  if (ChkArgCnt(2, 2)
   && CheckCore(pOrder->Flags & eCoreAll)
   && DecodeAdr(&ArgStr[1]))
  {
    WAsmCode[0] = pOrder->Code1;
    WAsmCode[1] = pOrder->Code2 | AdrPart;
    WAsmCode[2] = AdrVal;
    AdrCnt1 = AdrCnt;
    if (DecodeAdr(&ArgStr[2]))
    {
      WAsmCode[1] |= AdrPart << 6;
      WAsmCode[2 + AdrCnt1] = AdrVal;
      CodeLen = (2 + AdrCnt1 + AdrCnt) << 1;
    }
  }
}

static void DecodeOne(Word Code)
{
  if (ChkArgCnt(2, 2)
   && DecodeAdr(&ArgStr[1]))
  {
    Word HPart;

    WAsmCode[0] = AdrPart;
    WAsmCode[1] = AdrVal;
    if (DecodeReg(&ArgStr[2], &HPart))
    {
      WAsmCode[0] += (HPart << 6) + (Code << 10);
      CodeLen = (1 + AdrCnt) << 1;
    }
  }
}

static void DecodeLDCR_STCR(Word Code)
{
  if (ChkArgCnt(2, 2)
   && DecodeAdr(&ArgStr[1]))
  {
    Word HPart;
    Boolean OK;

    WAsmCode[0] = Code + AdrPart;
    WAsmCode[1] = AdrVal;
    FirstPassUnknown = False;
    HPart = EvalStrIntExpression(&ArgStr[2], UInt5, &OK);
    if (FirstPassUnknown)
      HPart = 1;
    if (OK)
    {
      if (ChkRange(HPart, 1, 16))
      {
        WAsmCode[0] += (HPart & 15) << 6;
        CodeLen = (1 + AdrCnt) << 1;
      }
    }
  }
  return;
}

static void DecodeShift(Word Code)
{
  if (ChkArgCnt(2, 2)
   && DecodeReg(&ArgStr[1], WAsmCode + 0))
  {
    Word HPart;

    if (DecodeReg(&ArgStr[2], &HPart))
    {
      WAsmCode[0] += (HPart << 4) + (Code << 8);
      CodeLen = 2;
    }
  }
}

static void DecodeSLAM_SRAM(Word Code)
{
  Word HPart;

  if (ChkArgCnt(2, 2)
   && CheckCore(eCore990_12 | eCore99105 | eCore99110)
   && DecodeReg(&ArgStr[2], &HPart)
   && DecodeAdr(&ArgStr[1]))
  {
    WAsmCode[0] = Code;
    WAsmCode[1] = 0x4000 | (HPart << 6) | AdrPart;
    WAsmCode[2] = AdrVal;
    CodeLen = (2 + AdrCnt) << 1;
  }
}

static void DecodeImm(Word Index)
{
  const tOrder *pOrder = &ImmOrders[Index];

  if (ChkArgCnt(2, 2)
   && CheckCore(pOrder->Flags & eCoreAll)
   && DecodeReg(&ArgStr[1], WAsmCode + 0))
  {
    Boolean OK;

    WAsmCode[1] = EvalStrIntExpression(&ArgStr[2], Int16, &OK);
    if (OK)
    {
      WAsmCode[0] |= pOrder->Code;
      CodeLen = 4;
    }
  }
}

static void DecodeLIIM(Word Code)
{
  if (ChkArgCnt(1, 1)
   && CheckCore(eCore9940))
  {
    Boolean OK;

    WAsmCode[0] = EvalStrIntExpression(&ArgStr[1], UInt2, &OK);
    if (OK)
    {
      WAsmCode[0] |= Code;
      CodeLen = 2;
    }
  }
}

static void DecodeRegOrder(Word Index)
{
  const tOrder *pOrder = &RegOrders[Index];

  if (ChkArgCnt(1, 1)
   && CheckCore(pOrder->Flags & eCoreAll)
   && DecodeReg(&ArgStr[1], WAsmCode + 0))
  {
    WAsmCode[0] |= pOrder->Code;
    CodeLen = 2;
  }
}

static void DecodeLMF(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(2, 2)
   && CheckCore(eCore990_10 | eCore990_12 | eCore99105 | eCore99110)
   && DecodeReg(&ArgStr[1], WAsmCode + 0))
  {
    Boolean OK;

    WAsmCode[0] += 0x320 + (EvalStrIntExpression(&ArgStr[2], UInt1, &OK) << 4);
    if (OK)
      CodeLen = 2;
    CheckSupMode();
  }
}

static void DecodeMPYS_DIVS(Word Code)
{
  if (ChkArgCnt(1, 1)
   && CheckCore(eCore990_12 | eCore9995 | eCore99105 | eCore99110)
   && DecodeAdr(&ArgStr[1]))
  {
    WAsmCode[0] = Code + AdrPart;
    WAsmCode[1] = AdrVal;
    CodeLen = (1 + AdrCnt) << 1;
  }
}

static void DecodeSBit(Word Code)
{
  if (ChkArgCnt(1, 1))
  {
    Boolean OK;

    WAsmCode[0] = EvalStrIntExpression(&ArgStr[1], SInt8, &OK);
    if (OK)
    {
      WAsmCode[0] = (WAsmCode[0] & 0xff) | Code;
      CodeLen = 2;
    }
  }
}

static void DecodeBit(Word Code)
{
  if (ChkArgCnt(2, 2)
   && CheckCore(eCore990_12 | eCore99105 | eCore99110)
   && DecodeAdr(&ArgStr[1])
   && CheckNotMode3(&ArgStr[1]))
  {
    Boolean OK;

    WAsmCode[1] = (EvalStrIntExpression(&ArgStr[2], UInt4, &OK) << 6) | AdrPart;
    if (OK)
    {
      WAsmCode[0] = Code;
      WAsmCode[2] = AdrVal;
      CodeLen = (2 + AdrCnt) << 1;
    }
  }
}

static void DecodeJmp(Word Code)
{
  if (ChkArgCnt(1, 1))
  {
    Boolean OK;
    Integer AdrInt;

    AdrInt = EvalStrIntExpression(&ArgStr[1], UInt16, &OK) - (EProgCounter() + 2);
    if (OK)
    {
      if (Odd(AdrInt)) WrError(ErrNum_DistIsOdd);
      else if ((!SymbolQuestionable) && ((AdrInt < -256) || (AdrInt > 254))) WrError(ErrNum_JmpDistTooBig);
      else
      {
        WAsmCode[0] = ((AdrInt >> 1) & 0xff) | Code;
        CodeLen = 2;
      }
    }
  }
}

static void DecodeLWPI_LIMI(Word Code)
{
  if (ChkArgCnt(1, 1))
  {
    Boolean OK;

    WAsmCode[1] = EvalStrIntExpression(&ArgStr[1], UInt16, &OK);
    if (OK)
    {
      WAsmCode[0] = Code & 0x7fff;
      CodeLen = 4;
      if (Code & 0x8000)
        CheckSupMode();
    }
  }
}

static void DecodeSing(Word Index)
{
  const tOrder *pOrder = &SingOrders[Index];

  if (ChkArgCnt(1, 1)
   && CheckCore(pOrder->Flags & eCoreAll)
   && DecodeAdr(&ArgStr[1]))
  {
    WAsmCode[0] = pOrder->Code | AdrPart;
    WAsmCode[1] = AdrVal;
    CodeLen = (1 + AdrCnt) << 1;
    if (pOrder->Flags & eCoreFlagSupMode)
      CheckSupMode();
  }
}

static void DecodeFixed(Word Index)
{
  const tOrder *pOrder = &FixedOrders[Index];

  if (ChkArgCnt(0, 0)
   && CheckCore(pOrder->Flags & eCoreAll))
  {
    WAsmCode[0] = pOrder->Code;
    CodeLen = 2;
    if (pOrder->Flags & eCoreFlagSupMode)
      CheckSupMode();
  }
}

static void DecodeBYTE(Word Code)
{
  int z;
  Boolean OK;
  TempResult t;

  UNUSED(Code);

  if (ChkArgCnt(1, ArgCntMax))
  {
    z = 1; OK = True;
    do
    {
      KillBlanks(ArgStr[z].Str);
      FirstPassUnknown = False;
      EvalStrExpression(&ArgStr[z], &t);
      switch (t.Typ)
      {
        case TempInt:
          if (FirstPassUnknown)
            t.Contents.Int &= 0xff;
          if (!RangeCheck(t.Contents.Int, Int8)) WrError(ErrNum_OverRange);
          else if (SetMaxCodeLen(CodeLen + 1))
          {
            WrError(ErrNum_CodeOverflow);
            OK = False;
          }
          else
            PutByte(t.Contents.Int);
          break;
        case TempString:
          if (SetMaxCodeLen(t.Contents.Ascii.Length + CodeLen))
          {
            WrError(ErrNum_CodeOverflow);
            OK = False;
          }
          else
          {
            char *p, *pEnd = t.Contents.Ascii.Contents + t.Contents.Ascii.Length;

            TranslateString(t.Contents.Ascii.Contents, t.Contents.Ascii.Length);
            for (p = t.Contents.Ascii.Contents; p < pEnd; PutByte(*(p++)));
          }
          break;
        case TempFloat:
          WrStrErrorPos(ErrNum_StringOrIntButFloat, &ArgStr[z]);
          /* fall-through */
        default:
          OK = False;
      }
      z++;
    }
    while ((z <= ArgCnt) && (OK));
    if (!OK)
      CodeLen = 0;
    else if ((Odd(CodeLen)) && (DoPadding))
      PutByte(0);
  }
}

static void DecodeWORD(Word Code)
{
  int z;
  Boolean OK;
  Word HVal16;

  UNUSED(Code);

  if (ChkArgCnt(1, ArgCntMax))
  {
    z = 1;
    OK = True;
    do
    {
      HVal16 = EvalStrIntExpression(&ArgStr[z], Int16, &OK);
      if (OK)
      {
        WAsmCode[CodeLen >> 1] = HVal16;
        CodeLen += 2;
      }
      z++;
    }
    while ((z <= ArgCnt) && (OK));
    if (!OK)
      CodeLen = 0;
  }
}

static void DecodeFLOAT(Word DestLen)
{
  int z;
  Boolean OK;
  double FVal;

  if (ChkArgCnt(1, ArgCntMax))
  {
    z = 1;
    OK = True;
    do
    {
      FVal = EvalStrFloatExpression(&ArgStr[z], Float64, &OK);
      if (OK)
      {
        SetMaxCodeLen(CodeLen + DestLen);
        if (Double2IBMFloat(&WAsmCode[CodeLen >> 1], FVal, DestLen == 8))
          CodeLen += DestLen;
      }
      z++;
    }
    while ((z <= ArgCnt) && (OK));
    if (!OK)
      CodeLen = 0;
  }
}

static void DecodeBSS(Word Code)
{
  Boolean OK;
  Word HVal16;  

  UNUSED(Code);

  if (ChkArgCnt(1, 1))
  {
    FirstPassUnknown = False;
    HVal16 = EvalStrIntExpression(&ArgStr[1], Int16, &OK);
    if (FirstPassUnknown) WrError(ErrNum_FirstPassCalc);
    else if (OK)
    {
      if ((DoPadding) && (Odd(HVal16)))
        HVal16++;
      if (!HVal16) WrError(ErrNum_NullResMem);
      DontPrint = True;
      CodeLen = HVal16;
      BookKeeping();
    }
  }
}

/*-------------------------------------------------------------------------*/
/* dynamische Belegung/Freigabe Codetabellen */

static void AddTwo(char *NName16, char *NName8, Word NCode)
{
  AddInstTable(InstTable, NName16, (NCode << 13)         , DecodeTwo);
  AddInstTable(InstTable, NName8,  (NCode << 13) + 0x1000, DecodeTwo);
}

static void AddOne(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeOne);
}

static void AddSing(char *NName, Word NCode, Byte Flags)
{
  if (InstrZ >= SingOrderCnt)
    exit(42);
  SingOrders[InstrZ].Code = NCode;
  SingOrders[InstrZ].Flags = Flags;
  AddInstTable(InstTable, NName, InstrZ++, DecodeSing);
}

static void AddSBit(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode << 8, DecodeSBit);
}

static void AddBit(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeBit);
}

static void AddJmp(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode << 8, DecodeJmp);
}

static void AddShift(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeShift);
}

static void AddImm(char *NName, Word NCode, Word Flags)
{
  if (InstrZ >= ImmOrderCnt)
    exit(42);
  ImmOrders[InstrZ].Code = NCode << 4;
  ImmOrders[InstrZ].Flags = Flags;
  AddInstTable(InstTable, NName, InstrZ++, DecodeImm);
}

static void AddReg(char *NName, Word NCode, Word Flags)
{
  if (InstrZ >= RegOrderCnt)
    exit(42);
  RegOrders[InstrZ].Code = NCode << 4;
  RegOrders[InstrZ].Flags = Flags;
  AddInstTable(InstTable, NName, InstrZ++, DecodeRegOrder);
}

static void AddFixed(char *NName, Word NCode, Word Flags)
{
  if (InstrZ >= FixedOrderCnt)
    exit(42);
  FixedOrders[InstrZ].Code = NCode;
  FixedOrders[InstrZ].Flags = Flags;
  AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
}

static void AddTwoExt(char *NName, Word NCode1, Word NCode2, Word Flags)
{
  if (InstrZ >= TwoExtOrderCnt)
    exit(42);
  TwoExtOrders[InstrZ].Code1 = NCode1;
  TwoExtOrders[InstrZ].Code2 = NCode2;
  TwoExtOrders[InstrZ].Flags = Flags;
  AddInstTable(InstTable, NName, InstrZ++, DecodeTwoExt);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(203);
  AddInstTable(InstTable, "LDCR", 0x3000, DecodeLDCR_STCR);
  AddInstTable(InstTable, "STCR", 0x3400, DecodeLDCR_STCR);
  AddInstTable(InstTable, "LMF", 0, DecodeLMF);
  AddInstTable(InstTable, "MPYS", 0x01c0, DecodeMPYS_DIVS);
  AddInstTable(InstTable, "DIVS", 0x0180, DecodeMPYS_DIVS);
  AddInstTable(InstTable, "LWPI", 0x02e0, DecodeLWPI_LIMI);
  AddInstTable(InstTable, "LIMI", 0x8300, DecodeLWPI_LIMI);
  AddInstTable(InstTable, "BYTE", 0, DecodeBYTE);
  AddInstTable(InstTable, "WORD", 0, DecodeWORD);
  AddInstTable(InstTable, "SINGLE", 4, DecodeFLOAT);
  AddInstTable(InstTable, "DOUBLE", 8, DecodeFLOAT);
  AddInstTable(InstTable, "BSS", 0, DecodeBSS);

  AddTwo("A"   , "AB"   , 5); AddTwo("C"   , "CB"   , 4); AddTwo("S"   , "SB"   , 3);
  AddTwo("SOC" , "SOCB" , 7); AddTwo("SZC" , "SZCB" , 2); AddTwo("MOV" , "MOVB" , 6);

  AddOne("COC" , 0x08); AddOne("CZC" , 0x09); AddOne("XOR" , 0x0a);
  AddOne("MPY" , 0x0e); AddOne("DIV" , 0x0f); AddOne("XOP" , 0x0b);

  SingOrders = (tOrder*)malloc(sizeof(*SingOrders) * SingOrderCnt); InstrZ = 0;
  AddSing("B"   , 0x0440, eCoreAll);
  AddSing("BL"  , 0x0680, eCoreAll);
  AddSing("BLWP", 0x0400, eCoreAll);
  AddSing("CLR" , 0x04c0, eCoreAll);
  AddSing("SETO", 0x0700, eCoreAll);
  AddSing("INV" , 0x0540, eCoreAll);
  AddSing("NEG" , 0x0500, eCoreAll);
  AddSing("ABS" , 0x0740, eCoreAll);
  AddSing("SWPB", 0x06c0, eCoreAll);
  AddSing("INC" , 0x0580, eCoreAll);
  AddSing("INCT", 0x05c0, eCoreAll);
  AddSing("DEC" , 0x0600, eCoreAll);
  AddSing("DECT", 0x0640, eCoreAll);
  AddSing("X"   , 0x0480, eCoreAll);
  AddSing("LDS" , 0x0780, eCoreFlagSupMode | eCore990_10 | eCore990_12 | eCore99110);
  AddSing("LDD" , 0x07c0, eCoreFlagSupMode | eCore990_10 | eCore990_12 | eCore99110);
  AddSing("DCA" , 0x2c00, eCore9940);
  AddSing("DCS" , 0x2c40, eCore9940);
  AddSing("BIND", 0x0140, eCore990_12 | eCore99105 | eCore99110);
  AddSing("AR"  , 0x0c40, eCore990_12 | eCore99110);
  AddSing("SR"  , 0x0cc0, eCore990_12 | eCore99110);
  AddSing("MR"  , 0x0d00, eCore990_12 | eCore99110);
  AddSing("DR"  , 0x0d40, eCore990_12 | eCore99110);
  AddSing("LR"  , 0x0d80, eCore990_12 | eCore99110);
  AddSing("STR" , 0x0dc0, eCore990_12 | eCore99110);
  AddSing("CIR" , 0x0c80, eCore990_12 | eCore99110);
  AddSing("EVAD", 0x0100, eCore99105 | eCore99110);
#if 0
  AddSing("AD"  , 0x0e40, eCore990_12);
  AddSing("CID" , 0x0e80, eCore990_12);
  AddSing("SD"  , 0x0ec0, eCore990_12);
  AddSing("MD"  , 0x0f00, eCore990_12);
  AddSing("DD"  , 0x0f40, eCore990_12);
  AddSing("LD"  , 0x0f80, eCore990_12);
  AddSing("STD" , 0x0fc0, eCore990_12);
#endif

  AddSBit("SBO" , 0x1d); AddSBit("SBZ", 0x1e); AddSBit("TB" , 0x1f);

  AddJmp("JEQ", 0x13); AddJmp("JGT", 0x15); AddJmp("JH" , 0x1b);
  AddJmp("JHE", 0x14); AddJmp("JL" , 0x1a); AddJmp("JLE", 0x12);
  AddJmp("JLT", 0x11); AddJmp("JMP", 0x10); AddJmp("JNC", 0x17);
  AddJmp("JNE", 0x16); AddJmp("JNO", 0x19); AddJmp("JOC", 0x18);
  AddJmp("JOP", 0x1c);

  AddShift("SLA", 0x0a); AddShift("SRA", 0x08);
  AddShift("SRC", 0x0b); AddShift("SRL", 0x09);

  ImmOrders = (tOrder*)malloc(sizeof(*ImmOrders) * ImmOrderCnt); InstrZ = 0;
  AddImm("AI"  , 0x022, eCoreAll);
  AddImm("ANDI", 0x024, eCoreAll);
  AddImm("CI"  , 0x028, eCoreAll);
  AddImm("LI"  , 0x020, eCoreAll);
  AddImm("ORI" , 0x026, eCoreAll);
  AddImm("BLSK", 0x00b, eCore990_12 | eCore99105 | eCore99110);

  TwoExtOrders = (tExtOrder*)malloc(sizeof(*TwoExtOrders) * TwoExtOrderCnt); InstrZ = 0;
  AddTwoExt("AM"  , 0x002a, (pCurrCPUProps->CoreFlags & eCore990_12) ? 0x0000 : 0x4000, eCore990_12 | eCore99105 | eCore99110);
  AddTwoExt("SM"  , 0x0029, (pCurrCPUProps->CoreFlags & eCore990_12) ? 0x0000 : 0x4000, eCore990_12 | eCore99105 | eCore99110);
  AddTwoExt("CR"  , 0x0301, 0x0000, eCore99110);
  AddTwoExt("MM"  , 0x0302, 0x0000, eCore99110);
#if 0
  AddTwoExt("NRM" , 0x0c08, 0x0000, eCore990_12);
  AddTwoExt("RTO" , 0x001e, 0x0000, eCore990_12);
  AddTwoExt("LTO" , 0x001f, 0x0000, eCore990_12);
  AddTwoExt("CNTO", 0x0020, 0x0000, eCore990_12);
  AddTwoExt("BDC" , 0x0023, 0x0000, eCore990_12);
  AddTwoExt("DBC" , 0x0024, 0x0000, eCore990_12);
  AddTwoExt("SWPM", 0x0025, 0x0000, eCore990_12);
  AddTwoExt("XORM", 0x0026, 0x0000, eCore990_12);
  AddTwoExt("ORM" , 0x0027, 0x0000, eCore990_12);
  AddTwoExt("ANDM", 0x0028, 0x0000, eCore990_12);
#endif

  AddInstTable(InstTable, "SLAM", 0x001d, DecodeSLAM_SRAM);
  AddInstTable(InstTable, "SRAM", 0x001c, DecodeSLAM_SRAM);

  RegOrders = (tOrder*)malloc(sizeof(*RegOrders) * RegOrderCnt); InstrZ = 0;
  AddReg("STST", 0x02c, eCoreAll);
  AddReg("LST" , 0x008, eCore9995 | eCore99105 | eCore99110);
  AddReg("STWP", 0x02a, eCoreAll);
  AddReg("LWP" , 0x009, eCore9995 | eCore99105 | eCore99110);

  AddBit("TMB" , 0x0c09);
  AddBit("TCMB", 0x0c0a);
  AddBit("TSMB", 0x0c0b);

  FixedOrders = (tOrder*)malloc(sizeof(*FixedOrders) * FixedOrderCnt); InstrZ = 0;
  AddFixed("RTWP", 0x0380, eCoreAll);
  AddFixed("IDLE", 0x0340, eCoreFlagSupMode | eCoreAll);
  AddFixed("RSET", 0x0360, eCoreFlagSupMode | (eCoreAll & ~eCore9940));
  AddFixed("CKOF", 0x03c0, eCoreFlagSupMode | (eCoreAll & ~eCore9940));
  AddFixed("CKON", 0x03a0, eCoreFlagSupMode | (eCoreAll & ~eCore9940));
  AddFixed("LREX", 0x03e0, eCoreFlagSupMode | (eCoreAll & ~eCore9940));
  AddFixed("CER" , 0x0c06, eCore990_12 | eCore99110);
  AddFixed("CRE" , 0x0c04, eCore990_12 | eCore99110);
  AddFixed("NEGR", 0x0c02, eCore990_12 | eCore99110);
  AddFixed("CRI" , 0x0c00, eCore990_12 | eCore99110);
#if 0
  AddFixed("EMD" , 0x002d, eCore990_12);
  AddFixed("EINT", 0x002e, eCore990_12);
  AddFixed("DINT", 0x002f, eCore990_12);
  AddFixed("CDI" , 0x0c01, eCore990_12);
  AddFixed("NEGD", 0x0c03, eCore990_12);
  AddFixed("CDE" , 0x0c05, eCore990_12);
  AddFixed("CED" , 0x0c07, eCore990_12);
  AddFixed("XIT" , 0x0c0e, eCore990_12);
#endif

#if 0
  AddType12("SNEB",, eCore990_12);
  AddType12("CRC" ,, eCore990_12);
  AddType12("TS"  ,, eCore990_12);
  AddType12("CS"  ,, eCore990_12);
  AddType12("SEQB",, eCore990_12);
  AddType12("MOVS",, eCore990_12);
  AddType12("MVSR",, eCore990_12);
  AddType12("MVSK",, eCore990_12);
  AddType12("POPS",, eCore990_12);
  AddType12("PSHS",, eCore990_12);

  AddType15("IOF" ,, eCore990_12);

  AddType16("INSF",, eCore990_12);
  AddType16("XV"  ,, eCore990_12);
  AddType16("XF"  ,, eCore990_12);

  AddType17("SRJ" ,, eCore990_12);
  AddType17("ARJ" ,, eCore990_12);

  AddType18("STPC",, eCore990_12);
  AddType18("LIM" ,, eCore990_12);
  AddType18("LST" ,, eCore990_12);
  AddType18("LWP" ,, eCore990_12);
  AddType18("LCS" ,, eCore990_12);

  AddType19("MOVA",, eCore990_12);

  AddType20("SLSL",, eCore990_12);
  AddType20("SLSP",, eCore990_12);

  AddType21("EP"  ,, eCore990_12);
#endif

  AddInstTable(InstTable, "LIIM", 0x2c80, DecodeLIIM);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
  free(SingOrders);
  free(ImmOrders);
  free(FixedOrders);
  free(RegOrders);
  free(TwoExtOrders);
}

/*-------------------------------------------------------------------------*/

static void MakeCode_9900(void)
{
  CodeLen = 0;
  DontPrint = False;
  IsWord = False;

  /* zu ignorierendes */

  if (Memo("")) return;

  if (!LookupInstTable(InstTable, OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
}

static Boolean IsDef_9900(void)
{
  return False;
}

static void SwitchFrom_9900(void)
{
  DeinitFields();
  ClearONOFF();
}

static void InternSymbol_9900(char *Asc, TempResult*Erg)
{
  Boolean err;
  char *h = Asc;

  Erg->Typ = TempNone;
  if ((strlen(Asc) >= 2) && (mytoupper(*Asc) == 'R'))
    h = Asc + 1;
  else if ((strlen(Asc) >= 3) && (mytoupper(*Asc) == 'W') && (mytoupper(Asc[1]) == 'R'))
    h = Asc + 2;

  Erg->Contents.Int = ConstLongInt(h, &err, 10);
  if ((!err) || (Erg->Contents.Int < 0) || (Erg->Contents.Int > 15))
    return;

  Erg->Typ = TempInt;
}

static void SwitchTo_9900(void *pUser)
{
  TurnWords = True;
  ConstMode = ConstModeIntel;

  PCSymbol = "$";
  HeaderID = 0x48;
  NOPCode = 0x0000;
  DivideChars = ",";
  HasAttrs = False;

  ValidSegs = 1 << SegCode;
  Grans[SegCode] = 1; ListGrans[SegCode] = 2; SegInits[SegCode] = 0;
  SegLimits[SegCode] = 0xffff;

  pCurrCPUProps = (const tCPUProps*)pUser;

  MakeCode = MakeCode_9900;
  IsDef = IsDef_9900;
  SwitchFrom = SwitchFrom_9900;
  InternSymbol = InternSymbol_9900;
  AddONOFF("PADDING", &DoPadding , DoPaddingName , False);
  AddONOFF("SUPMODE", &SupAllowed, SupAllowedName, False);

  InitFields();
}

static const tCPUProps CPUProps[] =
{
  { "TI990/4"  , eCore9900                      },
  { "TI990/10" , eCore990_10 | eCoreFlagSupMode },
#if 0
  { "TI990/12" , eCore990_12 | eCoreFlagSupMode },
#endif
  { "TMS9900"  , eCore9900                      },
  { "TMS9940"  , eCore9940                      },
  { "TMS9995"  , eCore9995                      },
  { "TMS99105" , eCore99105  | eCoreFlagSupMode },
  { "TMS99110" , eCore99110  | eCoreFlagSupMode },
  { NULL       , eCoreNone                      },
};

void code9900_init(void)
{
  const tCPUProps *pProp;

  for (pProp = CPUProps; pProp->pName; pProp++)
    (void)AddCPUUser(pProp->pName, SwitchTo_9900, (void*)pProp, NULL);
}
