/* code65.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator 65xx/MELPS740                                               */
/*                                                                           */
/* Historie: 17. 8.1996 Grundsteinlegung                                     */
/*           17.11.1998 ungueltiges Register wurde bei Indizierung nicht ab- */
/*                      gefangen                                             */
/*            2. 1.1999 ChkPC umgestellt                                     */
/*            9. 3.2000 'ambigious else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: code65.c,v 1.13 2017/06/28 16:49:02 alfred Exp $                      */
/*****************************************************************************
 * $Log: code65.c,v $
 * Revision 1.13  2017/06/28 16:49:02  alfred
 * - complete HuC6280 support
 *
 * Revision 1.12  2017/06/11 16:33:05  alfred
 * - added HuC6280 target (larger address space yet unimplemented)
 *
 * Revision 1.11  2017/06/04 10:41:43  alfred
 * - add 65C19 target
 *
 * Revision 1.10  2017/06/02 20:01:21  alfred
 * - added W65C02S target
 *
 * Revision 1.9  2017/06/02 19:12:42  alfred
 * - use symbolic values for CPU support masks of instructions
 *
 * Revision 1.8  2014/11/16 13:15:07  alfred
 * - remove some superfluous semicolons
 *
 * Revision 1.7  2014/11/12 16:55:43  alfred
 * - rework to current style
 *
 * Revision 1.6  2014/11/05 15:47:14  alfred
 * - replace InitPass callchain with registry
 *
 * Revision 1.5  2014/03/08 21:06:35  alfred
 * - rework ASSUME framework
 *
 * Revision 1.4  2010/08/27 14:52:41  alfred
 * - some more overlapping strcpy() cleanups
 *
 * Revision 1.3  2005/09/08 17:31:03  alfred
 * - add missing include
 *
 * Revision 1.2  2004/05/29 12:04:46  alfred
 * - relocated DecodeMot(16)Pseudo into separate module
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmpars.h"
#include "asmsub.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "motpseudo.h"
#include "codevars.h"
#include "errmsg.h"

#include "code65.h"

/*---------------------------------------------------------------------------*/

enum
{
  ModZA    = 0,   /* aa */
  ModA     = 1,   /* aabb */
  ModZIX   = 2,   /* aa,X */
  ModIX    = 3,   /* aabb,X */
  ModZIY   = 4,   /* aa,Y */
  ModIY    = 5,   /* aabb,Y */
  ModIndIX = 6,   /* (aa,X) */
  ModIndOX = 7,   /* (aa),X */
  ModIndOY = 8,   /* (aa),Y */
  ModInd16 = 9,   /* (aabb) */
  ModImm  = 10,   /* #aa */
  ModAcc  = 11,   /* A */
  ModNone = 12,   /* */
  ModInd8 = 13,   /* (aa) */
  ModSpec = 14,   /* \aabb */
};

typedef struct
{
  Byte CPUFlag;
  Byte Code;
} FixedOrder;

typedef struct
{
  Integer Codes[ModSpec + 1];
} NormOrder;

typedef struct
{
  Byte CPUFlag;
  Byte Code;
} CondOrder;

typedef struct
{
  ShortInt ErgMode;
  int AdrCnt;
  Byte AdrVals[2];
} tAdrResult;

#define FixedOrderCount 67
#define NormOrderCount 63
#define CondOrderCount 10

/* NOTE: keep in the same order as in registration in code65_init()! */

#define M_6502      (1 << 0)
#define M_65SC02    (1 << 1)
#define M_65C02     (1 << 2)
#define M_W65C02S   (1 << 3)
#define M_65C19     (1 << 4)
#define M_MELPS740  (1 << 5)
#define M_HUC6280   (1 << 6)
#define M_6502U     (1 << 7)

static Boolean CLI_SEI_Flag, ADC_SBC_Flag;

static FixedOrder *FixedOrders;
static NormOrder *NormOrders;
static CondOrder *CondOrders;

static CPUVar CPU6502, CPU65SC02, CPU65C02, CPUW65C02S, CPU65C19, CPUM740, CPUHUC6280, CPU6502U;
static LongInt SpecPage, MPR[8];

/*---------------------------------------------------------------------------*/

static void ChkZero(char *Asc, Byte *erg)
{
 if ((strlen(Asc) > 1) && ((*Asc == '<') || (*Asc == '>')))
 {
   *erg = Ord(*Asc == '<') + 1;
   strmov(Asc, Asc + 1);
 }
 else
   *erg = 0;
}

static void ChkFlags(void)
{
  /* Spezialflags ? */

  CLI_SEI_Flag = (Memo("CLI") || Memo("SEI"));
  ADC_SBC_Flag = (Memo("ADC") || Memo("SBC"));
}

static Boolean CPUAllowed(Byte Flag)
{
  return (((Flag >> (MomCPU - CPU6502)) & 1) || False);
}

static void InsNOP(void)
{
  memmove(BAsmCode, BAsmCode + 1, CodeLen);
  CodeLen++;
  BAsmCode[0] = NOPCode;
}

static Boolean IsAllowed(Word Val)
{
  return (CPUAllowed(Hi(Val)) && (Val != 0xffff));
}

static void ChkZeroMode(tAdrResult *pResult, const NormOrder *pOrder, ShortInt ZeroMode)
{
  if (pOrder && (IsAllowed(pOrder->Codes[ZeroMode])))
  {
    pResult->ErgMode = ZeroMode;
    pResult->AdrCnt--;
  }
}

/*---------------------------------------------------------------------------*/

static Word EvalAddress(const char *pAsc, IntType Type, Boolean *pOK)
{
  /* for the HUC6280, check if the address is within one of the selected pages */

  if (MomCPU == CPUHUC6280)
  {
    Word Page;
    LongWord AbsAddress;

    /* get the absolute address */

    FirstPassUnknown = False;
    AbsAddress = EvalIntExpression(pAsc, UInt21, pOK);
    if (!*pOK)
      return 0;

    /* within one of the 8K pages? */

    for (Page = 0; Page < 8; Page++)
      if ((LargeInt)((AbsAddress >> 13) & 255) == MPR[Page])
        break;
    if (Page >= 8)
    {
      WrError(110);
      AbsAddress &= 0x1fff;
    }
    else
      AbsAddress = (AbsAddress & 0x1fff) | (Page << 13);

    /* short address requested? */

    if ((Type != UInt16) && (Type != Int16) && Hi(AbsAddress))
    {
      if (FirstPassUnknown)
        AbsAddress &= 0xff;
      else
      {
        pOK = False;
        WrError(1320);
      }
    }
    return AbsAddress;
  }
  else
    return EvalIntExpression(pAsc, Type, pOK);
}

static void DecodeAdr(tAdrResult *pResult, const NormOrder *pOrder)
{
  Word AdrWord;
  Boolean ValOK;
  String s1;
  Byte ZeroMode;

  /* normale Anweisungen: Adressausdruck parsen */

  pResult->ErgMode = -1;

  if (ArgCnt == 0)
  {
    pResult->AdrCnt = 0;
    pResult->ErgMode = ModNone;
  }

  else if (ArgCnt == 1)
  {
    /* 1. Akkuadressierung */

    if (!strcasecmp(ArgStr[1], "A"))
    {
      pResult->AdrCnt = 0;
      pResult->ErgMode = ModAcc;
    }

    /* 2. immediate ? */

    else if (*ArgStr[1] == '#')
    {
      pResult->AdrVals[0] = EvalIntExpression(ArgStr[1] + 1, Int8, &ValOK);
      if (ValOK)
      {
        pResult->ErgMode = ModImm;
        pResult->AdrCnt = 1;
      }
    }

    /* 3. Special Page ? */

    else if (*ArgStr[1] == '\\')
    {
      AdrWord = EvalIntExpression(ArgStr[1] + 1, UInt16, &ValOK);
      if (ValOK)
      {
        if (FirstPassUnknown)
          AdrWord = (SpecPage << 8) | Lo(AdrWord);
        if (Hi(AdrWord) != SpecPage) WrError(1315);
        else
        {
          pResult->ErgMode = ModSpec;
          pResult->AdrVals[0] = Lo(AdrWord);
          pResult->AdrCnt = 1;
        }
      }
    }

    /* 4. X-inner-indirekt ? */

    else if ((strlen(ArgStr[1]) >= 5) && (!strcasecmp(ArgStr[1] + strlen(ArgStr[1]) - 3, ",X)")))
    {
      if (*ArgStr[1] != '(') WrError(1350);
      else
      {
        strmaxcpy(s1, ArgStr[1] + 1, 255);
        s1[strlen(s1) - 3] = '\0';
        ChkZero(s1, &ZeroMode);
        if (Memo("JMP"))
        {
          AdrWord = EvalAddress(s1, UInt16, &ValOK);
          if (ValOK)
          {
            pResult->AdrVals[0] = Lo(AdrWord);
            pResult->AdrVals[1] = Hi(AdrWord);
            pResult->ErgMode = ModIndIX;
            pResult->AdrCnt = 2;
          }
        }
        else
        {
          pResult->AdrVals[0] = EvalAddress(s1, UInt8, &ValOK);
          if (ValOK)
          {
            pResult->ErgMode = ModIndIX;
            pResult->AdrCnt = 1;
          }
        }
      }
    }

    else
    {
      /* 5. indirekt absolut ? */

      if (IsIndirect(ArgStr[1]))
      {
        strcpy(s1, ArgStr[1] + 1);
        s1[strlen(s1) - 1] = '\0';
        ChkZero(s1, &ZeroMode);
        if (ZeroMode == 2)
        {
          pResult->AdrVals[0] = EvalAddress(s1, UInt8, &ValOK);
          if (ValOK)
          {
            pResult->ErgMode = ModInd8;
            pResult->AdrCnt = 1;
          }
        }
        else
        {
          AdrWord = EvalAddress(s1, UInt16, &ValOK);
          if (ValOK)
          {
            pResult->ErgMode = ModInd16;
            pResult->AdrCnt = 2;
            pResult->AdrVals[0] = Lo(AdrWord);
            pResult->AdrVals[1] = Hi(AdrWord);
            if ((ZeroMode == 0) && (pResult->AdrVals[1] == 0))
              ChkZeroMode(pResult, pOrder, ModInd8);
          }
        }
      }

      /* 6. absolut */

      else
      {
        ChkZero(ArgStr[1], &ZeroMode);
        if (ZeroMode == 2)
        {
          pResult->AdrVals[0] = EvalAddress(ArgStr[1], UInt8, &ValOK);
          if (ValOK)
          {
            pResult->ErgMode = ModZA;
            pResult->AdrCnt = 1;
          }
        }
        else
        {
          AdrWord = EvalAddress(ArgStr[1], UInt16, &ValOK);
          if (ValOK)
          {
            pResult->ErgMode = ModA;
            pResult->AdrCnt = 2;
            pResult->AdrVals[0] = Lo(AdrWord);
            pResult->AdrVals[1] = Hi(AdrWord);
            if ((ZeroMode == 0) && (pResult->AdrVals[1] == 0))
              ChkZeroMode(pResult, pOrder, ModZA);
          }
        }
      }
    }
  }

  else if (ArgCnt == 2)
  {
    /* 7. X,Y-outer-indirekt ? */

    if ((IsIndirect(ArgStr[1]))
     && ((!strcasecmp(ArgStr[2], "X") || !strcasecmp(ArgStr[2], "Y"))))
    {
      Boolean IsY = (toupper(ArgStr[2][0]) == 'Y');

      strcpy(s1, ArgStr[1] + 1);
      s1[strlen(s1) - 1] = '\0';
      ChkZero(s1, &ZeroMode);
      pResult->AdrVals[0] = EvalAddress(s1, UInt8, &ValOK);
      if (ValOK)
      {
        pResult->ErgMode = IsY ? ModIndOY :ModIndOX;
        pResult->AdrCnt = 1;
      }
    }

    /* 8. X,Y-indiziert ? */

    else
    {
      strcpy(s1, ArgStr[1]); 
      ChkZero(s1, &ZeroMode);
      if (ZeroMode == 2)
      {
        pResult->AdrVals[0] = EvalAddress(s1, UInt8, &ValOK);
        if (ValOK)
        {
          pResult->AdrCnt = 1;
          if (!strcasecmp(ArgStr[2], "X"))
            pResult->ErgMode = ModZIX;
          else if (!strcasecmp(ArgStr[2], "Y"))
            pResult->ErgMode = ModZIY;
          else
            WrXErrorPos(ErrNum_InvReg, ArgStr[2], &ArgStrPos[2]);
        }
      }
      else
      {
        AdrWord = EvalAddress(s1, Int16, &ValOK);
        if (ValOK)
        {
          pResult->AdrCnt = 2;
          pResult->AdrVals[0] = Lo(AdrWord);
          pResult->AdrVals[1] = Hi(AdrWord);
          if (!strcasecmp(ArgStr[2], "X"))
            pResult->ErgMode = ModIX;
          else if (!strcasecmp(ArgStr[2], "Y"))
            pResult->ErgMode = ModIY;
          else
            WrXErrorPos(ErrNum_InvReg, ArgStr[2], &ArgStrPos[2]);
          if (pResult->ErgMode != -1)
          {
            if ((pResult->AdrVals[1] == 0) && (ZeroMode == 0))
              ChkZeroMode(pResult, pOrder, (!strcasecmp(ArgStr[2], "X")) ? ModZIX : ModZIY);
          }
        }
      }
    }
  }

  else
  {
    (void)ChkArgCnt(0, 2);
    ChkFlags();
  }
}

static const char *ImmStart(const char *pArg)
{
  return (*pArg == '#') ? pArg + 1 : pArg;
}

/*---------------------------------------------------------------------------*/

/* Anweisungen ohne Argument */

static void DecodeFixed(Word Index)
{
  const FixedOrder *pOrder = FixedOrders + Index;

  if (ChkArgCnt(0, 0)
   && (ChkExactCPUMask(pOrder->CPUFlag, CPU6502) >= 0))
  {
    CodeLen = 1;
    BAsmCode[0] = pOrder->Code;
    if (Memo("BRK"))
      BAsmCode[CodeLen++] = NOPCode;
    else if (MomCPU == CPUM740)
    {
      if (Memo("PLP"))
        BAsmCode[CodeLen++] = NOPCode; 
      if ((ADC_SBC_Flag) && (Memo("SEC") || Memo("CLC") || Memo("CLD")))
        InsNOP();
    }
  }
  ChkFlags();
}

static void DecodeSEB_CLB(Word Code)
{
  if (ChkArgCnt(2, 2)
   && ChkExactCPU(CPUM740))
  {
    Boolean ValOK;
    Byte BitNo = EvalIntExpression(ArgStr[1], UInt3, &ValOK);

    if (ValOK)
    {
      BAsmCode[0] = Code + (BitNo << 5);
      if (!strcasecmp(ArgStr[2], "A"))
        CodeLen = 1;
      else
      {
        BAsmCode[1] = EvalIntExpression(ArgStr[2], UInt8, &ValOK);
        if (ValOK)
        {
          CodeLen = 2;
          BAsmCode[0] += 4;
        }
      }
    }
  }
  ChkFlags();
}

static void DecodeBBC_BBS(Word Code)
{
  Boolean ValOK;
  int b;

  if (ChkArgCnt(3, 3)
   && ChkExactCPU(CPUM740))
  {
    BAsmCode[0] = EvalIntExpression(ArgStr[1], UInt3, &ValOK);
    if (ValOK)
    {
      BAsmCode[0] = (BAsmCode[0] << 5) + Code;
      b = (strcasecmp(ArgStr[2], "A") != 0);
      if (!b)
        ValOK = True;
      else
      {
        BAsmCode[0] += 4;
        BAsmCode[1] = EvalIntExpression(ArgStr[2], UInt8, &ValOK);
      }
      if (ValOK)
      {
        Integer AdrInt = EvalIntExpression(ArgStr[3], Int16, &ValOK) - (EProgCounter() + 2 + Ord(b) + Ord(CLI_SEI_Flag));

        if (ValOK)
        {
          if (((AdrInt > 127) || (AdrInt < -128)) && (!SymbolQuestionable)) WrError(1370);
          else
          {
            CodeLen = 2 + Ord(b);
            BAsmCode[CodeLen - 1] = AdrInt & 0xff;
            if (CLI_SEI_Flag)
              InsNOP();
          }
        }
      }
    }
  }
  ChkFlags();
}

static void DecodeBBR_BBS(Word Code)
{
  Boolean ValOK;

  if (ChkArgCnt(2, 2)
   && (ChkExactCPUMask(M_65C02 | M_W65C02S | M_65C19 | M_HUC6280, CPU6502) >= 0))
  {
    BAsmCode[1] = EvalIntExpression(ArgStr[1], UInt8, &ValOK);
    if (ValOK)
    {
      Integer AdrInt;

      BAsmCode[0] = Code;
      AdrInt = EvalIntExpression(ArgStr[2], UInt16, &ValOK) - (EProgCounter() + 3);
      if (ValOK)
      {
        if (((AdrInt > 127) || (AdrInt < -128)) && (!SymbolQuestionable)) WrError(1370);
        else
        {
          CodeLen = 3;
          BAsmCode[2] = AdrInt & 0xff;
        }
      }
    }
  }
  ChkFlags();
}

static void DecodeRMB_SMB(Word Code)
{
  if (ChkArgCnt(1, 1)
   && (ChkExactCPUMask(M_65C02 | M_W65C02S | M_65C19 | M_HUC6280, CPU6502) >= 0))
  {
    Boolean ValOK;

    BAsmCode[1] = EvalIntExpression(ArgStr[1], UInt8, &ValOK);
    if (ValOK)
    {
      BAsmCode[0] = Code;
      CodeLen = 2;
    }
  }
  ChkFlags();
}

static void DecodeRBA_SBA(Word Code)
{
  if (ChkArgCnt(2, 2)
   && ChkExactCPU(CPU65C19))
  {
    Boolean OK;
    Word Addr;

    Addr = EvalIntExpression(ArgStr[1], UInt16, &OK);
    if (OK)
    {
      BAsmCode[3] = EvalIntExpression(ImmStart(ArgStr[2]), UInt8, &OK);
      if (OK)
      {
        BAsmCode[0] = Code;
        BAsmCode[1] = Lo(Addr);
        BAsmCode[2] = Hi(Addr);
        CodeLen = 4;
      }
    }
  }
}

static void DecodeBAR_BAS(Word Code)
{
  if (ChkArgCnt(3, 3)
   && ChkExactCPU(CPU65C19))
  {
    Boolean OK;
    Word Addr;

    Addr = EvalIntExpression(ArgStr[1], UInt16, &OK);
    if (OK)
    {
      BAsmCode[3] = EvalIntExpression(ImmStart(ArgStr[2]), UInt8, &OK);
      if (OK)
      {
        Integer Dist = EvalIntExpression(ArgStr[3], UInt16, &OK) - (EProgCounter() + 5);

        if (OK)
        {
          if (((Dist > 127) || (Dist < -128)) && (!SymbolQuestionable)) WrError(1370);
          else
          {
            BAsmCode[0] = Code;
            BAsmCode[1] = Lo(Addr);
            BAsmCode[2] = Hi(Addr);
            BAsmCode[4] = Dist & 0xff;
            CodeLen = 5;
          }
        }
      }
    }
  }
}

static void DecodeSTI(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(2, 2)
   && ChkExactCPU(CPU65C19))
  {
    Boolean OK;

    BAsmCode[1] = EvalIntExpression(ArgStr[1], UInt8, &OK);
    if (OK)
    {
      BAsmCode[2] = EvalIntExpression(ImmStart(ArgStr[2]), UInt8, &OK);
      if (OK)
      {
        BAsmCode[0] = 0xb2;
        CodeLen = 3;
      }
    }
  }
}

static void DecodeLDM(Word Code)
{
  Boolean ValOK;

  UNUSED(Code);

  if (ChkArgCnt(2, 2)
   && ChkExactCPU(CPUM740))
  {
    BAsmCode[0] = 0x3c;
    BAsmCode[2] = EvalIntExpression(ArgStr[2], UInt8, &ValOK);
    if (ValOK)
    {
      if (*ArgStr[1] != '#') WrError(1350);
      else
      {
        BAsmCode[1] = EvalIntExpression(ArgStr[1] + 1, Int8, &ValOK);
        if (ValOK)
          CodeLen = 3;
      }
    }
  }
  ChkFlags();
}

static void DecodeJSB(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(1, 1)
   && ChkExactCPU(CPU65C19))
  {
    Boolean OK;
    Word Addr;

    FirstPassUnknown = False;
    Addr = EvalIntExpression(ArgStr[1], UInt16, &OK);
    if (FirstPassUnknown)
      Addr = 0xffe0;
    if (OK)
    {
      if ((Addr & 0xffe1) != 0xffe0) WrError(1910);
      else
      {
        BAsmCode[0] = 0x0b | ((Addr & 0x000e) << 3);
        CodeLen = 1;
      }
    }
  }
}

static void DecodeNorm(Word Index)
{
  const NormOrder *pOrder = NormOrders + Index;
  tAdrResult AdrResult;

  DecodeAdr(&AdrResult, pOrder);
  if (AdrResult.ErgMode == -1) WrError(1350);
  else
  {
    if (pOrder->Codes[AdrResult.ErgMode] == -1)
    {
      if (AdrResult.ErgMode == ModZA) AdrResult.ErgMode = ModA;
      if (AdrResult.ErgMode == ModZIX) AdrResult.ErgMode = ModIX;
      if (AdrResult.ErgMode == ModZIY) AdrResult.ErgMode = ModIY;
      if (AdrResult.ErgMode == ModInd8) AdrResult.ErgMode = ModInd16;
      AdrResult.AdrVals[AdrCnt++] = 0;
    }
    if (pOrder->Codes[AdrResult.ErgMode] == -1) WrError(1350);
    else if (ChkExactCPUMask(Hi(pOrder->Codes[AdrResult.ErgMode]), CPU6502) >= 0)
    {
      BAsmCode[0] = Lo(pOrder->Codes[AdrResult.ErgMode]); 
      memcpy(BAsmCode + 1, AdrResult.AdrVals, AdrResult.AdrCnt);
      CodeLen = AdrResult.AdrCnt + 1;
      if ((AdrResult.ErgMode == ModInd16) && (MomCPU != CPU65C02) && (BAsmCode[1] == 0xff))
      {
        WrError(1900);
        CodeLen = 0;
      }
    }
  }
  ChkFlags();
}

static void DecodeTST(Word Index)
{
  Byte ImmVal = 0;

  /* split off immediate argument for HUC6280 TST? */

  if (MomCPU == CPUHUC6280)
  {
    Boolean OK;
    int z;

    if (!ChkArgCnt(1, ArgCntMax))
      return;

    ImmVal = EvalIntExpression(ImmStart(ArgStr[1]), Int8, &OK);
    if (!OK)
      return;
    for (z = 1; z <= ArgCnt - 1; z++)
      strcpy(ArgStr[z], ArgStr[z + 1]);
    ArgCnt--;
  }

  /* generic generation */

  DecodeNorm(Index);

  /* if succeeded, insert immediate value */

  if ((CodeLen > 0) && (MomCPU == CPUHUC6280))
  {
    memmove(BAsmCode + 2, BAsmCode + 1, CodeLen - 1);
    BAsmCode[1] = ImmVal;
  }
}

/* relativer Sprung ? */

static void DecodeCond(Word Index)
{
  const CondOrder *pOrder = CondOrders + Index;

  if (ChkArgCnt(1, 1)
   && (ChkExactCPUMask(pOrder->CPUFlag, CPU6502) >= 0))
  {
    Integer AdrInt;
    Boolean ValOK;

    AdrInt = EvalIntExpression(ArgStr[1], UInt16, &ValOK) - (EProgCounter() + 2);
    if (ValOK)
    {
      if (((AdrInt > 127) || (AdrInt < -128)) && (!SymbolQuestionable)) WrError(1370);
      else
      {
        BAsmCode[0] = pOrder->Code;
        BAsmCode[1] = AdrInt & 0xff; 
        CodeLen = 2;
      }
      ChkFlags();
    }
  }
}

static void DecodeTransfer(Word Code)
{
  if (ChkArgCnt(3, 3)
   && ChkExactCPU(CPUHUC6280))
  {
    Boolean OK;
    Word Address;
    int z;

    for (z = 1; z <= 3; z++)
    {
      Address = EvalIntExpression(ArgStr[z], UInt16, &OK);
      if (!OK)
        return;
      BAsmCode[z * 2 - 1] = Lo(Address);
      BAsmCode[z * 2    ] = Hi(Address);
    }
    BAsmCode[0] = Code;
    CodeLen = 7;
  }
}

/*---------------------------------------------------------------------------*/

static void AddFixed(char *NName, Byte NFlag, Byte NCode)
{
  if (InstrZ >= FixedOrderCount) exit(255);
  FixedOrders[InstrZ].CPUFlag = NFlag;
  FixedOrders[InstrZ].Code = NCode;
  AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
}

static void AddNorm(char *NName, Word ZACode, Word ACode, Word ZIXCode,
                    Word IXCode, Word ZIYCode, Word IYCode, Word IndIXCode,
                    Word IndOXCode, Word IndOYCode, Word Ind16Code, Word ImmCode, Word AccCode,
                    Word NoneCode, Word Ind8Code, Word SpecCode)
{
  if (InstrZ >= NormOrderCount) exit(255);
  NormOrders[InstrZ].Codes[ModZA] = ZACode;
  NormOrders[InstrZ].Codes[ModA] = ACode;
  NormOrders[InstrZ].Codes[ModZIX] = ZIXCode;
  NormOrders[InstrZ].Codes[ModIX] = IXCode;
  NormOrders[InstrZ].Codes[ModZIY] = ZIYCode;  
  NormOrders[InstrZ].Codes[ModIY] = IYCode;
  NormOrders[InstrZ].Codes[ModIndIX] = IndIXCode;
  NormOrders[InstrZ].Codes[ModIndOX] = IndOXCode;
  NormOrders[InstrZ].Codes[ModIndOY] = IndOYCode;
  NormOrders[InstrZ].Codes[ModInd16] = Ind16Code;
  NormOrders[InstrZ].Codes[ModImm] = ImmCode;
  NormOrders[InstrZ].Codes[ModAcc] = AccCode;
  NormOrders[InstrZ].Codes[ModNone] = NoneCode;
  NormOrders[InstrZ].Codes[ModInd8] = Ind8Code;
  NormOrders[InstrZ].Codes[ModSpec] = SpecCode;
  AddInstTable(InstTable, NName, InstrZ++, strcmp(NName, "TST") ? DecodeNorm : DecodeTST);
}

static void AddCond(char *NName, Byte NFlag, Byte NCode)
{
  if (InstrZ >= CondOrderCount) exit(255);
  CondOrders[InstrZ].CPUFlag = NFlag;
  CondOrders[InstrZ].Code = NCode;
  AddInstTable(InstTable, NName, InstrZ++, DecodeCond);
}

static Word MkMask(Byte CPUMask, Byte Code)
{
  return (((Word)CPUMask) << 8) | Code;
}

static void InitFields(void)
{
  Boolean Is740 = (MomCPU == CPUM740),
          Is65C19 = (MomCPU == CPU65C19);
  int Bit;
  char Name[20];

  InstTable = CreateInstTable(207);
  SetDynamicInstTable(InstTable);

  AddInstTable(InstTable, "SEB", 0x0b, DecodeSEB_CLB);
  AddInstTable(InstTable, "CLB", 0x1b, DecodeSEB_CLB);
  AddInstTable(InstTable, "BBC", 0x13, DecodeBBC_BBS);
  AddInstTable(InstTable, "BBS", 0x03, DecodeBBC_BBS);
  AddInstTable(InstTable, "RBA", 0xc2, DecodeRBA_SBA);
  AddInstTable(InstTable, "SBA", 0xd2, DecodeRBA_SBA);
  AddInstTable(InstTable, "BAR", 0xe2, DecodeBAR_BAS);
  AddInstTable(InstTable, "BAS", 0xf2, DecodeBAR_BAS);
  AddInstTable(InstTable, "STI", 0, DecodeSTI);
  for (Bit = 0; Bit < 8; Bit++)
  {
    sprintf(Name, "BBR%d", Bit);
    AddInstTable(InstTable, Name, (Bit << 4) + 0x0f, DecodeBBR_BBS);
    sprintf(Name, "BBS%d", Bit);
    AddInstTable(InstTable, Name, (Bit << 4) + 0x8f, DecodeBBR_BBS);
    sprintf(Name, "RMB%d", Bit);
    AddInstTable(InstTable, Name, (Bit << 4) + 0x07, DecodeRMB_SMB);
    sprintf(Name, "SMB%d", Bit);
    AddInstTable(InstTable, Name, (Bit << 4) + 0x87, DecodeRMB_SMB);
  }
  AddInstTable(InstTable, "LDM", 0, DecodeLDM);
  AddInstTable(InstTable, "JSB", 0, DecodeJSB);

  AddInstTable(InstTable, "TAI"  , 0xf3, DecodeTransfer);
  AddInstTable(InstTable, "TDD"  , 0xc3, DecodeTransfer);
  AddInstTable(InstTable, "TIA"  , 0xe3, DecodeTransfer);
  AddInstTable(InstTable, "TII"  , 0x73, DecodeTransfer);
  AddInstTable(InstTable, "TIN"  , 0xd3, DecodeTransfer);

  FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCount); InstrZ = 0;
  AddFixed("RTS", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x60);
  AddFixed("RTI", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x40);
  AddFixed("TAX", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xaa);
  AddFixed("TXA", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x8a);
  AddFixed("TAY", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xa8);
  AddFixed("TYA", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x98);
  AddFixed("TXS", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x9a);
  AddFixed("TSX", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xba);
  AddFixed("DEX", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xca);
  AddFixed("DEY", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x88);
  AddFixed("INX", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xe8);
  AddFixed("INY", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xc8);
  AddFixed("PHA", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x48);
  AddFixed("PLA", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x68);
  AddFixed("PHP", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x08);
  AddFixed("PLP", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x28);
  AddFixed("PHX",          M_65SC02 | M_65C02 | M_W65C02S | M_65C19              | M_HUC6280          , 0xda);
  AddFixed("PLX",          M_65SC02 | M_65C02 | M_W65C02S | M_65C19              | M_HUC6280          , 0xfa);
  AddFixed("PHY",          M_65SC02 | M_65C02 | M_W65C02S | M_65C19              | M_HUC6280          , 0x5a);
  AddFixed("PLY",          M_65SC02 | M_65C02 | M_W65C02S | M_65C19              | M_HUC6280          , 0x7a);
  AddFixed("BRK", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x00);
  AddFixed("STP",                               M_W65C02S |           M_MELPS740                      , Is740 ? 0x42 : 0xdb);
  AddFixed("WAI",                               M_W65C02S                                             , 0xcb);
  AddFixed("SLW",                                                     M_MELPS740                      , 0xc2);
  AddFixed("FST",                                                     M_MELPS740                      , 0xe2);
  AddFixed("WIT",                                                     M_MELPS740                      , 0xc2);
  AddFixed("CLI", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x58);
  AddFixed("SEI", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x78);
  AddFixed("CLC", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x18);
  AddFixed("SEC", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x38);
  AddFixed("CLD", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xd8);
  AddFixed("SED", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xf8);
  AddFixed("CLV", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xb8);
  AddFixed("CLT",                                                     M_MELPS740                      , 0x12);
  AddFixed("SET",                                                     M_MELPS740 | M_HUC6280          , Is740 ? 0x32 : 0xf4); /* !!! for HUC6280, is prefix for (x) instead of ACC as dest */
  AddFixed("JAM",                                                                              M_6502U, 0x02);
  AddFixed("CRS",                                                                              M_6502U, 0x02);
  AddFixed("KIL",                                                                              M_6502U, 0x02);
  AddFixed("CLW",                                           M_65C19                                   , 0x52);
  AddFixed("MPY",                                           M_65C19                                   , 0x02);
  AddFixed("MPA",                                           M_65C19                                   , 0x12);
  AddFixed("PSH",                                           M_65C19                                   , 0x22);
  AddFixed("PUL",                                           M_65C19                                   , 0x32);
  AddFixed("PHW",                                           M_65C19                                   , 0x23);
  AddFixed("PLW",                                           M_65C19                                   , 0x33);
  AddFixed("RND",                                           M_65C19                                   , 0x42);
  AddFixed("TAW",                                           M_65C19                                   , 0x62);
  AddFixed("TWA",                                           M_65C19                                   , 0x72);
  AddFixed("NXT",                                           M_65C19                                   , 0x8b);
  AddFixed("LII",                                           M_65C19                                   , 0x9b);
  AddFixed("LAI",                                           M_65C19                                   , 0xeb);
  AddFixed("INI",                                           M_65C19                                   , 0xbb);
  AddFixed("PHI",                                           M_65C19                                   , 0xcb);
  AddFixed("PLI",                                           M_65C19                                   , 0xdb);
  AddFixed("TIP",                                           M_65C19                                   , 0x03);
  AddFixed("PIA",                                           M_65C19                                   , 0xfb);
  AddFixed("LAN",                                           M_65C19                                   , 0xab);
  AddFixed("CLA",                                                                  M_HUC6280          , 0x62);
  AddFixed("CLX",                                                                  M_HUC6280          , 0x82);
  AddFixed("CLY",                                                                  M_HUC6280          , 0xc2);
  AddFixed("CSL",                                                                  M_HUC6280          , 0x54);
  AddFixed("CSH",                                                                  M_HUC6280          , 0xd4);
  AddFixed("SAY",                                                                  M_HUC6280          , 0x42);
  AddFixed("SXY",                                                                  M_HUC6280          , 0x02);

  NormOrders = (NormOrder *) malloc(sizeof(NormOrder) * NormOrderCount); InstrZ = 0;
  AddNorm("NOP",
  /* ZA    */ MkMask(                                                                             M_6502U, 0x04),
  /* A     */ MkMask(                                                                             M_6502U, 0x0c),
  /* ZIX   */ MkMask(                                                                             M_6502U, 0x14),
  /* IX    */ MkMask(                                                                             M_6502U, 0x1c),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                                                             M_6502U, 0x80),
  /* ACC   */     -1,
  /* NON   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xea),
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("LDA",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xa5),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xad),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xb5),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xbd),
  /* ZIY   */     -1,
  /* IY    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xb9),
  /* (n,X) */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280 | M_6502U, 0xa1),
  /* (n),X */ MkMask(                                          M_65C19                                   , 0xb1),
  /* (n),Y */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280 | M_6502U, 0xb1),
  /* (n16) */ MkMask(                                                                 M_HUC6280          , 0xb2),
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xa9),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740             | M_6502U, Is65C19 ? 0xa1 : 0xb2),
  /* spec  */     -1);
  AddNorm("LDX",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xa6),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xae),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xb6),
  /* IY    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xbe),
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xa2),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("LDY",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xa4),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xac),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xb4),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xbc),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xa0),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("STA",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x85),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x8d),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x95),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x9d),
  /* ZIY   */     -1,
  /* IY    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x99),
  /* (n,X) */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280 | M_6502U, 0x81),
  /* (n),X */ MkMask(                                          M_65C19                                   , 0x91),
  /* (n),Y */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280 | M_6502U, 0x91),
  /* (n16) */ MkMask(                                                                 M_HUC6280          , 0x92),
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S | M_65C19                                   , Is65C19 ? 0x81 : 0x92),
  /* spec  */     -1);
  AddNorm("ST0",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                                                 M_HUC6280          , 0x03),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("ST1",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                                                 M_HUC6280          , 0x13),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("ST2",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                                                 M_HUC6280          , 0x23),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("STX",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x86),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x8e),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x96),
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("STY",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x84),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x8c),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x94),
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("STZ",
  /* ZA    */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                        | M_HUC6280          , 0x64),
  /* A     */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                        | M_HUC6280          , 0x9c),
  /* ZIX   */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                        | M_HUC6280          , 0x74),
  /* IX    */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                        | M_HUC6280          , 0x9e),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("ADC",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x65),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x6d),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x75),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x7d),
  /* ZIY   */     -1,
  /* IY    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x79),
  /* (n,X) */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280 | M_6502U, 0x61),
  /* (n),X */ MkMask(                                          M_65C19                                   , 0x71),
  /* (n),Y */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280 | M_6502U, 0x71),
  /* (n16) */ MkMask(                                                                 M_HUC6280          , 0x72),
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x69),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S | M_65C19                                   , Is65C19 ? 0x61 : 0x72),
  /* spec  */     -1);
  AddNorm("ADD",
  /* ZA    */ MkMask(                                          M_65C19                                   , 0x64),
  /* A     */     -1,
  /* ZIX   */ MkMask(                                          M_65C19                                   , 0x74),
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                          M_65C19                                   , 0x89),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("SBC",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xe5),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xed),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xf5),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xfd),
  /* ZIY   */     -1,
  /* IY    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xf9),
  /* (n,X) */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280 | M_6502U, 0xe1),
  /* (n),X */ MkMask(                                          M_65C19                                   , 0xf1),
  /* (n),Y */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280 | M_6502U, 0xf1),
  /* (n16) */ MkMask(                                                                 M_HUC6280          , 0xf2),
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xe9),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S | M_65C19                                   , Is65C19 ? 0xe1 : 0xf2),
  /* spec  */     -1);
  AddNorm("MUL",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */ MkMask(                                                    M_MELPS740                      , 0x62),
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("DIV",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */ MkMask(                                                    M_MELPS740                      , 0xe2),
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("AND",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x25),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x2d),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x35),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x3d),
  /* ZIY   */     -1,
  /* IY    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x39),
  /* (n,X) */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280 | M_6502U, 0x21),
  /* (n),X */ MkMask(                                          M_65C19                                   , 0x31),
  /* (n),Y */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280 | M_6502U, 0x31),
  /* (n16) */ MkMask(                                                                 M_HUC6280          , 0x32),
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x29),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S | M_65C19                                   , Is65C19 ? 0x21 : 0x32),
  /* spec  */     -1);
  AddNorm("ORA",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x05),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x0d),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x15),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x1d),
  /* ZIY   */     -1,
  /* IY    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x19),
  /* (n,X) */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280 | M_6502U, 0x01),
  /* (n),X */ MkMask(                                          M_65C19                                   , 0x11),
  /* (n),Y */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280 | M_6502U, 0x11),
  /* (n16) */ MkMask(                                                                 M_HUC6280          , 0x12),
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x09),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S | M_65C19                                   , Is65C19 ? 0x01 : 0x12),
  /* spec  */     -1);
  AddNorm("EOR",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x45),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x4d),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x55),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x5d),
  /* ZIY   */     -1,
  /* IY    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x59),
  /* (n,X) */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280 | M_6502U, 0x41),
  /* (n),X */ MkMask(                                          M_65C19                                   , 0x51),
  /* (n),Y */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280 | M_6502U, 0x51),
  /* (n16) */ MkMask(                                                                 M_HUC6280          , 0x52),
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x49),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S | M_65C19                                   , Is65C19 ? 0x41 : 0x52),
  /* spec  */     -1);
  AddNorm("COM",
  /* ZA    */ MkMask(                                                    M_MELPS740          , 0x44),
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("BIT",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_HUC6280 | M_MELPS740 | M_6502U, 0x24),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_HUC6280 | M_MELPS740 | M_6502U, 0x2c),
  /* ZIX   */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S           | M_HUC6280                       , 0x34),
  /* IX    */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S           | M_HUC6280                       , 0x3c),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S           | M_HUC6280                       , 0x89),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("TST", /* TODO: 6280 */
  /* ZA    */ MkMask(                                                    M_MELPS740 | M_HUC6280          , Is740 ? 0x64 : 0x83),
  /* A     */ MkMask(                                                                 M_HUC6280          , 0x93),
  /* ZIX   */ MkMask(                                                                 M_HUC6280          , 0xa3),
  /* IX    */ MkMask(                                                                 M_HUC6280          , 0xb3),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("ASL",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x06),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x0e),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x16),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x1e),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x0a),
  /* NON   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x0a),
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("ASR",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                                                             M_6502U, 0x4b),
  /* ACC   */ MkMask(                                          M_65C19                                   , 0x3a),
  /* NON   */ MkMask(                                          M_65C19                                   , 0x3a),
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("LAB",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */ MkMask(                                          M_65C19                                   , 0x13),
  /* NON   */ MkMask(                                          M_65C19                                   , 0x13),
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("NEG",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */ MkMask(                                          M_65C19                                   , 0x1a),
  /* NON   */ MkMask(                                          M_65C19                                   , 0x1a),
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("LSR",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x46),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x4e),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x56),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x5e),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x4a),
  /* NON   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x4a),
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("ROL",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x26),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x2e),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x36),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x3e),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x2a),
  /* NON   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x2a),
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("ROR",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x66),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x6e),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x76),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x7e),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x6a),
  /* NON   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x6a),
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("RRF",
  /* ZA    */ MkMask(                                                    M_MELPS740                      , 0x82),
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("TSB",
  /* ZA    */ MkMask(                   M_65SC02 | M_65C02 | M_W65C02S              | M_HUC6280          , 0x04),
  /* A     */ MkMask(                   M_65SC02 | M_65C02 | M_W65C02S              | M_HUC6280          , 0x0c),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("TRB",
  /* ZA    */ MkMask(                   M_65SC02 | M_65C02 | M_W65C02S              | M_HUC6280          , 0x14),
  /* A     */ MkMask(                   M_65SC02 | M_65C02 | M_W65C02S              | M_HUC6280          , 0x1c),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("INC",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xe6),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xee),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xf6),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xfe),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280          , Is740 ? 0x3a : 0x1a), 
  /* NON   */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280          , Is740 ? 0x3a : 0x1a),
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("DEC",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xc6),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xce),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xd6),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xde),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280          , Is740 ? 0x1a : 0x3a), 
  /* NON   */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280          , Is740 ? 0x1a : 0x3a), 
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("CMP",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xc5),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xcd),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xd5),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xdd),
  /* ZIY   */     -1,
  /* IY    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xd9),
  /* (n,X) */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280 | M_6502U, 0xc1),
  /* (n),X */ MkMask(                                          M_65C19                                   , 0xd1),
  /* (n),Y */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S           | M_MELPS740 | M_HUC6280 | M_6502U, 0xd1),
  /* (n16) */ MkMask(                                                                 M_HUC6280          , 0xd2),
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xc9),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S | M_65C19                                   , Is65C19 ? 0xc1 : 0xd2),
  /* spec  */     -1);
  AddNorm("CPX",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xe4),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xec),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xe0),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("CPY",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xc4),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xcc),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xc0),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("JMP",
  /* ZA    */     -1,
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x4c),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S | M_65C19              | M_HUC6280          , 0x7c),
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x6c),
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(                                                    M_MELPS740                      , 0xb2),
  /* spec  */     -1);
  AddNorm("JPI",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */ MkMask(                                          M_65C19                                   , 0x0c),
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("JSR",
  /* ZA    */     -1,
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x20),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(                                                    M_MELPS740                      , 0x02),
  /* spec  */ MkMask(                                                    M_MELPS740                      , 0x22));
  AddNorm("SLO",
  /* ZA    */ MkMask(                                                                             M_6502U, 0x07),
  /* A     */ MkMask(                                                                             M_6502U, 0x0f),
  /* ZIX   */ MkMask(                                                                             M_6502U, 0x17),
  /* IX    */ MkMask(                                                                             M_6502U, 0x1f),
  /* ZIY   */     -1,
  /* IY    */ MkMask(                                                                             M_6502U, 0x1b),
  /* (n,X) */ MkMask(                                                                             M_6502U, 0x03),
  /* (n),X */     -1,
  /* (n),Y */ MkMask(                                                                             M_6502U, 0x13),
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("ANC",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                                                             M_6502U, 0x0b),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("RLA",
  /* ZA    */ MkMask(                                                                             M_6502U, 0x27),
  /* A     */ MkMask(                                                                             M_6502U, 0x2f),
  /* ZIX   */ MkMask(                                                                             M_6502U, 0x37),
  /* IX    */ MkMask(                                                                             M_6502U, 0x3f),
  /* ZIY   */     -1,
  /* IY    */ MkMask(                                                                             M_6502U, 0x3b),
  /* (n,X) */ MkMask(                                                                             M_6502U, 0x23),
  /* (n),X */     -1,
  /* (n),Y */ MkMask(                                                                             M_6502U, 0x33),
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("SRE",
  /* ZA    */ MkMask(                                                                             M_6502U, 0x47),
  /* A     */ MkMask(                                                                             M_6502U, 0x4f),
  /* ZIX   */ MkMask(                                                                             M_6502U, 0x57),
  /* IX    */ MkMask(                                                                             M_6502U, 0x5f),
  /* ZIY   */     -1,
  /* IY    */ MkMask(                                                                             M_6502U, 0x5b),
  /* (n,X) */ MkMask(                                                                             M_6502U, 0x43),
  /* (n),X */     -1,
  /* (n),Y */ MkMask(                                                                             M_6502U, 0x53),
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("RRA",
  /* ZA    */ MkMask(                                                                             M_6502U, 0x67),
  /* A     */ MkMask(                                                                             M_6502U, 0x6f),
  /* ZIX   */ MkMask(                                                                             M_6502U, 0x77),
  /* IX    */ MkMask(                                                                             M_6502U, 0x7f),
  /* ZIY   */     -1,
  /* IY    */ MkMask(                                                                             M_6502U, 0x7b),
  /* (n,X) */ MkMask(                                                                             M_6502U, 0x63),
  /* (n),X */     -1,
  /* (n),Y */ MkMask(                                                                             M_6502U, 0x73),
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */    -1);
  AddNorm("ARR",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                                                             M_6502U, 0x6b),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("SAX",
  /* ZA    */ MkMask(                                                                             M_6502U, 0x87),
  /* A     */ MkMask(                                                                             M_6502U, 0x8f),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */ MkMask(                                                                             M_6502U, 0x97),
  /* IY    */     -1,
  /* (n,X) */ MkMask(                                                                             M_6502U, 0x83),
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */ MkMask(                                                                 M_HUC6280          , 0x22),
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("ANE",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                                                             M_6502U, 0x8b),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("SHA",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */ MkMask(                                                                             M_6502U, 0x93),
  /* ZIY   */     -1,
  /* IY    */ MkMask(                                                                             M_6502U, 0x9f),
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("SHS",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */ MkMask(                                                                             M_6502U, 0x9b),
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("SHY",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */ MkMask(                                                                             M_6502U, 0x9c),
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("SHX",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */ MkMask(                                                                             M_6502U, 0x9e),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("LAX",
  /* ZA    */ MkMask(                                                                             M_6502U, 0xa7),
  /* A     */ MkMask(                                                                             M_6502U, 0xaf),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */ MkMask(                                                                             M_6502U, 0xb7),
  /* IY    */ MkMask(                                                                             M_6502U, 0xbf),
  /* (n,X) */ MkMask(                                                                             M_6502U, 0xa3),
  /* (n),X */     -1,
  /* (n),Y */ MkMask(                                                                             M_6502U, 0xb3),
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("LXA",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                                                             M_6502U, 0xab),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("LAE",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */ MkMask(                                                                             M_6502U, 0xbb),
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("DCP",
  /* ZA    */ MkMask(                                                                             M_6502U, 0xc7),
  /* A     */ MkMask(                                                                             M_6502U, 0xcf),
  /* ZIX   */ MkMask(                                                                             M_6502U, 0xd7),
  /* IX    */ MkMask(                                                                             M_6502U, 0xdf),
  /* ZIY   */     -1,
  /* IY    */ MkMask(                                                                             M_6502U, 0xdb),
  /* (n,X) */ MkMask(                                                                             M_6502U, 0xc3),
  /* (n),X */     -1,
  /* (n),Y */ MkMask(                                                                             M_6502U, 0xd3),
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("SBX",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                                                             M_6502U, 0xcb),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("ISB",
  /* ZA    */ MkMask(                                                                             M_6502U, 0xe7),
  /* A     */ MkMask(                                                                             M_6502U, 0xef),
  /* ZIX   */ MkMask(                                                                             M_6502U, 0xf7),
  /* IX    */ MkMask(                                                                             M_6502U, 0xff),
  /* ZIY   */     -1,
  /* IY    */ MkMask(                                                                             M_6502U, 0xfb),
  /* (n,X) */ MkMask(                                                                             M_6502U, 0xe3),
  /* (n),X */     -1,
  /* (n),Y */ MkMask(                                                                             M_6502U, 0xf3),
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("EXC",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */ MkMask(                                          M_65C19                                   , 0xd4),
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("TAM",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                                                 M_HUC6280          , 0x53),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("TMA",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* (n,X) */     -1,
  /* (n),X */     -1,
  /* (n),Y */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                                                 M_HUC6280          , 0x43),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);

  CondOrders = (CondOrder *) malloc(sizeof(CondOrder) * CondOrderCount); InstrZ = 0;
  AddCond("BEQ", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xf0);
  AddCond("BNE", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xd0);
  AddCond("BPL", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x10);
  AddCond("BMI", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x30);
  AddCond("BCC", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x90);
  AddCond("BCS", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0xb0);
  AddCond("BVC", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x50);
  AddCond("BVS", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280 | M_6502U, 0x70);
  AddCond("BRA",          M_65SC02 | M_65C02 | M_W65C02S | M_65C19 | M_MELPS740 | M_HUC6280          , 0x80);
  AddCond("BSR",                                                                  M_HUC6280          , 0x44);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);

  free(FixedOrders);
  free(NormOrders);
  free(CondOrders);
}

/*---------------------------------------------------------------------------*/

static void MakeCode_65(void)
{
  CodeLen = 0;
  DontPrint = False;

  /* zu ignorierendes */

  if (Memo(""))
  {
    ChkFlags();
    return;
  }

  /* Pseudoanweisungen */

  if (DecodeMotoPseudo(False))
  {
    ChkFlags();
    return;
  }

  if (!LookupInstTable(InstTable, OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownOpcode, &OpPart);
}

static void InitCode_65(void)
{
  int z;

  CLI_SEI_Flag = False;
  ADC_SBC_Flag = False;
  for (z = 0; z < 8; z++)
    MPR[z] = z;
}

static Boolean IsDef_65(void)
{
  return False;
}

static void SwitchFrom_65(void)
{
  DeinitFields();
}

static void SwitchTo_65(void)
{
  TurnWords = False;
  ConstMode = ConstModeMoto;
  SetIsOccupied = (MomCPU == CPUM740);

  PCSymbol = "*";
  HeaderID = 0x11;
  NOPCode = 0xea;
  DivideChars = ",";
  HasAttrs = False;

  ValidSegs = 1 << SegCode;
  Grans[SegCode] = 1; ListGrans[SegCode] = 1; SegInits[SegCode] = 0;
  SegLimits[SegCode] = (MomCPU == CPUHUC6280) ? 0x1fffff : 0xffff;

  if (MomCPU == CPUM740)
  {
    static ASSUMERec ASSUME740s[] =
    {
      { "SP", &SpecPage, 0, 0xff, -1, NULL }
    };

    pASSUMERecs = ASSUME740s;
    ASSUMERecCnt = (sizeof(ASSUME740s) / sizeof(*ASSUME740s));
    SetIsOccupied = True;
  }
  else if (MomCPU == CPUHUC6280)
  {
    static ASSUMERec ASSUME6280s[] =
    {
      { "MPR0", MPR + 0, 0, 0xff, -1, NULL },
      { "MPR1", MPR + 1, 0, 0xff, -1, NULL },
      { "MPR2", MPR + 2, 0, 0xff, -1, NULL },
      { "MPR3", MPR + 3, 0, 0xff, -1, NULL },
      { "MPR4", MPR + 4, 0, 0xff, -1, NULL },
      { "MPR5", MPR + 5, 0, 0xff, -1, NULL },
      { "MPR6", MPR + 6, 0, 0xff, -1, NULL },
      { "MPR7", MPR + 7, 0, 0xff, -1, NULL },
    };

    pASSUMERecs = ASSUME6280s;
    ASSUMERecCnt = (sizeof(ASSUME6280s) / sizeof(*ASSUME6280s));
    SetIsOccupied = True;
  }
  else
    SetIsOccupied = False;

  MakeCode = MakeCode_65;
  IsDef = IsDef_65;
  SwitchFrom = SwitchFrom_65;
  InitFields();
}

void code65_init(void)
{
  CPU6502    = AddCPU("6502"     , SwitchTo_65);
  CPU65SC02  = AddCPU("65SC02"   , SwitchTo_65);
  CPU65C02   = AddCPU("65C02"    , SwitchTo_65);
  CPUW65C02S = AddCPU("W65C02S"  , SwitchTo_65);
  CPU65C19   = AddCPU("65C19"    , SwitchTo_65);
  CPUM740    = AddCPU("MELPS740" , SwitchTo_65);
  CPUHUC6280 = AddCPU("HUC6280"  , SwitchTo_65);
  CPU6502U   = AddCPU("6502UNDOC", SwitchTo_65);

  AddInitPassProc(InitCode_65);
}
