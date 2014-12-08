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
/* $Id: code65.c,v 1.8 2014/11/16 13:15:07 alfred Exp $                      */
/*****************************************************************************
 * $Log: code65.c,v $
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

#define FixedOrderCount 37
#define NormOrderCount 51
#define CondOrderCount 9


static Boolean CLI_SEI_Flag, ADC_SBC_Flag;

static FixedOrder *FixedOrders;
static NormOrder *NormOrders;
static CondOrder *CondOrders;

static CPUVar CPU6502, CPU65SC02, CPU65C02, CPUM740, CPU6502U;
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
  else if (MomCPU != CPU65C02) WrError(1500);
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
  else if (MomCPU != CPU65C02) WrError(1500);
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
  AddFixed("RTS", 31, 0x60);  AddFixed("RTI", 31, 0x40);
  AddFixed("TAX", 31, 0xaa);  AddFixed("TXA", 31, 0x8a);
  AddFixed("TAY", 31, 0xa8);  AddFixed("TYA", 31, 0x98);
  AddFixed("TXS", 31, 0x9a);  AddFixed("TSX", 31, 0xba);
  AddFixed("DEX", 31, 0xca);  AddFixed("DEY", 31, 0x88);
  AddFixed("INX", 31, 0xe8);  AddFixed("INY", 31, 0xc8);
  AddFixed("PHA", 31, 0x48);  AddFixed("PLA", 31, 0x68);
  AddFixed("PHP", 31, 0x08);  AddFixed("PLP", 31, 0x28);
  AddFixed("PHX",  6, 0xda);  AddFixed("PLX",  6, 0xfa);
  AddFixed("PHY",  6, 0x5a);  AddFixed("PLY",  6, 0x7a);
  AddFixed("BRK", 31, 0x00);  AddFixed("STP",  8, 0x42);
  AddFixed("SLW",  8, 0xc2);  AddFixed("FST",  8, 0xe2);
  AddFixed("WIT",  8, 0xc2);  AddFixed("CLI", 31, 0x58);
  AddFixed("SEI", 31, 0x78);  AddFixed("CLC", 31, 0x18);
  AddFixed("SEC", 31, 0x38);  AddFixed("CLD", 31, 0xd8);
  AddFixed("SED", 31, 0xf8);  AddFixed("CLV", 31, 0xb8);
  AddFixed("CLT",  8, 0x12);  AddFixed("SET",  8, 0x32);
  AddFixed("JAM", 16, 0x02);  AddFixed("CRS", 16, 0x02);
  AddFixed("KIL", 16, 0x02);


  NormOrders = (NormOrder *) malloc(sizeof(NormOrder) * NormOrderCount); InstrZ = 0;
              /* ZA      A       ZIX     IX      ZIY     IY      @X      @Y     (n16)    imm     ACC     NON    (n8)    spec */
  AddNorm("NOP", 0x1004, 0x100c, 0x1014, 0x101c,     -1,     -1,     -1,     -1,     -1, 0x1080,     -1, 0x1fea,     -1,     -1);
  AddNorm("LDA", 0x1fa5, 0x1fad, 0x1fb5, 0x1fbd,     -1, 0x1fb9, 0x1fa1, 0x1fb1,     -1, 0x1fa9,     -1,     -1, 0x06b2,     -1);
  AddNorm("LDX", 0x1fa6, 0x1fae,     -1,     -1, 0x1fb6, 0x1fbe,     -1,     -1,     -1, 0x1fa2,     -1,     -1,     -1,     -1);
  AddNorm("LDY", 0x1fa4, 0x1fac, 0x1fb4, 0x1fbc,     -1,     -1,     -1,     -1,     -1, 0x1fa0,     -1,     -1,     -1,     -1);
  AddNorm("STA", 0x1f85, 0x1f8d, 0x1f95, 0x1f9d,     -1, 0x1f99, 0x1f81, 0x1f91,     -1,     -1,     -1,     -1, 0x0692,     -1);
  AddNorm("STX", 0x1f86, 0x1f8e,     -1,     -1, 0x1f96,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("STY", 0x1f84, 0x1f8c, 0x1f94,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("STZ", 0x0664, 0x069c, 0x0674, 0x069e,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("ADC", 0x1f65, 0x1f6d, 0x1f75, 0x1f7d,     -1, 0x1f79, 0x1f61, 0x1f71,     -1, 0x1f69,     -1,     -1, 0x0672,     -1);
  AddNorm("SBC", 0x1fe5, 0x1fed, 0x1ff5, 0x1ffd,     -1, 0x1ff9, 0x1fe1, 0x1ff1,     -1, 0x1fe9,     -1,     -1, 0x06f2,     -1);
  AddNorm("MUL",     -1,     -1, 0x0862,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("DIV",     -1,     -1, 0x08e2,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("AND", 0x1f25, 0x1f2d, 0x1f35, 0x1f3d,     -1, 0x1f39, 0x1f21, 0x1f31,     -1, 0x1f29,     -1,     -1, 0x0632,     -1);
  AddNorm("ORA", 0x1f05, 0x1f0d, 0x1f15, 0x1f1d,     -1, 0x1f19, 0x1f01, 0x1f11,     -1, 0x1f09,     -1,     -1, 0x0612,     -1);
  AddNorm("EOR", 0x1f45, 0x1f4d, 0x1f55, 0x1f5d,     -1, 0x1f59, 0x1f41, 0x1f51,     -1, 0x1f49,     -1,     -1, 0x0652,     -1);
  AddNorm("COM", 0x0844,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("BIT", 0x1f24, 0x1f2c, 0x0634, 0x063c,     -1,     -1,     -1,     -1,     -1, 0x0689,     -1,     -1,     -1,     -1);
  AddNorm("TST", 0x0864,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("ASL", 0x1f06, 0x1f0e, 0x1f16, 0x1f1e,     -1,     -1,     -1,     -1,     -1,     -1, 0x1f0a, 0x1f0a,     -1,     -1);
  AddNorm("LSR", 0x1f46, 0x1f4e, 0x1f56, 0x1f5e,     -1,     -1,     -1,     -1,     -1,     -1, 0x1f4a, 0x1f4a,     -1,     -1);
  AddNorm("ROL", 0x1f26, 0x1f2e, 0x1f36, 0x1f3e,     -1,     -1,     -1,     -1,     -1,     -1, 0x1f2a, 0x1f2a,     -1,     -1);
  AddNorm("ROR", 0x1f66, 0x1f6e, 0x1f76, 0x1f7e,     -1,     -1,     -1,     -1,     -1,     -1, 0x1f6a, 0x1f6a,     -1,     -1);
  AddNorm("RRF", 0x0882,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("TSB", 0x0604, 0x060c,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("TRB", 0x0614, 0x061c,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("INC", 0x1fe6, 0x1fee, 0x1ff6, 0x1ffe,     -1,     -1,     -1,     -1,     -1,     -1, (Is740) ? 0x0e3a : 0x0e1a, 
                                                                                             (Is740) ? 0x0e3a : 0x0e1a, 
                                                                                                                     -1,     -1);
  AddNorm("DEC", 0x1fc6, 0x1fce, 0x1fd6, 0x1fde,     -1,     -1,     -1,     -1,     -1,     -1, (Is740) ? 0x0e1a : 0x0e3a, 
                                                                                             (Is740) ? 0x0e1a : 0x0e3a, 
                                                                                                                     -1,     -1);
  AddNorm("CMP", 0x1fc5, 0x1fcd, 0x1fd5, 0x1fdd,     -1, 0x1fd9, 0x1fc1, 0x1fd1,     -1, 0x1fc9,     -1,     -1, 0x06d2,     -1);
  AddNorm("CPX", 0x1fe4, 0x1fec,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 0x1fe0,     -1,     -1,     -1,     -1);
  AddNorm("CPY", 0x1fc4, 0x1fcc,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 0x1fc0,     -1,     -1,     -1,     -1);
  AddNorm("JMP",     -1, 0x1f4c,     -1,     -1,     -1,     -1, 0x067c,     -1, 0x1f6c,     -1,     -1,     -1, 0x08b2,     -1);
  AddNorm("JSR",     -1, 0x1f20,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 0x0802, 0x0822);
  AddNorm("SLO", 0x1007, 0x100f, 0x1017, 0x101f,     -1, 0x101b, 0x1003, 0x1013,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("ANC",     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 0x100b,     -1,     -1,     -1,     -1);
  AddNorm("RLA", 0x1027, 0x102f, 0x1037, 0x103f,     -1, 0x103b, 0x1023, 0x1033,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("SRE", 0x1047, 0x104f, 0x1057, 0x105f,     -1, 0x105b, 0x1043, 0x1053,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("ASR",     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 0x104b,     -1,     -1,     -1,     -1);
  AddNorm("RRA", 0x1067, 0x106f, 0x1077, 0x107f,     -1, 0x107b, 0x1063, 0x1073,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("ARR",     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 0x106b,     -1,     -1,     -1,     -1);
  AddNorm("SAX", 0x1087, 0x108f,     -1,     -1, 0x1097,     -1, 0x1083,     -1,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("ANE",     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 0x108b,     -1,     -1,     -1,     -1);
  AddNorm("SHA",     -1,     -1,     -1, 0x1093,     -1, 0x109f,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("SHS",     -1,     -1,     -1,     -1,     -1, 0x109b,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("SHY",     -1,     -1,     -1,     -1,     -1, 0x109c,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("SHX",     -1,     -1,     -1, 0x109e,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("LAX", 0x10a7, 0x10af,     -1,     -1, 0x10b7, 0x10bf, 0x10a3, 0x10b3,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("LXA",     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 0x10ab,     -1,     -1,     -1,     -1);
  AddNorm("LAE",     -1,     -1,     -1,     -1,     -1, 0x10bb,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("DCP", 0x10c7, 0x10cf, 0x10d7, 0x10df,     -1, 0x10db, 0x10c3, 0x10d3,     -1,     -1,     -1,     -1,     -1,     -1);
  AddNorm("SBX",     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1, 0x10cb,     -1,     -1,     -1,     -1);
  AddNorm("ISB", 0x10e7, 0x10ef, 0x10f7, 0x10ff,     -1, 0x10fb, 0x10e3, 0x10f3,     -1,     -1,     -1,     -1,     -1,     -1);

  CondOrders = (CondOrder *) malloc(sizeof(CondOrder) * CondOrderCount); InstrZ = 0;
  AddCond("BEQ", 31, 0xf0);
  AddCond("BNE", 31, 0xd0);
  AddCond("BPL", 31, 0x10);
  AddCond("BMI", 31, 0x30);
  AddCond("BCC", 31, 0x90);
  AddCond("BCS", 31, 0xb0);
  AddCond("BVC", 31, 0x50);
  AddCond("BVS", 31, 0x70);
  AddCond("BRA", 14, 0x80);
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
  CPU6502   = AddCPU("6502"     , SwitchTo_65);
  CPU65SC02 = AddCPU("65SC02"   , SwitchTo_65);
  CPU65C02  = AddCPU("65C02"    , SwitchTo_65);
  CPUM740   = AddCPU("MELPS740" , SwitchTo_65);
  CPU6502U  = AddCPU("6502UNDOC", SwitchTo_65);

  AddInitPassProc(InitCode_65);
}
