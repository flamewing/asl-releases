/* code68.c */ 
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator fuer 68xx Prozessoren                                       */
/*                                                                           */
/* Historie:  13. 8.1996 Grundsteinlegung                                    */
/*             2. 1.1998 ChkPC ersetzt                                       */
/*             9. 3.2000 'ambigious else'-Warnungen beseitigt                */
/*            14. 1.2001 silenced warnings about unused parameters           */
/*            29. 3.2001 added support for K4 banking scheme                 */
/*            25. 5.2001 banking support in address parser, indices to only  */
/*                       unsinged limited                                    */
/*                                                                           */
/*****************************************************************************/
/* $Id: code68.c,v 1.15 2017/06/07 20:41:31 alfred Exp $                      */
/*****************************************************************************
 * $Log: code68.c,v $
 * Revision 1.15  2017/06/07 20:41:31  alfred
 * - add missing ClearONOFF()
 *
 * Revision 1.14  2014/11/13 09:06:49  alfred
 * - adapt to current style
 *
 * Revision 1.13  2014/11/05 15:47:14  alfred
 * - replace InitPass callchain with registry
 *
 * Revision 1.12  2014/07/07 19:27:35  alfred
 * - do not allow JSR with direct mode on 6800
 *
 * Revision 1.11  2014/06/07 17:18:01  alfred
 * - rework to current style
 *
 * Revision 1.10  2014/03/08 21:06:35  alfred
 * - rework ASSUME framework
 *
 * Revision 1.9  2010/04/17 13:14:20  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.8  2009/04/13 07:53:46  alfred
 * - silence Borlanc C++ warning
 *
 * Revision 1.7  2007/11/24 22:48:04  alfred
 * - some NetBSD changes
 *
 * Revision 1.6  2005/11/16 22:03:34  alfred
 * - correct XGDX for 6301
 *
 * Revision 1.5  2005/09/08 16:53:41  alfred
 * - use common PInstTable
 *
 * Revision 1.4  2004/05/29 12:18:05  alfred
 * - relocated DecodeTIPseudo() to separate module
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmpars.h"
#include "asmallg.h"
#include "asmsub.h"
#include "errmsg.h"
#include "codepseudo.h"
#include "motpseudo.h"
#include "asmitree.h"
#include "codevars.h"
#include "nlmessages.h"
#include "as.rsc"

#include "code68.h"

/*---------------------------------------------------------------------------*/

typedef struct
{
  CPUVar MinCPU, MaxCPU;
  Word Code;
} FixedOrder;

typedef struct 
{
  Boolean MayImm;
  CPUVar MinCPU;    /* Shift  andere   ,Y   */
  Byte PageShift;   /* 0 :     nix    Pg 2  */
  Byte Code;        /* 1 :     Pg 3   Pg 4  */
} ALU16Order;       /* 2 :     nix    Pg 4  */
                    /* 3 :     Pg 2   Pg 3  */

enum
{
  ModNone = -1,
  ModAcc  = 0,
  ModDir  = 1,
  ModExt  = 2,
  ModInd  = 3,
  ModImm  = 4,
};

#define MModAcc (1 << ModAcc)
#define MModDir (1 << ModDir)
#define MModExt (1 << ModExt)
#define MModInd (1 << ModInd)
#define MModImm (1 << ModImm)
 
#define Page2Prefix 0x18
#define Page3Prefix 0x1a
#define Page4Prefix 0xcd

#define FixedOrderCnt 45
#define ALU16OrderCnt 13


static ShortInt OpSize;
static Byte PrefCnt;           /* Anzahl Befehlspraefixe */
static ShortInt AdrMode;       /* Ergebnisadressmodus */
static Byte AdrPart;           /* Adressierungsmodusbits im Opcode */
static Byte AdrVals[4];        /* Adressargument */

static FixedOrder *FixedOrders;
static ALU16Order *ALU16Orders;

static LongInt Reg_MMSIZ, Reg_MMWBR, Reg_MM1CR, Reg_MM2CR;
static LongWord Win1VStart, Win1VEnd, Win1PStart, Win1PEnd,
                Win2VStart, Win2VEnd, Win2PStart, Win2PEnd;

static CPUVar CPU6800, CPU6301, CPU6811, CPU68HC11K4;

/*---------------------------------------------------------------------------*/

static void SetK4Ranges(void)
{
  Byte WSize;

  /* window 1 first */

  WSize = Reg_MMSIZ & 0x3;
  if (WSize)
  {
    /* window size */

    Win1VEnd = Win1PEnd = 0x1000 << WSize;

    /* physical start: assume 8K window, systematically clip out bits for
       larger windows */

    Win1PStart = (Reg_MMWBR & 0x0e) << 12;
    if (WSize > 1)
      Win1PStart &= ~0x2000;
    if (WSize > 2)
      Win1PStart = (Win1PStart == 0xc000) ? 0x8000 : Win1PStart;

    /* logical start: mask out lower bits according to window size */

    Win1VStart = ((Reg_MM1CR & 0x7f & (~((1 << WSize) - 1))) << 12) + 0x10000;

    /* set end addresses */

    Win1VEnd += Win1VStart;
    Win1PEnd += Win1PStart;
  }
  else
    Win1VStart = Win1VEnd = Win1PStart = Win1PEnd = 0;

  /* window 2 similarly */

  WSize = Reg_MMSIZ & 0x30;
  if (WSize)
  {
    /* window size */

    WSize = WSize >> 4;
    Win2VEnd = Win2PEnd = 0x1000 << WSize;

    /* physical start: assume 8K window, systematically clip out bits for
       larger windows */

    Win2PStart = (Reg_MMWBR & 0x0e0) << 8;
    if (WSize > 1)
      Win2PStart &= ~0x2000;
    if (WSize > 2)
      Win2PStart = (Win2PStart == 0xc000) ? 0x8000 : Win2PStart;

    /* logical start: mask out lower bits according to window size */

    Win2VStart = ((Reg_MM2CR & 0x7f & (~((1 << WSize) - 1))) << 12) + 0x90000;

    /* set end addresses */

    Win2VEnd += Win2VStart;
    Win2PEnd += Win2PStart;
  }
  else
    Win2VStart = Win2VEnd = Win2PStart = Win2PEnd = 0;
}

static void TranslateAddress(LongWord *Address)
{
  /* do not translate the first 64K */

  if (*Address < 0x10000)
    return;

  /* in first window ? */

  if ((*Address >= Win1VStart) && (*Address < Win1VEnd))
  {
    *Address = Win1PStart + (Win1VStart - *Address);
    return;
  }

  /* in second window ?  After calculation, check against overlap into first
     window. */

  if ((*Address >= Win2VStart) && (*Address < Win2VEnd))
  {
    *Address = Win2PStart + (Win2VStart - *Address);
    if ((*Address >= Win1PStart) && (*Address < Win1PEnd))
      WrError(110);
    return;
  }

  /* print out warning if not mapped */

  *Address &= 0xffff;
  WrError(110);
}

/*---------------------------------------------------------------------------*/

static void DecodeAdr(int StartInd, int StopInd, Byte Erl)
{
  String Asc;
  Boolean OK, ErrOcc;
  LongWord AdrWord;
  Byte Bit8;

  AdrMode = ModNone;
  AdrPart = 0;
  strmaxcpy(Asc, ArgStr[StartInd], 255);
  ErrOcc = False;

  /* eine Komponente ? */

  if (StartInd == StopInd)
  {
    /* Akkumulatoren ? */

    if (!strcasecmp(Asc, "A"))
    {
      if (MModAcc & Erl)
        AdrMode = ModAcc;
    }
    else if (!strcasecmp(Asc, "B"))
    {
      if (MModAcc & Erl)
      {
        AdrMode = ModAcc;
        AdrPart = 1;
      }
    }

    /* immediate ? */

    else if ((strlen(Asc) > 1) && (*Asc == '#'))
    {
      if (MModImm & Erl)
      {
        if (OpSize == 1)
        {
          AdrWord = EvalIntExpression(Asc + 1, Int16, &OK);
          if (OK)
          {
            AdrMode = ModImm;
            AdrVals[AdrCnt++] = Hi(AdrWord);
            AdrVals[AdrCnt++] = Lo(AdrWord);
          }
          else
            ErrOcc = True;
        }
        else
        {
          AdrVals[AdrCnt] = EvalIntExpression(Asc + 1, Int8, &OK);
          if (OK)
          {
            AdrMode = ModImm;
            AdrCnt++;
          }
          else
            ErrOcc = True;
        }
      }
    }

    /* absolut ? */

    else
    {
      char *pAsc = Asc;

      Bit8 = 0;
      if (*pAsc == '<')
      {
        Bit8 = 2;
        pAsc++;
      }
      else if (*pAsc == '>')
      {
        Bit8 = 1;
        pAsc++;
      }
      FirstPassUnknown = False;
      if (MomCPU == CPU68HC11K4)
      {
        AdrWord = EvalIntExpression(pAsc, UInt21, &OK);
        if (OK)
          TranslateAddress(&AdrWord);
      }
      else
        AdrWord = EvalIntExpression(pAsc, UInt16, &OK);
      if (OK)
      {
        if ((MModDir & Erl) && (Bit8 != 1) && ((Bit8 == 2) || (!(MModExt & Erl)) || (Hi(AdrWord) == 0)))
        {
          if ((Hi(AdrWord) != 0) && (!FirstPassUnknown))
          {
            WrError(1340);
            ErrOcc = True;
          }
          else
          {
            AdrMode = ModDir;
            AdrPart = 1;
            AdrVals[AdrCnt++] = Lo(AdrWord);
          }
        }
        else if ((MModExt & Erl)!=0)
        {
          AdrMode = ModExt;
          AdrPart = 3;
          AdrVals[AdrCnt++] = Hi(AdrWord);
          AdrVals[AdrCnt++] = Lo(AdrWord);
        }
      }
      else
        ErrOcc = True;
    }
  }

  /* zwei Komponenten ? */

  else if (StartInd + 1 == StopInd)
  {
    Boolean IsX = !strcasecmp(ArgStr[StopInd], "X"),
            IsY = !strcasecmp(ArgStr[StopInd], "Y");

    /* indiziert ? */

    if (IsX || IsY)
    {
      if (MModInd & Erl)
      {
        AdrWord = EvalIntExpression(Asc, UInt8, &OK);
        if (OK)
        {
          if (IsY && !ChkMinCPUExt(CPU6811, ErrNum_AddrModeNotSupported))
            ErrOcc = True;
          else
          {
            AdrVals[AdrCnt++] = Lo(AdrWord);
            AdrMode = ModInd;
            AdrPart = 2;
            if (IsY)
            {
              BAsmCode[PrefCnt++] = 0x18;
            }
          }
        }
        else
          ErrOcc = True;
      }
    }
    else
    {
      WrXErrorPos(ErrNum_InvReg, ArgStr[StopInd], &ArgStrPos[StopInd]);
      ErrOcc = True;
    }
  }

  else
  {
    char Str[100];

    sprintf(Str, getmessage(Num_ErrMsgAddrArgCnt), 1, 2, StopInd - StartInd + 1);
    WrXError(ErrNum_WrongArgCnt, Str);
    ErrOcc = True;
  }

  if ((!ErrOcc) && (AdrMode == ModNone))
    WrError(1350);
}

static void AddPrefix(Byte Prefix)
{
  BAsmCode[PrefCnt++] = Prefix;
}

static void Try2Split(int Src)
{
  Integer z;
  char *p;

  KillPrefBlanks(ArgStr[Src]);
  KillPostBlanks(ArgStr[Src]);
  p = ArgStr[Src] + strlen(ArgStr[Src]) - 1;
  while ((p > ArgStr[Src]) && (!isspace((unsigned int) *p)))
    p--;
  if (p > ArgStr[Src])
  {
    for (z = ArgCnt; z >= Src; z--)
      strcpy(ArgStr[z + 1], ArgStr[z]);
    ArgCnt++;
    strcpy(ArgStr[Src + 1], p + 1);
    *p = '\0';
    KillPostBlanks(ArgStr[Src]);
    KillPrefBlanks(ArgStr[Src + 1]);
  }
}

/*---------------------------------------------------------------------------*/

static void DecodeFixed(Word Index)
{
  const FixedOrder *forder = FixedOrders + Index;

  if (!ChkArgCnt(0, 0));
  else if (!ChkRangeCPU(forder->MinCPU, forder->MaxCPU));
  else if (Hi(forder->Code) != 0)
  {
    CodeLen = 2;
    BAsmCode[0] = Hi(forder->Code);
    BAsmCode[1] = Lo(forder->Code);
  }
  else
  {
    CodeLen = 1;
    BAsmCode[0] = Lo(forder->Code);
  }
}

static void DecodeRel(Word Code)
{
  Integer AdrInt;
  Boolean OK;

  if (ChkArgCnt(1, 1))
  {
    AdrInt = EvalIntExpression(ArgStr[1], Int16, &OK);
    if (OK)
    {
      AdrInt -= EProgCounter() + 2;
      if (((AdrInt < -128) || (AdrInt > 127)) && (!SymbolQuestionable)) WrError(1370);
      else
      {
        CodeLen = 2;
        BAsmCode[0] = Code;
        BAsmCode[1] = Lo(AdrInt);
      }
    }
  }
}

static void DecodeALU16(Word Index)
{
  const ALU16Order *forder = ALU16Orders + Index;

  OpSize = 1;
  if (ChkArgCnt(1, 2)
   && ChkMinCPU(forder->MinCPU))
  {
    DecodeAdr(1, ArgCnt, (forder->MayImm ? MModImm : 0) | MModInd | MModExt | MModDir);
    if (AdrMode != ModNone)
    {
      switch (forder->PageShift)
      {
        case 1: 
          if (PrefCnt == 1)
            BAsmCode[PrefCnt - 1] = Page4Prefix;
          else
            AddPrefix(Page3Prefix);
          break;
        case 2:
          if (PrefCnt == 1)
            BAsmCode[PrefCnt - 1] = Page4Prefix;
          break;
        case 3:
          if (PrefCnt == 0)
            AddPrefix((AdrMode == ModInd) ? Page3Prefix : Page2Prefix);
          break;
      }
      BAsmCode[PrefCnt] = forder->Code + (AdrPart << 4);
      CodeLen = PrefCnt + 1 + AdrCnt;
      memcpy(BAsmCode + 1 + PrefCnt, AdrVals, AdrCnt);
    }
  }
}

static void DecodeBit63(Word Code)
{
  if (ChkArgCnt(2, 3)
   && ChkExactCPU(CPU6301))
  {
    DecodeAdr(1, 1, MModImm);
    if (AdrMode != ModNone)
    {
      DecodeAdr(2, ArgCnt, MModDir | MModInd);
      if (AdrMode != ModNone)
      {
        BAsmCode[PrefCnt] = Code;
        if (AdrMode == ModDir)
          BAsmCode[PrefCnt] |= 0x10;
        CodeLen = PrefCnt + 1 + AdrCnt;
        memcpy(BAsmCode + 1 + PrefCnt, AdrVals, AdrCnt);
      }
    }
  }
}

static void DecodeJMP(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(1, 2))
  {
    DecodeAdr(1, ArgCnt, MModExt | MModInd);
    if (AdrMode != ModImm)
    {
      CodeLen = PrefCnt + 1 + AdrCnt;
      BAsmCode[PrefCnt] = 0x4e + (AdrPart << 4);
      memcpy(BAsmCode + 1 + PrefCnt, AdrVals, AdrCnt);
    }
  }
}

static void DecodeJSR(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(1, 2))
  {
    DecodeAdr(1, ArgCnt, MModExt | MModInd | ((MomCPU >= CPU6301) ? MModDir : 0));
    if (AdrMode != ModImm)
    {
      CodeLen=PrefCnt + 1 + AdrCnt;
      BAsmCode[PrefCnt] = 0x8d + (AdrPart << 4);
      memcpy(BAsmCode + 1 + PrefCnt, AdrVals, AdrCnt);
    }
  }
}

static void DecodeBRxx(Word Index)
{
  Boolean OK;
  Byte Mask;
  Integer AdrInt;

  if (ArgCnt == 1)
  {
    Try2Split(1);
    Try2Split(1);
  }
  else if (ArgCnt == 2)
  {
    Try2Split(ArgCnt);
    Try2Split(2);
  }
  if (ChkArgCnt(3, 4)
   && ChkMinCPU(CPU6811))
  {
    char *pArg1 = ArgStr[ArgCnt - 1];

    if (*pArg1 == '#')
      pArg1++;
    Mask = EvalIntExpression(pArg1, Int8,& OK);
    if (OK)
    {
      DecodeAdr(1, ArgCnt-2, MModDir | MModInd);
      if (AdrMode != ModNone)
      {
        AdrInt = EvalIntExpression(ArgStr[ArgCnt], Int16, &OK);
        if (OK)
        {
          AdrInt -= EProgCounter() + 3 + PrefCnt + AdrCnt;
          if ((AdrInt < -128) || (AdrInt > 127)) WrError(1370);
          else
          {
            CodeLen = PrefCnt + 3 + AdrCnt;
            BAsmCode[PrefCnt] = 0x12 + Index;
            if (AdrMode == ModInd)
              BAsmCode[PrefCnt] += 12;
            memcpy(BAsmCode + PrefCnt + 1, AdrVals, AdrCnt);
            BAsmCode[PrefCnt + 1 + AdrCnt] = Mask;
            BAsmCode[PrefCnt + 2 + AdrCnt] = Lo(AdrInt);
          }
        }
      }
    }
  }
}

static void DecodeBxx(Word Index)
{
  Byte Mask;
  Boolean OK;
  int z;

  if (MomCPU == CPU6301)
  {
    strcpy(ArgStr[ArgCnt + 1], ArgStr[1]);
    for (z = 1; z <= ArgCnt - 1; z++)
      strcpy(ArgStr[z], ArgStr[z + 1]);
    strcpy(ArgStr[ArgCnt], ArgStr[ArgCnt + 1]);
  }
  if ((ArgCnt >= 1) && (ArgCnt <= 2)) Try2Split(ArgCnt);
  if (ChkArgCnt(2, 3)
   && ChkMinCPU(CPU6301))
  {
    char *pArgN = ArgStr[ArgCnt];

    if (*pArgN == '#') pArgN++;
    Mask = EvalIntExpression(pArgN, Int8, &OK);
    if ((OK) && (MomCPU == CPU6301))
    {
      if (Mask > 7)
      {
        WrError(1320);
        OK = False;
      }
      else
      {
        Mask = 1 << Mask;
        if (Index == 1) Mask = 0xff - Mask;
      }
    }
    if (OK)
    {
      DecodeAdr(1, ArgCnt - 1, MModDir | MModInd);
      if (AdrMode != ModNone)
      {
        CodeLen = PrefCnt + 2 + AdrCnt;
        if (MomCPU == CPU6301)
        {
          BAsmCode[PrefCnt] = 0x62 - Index;
          if (AdrMode == ModDir)
            BAsmCode[PrefCnt] += 0x10;
          BAsmCode[1 + PrefCnt] = Mask;
          memcpy(BAsmCode + 2 + PrefCnt, AdrVals, AdrCnt);
        }
        else
        {
          BAsmCode[PrefCnt] = 0x14 + Index;
          if (AdrMode == ModInd)
            BAsmCode[PrefCnt] += 8;
          memcpy(BAsmCode + 1 + PrefCnt, AdrVals, AdrCnt);
          BAsmCode[1 + PrefCnt + AdrCnt] = Mask;
        }
      }
    }
  }
}

static void DecodeBTxx(Word Index)
{
  Boolean OK;
  Byte AdrByte;

  if (ChkArgCnt(2, 3)
   && ChkMinCPU(CPU6301))
  {
    AdrByte = EvalIntExpression(ArgStr[1], Int8, &OK);
    if (OK)
    {
      if (AdrByte > 7) WrError(1320);
      else
      {
        DecodeAdr(2, ArgCnt, MModDir | MModInd);
        if (AdrMode != ModNone)
        {
          CodeLen = PrefCnt + 2 + AdrCnt;
          BAsmCode[1 + PrefCnt] = 1 << AdrByte;
          memcpy(BAsmCode + 2 + PrefCnt, AdrVals, AdrCnt);
          BAsmCode[PrefCnt] = 0x65 + Index;
          if (AdrMode == ModDir)
            BAsmCode[PrefCnt] += 0x10;
        }
      }
    }
  }
}

static void DecodeALU8(Word Code)
{
  int MinArgCnt = (Code >> 8) & 3;

  if (ChkArgCnt(MinArgCnt, MinArgCnt + 1))
  {
    DecodeAdr(MinArgCnt , ArgCnt, ((Code & 0x8000) ? MModImm : 0) | MModInd | MModExt | MModDir);
    if (AdrMode != ModNone)
    {
      BAsmCode[PrefCnt] = Lo(Code) | (AdrPart << 4);
      if (MinArgCnt == 1)
      {
        AdrMode = ModAcc;
        AdrPart = (Code & 0x4000) >> 14;
      }
      else
        DecodeAdr(1, 1, MModAcc);
      if (AdrMode != ModNone)
      {
        BAsmCode[PrefCnt] |= AdrPart << 6;
        CodeLen = PrefCnt + 1 + AdrCnt;
        memcpy(BAsmCode + 1 + PrefCnt, AdrVals, AdrCnt);
      }
    }
  }
}

static void DecodeSing8(Word Code)
{
  if (ChkArgCnt(1, 2))
  {
    DecodeAdr(1, ArgCnt, MModAcc | MModExt | MModInd);
    if (AdrMode!=ModNone)
    {
      CodeLen = PrefCnt + 1 + AdrCnt;
      BAsmCode[PrefCnt] = Code | (AdrPart << 4);
      memcpy(BAsmCode + 1 + PrefCnt, AdrVals, AdrCnt);
    }
  }
}

static void DecodeSing8_Acc(Word Code) 
{
  if (ChkArgCnt(0, 0))
  {
    BAsmCode[PrefCnt] = Code;
    CodeLen = PrefCnt + 1;
  }
}

static void DecodePSH_PUL(Word Code)
{
  if (ChkArgCnt(1, 1))
  {
    DecodeAdr(1, 1, MModAcc);
    if (AdrMode != ModNone)
    {
      CodeLen = 1;
      BAsmCode[0]=Code | AdrPart;
    }
  }
}

static void DecodePRWINS(Word Code)
{
  UNUSED(Code);

  if (ChkExactCPU(CPU68HC11K4))
  {
    printf("\nMMSIZ %02x MMWBR %02x MM1CR %02x MM2CR %02x",
           (unsigned)Reg_MMSIZ, (unsigned)Reg_MMWBR, (unsigned)Reg_MM1CR, (unsigned)Reg_MM2CR);
    printf("\nWindow 1: %lx...%lx --> %lx...%lx",
           (long)Win1VStart, (long)Win1VEnd, (long)Win1PStart, (long)Win1PEnd);
    printf("\nWindow 2: %lx...%lx --> %lx...%lx\n",
           (long)Win2VStart, (long)Win2VEnd, (long)Win2PStart, (long)Win2PEnd);
  }
}

/*---------------------------------------------------------------------------*/

static void AddFixed(char *NName, CPUVar NMin, CPUVar NMax, Word NCode)
{
  if (InstrZ >= FixedOrderCnt) exit(255);

  FixedOrders[InstrZ].MinCPU = NMin;
  FixedOrders[InstrZ].MaxCPU = NMax;
  FixedOrders[InstrZ].Code = NCode;
  AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
}

static void AddALU8(char *NamePlain, char *NameA, char *NameB, Boolean MayImm, Byte NCode)
{
  Word BaseCode = NCode | (MayImm ? 0x8000 : 0);

  AddInstTable(InstTable, NamePlain, BaseCode | (2 << 8), DecodeALU8);
  AddInstTable(InstTable, NameA, BaseCode | (1 << 8), DecodeALU8);
  AddInstTable(InstTable, NameB, BaseCode | (1 << 8) | 0x4000, DecodeALU8);
}

static void AddALU16(char *NName, Boolean NMay, CPUVar NMin, Byte NShift, Byte NCode)
{
  if (InstrZ >= ALU16OrderCnt) exit(255);

  ALU16Orders[InstrZ].MayImm = NMay;
  ALU16Orders[InstrZ].MinCPU = NMin;
  ALU16Orders[InstrZ].PageShift = NShift;
  ALU16Orders[InstrZ].Code = NCode;
  AddInstTable(InstTable, NName, InstrZ++, DecodeALU16);
}

static void AddSing8(char *NamePlain, char *NameA, char *NameB, Byte NCode)
{
  AddInstTable(InstTable, NamePlain, NCode, DecodeSing8);
  AddInstTable(InstTable, NameA, NCode | 0, DecodeSing8_Acc);
  AddInstTable(InstTable, NameB, NCode | 0x10, DecodeSing8_Acc);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(302);
  AddInstTable(InstTable, "JMP"  , 0, DecodeJMP);
  AddInstTable(InstTable, "JSR"  , 0, DecodeJSR);
  AddInstTable(InstTable, "BRCLR", 1, DecodeBRxx);
  AddInstTable(InstTable, "BRSET", 0, DecodeBRxx);
  AddInstTable(InstTable, "BCLR" , 1, DecodeBxx);
  AddInstTable(InstTable, "BSET" , 0, DecodeBxx);
  AddInstTable(InstTable, "BTST" , 6, DecodeBTxx);
  AddInstTable(InstTable, "BTGL" , 0, DecodeBTxx);   

  FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCnt); InstrZ = 0;
  AddFixed("ABA"  ,CPU6800, CPU68HC11K4, 0x001b); AddFixed("ABX"  ,CPU6301, CPU68HC11K4, 0x003a);
  AddFixed("ABY"  ,CPU6811, CPU68HC11K4, 0x183a); AddFixed("ASLD" ,CPU6301, CPU68HC11K4, 0x0005);
  AddFixed("CBA"  ,CPU6800, CPU68HC11K4, 0x0011); AddFixed("CLC"  ,CPU6800, CPU68HC11K4, 0x000c);
  AddFixed("CLI"  ,CPU6800, CPU68HC11K4, 0x000e); AddFixed("CLV"  ,CPU6800, CPU68HC11K4, 0x000a);
  AddFixed("DAA"  ,CPU6800, CPU68HC11K4, 0x0019); AddFixed("DES"  ,CPU6800, CPU68HC11K4, 0x0034);
  AddFixed("DEX"  ,CPU6800, CPU68HC11K4, 0x0009); AddFixed("DEY"  ,CPU6811, CPU68HC11K4, 0x1809);
  AddFixed("FDIV" ,CPU6811, CPU68HC11K4, 0x0003); AddFixed("IDIV" ,CPU6811, CPU68HC11K4, 0x0002);
  AddFixed("INS"  ,CPU6800, CPU68HC11K4, 0x0031); AddFixed("INX"  ,CPU6800, CPU68HC11K4, 0x0008);
  AddFixed("INY"  ,CPU6811, CPU68HC11K4, 0x1808); AddFixed("LSLD" ,CPU6301, CPU68HC11K4, 0x0005);
  AddFixed("LSRD" ,CPU6301, CPU68HC11K4, 0x0004); AddFixed("MUL"  ,CPU6301, CPU68HC11K4, 0x003d);
  AddFixed("NOP"  ,CPU6800, CPU68HC11K4, 0x0001); AddFixed("PSHX" ,CPU6301, CPU68HC11K4, 0x003c);
  AddFixed("PSHY" ,CPU6811, CPU68HC11K4, 0x183c); AddFixed("PULX" ,CPU6301, CPU68HC11K4, 0x0038);
  AddFixed("PULY" ,CPU6811, CPU68HC11K4, 0x1838); AddFixed("RTI"  ,CPU6800, CPU68HC11K4, 0x003b);
  AddFixed("RTS"  ,CPU6800, CPU68HC11K4, 0x0039); AddFixed("SBA"  ,CPU6800, CPU68HC11K4, 0x0010);
  AddFixed("SEC"  ,CPU6800, CPU68HC11K4, 0x000d); AddFixed("SEI"  ,CPU6800, CPU68HC11K4, 0x000f);
  AddFixed("SEV"  ,CPU6800, CPU68HC11K4, 0x000b); AddFixed("SLP"  ,CPU6301, CPU6301    , 0x001a);
  AddFixed("STOP" ,CPU6811, CPU68HC11K4, 0x00cf); AddFixed("SWI"  ,CPU6800, CPU68HC11K4, 0x003f);
  AddFixed("TAB"  ,CPU6800, CPU68HC11K4, 0x0016); AddFixed("TAP"  ,CPU6800, CPU68HC11K4, 0x0006);
  AddFixed("TBA"  ,CPU6800, CPU68HC11K4, 0x0017); AddFixed("TPA"  ,CPU6800, CPU68HC11K4, 0x0007);
  AddFixed("TSX"  ,CPU6800, CPU68HC11K4, 0x0030); AddFixed("TSY"  ,CPU6811, CPU68HC11K4, 0x1830);
  AddFixed("TXS"  ,CPU6800, CPU68HC11K4, 0x0035); AddFixed("TYS"  ,CPU6811, CPU68HC11K4, 0x1835);
  AddFixed("WAI"  ,CPU6800, CPU68HC11K4, 0x003e);
  AddFixed("XGDX" ,CPU6301, CPU68HC11K4, (MomCPU == CPU6301) ? 0x0018 : 0x008f);
  AddFixed("XGDY" ,CPU6811, CPU68HC11K4, 0x188f);

  AddInstTable(InstTable, "BCC", 0x24, DecodeRel);
  AddInstTable(InstTable, "BCS", 0x25, DecodeRel);
  AddInstTable(InstTable, "BEQ", 0x27, DecodeRel);
  AddInstTable(InstTable, "BGE", 0x2c, DecodeRel);
  AddInstTable(InstTable, "BGT", 0x2e, DecodeRel);
  AddInstTable(InstTable, "BHI", 0x22, DecodeRel);
  AddInstTable(InstTable, "BHS", 0x24, DecodeRel);
  AddInstTable(InstTable, "BLE", 0x2f, DecodeRel);
  AddInstTable(InstTable, "BLO", 0x25, DecodeRel);
  AddInstTable(InstTable, "BLS", 0x23, DecodeRel);
  AddInstTable(InstTable, "BLT", 0x2d, DecodeRel);
  AddInstTable(InstTable, "BMI", 0x2b, DecodeRel);
  AddInstTable(InstTable, "BNE", 0x26, DecodeRel);
  AddInstTable(InstTable, "BPL", 0x2a, DecodeRel);
  AddInstTable(InstTable, "BRA", 0x20, DecodeRel);
  AddInstTable(InstTable, "BRN", 0x21, DecodeRel);
  AddInstTable(InstTable, "BSR", 0x8d, DecodeRel);
  AddInstTable(InstTable, "BVC", 0x28, DecodeRel);
  AddInstTable(InstTable, "BVS", 0x29, DecodeRel);

  AddALU8("ADC", "ADCA", "ADCB", True , 0x89);
  AddALU8("ADD", "ADDA", "ADDB", True , 0x8b);
  AddALU8("AND", "ANDA", "ANDB", True , 0x84);
  AddALU8("BIT", "BITA", "BITB", True , 0x85);
  AddALU8("CMP", "CMPA", "CMPB", True , 0x81);
  AddALU8("EOR", "EORA", "EORB", True , 0x88);
  AddALU8("LDA", "LDAA", "LDAB", True , 0x86);
  AddALU8("ORA", "ORAA", "ORAB", True , 0x8a);
  AddALU8("SBC", "SBCA", "SBCB", True , 0x82);
  AddALU8("STA", "STAA", "STAB", False, 0x87);
  AddALU8("SUB", "SUBA", "SUBB", True , 0x80);
                         
  ALU16Orders = (ALU16Order *) malloc(sizeof(ALU16Order) * ALU16OrderCnt); InstrZ = 0;
  AddALU16("ADDD", True , CPU6301, 0, 0xc3);
  AddALU16("CPD" , True , CPU6811, 1, 0x83);
  AddALU16("CPX" , True , CPU6800, 2, 0x8c);
  AddALU16("CPY" , True , CPU6811, 3, 0x8c);
  AddALU16("LDD" , True , CPU6301, 0, 0xcc);
  AddALU16("LDS" , True , CPU6800, 0, 0x8e);
  AddALU16("LDX" , True , CPU6800, 2, 0xce);
  AddALU16("LDY" , True , CPU6811, 3, 0xce);
  AddALU16("STD" , False, CPU6301, 0, 0xcd);
  AddALU16("STS" , False, CPU6800, 0, 0x8f);
  AddALU16("STX" , False, CPU6800, 2, 0xcf);
  AddALU16("STY" , False, CPU6811, 3, 0xcf);
  AddALU16("SUBD", True , CPU6301, 0, 0x83);

  AddSing8("ASL", "ASLA", "ASLB", 0x48);
  AddSing8("ASR", "ASRA", "ASRB", 0x47);
  AddSing8("CLR", "CLRA", "CLRB", 0x4f);
  AddSing8("COM", "COMA", "COMB", 0x43);
  AddSing8("DEC", "DECA", "DECB", 0x4a);
  AddSing8("INC", "INCA", "INCB", 0x4c);
  AddSing8("LSL", "LSLA", "LSLB", 0x48);
  AddSing8("LSR", "LSRA", "LSRB", 0x44);
  AddSing8("NEG", "NEGA", "NEGB", 0x40);
  AddSing8("ROL", "ROLA", "ROLB", 0x49);
  AddSing8("ROR", "RORA", "RORB", 0x46);
  AddSing8("TST", "TSTA", "TSTB", 0x4d);

  AddInstTable(InstTable, "PSH" , 0x36, DecodePSH_PUL);
  AddInstTable(InstTable, "PSHA", 0x36, DecodeSing8_Acc);
  AddInstTable(InstTable, "PSHB", 0x37, DecodeSing8_Acc);
  AddInstTable(InstTable, "PUL" , 0x32, DecodePSH_PUL);
  AddInstTable(InstTable, "PULA", 0x32, DecodeSing8_Acc);
  AddInstTable(InstTable, "PULB", 0x33, DecodeSing8_Acc);

  
  AddInstTable(InstTable, "AIM", 0x61, DecodeBit63);
  AddInstTable(InstTable, "EIM", 0x65, DecodeBit63); 
  AddInstTable(InstTable, "OIM", 0x62, DecodeBit63);
  AddInstTable(InstTable, "TIM", 0x6b, DecodeBit63);

  AddInstTable(InstTable, "PRWINS", 0, DecodePRWINS);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
  free(FixedOrders);
  free(ALU16Orders);
}

static void MakeCode_68(void)
{
  CodeLen = 0;
  DontPrint = False;
  PrefCnt = 0;
  AdrCnt = 0;
  OpSize = 0;

  /* Operandengroesse festlegen */

  if (*AttrPart!='\0')
    switch (mytoupper(*AttrPart))
    {
     case 'B':
       OpSize = 0; break;
     case 'W':
       OpSize = 1; break;
     case 'L':
       OpSize = 2; break;
     case 'Q':
       OpSize = 3; break;
     case 'S':
       OpSize = 4; break;
     case 'D':
       OpSize = 5; break;
     case 'X':
       OpSize = 6; break;
     case 'P':
       OpSize = 7; break;
     default:
       WrError(1107); return;
    }

  /* zu ignorierendes */

  if (*OpPart.Str == '\0')
    return;

  /* Pseudoanweisungen */

  if (DecodeMotoPseudo(True))
    return;
  if (DecodeMoto16Pseudo(OpSize, True))
    return;

  /* gehashtes */

  if (!LookupInstTable(InstTable, OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownOpcode, &OpPart);
}

static void InitCode_68(void)
{
  Reg_MMSIZ = Reg_MMWBR = Reg_MM1CR = Reg_MM2CR = 0;
  SetK4Ranges();
}

static Boolean IsDef_68(void)
{
  return False;
}

static void SwitchFrom_68()
{
  DeinitFields();
  ClearONOFF();
}


static void SwitchTo_68(void)
{
#define ASSUMEHC11Count (sizeof(ASSUMEHC11s) / sizeof(*ASSUMEHC11s))
  static const ASSUMERec ASSUMEHC11s[] = 
  {
    {"MMSIZ", &Reg_MMSIZ, 0, 0xff, 0, SetK4Ranges},
    {"MMWBR", &Reg_MMWBR, 0, 0xff, 0, SetK4Ranges},
    {"MM1CR", &Reg_MM1CR, 0, 0xff, 0, SetK4Ranges},
    {"MM2CR", &Reg_MM2CR, 0, 0xff, 0, SetK4Ranges}
  };
  TurnWords = False;
  ConstMode = ConstModeMoto;
  SetIsOccupied = False;

  PCSymbol = "*";
  HeaderID = 0x61;
  NOPCode = 0x01;
  DivideChars = ",";
  HasAttrs = True;
  AttrChars = ".";

  ValidSegs = 1 << SegCode;
  Grans[SegCode] = 1; ListGrans[SegCode] = 1; SegInits[SegCode] = 0;
  SegLimits[SegCode] = (MomCPU == CPU68HC11K4) ? 0x10ffffl : 0xffff;

  MakeCode = MakeCode_68;
  IsDef = IsDef_68;
  SwitchFrom = SwitchFrom_68;
  InitFields();
  AddMoto16PseudoONOFF();

  if (MomCPU == CPU68HC11K4)
  {
    pASSUMERecs = ASSUMEHC11s;
    ASSUMERecCnt = ASSUMEHC11Count;
  }

  SetFlag(&DoPadding, DoPaddingName, False);
}

void code68_init(void)
{
  CPU6800 = AddCPU("6800", SwitchTo_68);
  CPU6301 = AddCPU("6301", SwitchTo_68);
  CPU6811 = AddCPU("6811", SwitchTo_68);
  CPU68HC11K4 = AddCPU("68HC11K4", SwitchTo_68);

  AddInitPassProc(InitCode_68);
}
