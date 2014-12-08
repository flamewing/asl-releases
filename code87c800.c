/* code87c800.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator TLCS-870                                                    */
/*                                                                           */
/* Historie: 29.12.1996 Grundsteinlegung                                     */
/*            2. 1.1999 ChkPC umgebaut                                       */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: code87c800.c,v 1.10 2014/12/07 19:14:00 alfred Exp $                  */
/*****************************************************************************
 * $Log: code87c800.c,v $
 * Revision 1.10  2014/12/07 19:14:00  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.9  2014/11/22 11:45:58  alfred
 * - correct some formatting errors
 * - correct handling of JR
 *
 * Revision 1.8  2014/11/17 23:51:32  alfred
 * - begun with TLCS-870/C
 *
 * Revision 1.7  2014/11/17 21:25:22  alfred
 * - some remaining cleanups
 *
 * Revision 1.6  2014/07/13 22:10:58  alfred
 * - reworked to current style
 *
 * Revision 1.5  2010/04/17 13:14:22  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.4  2007/11/24 22:48:05  alfred
 * - some NetBSD changes
 *
 * Revision 1.3  2005/09/08 17:31:04  alfred
 * - add missing include
 *
 * Revision 1.2  2004/05/29 11:33:02  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 *****************************************************************************/

#include "stdinc.h" 

#include <ctype.h>
#include <string.h>

#include "nls.h"
#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "intpseudo.h"
#include "codevars.h"


typedef struct
{
  char *Name;
  Byte Code;
} CondRec;

#define ConditionCnt 12

enum
{
  ModNone = -1,
  ModReg8 = 0,
  ModReg16 = 1,
  ModImm = 2,
  ModAbs = 3,
  ModMem = 4,
};

#define MModReg8 (1 << ModReg8)
#define MModReg16 (1 << ModReg16)
#define MModImm (1 << ModImm)
#define MModAbs (1 << ModAbs)
#define MModMem (1 << ModMem)

#define AccReg 0
#define WAReg 0

#define Reg8Cnt 8
static char *Reg8Names = "AWCBEDLH";

static CPUVar CPU87C00, CPU87C20, CPU87C40, CPU87C70;
static ShortInt OpSize;
static Byte AdrVals[4];
static ShortInt AdrType;
static Byte AdrMode;

static CondRec *Conditions;

/*--------------------------------------------------------------------------*/

static void DecodeAdr(char *Asc, Byte Erl)
{
  static char *Reg16Names[] = 
  {
    "WA", "BC", "DE", "HL"
  };
  static const int Reg16Cnt = sizeof(Reg16Names) / sizeof(*Reg16Names);
  static char *AdrRegs[] = 
  {
    "HL", "DE", "C", "PC", "A"
  };
  static const int AdrRegCnt = sizeof(AdrRegs) / sizeof(*AdrRegs);

  int z;
  Byte RegFlag;
  LongInt DispAcc, DispPart;
  char *AdrPart;
  Boolean OK, NegFlag, NNegFlag, FirstFlag;
  char *PPos, *NPos, *EPos;

  AdrType = ModNone;
  AdrCnt = 0;

  if (strlen(Asc) == 1)
  {
    for (z = 0; z < Reg8Cnt; z++)
      if (mytoupper(*Asc) == Reg8Names[z])
      {
        AdrType = ModReg8;
        OpSize = 0;
        AdrMode = z;
        goto chk;
      }
  }

  for (z = 0; z < Reg16Cnt; z++)
    if (!strcasecmp(Asc, Reg16Names[z]))
    {
      AdrType = ModReg16;
      OpSize = 1;
      AdrMode = z;
      goto chk;
    }

  if (IsIndirect(Asc))
  {
    Asc++;
    Asc[strlen(Asc) - 1] = '\0';

    if (!strcasecmp(Asc, "-HL"))
    {
      AdrType = ModMem;
      AdrMode = 7;
      goto chk;
    }
    if (!strcasecmp(Asc, "HL+"))
    {
      AdrType = ModMem;
      AdrMode = 6;
      goto chk;
    }

    RegFlag = 0;
    DispAcc = 0;
    NegFlag = False;
    OK = True;
    FirstFlag = False;
    AdrPart = Asc;
    while ((OK) && (AdrPart))
    {
      PPos = QuotPos(AdrPart, '+');
      NPos = QuotPos(AdrPart, '-');
      if (!PPos)
        EPos = NPos;
      else if (!NPos)
        EPos = PPos;
      else
        EPos = min(NPos, PPos);
      NNegFlag = ((EPos) && (*EPos == '-'));
      if (EPos)
        *EPos = '\0';

      for (z = 0; z < AdrRegCnt; z++)
        if (!strcasecmp(AdrPart, AdrRegs[z]))
          break;
      if (z >= AdrRegCnt)
      {
        FirstPassUnknown = False;
        DispPart = EvalIntExpression(AdrPart, Int32, &OK);
        FirstFlag |= FirstPassUnknown;
        DispAcc = NegFlag ? DispAcc - DispPart :  DispAcc + DispPart;
      }
      else if ((NegFlag) || (RegFlag & (1 << z)))
      {
        WrError(1350);
        OK = False;
      }
      else
        RegFlag |= 1 << z;

      NegFlag = NNegFlag;
      AdrPart = EPos ? EPos + 1 : NULL;
    }
    if (DispAcc != 0)
      RegFlag |= 1 << AdrRegCnt;
    if (OK)
     switch (RegFlag)
     {
       case 0x20:
         if (FirstFlag)
           DispAcc &= 0xff;
         if (DispAcc > 0xff) WrError(1320);
         else
         {
           AdrType = ModAbs;
           AdrMode = 0;
           AdrCnt = 1;
           AdrVals[0] = DispAcc & 0xff;
         }
         break;
       case 0x02:
         AdrType = ModMem;
         AdrMode = 2;
         break;
       case 0x01:
         AdrType = ModMem;
         AdrMode = 3;
         break;
       case 0x21:
         if (FirstFlag)
           DispAcc &= 0x7f;
         if (ChkRange(DispAcc, -128, 127))
         {
           AdrType = ModMem;
           AdrMode = 4;
           AdrCnt = 1;
           AdrVals[0] = DispAcc & 0xff;
         }
         break;
       case 0x05:
         AdrType = ModMem;
         AdrMode = 5;
         break;
       case 0x18:
         AdrType = ModMem;
         AdrMode = 1;
         break;
       default:
         WrError(1350);
     }
    goto chk;
  }
  else
   switch (OpSize)
   {
     case -1:
       WrError(1132);
       break;
     case 0:
       AdrVals[0] = EvalIntExpression(Asc, Int8, &OK);
       if (OK)
       {
         AdrType = ModImm;
         AdrCnt = 1;
       }
       break;
     case 1:
       DispAcc = EvalIntExpression(Asc, Int16, &OK);
       if (OK)
       {
         AdrType = ModImm;
         AdrCnt = 2;
         AdrVals[0] = DispAcc & 0xff;
         AdrVals[1] = (DispAcc >> 8) & 0xff;
       }
       break;
   }

chk:
  if ((AdrType != ModNone) && (!((1<<AdrType) & Erl)))
  {
    AdrType = ModNone;
    AdrCnt = 0;
    WrError(1350);
  }
}

static Boolean SplitBit(char *Asc, Byte *Erg)
{
  char *p;

  p = RQuotPos(Asc, '.');
  if (!p)
    return False;
  *p++ = '\0';

  if (strlen(p) != 1) return False;
  else
   if ((*p >= '0') && (*p <= '7'))
   {
     *Erg = *p - '0';
     return True;
   }
   else
   {
     for (*Erg = 0; *Erg < Reg8Cnt; (*Erg)++)
       if (mytoupper(*p) == Reg8Names[*Erg])
         break;
     if (*Erg < Reg8Cnt)
     {
       *Erg += 8;
       return True;
     }
     else
       return False;
   }
}

static void CodeMem(Byte Entry, Byte Opcode)
{
  BAsmCode[0] = Entry + AdrMode;
  memcpy(BAsmCode + 1, AdrVals, AdrCnt);
  BAsmCode[1 + AdrCnt] = Opcode;
}

static int DecodeCondition(char *pCondStr, int Start)
{
  int Condition;

  NLS_UpString(pCondStr);
  for (Condition = Start; Condition < ConditionCnt; Condition++)
    if (!strcmp(ArgStr[1], Conditions[Condition].Name))
      break;

  return Condition;
}

/*--------------------------------------------------------------------------*/

static void DecodeFixed(Word Code)
{
  if (ArgCnt != 0) WrError(1110);
  else
  {
    CodeLen = 0;
    if (Hi(Code) != 0)
      BAsmCode[CodeLen++] = Hi(Code);
    BAsmCode[CodeLen++] = Lo(Code);
  }
}

static void DecodeLD(Word Code)
{
  Boolean OK;
  Byte HReg, HCnt, HMode, HVal;

  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else if (!strcasecmp(ArgStr[1], "SP"))
  {
    OpSize=1;
    DecodeAdr(ArgStr[2], MModImm+MModReg16);
    switch (AdrType)
    {
      case ModReg16:
        CodeLen = 2; BAsmCode[0] = 0xe8 | AdrMode;
        BAsmCode[1] = 0xfa;
        break;
      case ModImm:
        CodeLen = 3;
        BAsmCode[0] = 0xfa;
        memcpy(BAsmCode + 1, AdrVals, AdrCnt);
        break;
    }
  }
  else if (!strcasecmp(ArgStr[2], "SP"))
  {
    DecodeAdr(ArgStr[1], MModReg16);
    switch (AdrType)
    {
      case ModReg16:
        CodeLen = 2;
        BAsmCode[0] = 0xe8 + AdrMode;
        BAsmCode[1] = 0xfb;
        break;
    }
  }
  else if (!strcasecmp(ArgStr[1], "RBS"))
  {
    BAsmCode[1] = EvalIntExpression(ArgStr[2], Int4, &OK);
    if (OK)
    {
      CodeLen = 2;
      BAsmCode[0] = 0x0f;
    }
  }
  else if (!strcasecmp(ArgStr[1], "CF"))
  {
    if (!SplitBit(ArgStr[2], &HReg)) WrError(1510);
    else
    {
      DecodeAdr(ArgStr[2], MModReg8 | MModAbs | MModMem);
      switch (AdrType)
      {
        case ModReg8:
          if (HReg >= 8) WrError(1350);
          else
          {
            CodeLen = 2;
            BAsmCode[0] = 0xe8 | AdrMode;
            BAsmCode[1] = 0xd8 | HReg;
          }
          break;
        case ModAbs:
          if (HReg >= 8) WrError(1350);
          else
          {
            CodeLen = 2;
            BAsmCode[0] = 0xd8 | HReg;
            BAsmCode[1] = AdrVals[0];
          }
          break;
        case ModMem:
          if (HReg < 8)
          {
            CodeLen = 2 + AdrCnt;
            CodeMem(0xe0, 0xd8 | HReg);
          }
          else if ((AdrMode != 2) && (AdrMode != 3)) WrError(1350);
          else
          {
            CodeLen = 2;
            BAsmCode[0] = 0xe0 | HReg;
            BAsmCode[1] = 0x9c | AdrMode;
          }
          break;
      }
    }
  }
  else if (!strcasecmp(ArgStr[2], "CF"))
  {
    if (!SplitBit(ArgStr[1], &HReg)) WrError(1510);
    else
    {
      DecodeAdr(ArgStr[1], MModReg8 | MModAbs | MModMem);
      switch (AdrType)
      {
        case ModReg8:
          if (HReg >= 8) WrError(1350);
          else
          {
            CodeLen = 2;
            BAsmCode[0] = 0xe8 | AdrMode;
            BAsmCode[1] = 0xc8 | HReg;
          }
          break;
        case ModAbs:
        case ModMem:
          if (HReg < 8)
          {
            CodeLen = 2 + AdrCnt;
            CodeMem(0xe0, 0xc8 | HReg);
          }
          else if ((AdrMode != 2) && (AdrMode != 3)) WrError(1350);
          else
          {
            CodeLen = 2;
            BAsmCode[0] = 0xe0 | HReg;
            BAsmCode[1] = 0x98 | AdrMode;
          }
          break;
      }
    }
  }
  else
  {
    DecodeAdr(ArgStr[1], MModReg8 | MModReg16 | MModAbs | MModMem);
    switch (AdrType)
    {
      case ModReg8:
        HReg = AdrMode;
        DecodeAdr(ArgStr[2], MModReg8 | MModAbs | MModMem | MModImm);
        switch (AdrType)
        {
          case ModReg8:
            if (HReg == AccReg)
            {
              CodeLen = 1;
              BAsmCode[0] = 0x50 | AdrMode;
            }
            else if (AdrMode == AccReg)
            {
              CodeLen = 1;
              BAsmCode[0] = 0x58 | HReg;
            }
            else
            {
              CodeLen = 2;
              BAsmCode[0] = 0xe8 | AdrMode;
              BAsmCode[1] = 0x58 | HReg;
            }
            break;
          case ModAbs:
            if (HReg == AccReg)
            {
              CodeLen = 2;
              BAsmCode[0] = 0x22;
              BAsmCode[1] = AdrVals[0];
            }
            else
            {
              CodeLen = 3;
              BAsmCode[0] = 0xe0;
              BAsmCode[1] = AdrVals[0];
              BAsmCode[2] = 0x58 | HReg;
            }
            break;
          case ModMem:
            if ((HReg == AccReg) && (AdrMode == 3))   /* A,(HL) */
            {
              CodeLen = 1;
              BAsmCode[0] = 0x23;
            }
            else
            {
              CodeLen = 2 + AdrCnt;
              CodeMem(0xe0, 0x58 | HReg);
              if ((HReg >= 6) && (AdrMode == 6)) WrError(140);
            }
            break;
          case ModImm:
            CodeLen = 2;
            BAsmCode[0] = 0x30 | HReg;
            BAsmCode[1] = AdrVals[0];
            break;
        }
        break;
      case ModReg16:
        HReg = AdrMode;
        DecodeAdr(ArgStr[2], MModReg16 | MModAbs | MModMem | MModImm);
        switch (AdrType)
        {
          case ModReg16:
            CodeLen = 2;
            BAsmCode[0] = 0xe8 | AdrMode;
            BAsmCode[1] = 0x14 | HReg;
            break;
          case ModAbs:
            CodeLen = 3;
            BAsmCode[0] = 0xe0;
            BAsmCode[1] = AdrVals[0];
            BAsmCode[2] = 0x14 | HReg;
            break;
          case ModMem:
            if (AdrMode > 5) WrError(1350);   /* (-HL), (HL+) */
            else
            {
              CodeLen = 2 + AdrCnt;
              BAsmCode[0] = 0xe0 | AdrMode;
              memcpy(BAsmCode + 1, AdrVals, AdrCnt);
              BAsmCode[1 + AdrCnt] = 0x14 + HReg;
            }
            break;
          case ModImm:
            CodeLen = 3;
            BAsmCode[0] = 0x14 | HReg;
            memcpy(BAsmCode + 1, AdrVals, 2);
            break;
        }
        break;
      case ModAbs:
        HReg = AdrVals[0];
        OpSize = 0;
        DecodeAdr(ArgStr[2], MModReg8 | MModReg16 | MModAbs | MModMem | MModImm);
        switch (AdrType)
        {
          case ModReg8:
            if (AdrMode == AccReg)
            {
              CodeLen = 2;
              BAsmCode[0] = 0x2a;
              BAsmCode[1] = HReg;
            }
            else
            {
              CodeLen = 3;
              BAsmCode[0] = 0xf0;
              BAsmCode[1] = HReg;
              BAsmCode[2] = 0x50 | AdrMode;
            }
            break;
          case ModReg16:
            CodeLen = 3;
            BAsmCode[0] = 0xf0;
            BAsmCode[1] = HReg;
            BAsmCode[2] = 0x10 | AdrMode;
            break;
          case ModAbs:
            CodeLen = 3;
            BAsmCode[0] = 0x26;
            BAsmCode[1] = AdrVals[0];
            BAsmCode[2] = HReg;
            break;
          case ModMem:
            if (AdrMode > 5) WrError(1350);      /* (-HL),(HL+) */
            else
            {
              CodeLen = 3 + AdrCnt;
              BAsmCode[0] = 0xe0 | AdrMode;
              memcpy(BAsmCode + 1, AdrVals, AdrCnt);
              BAsmCode[1 + AdrCnt] = 0x26;
              BAsmCode[2 + AdrCnt] = HReg;
            }
            break;
          case ModImm:
            CodeLen = 3;
            BAsmCode[0] = 0x2c;
            BAsmCode[1] = HReg;
            BAsmCode[2] = AdrVals[0];
            break;
        }
        break;
      case ModMem:
        HVal = AdrVals[0];
        HCnt = AdrCnt;
        HMode = AdrMode;
        OpSize = 0;
        DecodeAdr(ArgStr[2], MModReg8 | MModReg16 | MModAbs | MModMem | MModImm);
        switch (AdrType)
        {
          case ModReg8:
            if ((HMode == 3) && (AdrMode == AccReg))   /* (HL),A */
            {
              CodeLen = 1;
              BAsmCode[0] = 0x2b;
            }
            else if ((HMode == 1) || (HMode == 5)) WrError(1350);
            else
            {
              CodeLen = 2 + HCnt;
              BAsmCode[0] = 0xf0 | HMode;
              memcpy(BAsmCode + 1, &HVal, HCnt);
              BAsmCode[1 + HCnt] = 0x50 | AdrMode;
              if ((HMode == 6) && (AdrMode >= 6)) WrError(140);
            }
            break;
          case ModReg16:
            if ((HMode < 2) || (HMode > 4)) WrError(1350);  /* (HL),(DE),(HL+d) */
            else
            {
              CodeLen = 2 + HCnt;
              BAsmCode[0] = 0xf0 | HMode;
              memcpy(BAsmCode + 1, &HVal, HCnt);
              BAsmCode[1 + HCnt] = 0x10 | AdrMode;
            }
            break;
          case ModAbs:
            if (HMode != 3) WrError(1350);  /* (HL) */
            else
            {
              CodeLen = 3;
              BAsmCode[0] = 0xe0;
              BAsmCode[1] = AdrVals[0];
              BAsmCode[2] = 0x27;
            }
            break;
          case ModMem:
            if (HMode != 3) WrError(1350);         /* (HL) */
            else if (AdrMode > 5) WrError(1350);   /* (-HL),(HL+) */
            else
            {
              CodeLen = 2 + AdrCnt;
              BAsmCode[0] = 0xe0 | AdrMode;
              memcpy(BAsmCode + 1, AdrVals, AdrCnt);
              BAsmCode[1 + AdrCnt] = 0x27;
            }
            break;
          case ModImm:
            if ((HMode == 1) || (HMode == 5)) WrError(1350);  /* (HL+C),(PC+A) */
            else if (HMode == 3)               /* (HL) */
            {
              CodeLen = 2;
              BAsmCode[0] = 0x2d;
              BAsmCode[1] = AdrVals[0];
            }
            else
            {
              CodeLen = 3 + HCnt;
              BAsmCode[0] = 0xf0 + HMode;
              memcpy(BAsmCode + 1, &HVal, HCnt);
              BAsmCode[1 + HCnt] = 0x2c;
              BAsmCode[2 + HCnt] = AdrVals[0];
            }
            break;
        }
        break;
    }
  }
}

static void DecodeXCH(Word Code)
{
  Byte HReg;

  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg8 | MModReg16 | MModAbs | MModMem);
    switch (AdrType)
    {
      case ModReg8:
        HReg = AdrMode;
        DecodeAdr(ArgStr[2], MModReg8 | MModAbs | MModMem);
        switch (AdrType)
        {
          case ModReg8:
            CodeLen = 2;
            BAsmCode[0] = 0xe8 | AdrMode;
            BAsmCode[1] = 0xa8 | HReg;
            break;
          case ModAbs:
          case ModMem:
            CodeLen = 2 + AdrCnt;
            CodeMem(0xe0, 0xa8 | HReg);
            if ((HReg >= 6) && (AdrMode == 6)) WrError(140);
            break;
        }
        break;
      case ModReg16:
        HReg = AdrMode;
        DecodeAdr(ArgStr[2], MModReg16);
        if (AdrType != ModNone)
        {
          CodeLen = 2;
          BAsmCode[0] = 0xe8 | AdrMode;
          BAsmCode[1] = 0x10 | HReg;
        }
        break;
      case ModAbs:
        BAsmCode[1] = AdrVals[0];
        DecodeAdr(ArgStr[2], MModReg8);
        if (AdrType != ModNone)
        {
          CodeLen = 3;
          BAsmCode[0] = 0xe0;
          BAsmCode[2] = 0xa8 | AdrMode;
        }
        break;
      case ModMem:
        BAsmCode[0] = 0xe0 | AdrMode;
        memcpy(BAsmCode + 1, AdrVals, AdrCnt);
        HReg = AdrCnt;
        DecodeAdr(ArgStr[2], MModReg8);
        if (AdrType != ModNone)
        {
          CodeLen = 2 + HReg;
          BAsmCode[1 + HReg] = 0xa8 | AdrMode;
          if ((AdrMode >= 6) && ((BAsmCode[0] & 0x0f) == 6)) WrError(140);
        }
        break;
    }
  }
}

static void DecodeCLR(Word Code)
{
  Byte HReg;;

  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else if (!strcasecmp(ArgStr[1], "CF"))
  {
    CodeLen = 1;
    BAsmCode[0] = 0x0c;
  }
  else if (SplitBit(ArgStr[1], &HReg))
  {
    DecodeAdr(ArgStr[1], MModReg8 | MModAbs | MModMem);
    switch (AdrType)
    {
      case ModReg8:
        if (HReg >= 8) WrError(1350);
        else
        {
          CodeLen = 2;
          BAsmCode[0] = 0xe8 | AdrMode;
          BAsmCode[1] = 0x48 | HReg;
        }
        break;
      case ModAbs:
        if (HReg >= 8) WrError(1350);
        else
        {
          CodeLen = 2;
          BAsmCode[0] = 0x48 | HReg;
          BAsmCode[1] = AdrVals[0];
        }
        break;
      case ModMem:
        if (HReg <= 8)
        {
          CodeLen = 2 + AdrCnt;
          CodeMem(0xe0, 0x48 | HReg);
        }
        else if ((AdrMode != 2) && (AdrMode != 3)) WrError(1350);
        else
        {
          CodeLen = 2;
          BAsmCode[0] = 0xe0 | HReg;
          BAsmCode[1] = 0x88 | AdrMode;
        }
        break;
    }
  }
  else
  {
    DecodeAdr(ArgStr[1], MModReg8 | MModReg16 | MModAbs | MModMem);
    switch (AdrType)
    {
      case ModReg8:
        CodeLen = 2;
        BAsmCode[0] = 0x30 | AdrMode;
        BAsmCode[1] = 0;
        break;
      case ModReg16:
        CodeLen = 3;
        BAsmCode[0] = 0x14 | AdrMode;
        BAsmCode[1] = 0;
        BAsmCode[2] = 0;
        break;
      case ModAbs:
        CodeLen = 2;
        BAsmCode[0] = 0x2e;
        BAsmCode[1] = AdrVals[0];
        break;
      case ModMem:
        if ((AdrMode == 5) || (AdrMode == 1)) WrError(1350);  /* (PC+A, HL+C) */
        else if (AdrMode == 3)     /* (HL) */
        {
          CodeLen = 1;
          BAsmCode[0] = 0x2f;
        }
        else
        {
          CodeLen = 3 + AdrCnt;
          BAsmCode[0] = 0xf0 | AdrMode;
          memcpy(BAsmCode + 1, AdrVals, AdrCnt);
          BAsmCode[1 + AdrCnt] = 0x2c;
          BAsmCode[2 + AdrCnt] = 0;
        }
        break;
    }
  }
}

static void DecodeLDW(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    Boolean OK;
    Integer AdrInt = EvalIntExpression(ArgStr[2], Int16, &OK);
    if (OK)
    {
      DecodeAdr(ArgStr[1], MModReg16 | MModAbs | MModMem);
      switch (AdrType)
      {
        case ModReg16:
          CodeLen = 3;
          BAsmCode[0] = 0x14 | AdrMode;
          BAsmCode[1] = AdrInt & 0xff;
          BAsmCode[2] = AdrInt >> 8;
          break;
        case ModAbs:
          CodeLen = 4;
          BAsmCode[0] = 0x24;
          BAsmCode[1] = AdrVals[0];
          BAsmCode[2] = AdrInt & 0xff;
          BAsmCode[3] = AdrInt >> 8;
          break;
        case ModMem:
          if (AdrMode != 3) WrError(1350);  /* (HL) */
          else
          {
            CodeLen = 3;
            BAsmCode[0] = 0x25;
            BAsmCode[1] = AdrInt & 0xff;
            BAsmCode[2] = AdrInt >> 8;
          }
          break;
      }
    }
  }
}

static void DecodePUSH_POP(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else if (!strcasecmp(ArgStr[1], "PSW"))
  {
    CodeLen = 1;
    BAsmCode[0] = Code;
  }
  else
  {
    DecodeAdr(ArgStr[1], MModReg16);
    if (AdrType != ModNone)
    {
      CodeLen = 2;
     BAsmCode[0] = 0xe8 | AdrMode;
     BAsmCode[1] = Code;
    }
  }
}

static void DecodeTEST_CPL_SET(Word Code)
{
  Byte HReg;

  if (ArgCnt != 1) WrError(1110);
  else if (!strcasecmp(ArgStr[1], "CF"))
  {
    if (Code == 0xd8) WrError(1350);
    else
    {
      CodeLen = 1;
      BAsmCode[0] = 0x0d + Ord(Code == 0xc0);
    }
  }
  else if (!SplitBit(ArgStr[1], &HReg)) WrError(1510);
  else
  {
    DecodeAdr(ArgStr[1], MModReg8 | MModAbs | MModMem);
    switch (AdrType)
    {
      case ModReg8:
        if (HReg >= 8) WrError(1350);
        else
        {
          CodeLen = 2;
          BAsmCode[0] = 0xe8 | AdrMode;
          BAsmCode[1] = Code | HReg;
        }
        break;
      case ModAbs:
        if (HReg >= 8) WrError(1350);
        else if (Code == 0xc0)
        {
          CodeLen = 3;
          CodeMem(0xe0, Code | HReg);
        }
        else
        {
          CodeLen = 2;
          BAsmCode[0] = Code | HReg;
          BAsmCode[1] = AdrVals[0];
        }
        break;
      case ModMem:
        if (HReg < 8)
        {
          CodeLen = 2 + AdrCnt;
          CodeMem(0xe0, Code | HReg);
        }
        else if ((AdrMode != 2) && (AdrMode != 3)) WrError(1350);
        else
        {
          CodeLen = 2;
          BAsmCode[0] = 0xe0 | HReg;
          BAsmCode[1] = ((Code & 0x18) >> 1) | ((Code & 0x80) >> 3) | 0x80 | AdrMode;
        }
        break;
    }
  }
}

static void DecodeReg(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg8);
    if (AdrType != ModNone)
    {
      if (AdrMode == AccReg)
      {
        CodeLen = 1;
        BAsmCode[0] = Code;
      }
      else
      {
        CodeLen = 2;
        BAsmCode[0] = 0xe8 | AdrMode;
        BAsmCode[1] = Code;
      }
    }
  }
}

static void DecodeALU(Word Code)
{
  Byte HReg;
  Boolean OK;

  if (ArgCnt != 2) WrError(1110);
  else if (!strcasecmp(ArgStr[1], "CF"))
  {
    if (Code != 5) WrError(1350); /* XOR */
    else if (!SplitBit(ArgStr[2], &HReg)) WrError(1510);
    else if (HReg >= 8) WrError(1350);
    else
    {
      DecodeAdr(ArgStr[2], MModReg8 | MModAbs | MModMem);
      switch (AdrType)
      {
        case ModReg8:
          CodeLen = 2;
          BAsmCode[0] = 0xe8 | AdrMode;
          BAsmCode[1] = 0xd0 | HReg;
          break;
        case ModAbs:
        case ModMem:
          CodeLen = 2 + AdrCnt;
          CodeMem(0xe0, 0xd0 | HReg);
          break;
      }
    }
  }
  else
  {
    DecodeAdr(ArgStr[1], MModReg8 | MModReg16 | MModMem | MModAbs);
    switch (AdrType)
    {
      case ModReg8:
        HReg = AdrMode;
        DecodeAdr(ArgStr[2], MModReg8 | MModMem | MModAbs | MModImm);
        switch (AdrType)
        {
          case ModReg8:
            if (HReg == AccReg)
            {
              CodeLen = 2;
              BAsmCode[0] = 0xe8 | AdrMode;
              BAsmCode[1] = 0x60 | Code;
            }
            else if (AdrMode == AccReg)
            {
              CodeLen = 2;
              BAsmCode[0] = 0xe8 | HReg;
              BAsmCode[1] = 0x68 | Code;
            }
            else WrError(1350);
            break;
          case ModMem:
            if (HReg != AccReg) WrError(1350);
            else
            {
              CodeLen = 2 + AdrCnt;
              BAsmCode[0] = 0xe0 | AdrMode;
              memcpy(BAsmCode + 1, AdrVals, AdrCnt);
              BAsmCode[1 + AdrCnt] = 0x78 | Code;
            }
            break;
          case ModAbs:
            if (HReg != AccReg) WrError(1350);
            else
            {
              CodeLen = 2;
              BAsmCode[0] = 0x78 | Code;
              BAsmCode[1] = AdrVals[0];
            }
            break;
          case ModImm:
            if (HReg == AccReg)
            {
              CodeLen = 2;
              BAsmCode[0] = 0x70 | Code;
              BAsmCode[1] = AdrVals[0];
            }
            else
            {
              CodeLen = 3;
              BAsmCode[0] = 0xe8 | HReg;
              BAsmCode[1] = 0x70 | Code;
              BAsmCode[2] = AdrVals[0];
            }
            break;
        }
        break;
      case ModReg16:
        HReg = AdrMode;
        DecodeAdr(ArgStr[2], MModImm | MModReg16);
        switch (AdrType)
        {
          case ModImm:
            CodeLen = 4;
            BAsmCode[0] = 0xe8 | HReg;
            BAsmCode[1] = 0x38 | Code;
            memcpy(BAsmCode + 2, AdrVals, AdrCnt);
            break;
          case ModReg16:
            if (HReg != WAReg) WrError(1350);
            else
            {
              CodeLen = 2;
              BAsmCode[0] = 0xe8 | AdrMode;
              BAsmCode[1] = 0x30 | Code;
            }
            break;
        }
        break;
      case ModAbs:
        if (!strcasecmp(ArgStr[2], "(HL)"))
        {
          CodeLen = 3;
          BAsmCode[0] = 0xe0;
          BAsmCode[1] = AdrVals[0];
          BAsmCode[2] = 0x60 | Code;
        }
        else
        {
          BAsmCode[3] = EvalIntExpression(ArgStr[2], Int8, &OK);
          if (OK)
          {
            CodeLen = 4;
            BAsmCode[0] = 0xe0;
            BAsmCode[1] = AdrVals[0];
            BAsmCode[2] = 0x70 | Code;
          }
        }
        break;
      case ModMem:
        if (!strcasecmp(ArgStr[2], "(HL)"))
        {
          CodeLen = 2 + AdrCnt;
          BAsmCode[0] = 0xe0 | AdrMode;
          memcpy(BAsmCode + 1, AdrVals, AdrCnt);
          BAsmCode[1 + AdrCnt] = 0x60 | Code;
        }
        else
        {
          BAsmCode[2 + AdrCnt] = EvalIntExpression(ArgStr[2], Int8, &OK);
          if (OK)
          {
            CodeLen = 3 + AdrCnt;
            BAsmCode[0] = 0xe0 | AdrMode;
            memcpy(BAsmCode + 1, AdrVals, AdrCnt);
            BAsmCode[1 + AdrCnt] = 0x70 | Code;
          }
        }
        break;
    }
  }
}

static void DecodeMCMP(Word Code)
{
  Byte HReg;
  Boolean OK;

  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    HReg = EvalIntExpression(ArgStr[2], Int8, &OK);
    if (OK)
    {
      DecodeAdr(ArgStr[1], MModMem | MModAbs);
      if (AdrType != ModNone)
      {
        CodeLen = 3 + AdrCnt;
        CodeMem(0xe0, 0x2f);
        BAsmCode[2 + AdrCnt] = HReg;
      }
    }
  }
}

static void DecodeINC_DEC(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg8 | MModReg16 | MModAbs | MModMem);
    switch (AdrType)
    {
      case ModReg8:
        CodeLen = 1;
        BAsmCode[0] = 0x60 | Code | AdrMode;
        break;
      case ModReg16:
        CodeLen = 1;
        BAsmCode[0] = 0x10 | Code | AdrMode;
        break;
      case ModAbs:
        CodeLen = 2;
        BAsmCode[0] = 0x20 | Code;
        BAsmCode[1] = AdrVals[0];
        break;
      case ModMem:
        if (AdrMode == 3)     /* (HL) */
        {
          CodeLen = 1;
          BAsmCode[0] = 0x21 | Code;
        }
        else
        {
          CodeLen = 2 + AdrCnt;
          BAsmCode[0] = 0xe0 | AdrMode;
          memcpy(BAsmCode + 1, AdrVals, AdrCnt);
          BAsmCode[1 + AdrCnt] = 0x20 | Code;
        }
        break;
    }
  }
}

static void DecodeMUL(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg8);
    if (AdrType == ModReg8)
    {
      Byte HReg = AdrMode;
      DecodeAdr(ArgStr[2], MModReg8);
      if (AdrType == ModReg8)
      {
        if ((HReg ^ AdrMode) != 1) WrError(1760);
        else
        {
          HReg = HReg >> 1;
          if (HReg == 0)
          {
            CodeLen = 1;
            BAsmCode[0] = 0x02;
          }
          else
          {
            CodeLen = 2;
            BAsmCode[0] = 0xe8 | HReg;
            BAsmCode[1] = 0x02;
          }
        }
      }
    }
  }
}

static void DecodeDIV(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg16);
    if (AdrType == ModReg16)
    {
      Byte HReg = AdrMode;
      DecodeAdr(ArgStr[2], MModReg8);
      if (AdrType == ModReg8)
      {
        if (AdrMode != 2) WrError(1350);  /* C */
        else if (HReg == 0)
        {
          CodeLen = 1;
          BAsmCode[0] = 0x03;
        }
        else
        {
          CodeLen = 2;
          BAsmCode[0] = 0xe8 | HReg;
          BAsmCode[1] = 0x03;
          if (HReg == 1)
            WrError(140);
        }
      }
    }
  }
}

static void DecodeROLD_RORD(Word Code)
{
  if (ArgCnt != 2) WrError(1110);
  else if (strcasecmp(ArgStr[1], "A")) WrError(1350);
  else
  {
    DecodeAdr(ArgStr[2], MModAbs | MModMem);
    if (AdrType != ModNone)
    {
      CodeLen = 2 + AdrCnt;
      CodeMem(0xe0, Code);
      if (AdrMode == 1)
        WrError(140);
    }
  }
}

static void DecodeJRS(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    Integer AdrInt, Condition;
    Boolean OK;

    Condition = DecodeCondition(ArgStr[1], ConditionCnt - 2);
    if (Condition >= ConditionCnt) WrXError(1360, ArgStr[1]);
    else
    {
      AdrInt = EvalIntExpression(ArgStr[2], Int16, &OK) - (EProgCounter() + 2);
      if (OK)
      {
        if (((AdrInt < -16) || (AdrInt > 15)) && (!SymbolQuestionable)) WrError(1370);
        else
        {
          CodeLen = 1;
          BAsmCode[0] = ((Conditions[Condition].Code - 2) << 5) | (AdrInt & 0x1f);
        }
      }
    }
  }
}

static void DecodeJR(Word Code)
{
  UNUSED(Code);

  if ((ArgCnt != 2) && (ArgCnt != 1)) WrError(1110);
  else
  {
    Integer Condition, AdrInt;
    Boolean OK;

    Condition = (ArgCnt == 1) ? -1 : DecodeCondition(ArgStr[1], 0);
    if (Condition >= ConditionCnt) WrXError(1360, ArgStr[1]);
    else
    {
      AdrInt = EvalIntExpression(ArgStr[ArgCnt], Int16, &OK) - (EProgCounter() + 2);
      if (OK)
      {
        if (((AdrInt < -128) || (AdrInt > 127)) && (!SymbolQuestionable)) WrError(1370);
        else
        {
          CodeLen = 2;
          BAsmCode[0] = (Condition == -1) ?  0xfb : 0xd0 | Conditions[Condition].Code;
          BAsmCode[1] = AdrInt & 0xff;
        }
      }
    }
  }
}

static void DecodeJP_CALL(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    OpSize = 1;
    DecodeAdr(ArgStr[1], MModReg16 | MModAbs | MModMem | MModImm);
    switch (AdrType)
    {
      case ModReg16:
        CodeLen = 2;
        BAsmCode[0] = 0xe8 | AdrMode;
        BAsmCode[1] = Code;
        break;
      case ModAbs:
        CodeLen = 3;
        BAsmCode[0] = 0xe0;
        BAsmCode[1] = AdrVals[0];
        BAsmCode[2] = Code;
        break;
      case ModMem:
        if (AdrMode > 5) WrError(1350);
        else
        {
          CodeLen = 2 + AdrCnt;
          BAsmCode[0] = 0xe0 | AdrMode;
          memcpy(BAsmCode + 1, AdrVals, AdrCnt);
          BAsmCode[1 + AdrCnt] = Code;
        }
        break;
      case ModImm:
        if ((AdrVals[1] == 0xff) && (Code == 0xfc))
        {
          CodeLen = 2;
          BAsmCode[0] = 0xfd;
          BAsmCode[1] = AdrVals[0];
        }
        else
        {
          CodeLen = 3;
          BAsmCode[0] = Code;
          memcpy(BAsmCode + 1, AdrVals, AdrCnt);
        }
        break;
    }
  }
}

static void DecodeCALLV(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;
    Byte HVal = EvalIntExpression(ArgStr[1], Int4, &OK);
    if (OK)
    {
      CodeLen = 1;
      BAsmCode[0] = 0xc0 | (HVal & 15);
    }
  }
}

static void DecodeCALLP(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;
    Integer AdrInt = EvalIntExpression(ArgStr[1], Int16, &OK);
    if (OK)
    {
      if ((Hi(AdrInt) != 0xff) && (Hi(AdrInt) != 0)) WrError(1320);
      else
      {
        CodeLen = 2;
        BAsmCode[0] = 0xfd;
        BAsmCode[1] = Lo(AdrInt);
      }
    }
  }
}

/*--------------------------------------------------------------------------*/

static void AddFixed(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeFixed);
}

static void AddCond(char *NName, Byte NCode)
{
  if (InstrZ >= ConditionCnt) exit(255);
  Conditions[InstrZ].Name = NName;
  Conditions[InstrZ++].Code = NCode;
}

static void AddReg(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeReg);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(203);
  AddInstTable(InstTable, "LD", 0, DecodeLD);
  AddInstTable(InstTable, "XCH", 0, DecodeXCH);
  AddInstTable(InstTable, "CLR", 0, DecodeCLR);
  AddInstTable(InstTable, "LDW", 0, DecodeLDW);
  AddInstTable(InstTable, "PUSH", 7, DecodePUSH_POP);
  AddInstTable(InstTable, "POP", 6, DecodePUSH_POP);
  AddInstTable(InstTable, "TEST", 0xd8, DecodeTEST_CPL_SET);
  AddInstTable(InstTable, "CPL", 0xc0, DecodeTEST_CPL_SET);
  AddInstTable(InstTable, "SET", 0x40, DecodeTEST_CPL_SET);
  AddInstTable(InstTable, "MCMP", 0, DecodeMCMP);
  AddInstTable(InstTable, "INC", 0, DecodeINC_DEC);
  AddInstTable(InstTable, "DEC", 8, DecodeINC_DEC);
  AddInstTable(InstTable, "MUL", 0, DecodeMUL);
  AddInstTable(InstTable, "DIV", 0, DecodeDIV);
  AddInstTable(InstTable, "ROLD", 8, DecodeROLD_RORD);
  AddInstTable(InstTable, "RORD", 9, DecodeROLD_RORD);
  AddInstTable(InstTable, "JRS", 0, DecodeJRS);
  AddInstTable(InstTable, "JR", 0, DecodeJR);
  AddInstTable(InstTable, "JP", 0xfe, DecodeJP_CALL);
  AddInstTable(InstTable, "CALL", 0xfc, DecodeJP_CALL);
  AddInstTable(InstTable, "CALLV", 0, DecodeCALLV);
  AddInstTable(InstTable, "CALLP", 0, DecodeCALLP);

  AddFixed("DI"  , 0x483a);
  AddFixed("EI"  , 0x403a);
  AddFixed("RET" , 0x0005);
  AddFixed("RETI", 0x0004);
  AddFixed("RETN", 0xe804);
  AddFixed("SWI" , 0x00ff);
  AddFixed("NOP" , 0x0000);

  Conditions = (CondRec *) malloc(sizeof(CondRec)*ConditionCnt); InstrZ = 0;
  AddCond("EQ", 0); AddCond("Z" , 0);
  AddCond("NE", 1); AddCond("NZ", 1);
  AddCond("CS", 2); AddCond("LT", 2);
  AddCond("CC", 3); AddCond("GE", 3);
  AddCond("LE", 4); AddCond("GT", 5);
  AddCond("T" , 6); AddCond("F" , 7);

  AddReg("DAA" , 0x0a);  AddReg("DAS" , 0x0b);
  AddReg("SHLC", 0x1c);  AddReg("SHRC", 0x1d);
  AddReg("ROLC", 0x1e);  AddReg("RORC", 0x1f);
  AddReg("SWAP", 0x01);

  InstrZ = 0;
  AddInstTable(InstTable, "ADDC", InstrZ++, DecodeALU);
  AddInstTable(InstTable, "ADD" , InstrZ++, DecodeALU);
  AddInstTable(InstTable, "SUBB", InstrZ++, DecodeALU);
  AddInstTable(InstTable, "SUB" , InstrZ++, DecodeALU);
  AddInstTable(InstTable, "AND" , InstrZ++, DecodeALU);
  AddInstTable(InstTable, "XOR" , InstrZ++, DecodeALU);
  AddInstTable(InstTable, "OR"  , InstrZ++, DecodeALU);
  AddInstTable(InstTable, "CMP" , InstrZ++, DecodeALU);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);

  free(Conditions);
}

/*--------------------------------------------------------------------------*/

static void MakeCode_87C800(void)
{
  CodeLen = 0;
  DontPrint = False;
  OpSize = -1;

  /* zu ignorierendes */

  if (Memo(""))
    return;

  /* Pseudoanweisungen */

  if (DecodeIntelPseudo(False))
    return;

  if (!LookupInstTable(InstTable, OpPart))
    WrXError(1200, OpPart);
}

static Boolean IsDef_87C800(void)
{
  return False;
}

static void SwitchFrom_87C800(void)
{
  DeinitFields();
}

static void SwitchTo_87C800(void)
{
  TurnWords = False;
  ConstMode = ConstModeIntel;
  SetIsOccupied = True;

  PCSymbol = "$";
  HeaderID = 0x54;
  NOPCode = 0x00;
  DivideChars = ",";
  HasAttrs = False;

  ValidSegs = 1 << SegCode;
  Grans[SegCode] = 1;
  ListGrans[SegCode] = 1;
  SegInits[SegCode] = 0;
  SegLimits[SegCode] = 0xffff;

  MakeCode = MakeCode_87C800;
  IsDef = IsDef_87C800;
  SwitchFrom = SwitchFrom_87C800;
  InitFields();
}

void code87c800_init(void)
{
  CPU87C00 = AddCPU("87C00", SwitchTo_87C800);
  CPU87C20 = AddCPU("87C20", SwitchTo_87C800);
  CPU87C40 = AddCPU("87C40", SwitchTo_87C800);
  CPU87C70 = AddCPU("87C70", SwitchTo_87C800);
}
