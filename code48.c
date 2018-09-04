/* code48.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegeneratormodul MCS-48-Familie                                         */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*            2. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*            5. 1.2000 fixed accessing P1/P2 with lower case in ANL/ORL     */
/*                                                                           */
/*****************************************************************************/
/* $Id: code48.c,v 1.6 2016/01/30 16:46:37 alfred Exp $                      */
/*****************************************************************************
 * $Log: code48.c,v $
 * Revision 1.6  2016/01/30 16:46:37  alfred
 * - add register symbols for MCS-48
 *
 * Revision 1.5  2014/11/06 18:19:35  alfred
 * - rework to current style
 *
 * Revision 1.4  2007/11/24 22:48:04  alfred
 * - some NetBSD changes
 *
 * Revision 1.3  2005/09/08 17:31:03  alfred
 * - add missing include
 *
 * Revision 1.2  2004/05/29 11:33:00  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 *****************************************************************************/

#include "stdinc.h"
#include "bpemu.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "strutil.h"
#include "stringlists.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "intpseudo.h"
#include "codevars.h"
#include "errmsg.h"

#include "code48.h"

typedef struct
{
  Byte Code;
  Byte May2X, MaySiemens;
  Byte UPIFlag;
} CondOrder;

typedef struct
{
  char *Name;
  Byte Code;
  Boolean Is22;
  Boolean IsNUPI;
} SelOrder;

enum
{
  ModImm = 0,
  ModReg = 1,
  ModInd = 2,
  ModAcc = 3,
  ModNone = -1
};

#define ClrCplCnt 4
#define CondOrderCnt 22
#define SelOrderCnt 6

static ShortInt AdrMode;
static Byte AdrVal;
static CPUVar CPU8021, CPU8022, CPU8039, CPU8048, CPU80C39, CPU80C48, CPU8041, CPU8042,
              CPU80C382, CPUMSM80C39, CPUMSM80C48;

static char **ClrCplVals;
static Byte *ClrCplCodes;
static CondOrder *CondOrders;
static SelOrder *SelOrders;

/****************************************************************************/

static Boolean DecodeReg(const char *pAsc, Byte *pErg)
{
  char *s;

  if (FindRegDef(pAsc, &s))
    pAsc = s;

  if ((strlen(pAsc) != 2)
   || (mytoupper(pAsc[0]) != 'R')
   || (!isdigit(pAsc[1])))
    return False;

  *pErg = pAsc[1] - '0';
  return (*pErg <= 7);
}

static void DecodeAdr(const tStrComp *pArg)
{
  Boolean OK;

  AdrMode = ModNone;

  if (*pArg->Str == '\0') return;

  if (!strcasecmp(pArg->Str, "A"))
    AdrMode = ModAcc;

  else if (*pArg->Str == '#')
  {
    AdrVal = EvalStrIntExpressionOffs(pArg, 1, Int8, &OK);
    if (OK)
    {
      AdrMode = ModImm;
      BAsmCode[1] = AdrVal;
    }
  }

  else if (DecodeReg(pArg->Str, &AdrVal))
    AdrMode = ModReg;

  else if ((*pArg->Str == '@')
        && (DecodeReg(pArg->Str + 1, &AdrVal))
        && (AdrVal <= 1))
    AdrMode = ModInd;
}

static void ChkN802X(void)
{
  if (CodeLen == 0) return;
  if (ChkExcludeCPUList(0, CPU8021, CPU8022, CPUNone) < 0)
  {
    CodeLen = 0;
  }
}

static void Chk802X(void)
{
  if (CodeLen == 0) return;
  if (ChkExactCPUList(0, CPU8021, CPU8022, CPUNone) < 0)
  {
    CodeLen = 0;
  }
}

static void ChkCMOS(void)
{
  if (CodeLen == 0) return;
  if (ChkExactCPUList(0, CPU80C39, CPU80C48, CPU80C382, CPUMSM80C39, CPUMSM80C48, CPUNone) < 0)
  {
    CodeLen = 0;
  }
}

static void ChkNUPI(void)
{
  if (CodeLen == 0) return;
  if (ChkExcludeCPUList(0, CPU8041, CPU8042, CPUNone) < 0)
  {
    CodeLen = 0;
  }
}

static void ChkUPI(void)
{
  if (CodeLen == 0) return;
  if (ChkExactCPUList(0, CPU8041, CPU8042, CPUNone) < 0)
  {
    CodeLen = 0;
  }
}

static void ChkNSiemens(void)
{
  if (CodeLen == 0) return;
  if (!ChkExcludeCPU(CPU80C382))
  {
    CodeLen = 0;
  }
}

static void ChkOKI(void)
{
  if (CodeLen == 0) return;
  if (ChkExactCPUList(0, CPUMSM80C39, CPUMSM80C48, CPUNone) < 0)
  {
    CodeLen = 0;
  }
}

static void ChkSiemensOrOKI(void)
{
  if (CodeLen == 0) return;
  if (ChkExactCPUList(0, CPU80C382, CPUMSM80C39, CPUMSM80C48, CPUNone) < 0)
  {
    CodeLen = 0;
  }
}

static void ChkExt(void)
{
  if (CodeLen == 0) return;
  if (ChkExcludeCPUList(0, CPU8039, CPU80C39, CPUNone) < 0)
  {
    CodeLen = 0;
  }
}

static void ChkPx(Byte PortNum)
{
  switch (PortNum)
  {
    case 0:
      Chk802X();
      break;
    case 2:
      ChkNSiemens();
      break;
    default:
      break;
  }
}

static Boolean IsIReg3(const char *pAsc)
{
  Byte RegNum;

  if (*pAsc != '@')
    return False;
  return DecodeReg(pAsc + 1, &RegNum) && (RegNum == 3);
}

/****************************************************************************/

static void DecodeADD_ADDC(Word Code)
{
  if (!ChkArgCnt(2, 2));
  else if (strcasecmp(ArgStr[1].Str, "A")) WrError(ErrNum_InvOpType);
  else
  {
    DecodeAdr(&ArgStr[2]);
    if ((AdrMode == ModNone) || (AdrMode == ModAcc)) WrError(ErrNum_InvAddrMode);
    else
    {
      switch (AdrMode)
      {
        case ModImm:
          CodeLen = 2;
          BAsmCode[0] = Code + 0x03;
          break;
        case ModReg:
          CodeLen = 1;
          BAsmCode[0] = Code + 0x68 + AdrVal;
          break;
        case ModInd:
          CodeLen = 1;
          BAsmCode[0] = Code + 0x60 + AdrVal;
          break;
      }
    }
  }
}

static void DecodeANL_ORL_XRL(Word Code)
{
  if (!ChkArgCnt(2, 2));
  else if (!strcasecmp(ArgStr[1].Str, "A"))
  {
    DecodeAdr(&ArgStr[2]);
    if ((AdrMode == -1) || (AdrMode == ModAcc)) WrError(ErrNum_InvAddrMode);
    else
    {
      switch (AdrMode)
      {
        case ModImm:
          CodeLen = 2;
          BAsmCode[0] = Code + 0x43;
          break;
        case ModReg:
          CodeLen = 1;
          BAsmCode[0] = Code + 0x48 + AdrVal;
          break;
        case ModInd:
          CodeLen = 1;
          BAsmCode[0] = Code + 0x40 + AdrVal;
          break;
      }
    }
  }
  else if ((!strcasecmp(ArgStr[1].Str, "BUS")) || (!strcasecmp(ArgStr[1].Str, "P1")) || (!strcasecmp(ArgStr[1].Str, "P2")))
  {
    if (Code == 0x90) WrError(ErrNum_InvAddrMode);
    else
    {
      DecodeAdr(&ArgStr[2]);
      if (AdrMode != ModImm) WrError(ErrNum_InvAddrMode);
      else
      {
        CodeLen = 2;
        BAsmCode[0] = Code + 0x88;
        if (mytoupper(*ArgStr[1].Str) == 'P')
        {
          Byte PortNum = ArgStr[1].Str[1] - '0';

          BAsmCode[0] += PortNum;
          ChkPx(PortNum);
        }
        if (!strcasecmp(ArgStr[1].Str, "BUS"))
        {
          ChkExt();
          ChkNUPI();
        }
        ChkN802X();
      }
    }
  }
  else
    WrError(ErrNum_InvAddrMode);
}

static void DecodeCALL_JMP(Word Code)
{
  if (!ChkArgCnt(1, 1));
  else if ((EProgCounter() & 0x7fe) == 0x7fe) WrError(ErrNum_NotOnThisAddress);
  else
  {
    Boolean OK;
    Word AdrWord = EvalStrIntExpression(&ArgStr[1], Int16, &OK);

    if (OK)
    {
      if (AdrWord > 0xfff) WrError(ErrNum_OverRange);
      else
      {
        if ((((int)EProgCounter()) & 0x800) != (AdrWord&0x800))
        {
          BAsmCode[0] = 0xe5 + ((AdrWord & 0x800) >> 7);
          CodeLen = 1;
        }
        BAsmCode[CodeLen + 1] = AdrWord & 0xff;
        BAsmCode[CodeLen] = Code + ((AdrWord & 0x700) >> 3);
        CodeLen += 2;
        ChkSpace(SegCode);
      }
    }
  }
}

static void DecodeCLR_CPL(Word Code)
{
  if (!ChkArgCnt(1, 1));
  else
  {
    int z = 0;
    Boolean OK = False;

    NLS_UpString(ArgStr[1].Str);
    do
    {
      if (!strcmp(ClrCplVals[z], ArgStr[1].Str))
      {
        CodeLen = 1;
        BAsmCode[0] = ClrCplCodes[z];
        OK = True;
        if (*ArgStr[1].Str == 'F')
        {
          ChkN802X();
          ChkNSiemens();
        }
      }
      z++;
    }
    while ((z < ClrCplCnt) && (CodeLen != 1));
    if (!OK)
      WrError(ErrNum_InvOpType);
    else
      BAsmCode[0] += Code;
  }
}

static void DecodeAcc(Word Code)
{
  if (!ChkArgCnt(1, 1));
  else if (strcasecmp(ArgStr[1].Str, "A")) WrError(ErrNum_InvAddrMode);
  else
  {
    CodeLen = 1;
    BAsmCode[0] = Code;
  }
}

static void DecodeDEC(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(1, 1));
  else
  {
    DecodeAdr(&ArgStr[1]);
    switch (AdrMode)
    {
      case ModAcc:
        CodeLen = 1;
        BAsmCode[0] = 0x07;
        break;
      case ModReg:
        CodeLen = 1;
        BAsmCode[0] = 0xc8 + AdrVal;
        ChkN802X();
        break;
      case ModInd:
        CodeLen = 1;
        BAsmCode[0] = 0xc0 | AdrVal;
        ChkSiemensOrOKI();
        break;
      default:
        WrError(ErrNum_InvAddrMode);
    }
  }
}

static void DecodeDIS_EN(Word Code)
{
  if (!ChkArgCnt(1, 1));
  else if (ChkExcludeCPU(CPU8021))
  {
    NLS_UpString(ArgStr[1].Str);
    if (!strcmp(ArgStr[1].Str, "I"))
    {
      CodeLen = 1;
      BAsmCode[0] = Code + 0x05;
    }
    else if (!strcmp(ArgStr[1].Str, "TCNTI"))
    {
      CodeLen = 1;
      BAsmCode[0] = Code + 0x25;
    }
    else if ((Memo("EN")) && (!strcmp(ArgStr[1].Str, "DMA")))
    {
      BAsmCode[0] = Code + 0xe5;
      CodeLen = 1;
      ChkUPI();
    }
    else if ((Memo("EN")) && (!strcmp(ArgStr[1].Str, "FLAGS")))
    {
      BAsmCode[0] = Code + 0xf5;
      CodeLen = 1;
      ChkUPI();
    }
    else
      WrError(ErrNum_InvAddrMode);
  }
}

static void DecodeDJNZ(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else
  {
    DecodeAdr(&ArgStr[1]);
    switch (AdrMode)
    {
      case ModReg:
        CodeLen = 1;
        BAsmCode[0] = 0xe8 + AdrVal;
        break;
      case ModInd:
        CodeLen = 1;
        BAsmCode[0] = 0xe0 + AdrVal;
        ChkSiemensOrOKI();
        break;
      default:
        WrError(ErrNum_InvAddrMode);
    }
    if (CodeLen > 0)
    {
      Boolean OK;
      Word AdrWord;

      FirstPassUnknown = False;
      AdrWord = EvalStrIntExpression(&ArgStr[2], Int16, &OK);
      if (OK)
      {
        if (ChkSamePage(EProgCounter() + CodeLen, AdrWord, 8))
          BAsmCode[CodeLen++] = AdrWord & 0xff;
      }
    }
  }
}

static void DecodeENT0(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(1, 1));
  else if (strcasecmp(ArgStr[1].Str, "CLK")) WrError(ErrNum_InvOpType);
  else
  {
    CodeLen = 1;
    BAsmCode[0] = 0x75;
    ChkN802X();
    ChkNUPI();
    ChkNSiemens();
  }
}

static void DecodeINC(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(1, 1));
  else
  {
    DecodeAdr(&ArgStr[1]);
    switch (AdrMode)
    {
      case ModAcc:
        CodeLen = 1;
        BAsmCode[0] = 0x17;
        break;
      case ModReg:
        CodeLen = 1;
        BAsmCode[0] = 0x18 + AdrVal;
        break;
      case ModInd:
        CodeLen = 1;
        BAsmCode[0] = 0x10 + AdrVal;
        break;
      default:
        WrError(ErrNum_InvAddrMode);
    }
  }
}

static void DecodeIN(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else if (strcasecmp(ArgStr[1].Str, "A")) WrError(ErrNum_InvAddrMode);
  else if (!strcasecmp(ArgStr[2].Str, "DBB"))
  {
    CodeLen = 1;
    BAsmCode[0] = 0x22;
    ChkUPI();
  }
  else if ((strlen(ArgStr[2].Str) != 2) || (mytoupper(*ArgStr[2].Str) != 'P')) WrError(ErrNum_InvAddrMode);
  else switch (ArgStr[2].Str[1])
  {
    case '0':
    case '1':
    case '2':
    {
      Byte PortNum = ArgStr[2].Str[1] - '0';

      CodeLen = 1;
      BAsmCode[0] = 0x08 + PortNum;
      ChkPx(PortNum);
      break;
    }
    default:
      WrError(ErrNum_InvAddrMode);
  }
}

static void DecodeINS(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else if (strcasecmp(ArgStr[1].Str, "A")) WrError(ErrNum_InvAddrMode);
  else if (strcasecmp(ArgStr[2].Str, "BUS")) WrError(ErrNum_InvAddrMode);
  else
  {
    CodeLen = 1;
    BAsmCode[0] = 0x08;
    ChkExt();
    ChkNUPI();
  }
}

static void DecodeJMPP(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(1, 1));
  else if (strcasecmp(ArgStr[1].Str, "@A")) WrError(ErrNum_InvAddrMode);
  else
  {
    CodeLen = 1;
    BAsmCode[0] = 0xb3;
  }
}

static void DecodeCond(Word Index)
{
  const CondOrder *pOrder = CondOrders + Index;

  if (!ChkArgCnt(1, 1));
  else
  {
    Boolean OK;
    Word AdrWord;

    FirstPassUnknown = False;
    AdrWord = EvalStrIntExpression(&ArgStr[1], UInt12, &OK);
    if (OK && ChkSamePage(EProgCounter() + 1, AdrWord, 8))
    {
      CodeLen = 2;
      BAsmCode[0] = pOrder->Code;
      BAsmCode[1] = AdrWord & 0xff;
      ChkSpace(SegCode);
      switch (pOrder->May2X)
      {
        case 0:
          ChkN802X();
          break;
        case 1:
          if (!ChkExcludeCPU(CPU8021))
            CodeLen = 0;
          break;
        default:
          break;
      }
      switch (pOrder->UPIFlag)
      {
        case 1:
          ChkUPI();
          break;
        case 2:
          ChkNUPI();
          break;
        default:
          break;
      }
      if (!pOrder->MaySiemens)
        ChkNSiemens();
    }
  }
}

static void DecodeJB(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else
  {
    Boolean OK;
    AdrVal = EvalStrIntExpression(&ArgStr[1], UInt3, &OK);
    if (OK)
    {
      Word AdrWord;

      FirstPassUnknown = False;
      AdrWord = EvalStrIntExpression(&ArgStr[2], UInt12, &OK);
      if (OK && ChkSamePage(EProgCounter() + 1, AdrWord, 8))
      {
        CodeLen = 2;
        BAsmCode[0] = 0x12 + (AdrVal << 5);
        BAsmCode[1] = AdrWord & 0xff;
        ChkN802X();
      }
    }
  }
}

static void DecodeMOV(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else if (!strcasecmp(ArgStr[1].Str, "A"))
  {
    if (!strcasecmp(ArgStr[2].Str, "T"))
    {
      CodeLen = 1;
      BAsmCode[0] = 0x42;
    }
    else if (!strcasecmp(ArgStr[2].Str, "P1"))
    {
      CodeLen = 1;
      BAsmCode[0] = 0x63;
      ChkOKI();
    }
    else if (!strcasecmp(ArgStr[2].Str, "P2"))
    {
      CodeLen = 1;
      BAsmCode[0] = 0x73;
      ChkOKI();
    }
    else if (!strcasecmp(ArgStr[2].Str, "PSW"))
    {
      CodeLen = 1;
      BAsmCode[0] = 0xc7;
      ChkN802X();
      ChkNSiemens();
    }
    else
    {
       DecodeAdr(&ArgStr[2]);
       switch (AdrMode)
       {
         case ModReg:
           CodeLen = 1;
           BAsmCode[0] = 0xf8 + AdrVal;
           break;
         case ModInd:
           CodeLen = 1;
           BAsmCode[0] = 0xf0 + AdrVal;
           break;
         case ModImm:
           CodeLen = 2;
           BAsmCode[0] = 0x23;
           break;
         default:
           WrError(ErrNum_InvAddrMode);
       }
    }
  }
  else if (!strcasecmp(ArgStr[1].Str, "P1"))
  {
    if (IsIReg3(ArgStr[2].Str))
    {
      CodeLen = 1;
      BAsmCode[0] = 0xf3;
      ChkOKI();
    }
    else
      WrError(ErrNum_InvAddrMode);
  }
  else if (!strcasecmp(ArgStr[2].Str, "A"))
  {
    if (!strcasecmp(ArgStr[1].Str, "STS"))
    {
      CodeLen = 1;
      BAsmCode[0] = 0x90;
      ChkUPI();
    }
    else if (!strcasecmp(ArgStr[1].Str, "T"))
    {
      CodeLen = 1;
      BAsmCode[0] = 0x62;
    }
    else if (!strcasecmp(ArgStr[1].Str, "PSW"))
    {
      CodeLen = 1;
      BAsmCode[0] = 0xd7;
      ChkN802X();
      ChkNSiemens();
    }
    else
    {
      DecodeAdr(&ArgStr[1]);
      switch (AdrMode)
      {
        case ModReg:
          CodeLen = 1;
          BAsmCode[0] = 0xa8 + AdrVal;
          break;
        case ModInd:
          CodeLen = 1;
          BAsmCode[0] = 0xa0 + AdrVal;
          break;
        default:
          WrError(ErrNum_InvAddrMode);
      }
    }
  }
  else if (*ArgStr[2].Str == '#')
  {
    Boolean OK;
    Word AdrWord = EvalStrIntExpressionOffs(&ArgStr[2], 1, Int8, &OK);
    if (OK)
    {
      DecodeAdr(&ArgStr[1]);
      switch (AdrMode)
      {
        case ModReg:
          CodeLen = 2;
          BAsmCode[0] = 0xb8 + AdrVal;
          BAsmCode[1] = AdrWord;
          break;
        case ModInd:
          CodeLen = 2;
          BAsmCode[0] = 0xb0 + AdrVal;
          BAsmCode[1] = AdrWord;
          break;
        default:
          WrError(ErrNum_InvAddrMode);
      }
    }
  }
  else
    WrError(ErrNum_InvOpType);
}

static void DecodeANLD_ORLD_MOVD(Word Code)
{
  if (!ChkArgCnt(2, 2));
  else
  {
    const char *pArg1 = ArgStr[1].Str, *pArg2 = ArgStr[2].Str;

    if ((Code == 0x3c) && (!strcasecmp(ArgStr[1].Str, "A"))) /* MOVD */
    {
      pArg1 = ArgStr[2].Str;
      pArg2 = "A";
      Code = 0x0c;
    }
    if (strcasecmp(pArg2, "A")) WrError(ErrNum_InvAddrMode);
    else if ((strlen(pArg1) != 2) || (mytoupper(*pArg1) != 'P')) WrError(ErrNum_InvAddrMode);
    else if ((pArg1[1] < '4') || (pArg1[1] > '7')) WrError(ErrNum_OverRange);
    else
    {
      Byte PortNum = pArg1[1] - '4';

      CodeLen = 1;
      BAsmCode[0] = Code + PortNum;
      ChkN802X();
      if (PortNum == 3)
        ChkNSiemens();
    }
  }
}

static void DecodeMOVP_MOVP3(Word Code)
{
  if (!ChkArgCnt(2, 2));
  else if ((strcasecmp(ArgStr[1].Str, "A")) || (strcasecmp(ArgStr[2].Str, "@A"))) WrError(ErrNum_InvAddrMode);
  else
  {
    CodeLen = 1;
    BAsmCode[0] = Code;
    ChkN802X();
  }
}

static void DecodeMOVP1(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else if (strcasecmp(ArgStr[1].Str, "P")) WrError(ErrNum_InvAddrMode);
  else if (!IsIReg3(ArgStr[2].Str)) WrError(ErrNum_InvAddrMode);
  else
  {
    CodeLen = 1;
    BAsmCode[0] = 0xc3;
    ChkOKI();
  }
}

static void DecodeMOVX(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else
  {
    const tStrComp *pArg1 = &ArgStr[1], *pArg2 = &ArgStr[2];
    Byte Code = 0x80;

    if (!strcasecmp(pArg2->Str, "A"))
    {
      pArg2 = &ArgStr[1];
      pArg1 = &ArgStr[2];
      Code += 0x10;
    }
    if (strcasecmp(pArg1->Str, "A")) WrError(ErrNum_InvAddrMode);
    else
    {
      DecodeAdr(pArg2);
      if (AdrMode != ModInd) WrError(ErrNum_InvAddrMode);
      else
      {
        CodeLen = 1;
        BAsmCode[0] = Code + AdrVal;
        ChkN802X();
        ChkNUPI();
      }
    }
  }
}

static void DecodeNOP(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(0, 0));
  else
  {
    CodeLen = 1;
    BAsmCode[0] = 0x00;
  }
}

static void DecodeOUT(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else if (strcasecmp(ArgStr[1].Str, "DBB")) WrError(ErrNum_InvAddrMode);
  else if (strcasecmp(ArgStr[2].Str, "A")) WrError(ErrNum_InvAddrMode);
  else
  {
    BAsmCode[0] = 0x02;
    CodeLen = 1;
    ChkUPI();
  }
}

static void DecodeOUTL(Word Code)
{
  UNUSED(Code);

  NLS_UpString(ArgStr[1].Str);
  if (!ChkArgCnt(2, 2));
  else
  {
    NLS_UpString(ArgStr[1].Str);
    if (strcasecmp(ArgStr[2].Str, "A")) WrError(ErrNum_InvAddrMode);
    else if (!strcmp(ArgStr[1].Str, "BUS"))
    {
      CodeLen = 1;
      BAsmCode[0] = 0x02;
      ChkN802X();
      ChkExt();
      ChkNUPI();
    }
    else if (!strcmp(ArgStr[1].Str, "P0"))
    {
      CodeLen = 1;
      BAsmCode[0] = 0x90;
      /* Chk802X? */
    }
    else if ((!strcmp(ArgStr[1].Str, "P1")) || (!strcmp(ArgStr[1].Str, "P2")))
    {
      Byte PortNum = ArgStr[1].Str[1] - '0';
      CodeLen = 1;
      BAsmCode[0] = 0x38 + PortNum;
      ChkPx(PortNum);
    }
    else
      WrError(ErrNum_InvAddrMode);
  }
}

static void DecodeRET_RETR(Word Code)
{
  if (!ChkArgCnt(0, 0));
  else
  {
    CodeLen = 1;
    BAsmCode[0] = Code;
    if (Code == 0x93)
      ChkN802X();
  }
}

static void DecodeSEL(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(1, 1));
  else if (ChkExcludeCPU(CPU8021))
  {
    Boolean OK = False;
    int z;

    NLS_UpString(ArgStr[1].Str);
    for (z = 0; z < SelOrderCnt; z++)
    if (!strcmp(ArgStr[1].Str, SelOrders[z].Name))
    {
      CodeLen = 1;
      BAsmCode[0] = SelOrders[z].Code;
      OK = True;
      if ((SelOrders[z].Is22) && !ChkExactCPU(CPU8022))
        CodeLen = 0;
      if (SelOrders[z].IsNUPI)
        ChkNUPI();
    }
    if (!OK)
      WrError(ErrNum_InvAddrMode);
  }
}

static void DecodeSTOP(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(1, 1));
  else if (strcasecmp(ArgStr[1].Str, "TCNT")) WrError(ErrNum_InvAddrMode);
  else
  {
    CodeLen = 1;
    BAsmCode[0] = 0x65;
  }
}

static void DecodeSTRT(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(1, 1));
  else
  {
    NLS_UpString(ArgStr[1].Str);
    if (!strcmp(ArgStr[1].Str, "CNT"))
    {
      CodeLen = 1;
      BAsmCode[0] = 0x45;
    }
    else if (!strcmp(ArgStr[1].Str, "T"))
    {
      CodeLen = 1;
      BAsmCode[0] = 0x55;
    }
    else
      WrError(ErrNum_InvAddrMode);
  }
}

static void DecodeXCH(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else
  {
    const tStrComp *pArg1 = &ArgStr[1], *pArg2 = &ArgStr[2];

    if (!strcasecmp(pArg2->Str, "A"))
    {
      pArg2 = &ArgStr[1];
      pArg1 = &ArgStr[2];
    }
    if (strcasecmp(pArg1->Str, "A")) WrError(ErrNum_InvAddrMode);
    else
    {
      DecodeAdr(pArg2);
      switch (AdrMode)
      {
        case ModReg:
          CodeLen = 1;
          BAsmCode[0] = 0x28 + AdrVal;
          break;
        case ModInd:
          CodeLen = 1;
          BAsmCode[0] = 0x20 + AdrVal;
          break;
        default:
          WrError(ErrNum_InvAddrMode);
      }
    }
  }
}

static void DecodeXCHD(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else
  {
    const tStrComp *pArg1 = &ArgStr[1], *pArg2 = &ArgStr[2];

    if (!strcasecmp(pArg2->Str, "A"))
    {
      pArg2 = &ArgStr[1];
      pArg1 = &ArgStr[2];
    }
    if (strcasecmp(pArg1->Str, "A")) WrError(ErrNum_InvAddrMode);
    else
    {
      DecodeAdr(pArg2);
      if (AdrMode != ModInd) WrError(ErrNum_InvAddrMode);
      else
      {
        CodeLen = 1;
        BAsmCode[0] = 0x30 + AdrVal;
      }
    }
  }
}

static void DecodeRAD(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(0, 0));
  else if (ChkExactCPU(CPU8022))
  {
    CodeLen = 1;
    BAsmCode[0] = 0x80;
  }
}

static void DecodeRETI(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(0, 0));
  else if (ChkExactCPU(CPU8022))
  {
    CodeLen = 1;
    BAsmCode[0] = 0x93;
  }
}

static void DecodeIDL_HALT(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(0, 0))
  {
    CodeLen = 1;
    BAsmCode[0] = (MomCPU == CPU80C382) ? 0xf3 : 0x01;
    ChkCMOS();
  }
}

static void DecodeOKIFixed(Word Code)
{
  if (!ChkArgCnt(0, 0));
  else
  {
    CodeLen = 1;
    BAsmCode[0] = Code;
    ChkOKI();
  }
}

static void DecodeREG(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(1, 1));
  else
    AddRegDef(LabPart.Str, ArgStr[1].Str);
}

/****************************************************************************/

static void AddAcc(char *Name, Byte Code)
{
  AddInstTable(InstTable, Name, Code, DecodeAcc);
}

static void AddCond(char *Name, Byte Code, Byte May2X, Byte MaySiemens, Byte UPIFlag)
{
  if (InstrZ == CondOrderCnt) exit(255);
  CondOrders[InstrZ].Code = Code;
  CondOrders[InstrZ].May2X = May2X;
  CondOrders[InstrZ].MaySiemens = MaySiemens;
  CondOrders[InstrZ].UPIFlag = UPIFlag;
  AddInstTable(InstTable, Name, InstrZ++, DecodeCond);
}

static void AddSel(char *Name, Byte Code, Byte Is22, Byte IsNUPI)
{
  if (InstrZ == SelOrderCnt) exit(255);
  SelOrders[InstrZ].Name = Name;
  SelOrders[InstrZ].Code = Code;
  SelOrders[InstrZ].Is22 = Is22;
  SelOrders[InstrZ++].IsNUPI = IsNUPI;
}

static void InitFields(void)
{
  InstTable = CreateInstTable(203);
  AddInstTable(InstTable, "ADD", 0x00, DecodeADD_ADDC);
  AddInstTable(InstTable, "ADDC", 0x10, DecodeADD_ADDC);
  AddInstTable(InstTable, "ORL", 0x00, DecodeANL_ORL_XRL);
  AddInstTable(InstTable, "ANL", 0x10, DecodeANL_ORL_XRL);
  AddInstTable(InstTable, "XRL", 0x90, DecodeANL_ORL_XRL);
  AddInstTable(InstTable, "CALL", 0x14, DecodeCALL_JMP);
  AddInstTable(InstTable, "JMP", 0x04, DecodeCALL_JMP);
  AddInstTable(InstTable, "CLR", 0x00, DecodeCLR_CPL);
  AddInstTable(InstTable, "CPL", 0x10, DecodeCLR_CPL);
  AddInstTable(InstTable, "DEC", 0, DecodeDEC);
  AddInstTable(InstTable, "DIS", 0x10, DecodeDIS_EN);
  AddInstTable(InstTable, "EN", 0x00, DecodeDIS_EN);
  AddInstTable(InstTable, "DJNZ", 0x00, DecodeDJNZ);
  AddInstTable(InstTable, "ENT0", 0x00, DecodeENT0);
  AddInstTable(InstTable, "INC", 0x00, DecodeINC);
  AddInstTable(InstTable, "IN", 0x00, DecodeIN);
  AddInstTable(InstTable, "INS", 0x00, DecodeINS);
  AddInstTable(InstTable, "JMPP", 0x00, DecodeJMPP);
  AddInstTable(InstTable, "JB", 0x00, DecodeJB);
  AddInstTable(InstTable, "MOV", 0x00, DecodeMOV);
  AddInstTable(InstTable, "ANLD", 0x9c, DecodeANLD_ORLD_MOVD);
  AddInstTable(InstTable, "ORLD", 0x8c, DecodeANLD_ORLD_MOVD);
  AddInstTable(InstTable, "MOVD", 0x3c, DecodeANLD_ORLD_MOVD);
  AddInstTable(InstTable, "MOVP", 0xa3, DecodeMOVP_MOVP3);
  AddInstTable(InstTable, "MOVP3", 0xe3, DecodeMOVP_MOVP3);
  AddInstTable(InstTable, "MOVP1", 0x00, DecodeMOVP1);
  AddInstTable(InstTable, "MOVX", 0x00, DecodeMOVX);
  AddInstTable(InstTable, "NOP", 0x00, DecodeNOP);
  AddInstTable(InstTable, "OUT", 0x00, DecodeOUT);
  AddInstTable(InstTable, "OUTL", 0x00, DecodeOUTL);
  AddInstTable(InstTable, "RET", 0x83, DecodeRET_RETR);
  AddInstTable(InstTable, "RETR", 0x93, DecodeRET_RETR);
  AddInstTable(InstTable, "SEL", 0x00, DecodeSEL);
  AddInstTable(InstTable, "STOP", 0x00, DecodeSTOP);
  AddInstTable(InstTable, "STRT", 0x00, DecodeSTRT);
  AddInstTable(InstTable, "XCH", 0x00, DecodeXCH);
  AddInstTable(InstTable, "XCHD", 0x00, DecodeXCHD);
  AddInstTable(InstTable, "RAD", 0x00, DecodeRAD);
  AddInstTable(InstTable, "RETI", 0x00, DecodeRETI);
  AddInstTable(InstTable, "IDL", 0x00, DecodeIDL_HALT);
  AddInstTable(InstTable, "HALT", 0x00, DecodeIDL_HALT);
  AddInstTable(InstTable, "HLTS", 0x82, DecodeOKIFixed);
  AddInstTable(InstTable, "FLT", 0xa2, DecodeOKIFixed);
  AddInstTable(InstTable, "FLTT", 0xc2, DecodeOKIFixed);
  AddInstTable(InstTable, "FRES", 0xe2, DecodeOKIFixed);

  ClrCplVals = (char **) malloc(sizeof(char *)*ClrCplCnt);
  ClrCplCodes = (Byte *) malloc(sizeof(Byte)*ClrCplCnt);
  ClrCplVals[0] = "A"; ClrCplVals[1] = "C"; ClrCplVals[2] = "F0"; ClrCplVals[3] = "F1";
  ClrCplCodes[0] = 0x27; ClrCplCodes[1] = 0x97; ClrCplCodes[2] = 0x85; ClrCplCodes[3] = 0xa5;

  CondOrders = (CondOrder *) malloc(sizeof(CondOrder) * CondOrderCnt); InstrZ = 0;
  AddCond("JTF"  , 0x16, 2, 1, 3); AddCond("JNI"  , (MomCPU == CPU80C382) ? 0x66 : 0x86, 0, 1, 2);
  AddCond("JC"   , 0xf6, 2, 1, 3); AddCond("JNC"  , 0xe6, 2, 1, 3);
  AddCond("JZ"   , 0xc6, 2, 1, 3); AddCond("JNZ"  , 0x96, 2, 1, 3);
  AddCond("JT0"  , 0x36, 1, 1, 3); AddCond("JNT0" , 0x26, 1, 1, 3);
  AddCond("JT1"  , 0x56, 2, 1, 3); AddCond("JNT1" , 0x46, 2, 1, 3);
  AddCond("JF0"  , 0xb6, 0, 0, 3); AddCond("JF1"  , 0x76, 0, 0, 3);
  AddCond("JNIBF", 0xd6, 2, 1, 1); AddCond("JOBF" , 0x86, 2, 1, 1);
  AddCond("JB0"  , 0x12, 0, 1, 3); AddCond("JB1"  , 0x32, 0, 1, 3);
  AddCond("JB2"  , 0x52, 0, 1, 3); AddCond("JB3"  , 0x72, 0, 1, 3);
  AddCond("JB4"  , 0x92, 0, 1, 3); AddCond("JB5"  , 0xb2, 0, 1, 3);
  AddCond("JB6"  , 0xd2, 0, 1, 3); AddCond("JB7"  , 0xf2, 0, 1, 3);

  AddAcc("DA"  , 0x57);
  AddAcc("RL"  , 0xe7);
  AddAcc("RLC" , 0xf7);
  AddAcc("RR"  , 0x77);
  AddAcc("RRC" , 0x67);
  AddAcc("SWAP", 0x47);

  SelOrders = (SelOrder *) malloc(sizeof(SelOrder) * SelOrderCnt); InstrZ = 0;
  AddSel("MB0" , 0xe5, False, True );
  AddSel("MB1" , 0xf5, False, True );
  AddSel("RB0" , 0xc5, False, False);
  AddSel("RB1" , 0xd5, False, False);
  AddSel("AN0" , 0x95, True , False);
  AddSel("AN1" , 0x85, True , False);

  AddInstTable(InstTable, "REG", 0, DecodeREG);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
  free(ClrCplVals);
  free(ClrCplCodes);
  free(CondOrders);
  free(SelOrders);
}

static void MakeCode_48(void)
{
  CodeLen = 0;
  DontPrint = False;

  /* zu ignorierendes */

  if (Memo(""))
    return;

  /* Pseudoanweisungen */

  if (DecodeIntelPseudo(False))
    return;

  if (!LookupInstTable(InstTable, OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownOpcode, &OpPart);
}

static Boolean IsDef_48(void)
{
  return Memo("REG");
}

static void SwitchFrom_48(void)
{
  DeinitFields();
}

static void SwitchTo_48(void)
{
  TurnWords = False;
  ConstMode = ConstModeIntel;
  SetIsOccupied = False;

  PCSymbol = "$";
  HeaderID = 0x21;
  NOPCode = 0x00;
  DivideChars = ",";
  HasAttrs = False;

  /* limit code segement size only vor variants known to have no
     external program memory */

  ValidSegs = (1 << SegCode) | (1 << SegIData) | (1 << SegXData);
  Grans[SegCode ] = 1; ListGrans[SegCode ] = 1; SegInits[SegCode ] = 0;
  if ((MomCPU == CPU8041) || (MomCPU == CPU8021))
    SegLimits[SegCode] = 0x3ff;
  else if ((MomCPU == CPU8042) || (MomCPU == CPU8022))
    SegLimits[SegCode] = 0x7ff;
  else
    SegLimits[SegCode] = 0xfff;
  Grans[SegIData] = 1; ListGrans[SegIData] = 1; SegInits[SegIData] = 0x20;
  SegLimits[SegIData] = 0xff;
  Grans[SegXData] = 1; ListGrans[SegXData] = 1; SegInits[SegXData] = 0;
  SegLimits[SegXData] = 0xff;

  MakeCode = MakeCode_48;
  IsDef = IsDef_48;
  SwitchFrom = SwitchFrom_48;
  InitFields();
}

void code48_init(void)
{
  CPU8021  = AddCPU("8021" , SwitchTo_48);
  CPU8022  = AddCPU("8022" , SwitchTo_48);
  CPU8039  = AddCPU("8039" , SwitchTo_48);
  CPU8048  = AddCPU("8048" , SwitchTo_48);
  CPU80C39 = AddCPU("80C39", SwitchTo_48);
  CPU80C48 = AddCPU("80C48", SwitchTo_48);
  CPU8041  = AddCPU("8041" , SwitchTo_48);
  CPU8042  = AddCPU("8042" , SwitchTo_48);
  CPU80C382 = AddCPU("80C382" , SwitchTo_48);
  CPUMSM80C39 = AddCPU("MSM80C39" , SwitchTo_48);
  CPUMSM80C48 = AddCPU("MSM80C48" , SwitchTo_48);
}
