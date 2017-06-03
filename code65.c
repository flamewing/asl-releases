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
/* $Id: code65.c,v 1.10 2017/06/02 20:01:21 alfred Exp $                      */
/*****************************************************************************
 * $Log: code65.c,v $
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
  ModIndX  = 6,   /* (aa,X) */
  ModIndY  = 7,   /* (aa),Y */
  ModInd16 = 8,   /* (aabb) */
  ModImm   = 9,   /* #aa */
  ModAcc  = 10,   /* A */
  ModNone = 11,   /* */
  ModInd8 = 12,   /* (aa) */
  ModSpec = 13,   /* \aabb */
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

#define FixedOrderCount 38
#define NormOrderCount 51
#define CondOrderCount 9

/* NOTE: keep in the same order as in registration in code65_init()! */

#define M_6502      (1 << 0)
#define M_65SC02    (1 << 1)
#define M_65C02     (1 << 2)
#define M_W65C02S   (1 << 3)
#define M_MELPS740  (1 << 4)
#define M_6502U     (1 << 5)

static Boolean CLI_SEI_Flag, ADC_SBC_Flag;

static FixedOrder *FixedOrders;
static NormOrder *NormOrders;
static CondOrder *CondOrders;

static CPUVar CPU6502, CPU65SC02, CPU65C02, CPUW65C02S, CPUM740, CPU6502U;
static LongInt SpecPage;

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
        if (Hi(AdrWord) != SpecPage) WrError(1315);
        else
        {
          pResult->ErgMode = ModSpec;
          pResult->AdrVals[0] = Lo(AdrWord);
          pResult->AdrCnt = 1;
        }
      }
    }

    /* 4. X-indirekt ? */

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
          AdrWord = EvalIntExpression(s1, UInt16, &ValOK);
          if (ValOK)
          {
            pResult->AdrVals[0] = Lo(AdrWord);
            pResult->AdrVals[1] = Hi(AdrWord);
            pResult->ErgMode = ModIndX;
            pResult->AdrCnt = 2;
          }
        }
        else
        {
          pResult->AdrVals[0] = EvalIntExpression(s1, UInt8, &ValOK);
          if (ValOK)
          {
            pResult->ErgMode = ModIndX;
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
          pResult->AdrVals[0] = EvalIntExpression(s1, UInt8, &ValOK);
          if (ValOK)
          {
            pResult->ErgMode = ModInd8;
            pResult->AdrCnt = 1;
          }
        }
        else
        {
          AdrWord = EvalIntExpression(s1, UInt16, &ValOK);
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
          pResult->AdrVals[0] = EvalIntExpression(ArgStr[1], UInt8, &ValOK);
          if (ValOK)
          {
            pResult->ErgMode = ModZA;
            pResult->AdrCnt = 1;
          }
        }
        else
        {
          AdrWord = EvalIntExpression(ArgStr[1], UInt16, &ValOK);
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
    /* 7. Y-indirekt ? */

    if ((IsIndirect(ArgStr[1])) && (!strcasecmp(ArgStr[2], "Y")))
    {
      strcpy(s1, ArgStr[1] + 1);
      s1[strlen(s1) - 1] = '\0';
      ChkZero(s1, &ZeroMode);
      pResult->AdrVals[0] = EvalIntExpression(s1, UInt8, &ValOK);
      if (ValOK)
      {
        pResult->ErgMode = ModIndY;
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
        pResult->AdrVals[0] = EvalIntExpression(s1, UInt8, &ValOK);
        if (ValOK)
        {
          pResult->AdrCnt = 1;
          if (!strcasecmp(ArgStr[2], "X"))
            pResult->ErgMode = ModZIX;
          else if (!strcasecmp(ArgStr[2], "Y"))
            pResult->ErgMode = ModZIY;
          else
            WrXError(1445, ArgStr[2]);
        }
      }
      else
      {
        AdrWord = EvalIntExpression(s1, Int16, &ValOK);
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
            WrXError(1445, ArgStr[2]);
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
    WrError(1110);
    ChkFlags();
  }
}

/*---------------------------------------------------------------------------*/

/* Anweisungen ohne Argument */

static void DecodeFixed(Word Index)
{
  const FixedOrder *pOrder = FixedOrders + Index;

  if (ArgCnt != 0) WrError(1110);
  else if (!CPUAllowed(pOrder->CPUFlag)) WrError(1500);
  else
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
  if (ArgCnt != 2) WrError(1110);
  else if (MomCPU != CPUM740) WrError(1500);
  else
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

  if (ArgCnt != 3) WrError(1110);
  else if (MomCPU != CPUM740) WrError(1500);
  else
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

  if (ArgCnt != 2) WrError(1110);
  else if (!CPUAllowed(M_65C02 | M_W65C02S)) WrError(1500);
  else
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
  if (ArgCnt != 1) WrError(1110);
  else if (!CPUAllowed(M_65C02 | M_W65C02S)) WrError(1500);
  else
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

static void DecodeLDM(Word Code)
{
  Boolean ValOK;

  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else if (MomCPU != CPUM740) WrError(1500);
  else
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
    else if (!CPUAllowed(Hi(pOrder->Codes[AdrResult.ErgMode]))) WrError(1500);
    else
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

/* relativer Sprung ? */

static void DecodeCond(Word Index)
{
  const CondOrder *pOrder = CondOrders + Index;

  if (ArgCnt != 1) WrError(1110);
  else if (!CPUAllowed(pOrder->CPUFlag)) WrError(1500);
  else
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

/*---------------------------------------------------------------------------*/

static void AddFixed(char *NName, Byte NFlag, Byte NCode)
{
  if (InstrZ >= FixedOrderCount) exit(255);
  FixedOrders[InstrZ].CPUFlag = NFlag;
  FixedOrders[InstrZ].Code = NCode;
  AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
}

static void AddNorm(char *NName, Word ZACode, Word ACode, Word ZIXCode,
                    Word IXCode, Word ZIYCode, Word IYCode, Word IndXCode,
                    Word IndYCode, Word Ind16Code, Word ImmCode, Word AccCode,
                    Word NoneCode, Word Ind8Code, Word SpecCode)
{
  if (InstrZ >= NormOrderCount) exit(255);
  NormOrders[InstrZ].Codes[ModZA] = ZACode;
  NormOrders[InstrZ].Codes[ModA] = ACode;
  NormOrders[InstrZ].Codes[ModZIX] = ZIXCode;
  NormOrders[InstrZ].Codes[ModIX] = IXCode;
  NormOrders[InstrZ].Codes[ModZIY] = ZIYCode;  
  NormOrders[InstrZ].Codes[ModIY] = IYCode;
  NormOrders[InstrZ].Codes[ModIndX] = IndXCode;  
  NormOrders[InstrZ].Codes[ModIndY] = IndYCode;
  NormOrders[InstrZ].Codes[ModInd16] = Ind16Code;
  NormOrders[InstrZ].Codes[ModImm] = ImmCode;
  NormOrders[InstrZ].Codes[ModAcc] = AccCode;
  NormOrders[InstrZ].Codes[ModNone] = NoneCode;
  NormOrders[InstrZ].Codes[ModInd8] = Ind8Code;
  NormOrders[InstrZ].Codes[ModSpec] = SpecCode;
  AddInstTable(InstTable, NName, InstrZ++, DecodeNorm);
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
  Boolean Is740 = (MomCPU == CPUM740);
  int Bit;
  char Name[20];

  InstTable = CreateInstTable(207);
  SetDynamicInstTable(InstTable);

  AddInstTable(InstTable, "SEB", 0x0b, DecodeSEB_CLB);
  AddInstTable(InstTable, "CLB", 0x1b, DecodeSEB_CLB);
  AddInstTable(InstTable, "BBC", 0x13, DecodeBBC_BBS);
  AddInstTable(InstTable, "BBS", 0x03, DecodeBBC_BBS);
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

  FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCount); InstrZ = 0;
  AddFixed("RTS", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x60);
  AddFixed("RTI", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x40);
  AddFixed("TAX", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xaa);
  AddFixed("TXA", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x8a);
  AddFixed("TAY", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xa8);
  AddFixed("TYA", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x98);
  AddFixed("TXS", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x9a);
  AddFixed("TSX", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xba);
  AddFixed("DEX", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xca);
  AddFixed("DEY", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x88);
  AddFixed("INX", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xe8);
  AddFixed("INY", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xc8);
  AddFixed("PHA", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x48);
  AddFixed("PLA", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x68);
  AddFixed("PHP", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x08);
  AddFixed("PLP", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x28);
  AddFixed("PHX",          M_65SC02 | M_65C02 | M_W65C02S                       , 0xda);
  AddFixed("PLX",          M_65SC02 | M_65C02 | M_W65C02S                       , 0xfa);
  AddFixed("PHY",          M_65SC02 | M_65C02 | M_W65C02S                       , 0x5a);
  AddFixed("PLY",          M_65SC02 | M_65C02 | M_W65C02S                       , 0x7a);
  AddFixed("BRK", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x00);
  AddFixed("STP",                               M_W65C02S | M_MELPS740          , Is740 ? 0x42 : 0xdb);
  AddFixed("WAI",                               M_W65C02S                       , 0xcb);
  AddFixed("SLW",                                           M_MELPS740          , 0xc2);
  AddFixed("FST",                                           M_MELPS740          , 0xe2);
  AddFixed("WIT",                                           M_MELPS740          , 0xc2);
  AddFixed("CLI", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x58);
  AddFixed("SEI", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x78);
  AddFixed("CLC", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x18);
  AddFixed("SEC", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x38);
  AddFixed("CLD", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xd8);
  AddFixed("SED", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xf8);
  AddFixed("CLV", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xb8);
  AddFixed("CLT",                                           M_MELPS740          , 0x12);
  AddFixed("SET",                                           M_MELPS740          , 0x32);
  AddFixed("JAM",                                                        M_6502U, 0x02);
  AddFixed("CRS",                                                        M_6502U, 0x02);
  AddFixed("KIL",                                                        M_6502U, 0x02);


  NormOrders = (NormOrder *) malloc(sizeof(NormOrder) * NormOrderCount); InstrZ = 0;
              /* ZA      A       ZIX     IX      ZIY     IY      @X      @Y     (n16)    imm     ACC     NON    (n8)    spec */
  AddNorm("NOP",
  /* ZA    */ MkMask(                                                       M_6502U, 0x04),
  /* A     */ MkMask(                                                       M_6502U, 0x0c),
  /* ZIX   */ MkMask(                                                       M_6502U, 0x14),
  /* IX    */ MkMask(                                                       M_6502U, 0x1c),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                                       M_6502U, 0x80),
  /* ACC   */     -1,
  /* NON   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xea),
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("LDA",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xa5),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xad),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xb5),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xbd),
  /* ZIY   */     -1,
  /* IY    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xb9),
  /* @X    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xa1),
  /* @Y    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xb1),
  /* (n16) */     -1,
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xa9),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xb2),
  /* spec  */     -1);
  AddNorm("LDX",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xa6),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xae),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xb6),
  /* IY    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xbe),
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xa2),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("LDY",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xa4),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xac),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xb4),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xbc),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xa0),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("STA",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x85),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x8d),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x95),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x9d),
  /* ZIY   */     -1,
  /* IY    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x99),
  /* @X    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x81),
  /* @Y    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x91),
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                       , 0x92),
  /* spec  */     -1);
  AddNorm("STX",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x86),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x8e),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x96),
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("STY",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x84),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x8c),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x94),
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("STZ",
  /* ZA    */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                       , 0x64),
  /* A     */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                       , 0x9c),
  /* ZIX   */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                       , 0x74),
  /* IX    */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                       , 0x9e),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("ADC",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x65),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x6d),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x75),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x7d),
  /* ZIY   */     -1,
  /* IY    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x79),
  /* @X    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x61),
  /* @Y    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x71),
  /* (n16) */     -1,
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x69),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                       , 0x72),
  /* spec  */     -1);
  AddNorm("SBC",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xe5),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xed),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xf5),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xfd),
  /* ZIY   */     -1,
  /* IY    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xf9),
  /* @X    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xe1),
  /* @Y    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xf1),
  /* (n16) */     -1,
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xe9),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                       , 0xf2),
  /* spec  */     -1);
  AddNorm("MUL",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */ MkMask(                                          M_MELPS740          , 0x62),
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("DIV",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */ MkMask(                                          M_MELPS740          , 0xe2),
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("AND",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x25),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x2d),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x35),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x3d),
  /* ZIY   */     -1,
  /* IY    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x39),
  /* @X    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x21),
  /* @Y    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x31),
  /* (n16) */     -1,
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x29),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                       , 0x32),
  /* spec  */     -1);
  AddNorm("ORA",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x05),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x0d),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x15),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x1d),
  /* ZIY   */     -1,
  /* IY    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x19),
  /* @X    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x01),
  /* @Y    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x11),
  /* (n16) */     -1,
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x09),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                       , 0x12),
  /* spec  */     -1);
  AddNorm("EOR",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x45),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x4d),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x55),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x5d),
  /* ZIY   */     -1,
  /* IY    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x59),
  /* @X    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x41),
  /* @Y    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x51),
  /* (n16) */     -1,
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x49),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                       , 0x52),
  /* spec  */     -1);
  AddNorm("COM",
  /* ZA    */ MkMask(                                          M_MELPS740          , 0x44),
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("BIT",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x24),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x2c),
  /* ZIX   */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                       , 0x34),
  /* IX    */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                       , 0x3c),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                       , 0x89),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("TST",
  /* ZA    */ MkMask(                                          M_MELPS740          , 0x64),
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("ASL",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x06),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x0e),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x16),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x1e),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x0a),
  /* NON   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x0a),
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("LSR",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x46),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x4e),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x56),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x5e),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x4a),
  /* NON   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x4a),
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("ROL",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x26),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x2e),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x36),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x3e),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x2a),
  /* NON   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x2a),
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("ROR",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x66),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x6e),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x76),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x7e),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x6a),
  /* NON   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x6a),
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("RRF",
  /* ZA    */ MkMask(                                          M_MELPS740          , 0x82),
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("TSB",
  /* ZA    */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                       , 0x04),
  /* A     */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                       , 0x0c),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("TRB",
  /* ZA    */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                       , 0x14),
  /* A     */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                       , 0x1c),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("INC",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xe6),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xee),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xf6),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xfe),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740          , Is740 ? 0x3a : 0x1a), 
  /* NON   */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740          , Is740 ? 0x3a : 0x1a),
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("DEC",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xc6),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xce),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xd6),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xde),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740          , Is740 ? 0x1a : 0x3a), 
  /* NON   */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740          , Is740 ? 0x1a : 0x3a), 
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("CMP",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xc5),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xcd),
  /* ZIX   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xd5),
  /* IX    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xdd),
  /* ZIY   */     -1,
  /* IY    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xd9),
  /* @X    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xc1),
  /* @Y    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xd1),
  /* (n16) */     -1,
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xc9),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                       , 0xd2),
  /* spec  */     -1);
  AddNorm("CPX",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xe4),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xec),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xe0),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("CPY",
  /* ZA    */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xc4),
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xcc),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xc0),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("JMP",
  /* ZA    */     -1,
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x4c),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */ MkMask(         M_65SC02 | M_65C02 | M_W65C02S                       , 0x7c),
  /* @Y    */     -1,
  /* (n16) */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x6c),
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(                                          M_MELPS740          , 0xb2),
  /* spec  */     -1);
  AddNorm("JSR",
  /* ZA    */     -1,
  /* A     */ MkMask(M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x20),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */ MkMask(                                          M_MELPS740          , 0x02),
  /* spec  */ MkMask(                                          M_MELPS740          , 0x22));
  AddNorm("SLO",
  /* ZA    */ MkMask(                                                       M_6502U, 0x07),
  /* A     */ MkMask(                                                       M_6502U, 0x0f),
  /* ZIX   */ MkMask(                                                       M_6502U, 0x17),
  /* IX    */ MkMask(                                                       M_6502U, 0x1f),
  /* ZIY   */     -1,
  /* IY    */ MkMask(                                                       M_6502U, 0x1b),
  /* @X    */ MkMask(                                                       M_6502U, 0x03),
  /* @Y    */ MkMask(                                                       M_6502U, 0x13),
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
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                                       M_6502U, 0x0b),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("RLA",
  /* ZA    */ MkMask(                                                       M_6502U, 0x27),
  /* A     */ MkMask(                                                       M_6502U, 0x2f),
  /* ZIX   */ MkMask(                                                       M_6502U, 0x37),
  /* IX    */ MkMask(                                                       M_6502U, 0x3f),
  /* ZIY   */     -1,
  /* IY    */ MkMask(                                                       M_6502U, 0x3b),
  /* @X    */ MkMask(                                                       M_6502U, 0x23),
  /* @Y    */ MkMask(                                                       M_6502U, 0x33),
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("SRE",
  /* ZA    */ MkMask(                                                       M_6502U, 0x47),
  /* A     */ MkMask(                                                       M_6502U, 0x4f),
  /* ZIX   */ MkMask(                                                       M_6502U, 0x57),
  /* IX    */ MkMask(                                                       M_6502U, 0x5f),
  /* ZIY   */     -1,
  /* IY    */ MkMask(                                                       M_6502U, 0x5b),
  /* @X    */ MkMask(                                                       M_6502U, 0x43),
  /* @Y    */ MkMask(                                                       M_6502U, 0x53),
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("ASR",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                                       M_6502U, 0x4b),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("RRA",
  /* ZA    */ MkMask(                                                       M_6502U, 0x67),
  /* A     */ MkMask(                                                       M_6502U, 0x6f),
  /* ZIX   */ MkMask(                                                       M_6502U, 0x77),
  /* IX    */ MkMask(                                                       M_6502U, 0x7f),
  /* ZIY   */     -1,
  /* IY    */ MkMask(                                                       M_6502U, 0x7b),
  /* @X    */ MkMask(                                                       M_6502U, 0x63),
  /* @Y    */ MkMask(                                                       M_6502U, 0x73),
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
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                                       M_6502U, 0x6b),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("SAX",
  /* ZA    */ MkMask(                                                       M_6502U, 0x87),
  /* A     */ MkMask(                                                       M_6502U, 0x8f),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */ MkMask(                                                       M_6502U, 0x97),
  /* IY    */     -1,
  /* @X    */ MkMask(                                                       M_6502U, 0x83),
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("ANE",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                                       M_6502U, 0x8b),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("SHA",
  /* ZA    */     -1,
  /* A     */     -1,
  /* ZIX   */     -1,
  /* IX    */ MkMask(                                                       M_6502U, 0x93),
  /* ZIY   */     -1,
  /* IY    */ MkMask(                                                       M_6502U, 0x9f),
  /* @X    */     -1,
  /* @Y    */     -1,
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
  /* IY    */ MkMask(                                                       M_6502U, 0x9b),
  /* @X    */     -1,
  /* @Y    */     -1,
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
  /* IY    */ MkMask(                                                       M_6502U, 0x9c),
  /* @X    */     -1,
  /* @Y    */     -1,
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
  /* IX    */ MkMask(                                                       M_6502U, 0x9e),
  /* ZIY   */     -1,
  /* IY    */     -1,
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("LAX",
  /* ZA    */ MkMask(                                                       M_6502U, 0xa7),
  /* A     */ MkMask(                                                       M_6502U, 0xaf),
  /* ZIX   */     -1,
  /* IX    */     -1,
  /* ZIY   */ MkMask(                                                       M_6502U, 0xb7),
  /* IY    */ MkMask(                                                       M_6502U, 0xbf),
  /* @X    */ MkMask(                                                       M_6502U, 0xa3),
  /* @Y    */ MkMask(                                                       M_6502U, 0xb3),
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
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                                       M_6502U, 0xab),
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
  /* IY    */ MkMask(                                                       M_6502U, 0xbb),
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("DCP",
  /* ZA    */ MkMask(                                                       M_6502U, 0xc7),
  /* A     */ MkMask(                                                       M_6502U, 0xcf),
  /* ZIX   */ MkMask(                                                       M_6502U, 0xd7),
  /* IX    */ MkMask(                                                       M_6502U, 0xdf),
  /* ZIY   */     -1,
  /* IY    */ MkMask(                                                       M_6502U, 0xdb),
  /* @X    */ MkMask(                                                       M_6502U, 0xc3),
  /* @Y    */ MkMask(                                                       M_6502U, 0xd3),
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
  /* @X    */     -1,
  /* @Y    */     -1,
  /* (n16) */     -1,
  /* imm   */ MkMask(                                                       M_6502U, 0xcb),
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);
  AddNorm("ISB",
  /* ZA    */ MkMask(                                                       M_6502U, 0xe7),
  /* A     */ MkMask(                                                       M_6502U, 0xef),
  /* ZIX   */ MkMask(                                                       M_6502U, 0xf7),
  /* IX    */ MkMask(                                                       M_6502U, 0xff),
  /* ZIY   */     -1,
  /* IY    */ MkMask(                                                       M_6502U, 0xfb),
  /* @X    */ MkMask(                                                       M_6502U, 0xe3),
  /* @Y    */ MkMask(                                                       M_6502U, 0xf3),
  /* (n16) */     -1,
  /* imm   */     -1,
  /* ACC   */     -1,
  /* NON   */     -1,
  /* (n8)  */     -1,
  /* spec  */     -1);

  CondOrders = (CondOrder *) malloc(sizeof(CondOrder) * CondOrderCount); InstrZ = 0;
  AddCond("BEQ", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xf0);
  AddCond("BNE", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xd0);
  AddCond("BPL", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x10);
  AddCond("BMI", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x30);
  AddCond("BCC", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x90);
  AddCond("BCS", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0xb0);
  AddCond("BVC", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x50);
  AddCond("BVS", M_6502 | M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740 | M_6502U, 0x70);
  AddCond("BRA",          M_65SC02 | M_65C02 | M_W65C02S | M_MELPS740          , 0x80);
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

  if (!LookupInstTable(InstTable, OpPart))
    WrXError(1200, OpPart);
}

static void InitCode_65(void)
{
  CLI_SEI_Flag = False;
  ADC_SBC_Flag = False;
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
  SegLimits[SegCode] = 0xffff;

  if (MomCPU == CPUM740)
  {
#define ASSUME740Count (sizeof(ASSUME740s) / sizeof(*ASSUME740s))
    static ASSUMERec ASSUME740s[] =
    {
      { "SP", &SpecPage, 0, 0xff, -1 }
    };

    pASSUMERecs = ASSUME740s;
    ASSUMERecCnt = ASSUME740Count;
  }

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
  CPUM740    = AddCPU("MELPS740" , SwitchTo_65);
  CPU6502U   = AddCPU("6502UNDOC", SwitchTo_65);

  AddInitPassProc(InitCode_65);
}
