/* codest7.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator SGS-Thomson ST7                                             */
/*                                                                           */
/* Historie: 21. 5.1997 Grundsteinlegung                                     */
/*            2. 1.1999 ChkPC ersetzt                                        */ 
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: codest7.c,v 1.7 2014/12/07 19:14:01 alfred Exp $                     */
/*****************************************************************************
 * $Log: codest7.c,v $
 * Revision 1.7  2014/12/07 19:14:01  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.6  2014/06/09 10:35:13  alfred
 * - rework to current style
 *
 * Revision 1.5  2010/04/17 13:14:23  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.4  2007/11/24 22:48:07  alfred
 * - some NetBSD changes
 *
 * Revision 1.3  2005/09/08 17:31:05  alfred
 * - add missing include
 *
 * Revision 1.2  2004/05/29 12:04:48  alfred
 * - relocated DecodeMot(16)Pseudo into separate module
 *
 *****************************************************************************/

#include "stdinc.h"

#include <ctype.h>
#include <string.h>

#include "bpemu.h"
#include "strutil.h"
#include "nls.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "motpseudo.h"
#include "codevars.h"
#include "errmsg.h"

#include "codest7.h"


#define ModNone (-1)
#define ModImm 0
#define MModImm (1l << ModImm)
#define ModAbs8 1
#define MModAbs8 (1l << ModAbs8)
#define ModAbs16 2
#define MModAbs16 (1l << ModAbs16)
#define ModIX 3
#define MModIX (1l << ModIX)
#define ModIX8 4
#define MModIX8 (1l << ModIX8)
#define ModIX16 5
#define MModIX16 (1l << ModIX16)
#define ModIY 6
#define MModIY (1l << ModIY)
#define ModIY8 7
#define MModIY8 (1l << ModIY8)
#define ModIY16 8
#define MModIY16 (1l << ModIY16)
#define ModIAbs8 9
#define MModIAbs8 (1l << ModIAbs8)
#define ModIAbs16 10
#define MModIAbs16 (1l << ModIAbs16)
#define ModIXAbs8 11
#define MModIXAbs8 (1l << ModIXAbs8)
#define ModIXAbs16 12
#define MModIXAbs16 (1l << ModIXAbs16)
#define ModIYAbs8 13
#define MModIYAbs8 (1l << ModIYAbs8)
#define ModIYAbs16 14
#define MModIYAbs16 (1l << ModIYAbs16)
#define ModA 15
#define MModA (1l << ModA)
#define ModX 16
#define MModX (1l << ModX)
#define ModY 17
#define MModY (1l << ModY)
#define ModS 18
#define MModS (1l << ModS)
#define ModCCR 19
#define MModCCR (1l << ModCCR)


static CPUVar CPUST7;

static ShortInt AdrType, OpSize;
static Byte AdrPart, PrefixCnt;
static Byte AdrVals[3];

/*--------------------------------------------------------------------------*/

static void AddPrefix(Byte Pref)
{
  BAsmCode[PrefixCnt++] = Pref;
}

static void DecideSize(LongInt Mask, const tStrComp *pArg, LongInt Type1, LongInt Type2, Byte Part1, Byte Part2)
{
  tSymbolSize Size;
  Word Value;
  Boolean OK;
  int l = strlen(pArg->Str);

  if ((l >= 3) && (pArg->Str[l - 2] == '.'))
  {
    if (mytoupper(pArg->Str[l - 1]) == 'B')
    {
      Size = eSymbolSize8Bit; pArg->Str[l - 2] = '\0'; 
    }
    else if (mytoupper(pArg->Str[l - 1]) == 'W')
    {
      Size = eSymbolSize16Bit; pArg->Str[l - 2] = '\0';
    }
    else
      Size = eSymbolSizeUnknown;
  }
  else
    Size = eSymbolSizeUnknown;

  Value = EvalStrIntExpression(pArg, (Size == eSymbolSize8Bit) ? UInt8 : Int16, &OK);

  if (OK)
  {
    if ((Size == eSymbolSize8Bit) || ((Mask & (1l << Type1)) && (Size == eSymbolSizeUnknown) && (Hi(Value) == 0)))
    {
      AdrVals[0] = Lo(Value);
      AdrCnt = 1;
      AdrPart = Part1;
      AdrType=Type1;
    }
    else
    {
      AdrVals[0] = Hi(Value);
      AdrVals[1] = Lo(Value);
      AdrCnt = 2;
      AdrPart = Part2;
      AdrType = Type2;
    }
  }
}

static void DecideASize(LongInt Mask, const tStrComp *pArg, LongInt Type1, LongInt Type2, Byte Part1, Byte Part2)
{
  Boolean I16;
  Boolean OK;
  int l = strlen(pArg->Str);

  if ((l >= 3) && (pArg->Str[l - 2] == '.') && (mytoupper(pArg->Str[l - 1]) == 'W'))
  {
    I16 = True;
    pArg->Str[l - 2] = '\0';
  }
  else if (!((Mask & (1l << Type1))))
    I16 = True;
  else
    I16 = False;

  AdrVals[0] = EvalStrIntExpression(pArg, UInt8, &OK);
  if (OK)
  {
    AdrCnt = 1;
    if (I16)
    {
      AdrPart = Part2;
      AdrType = Type2;
    }
    else
    {
      AdrPart = Part1;
      AdrType = Type1;
    }
  }
}

static Boolean ChkAdrType(ShortInt *pAdrType, LongInt Mask)
{
  if ((*pAdrType != ModNone) && (!(Mask & (1l << *pAdrType))))
  {
    WrError(ErrNum_InvAddrMode);
    *pAdrType = ModNone;
    return False;
  }
  return True;
}

static Boolean DecodeRegCore(const tStrComp *pArg, ShortInt *pResult)
{
  *pResult = ModNone;

  if (!strcasecmp(pArg->Str, "A"))
  {
    *pResult = ModA;
    return True;
  }

  if (!strcasecmp(pArg->Str, "X"))
  {
    *pResult = ModX;
    return True;
  }

  if (!strcasecmp(pArg->Str, "Y"))
  {
    *pResult = ModY;
    return True;
  }

  if (!strcasecmp(pArg->Str, "S"))
  {
    *pResult = ModS;
    return True;
  }

  if (!strcasecmp(pArg->Str, "CC"))
  {
    *pResult = ModCCR;
    return True;
  }

  return False;
}

static Boolean DecodeReg(const tStrComp *pArg, ShortInt *pResult, LongInt Mask)
{
  return DecodeRegCore(pArg, pResult)
      && ChkAdrType(pResult, Mask);
}

static void DecodeAdr(const tStrComp *pArg, LongInt Mask)
{
  Boolean OK, YReg;
  char *p;
  int ArgLen;

  ArgLen = strlen(pArg->Str);

  AdrType = ModNone;
  AdrCnt = 0;

  /* Register ? */

  if (DecodeRegCore(pArg, &AdrType))
  {
    if (ModY == AdrType)
      AddPrefix(0x90);
    goto chk;
  }

  /* immediate ? */

  if (*pArg->Str == '#')
  {
    AdrVals[0] = EvalStrIntExpressionOffs(pArg, 1, Int8, &OK);
    if (OK)
    {
      AdrType = ModImm;
      AdrPart = 0xa;
      AdrCnt = 1;
    }
    goto chk;
  }

  /* speicherindirekt ? */

  if ((*pArg->Str == '[') && (pArg->Str[ArgLen - 1] == ']'))
  {
    tStrComp Comp;

    StrCompRefRight(&Comp, pArg, 1);
    Comp.Str[ArgLen - 2] = '\0'; Comp.Pos.Len--;
    DecideASize(Mask, &Comp, ModIAbs8, ModIAbs16, 0xb, 0xc);
    Comp.Str[ArgLen - 2] = ']';
    if (AdrType != ModNone) AddPrefix(0x92);
    goto chk;
  }

  /* sonstwie indirekt ? */

  if (IsIndirect(pArg->Str))
  {
    tStrComp Comp, Left, Right;

    StrCompRefRight(&Comp, pArg, 1);
    Comp.Str[ArgLen - 2] = '\0'; Comp.Pos.Len--;

    /* ein oder zwei Argumente ? */

    p = QuotPos(Comp.Str, ',');
    if (!p)
    {
      AdrPart = 0xf;
      if (!strcasecmp(Comp.Str, "X")) AdrType = ModIX;
      else if (!strcasecmp(Comp.Str, "Y"))
      {
        AdrType = ModIY;
        AddPrefix(0x90);
      }
      else WrStrErrorPos(ErrNum_InvReg, &Comp);
      goto chk;
    }

    StrCompSplitRef(&Left, &Right, &Comp, p);

    if (!strcasecmp(Left.Str, "X"))
    {
      Left = Right;
      YReg = False;
    }
    else if (!strcasecmp(Right.Str, "X"))
      YReg = False;
    else if (!strcasecmp(Left.Str, "Y"))
    {
      Left = Right;
      YReg = True;
    }
    else if (!strcasecmp(Right.Str, "Y"))
      YReg = True;
    else
    {
      WrStrErrorPos(ErrNum_InvAddrMode, &Comp);
      return;
    }

    /* speicherindirekt ? */

    ArgLen = strlen(Left.Str);
    if ((*Left.Str == '[') && (Left.Str[ArgLen - 1] == ']'))
    {
      StrCompRefRight(&Right, &Left, 1);
      Right.Str[ArgLen - 2] = '\0'; Right.Pos.Len--;
      if (YReg)
      {
        DecideASize(Mask, &Right, ModIYAbs8, ModIYAbs16, 0xe, 0xd);
        if (AdrType != ModNone) AddPrefix(0x91);
      }
      else
      {
        DecideASize(Mask, &Right, ModIXAbs8, ModIXAbs16, 0xe, 0xd);
        if (AdrType != ModNone) AddPrefix(0x92);
      }
    }
    else
    {
      if (YReg) DecideSize(Mask, &Left, ModIY8, ModIY16, 0xe, 0xd);
      else DecideSize(Mask, &Left, ModIX8, ModIX16, 0xe, 0xd);
      if ((AdrType != ModNone) && YReg) AddPrefix(0x90);
    }

    goto chk;
  }

  /* dann absolut */

  DecideSize(Mask, pArg, ModAbs8, ModAbs16, 0xb, 0xc);

chk:
  if (!ChkAdrType(&AdrType, Mask))
  {
    WrError(ErrNum_InvAddrMode);
    AdrCnt = 0;
  }
}

/*--------------------------------------------------------------------------*/
/* code generators */

static void DecodeFixed(Word Code)
{
  if (!ChkArgCnt(0, 0));
  else if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else
  {
    BAsmCode[PrefixCnt] = Code;
    CodeLen = PrefixCnt + 1;
  }
}

static void DecodeLD(Word Code)
{
  LongInt Mask;

  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else
  {
    DecodeAdr(&ArgStr[1], MModA | MModX | MModY | MModS |
                         MModImm | MModAbs8 | MModAbs16 | MModIX | MModIX8 | MModIX16 | MModIY |
                         MModIY8 | MModIY16 | MModIAbs8 | MModIAbs16 | MModIXAbs8 | MModIXAbs16 |
                         MModIYAbs8 | MModIYAbs16);

    switch (AdrType)
    {
      case ModA:
        DecodeAdr(&ArgStr[2], MModImm | MModAbs8 | MModAbs16 | MModIX | MModIX8 | MModIX16 | MModIY |
                             MModIY8 | MModIY16 | MModIAbs8 | MModIAbs16 | MModIXAbs8 | MModIXAbs16 |
                             MModIYAbs8 | MModIYAbs16 | MModX | MModY | MModS);
        switch (AdrType)
        {
          case ModX:
          case ModY:
            BAsmCode[PrefixCnt] = 0x9f;
            CodeLen = PrefixCnt + 1;
            break;
          case ModS:
            BAsmCode[PrefixCnt] = 0x9e;
            CodeLen = PrefixCnt + 1;
            break;
          default:
            if (AdrType != ModNone)
            {
              BAsmCode[PrefixCnt] = 0x06 + (AdrPart << 4);
              memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
              CodeLen = PrefixCnt + 1 + AdrCnt;
            }
        }
        break;
      case ModX:
        DecodeAdr(&ArgStr[2], MModImm | MModAbs8 | MModAbs16 | MModIX | MModIX8 |
                             MModIX16 | MModIAbs8 | MModIAbs16 | MModIXAbs8 | MModIXAbs16 |
                             MModA | MModY | MModS);
        switch (AdrType)
        {
          case ModA:
            BAsmCode[PrefixCnt] = 0x97;
            CodeLen = PrefixCnt + 1;
            break;
          case ModY:
            BAsmCode[0] = 0x93;
            CodeLen = 1;
            break;
          case ModS:
            BAsmCode[PrefixCnt] = 0x96;
            CodeLen = PrefixCnt + 1;
            break;
          default:
            if (AdrType != ModNone)
            {
              BAsmCode[PrefixCnt] = 0x0e + (AdrPart << 4); /* ANSI :-O */
              memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
              CodeLen = PrefixCnt + 1 + AdrCnt;
            }
        }
        break;
      case ModY:
        PrefixCnt = 0;
        DecodeAdr(&ArgStr[2], MModImm | MModAbs8 | MModAbs16 | MModIY | MModIY8 |
                             MModIY16 | MModIAbs8 | MModIAbs16 | MModIYAbs8 | MModIYAbs16 |
                             MModA | MModX | MModS);
        switch (AdrType)
        {
          case ModA:
            AddPrefix(0x90);
            BAsmCode[PrefixCnt] = 0x97;
            CodeLen = PrefixCnt + 1;
            break;
          case ModX:
            AddPrefix(0x90);
            BAsmCode[PrefixCnt] = 0x93;
            CodeLen = PrefixCnt + 1;
            break;
          case ModS:
            AddPrefix(0x90);
            BAsmCode[PrefixCnt] = 0x96;
            CodeLen = PrefixCnt + 1;
            break;
          default:
            if (AdrType != ModNone)
            {
              if (PrefixCnt == 0) AddPrefix(0x90);
              if (BAsmCode[0] == 0x92) BAsmCode[0]--;
              BAsmCode[PrefixCnt] = 0x0e + (AdrPart << 4); /* ANSI :-O */
              memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
              CodeLen = PrefixCnt + 1 + AdrCnt;
            }
        }
        break;
      case ModS:
        DecodeAdr(&ArgStr[2], MModA | MModX | MModY);
        switch (AdrType)
        {
          case ModA:
            BAsmCode[PrefixCnt] = 0x95;
            CodeLen = PrefixCnt + 1;
            break;
          case ModX:
          case ModY:
            BAsmCode[PrefixCnt] = 0x94;
            CodeLen = PrefixCnt + 1;
            break;
        }
        break;
      default:
        if (AdrType != ModNone)
        {
          ShortInt RegAdrType;

          if (!DecodeReg(&ArgStr[2], &RegAdrType, MModA | MModX | MModY)) WrStrErrorPos(ErrNum_InvReg, &ArgStr[2]);
          else switch (RegAdrType)
          {
            case ModA:
              Mask = MModAbs8 | MModAbs16 | MModIX | MModIX8 | MModIX16 | MModIY |
                     MModIY8 | MModIY16 | MModIAbs8 | MModIAbs16 | MModIXAbs8 | MModIXAbs16 |
                     MModIYAbs8 | MModIYAbs16;
              if (!ChkAdrType(&AdrType, Mask)) WrError(ErrNum_InvAddrMode);
              else
              {
                BAsmCode[PrefixCnt] = 0x07 + (AdrPart << 4);
                memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                CodeLen = PrefixCnt + 1 + AdrCnt;
              }
              break;
            case ModX:
              if (!ChkAdrType(&AdrType, MModAbs8 | MModAbs16 | MModIX | MModIX8 |
                                        MModIX16 | MModIAbs8 | MModIAbs16 | MModIXAbs8 | MModIXAbs16))
                WrError(ErrNum_InvAddrMode);
              else
              {
                BAsmCode[PrefixCnt] = 0x0f + (AdrPart << 4);
                memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                CodeLen = PrefixCnt + 1 + AdrCnt;
              }
              break;
            case ModY:
              if (!ChkAdrType(&AdrType, MModAbs8 | MModAbs16 | MModIY | MModIY8 |
                                        MModIY16 | MModIAbs8 | MModIAbs16 | MModIYAbs8 | MModIYAbs16))
                WrError(ErrNum_InvAddrMode);
              else
              {
                if (PrefixCnt == 0) AddPrefix(0x90);
                if (BAsmCode[0] == 0x92) BAsmCode[0]--;
                BAsmCode[PrefixCnt] = 0x0f + (AdrPart << 4);
                memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                CodeLen = PrefixCnt + 1 + AdrCnt;
              }
              break;
          }
        }
    }
  }
}

static void DecodePUSH_POP(Word Code)
{
  if (!ChkArgCnt(1, 1));
  else if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else
  {
    DecodeAdr(&ArgStr[1], MModA | MModX | MModY | MModCCR);
    if (AdrType != ModNone)
    {
      switch (AdrType)
      {
        case ModA: BAsmCode[PrefixCnt] = 0x84 + Code; break;
        case ModX:
        case ModY: BAsmCode[PrefixCnt] = 0x85 + Code; break;
        case ModCCR: BAsmCode[PrefixCnt] = 0x86 + Code; break;
      }
      CodeLen = PrefixCnt + 1;
    }
  }
}

static void DecodeCP(Word Code)
{
  LongInt Mask;

  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else
  {
    DecodeAdr(&ArgStr[1], MModA | MModX | MModY);
    switch (AdrType)
    {
      case ModA:
        Mask = MModImm | MModAbs8 | MModAbs16 | MModIX | MModIX8 | MModIX16 | MModIY |
               MModIY8 | MModIY16 | MModIAbs8 | MModIAbs16 | MModIXAbs8 | MModIXAbs16 |
               MModIYAbs8 | MModIYAbs16;
        DecodeAdr(&ArgStr[2], Mask);
        if (AdrType != ModNone)
        {
          BAsmCode[PrefixCnt] = 0x01 + (AdrPart << 4);
          memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
          CodeLen = PrefixCnt + 1 + AdrCnt;
        }
        break;
      case ModX:
        DecodeAdr(&ArgStr[2], MModImm | MModAbs8 | MModAbs16 | MModIX | MModIX8 |
                             MModIX16 | MModIAbs8 | MModIAbs16 | MModIXAbs8 | MModIXAbs16);
        if (AdrType != ModNone)
        {
          BAsmCode[PrefixCnt] = 0x03 + (AdrPart << 4);
          memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
          CodeLen = PrefixCnt + 1 + AdrCnt;
        }
        break;
      case ModY:
        PrefixCnt = 0;
        DecodeAdr(&ArgStr[2], MModImm | MModAbs8 | MModAbs16 | MModIY | MModIY8 |
                             MModIY16 | MModIAbs8 | MModIAbs16 | MModIYAbs8 | MModIYAbs16);
        if (AdrType != ModNone)
        {
          if (PrefixCnt == 0) AddPrefix(0x90);
          if (BAsmCode[0] == 0x92) BAsmCode[0]--;
          BAsmCode[PrefixCnt] = 0x03 + (AdrPart << 4);
          memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
          CodeLen = PrefixCnt + 1 + AdrCnt;
        }
        break;
    }
  }
}

static void DecodeAri(Word Code)
{
  LongInt Mask;

  if (!ChkArgCnt(2, 2));
  else if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else
  {
    DecodeAdr(&ArgStr[1], MModA);
    if (AdrType == ModA)
    {
      Mask = MModAbs8 | MModAbs16 | MModIX | MModIX8 | MModIX16 | MModIY |
             MModIY8 | MModIY16 | MModIAbs8 | MModIAbs16 | MModIXAbs8 | MModIXAbs16 |
             MModIYAbs8 | MModIYAbs16;
      if (Hi(Code)) Mask |= MModImm;
      DecodeAdr(&ArgStr[2], Mask);
      if (AdrType != ModNone)
      {
        BAsmCode[PrefixCnt] = Lo(Code) + (AdrPart << 4);
        memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
        CodeLen = PrefixCnt + 1 + AdrCnt;
      }
    }
  }
}

static void DecodeRMW(Word Code)
{
  if (!ChkArgCnt(1, 1));
  else if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else
  {
    DecodeAdr(&ArgStr[1], MModA | MModX | MModY | MModAbs8 | MModIX | MModIX8 |
                         MModIY | MModIY8 | MModIAbs8 | MModIXAbs8 | MModIYAbs8);
    switch (AdrType)
    {
      case ModA:
        BAsmCode[PrefixCnt] = 0x40 + Code;
        CodeLen = PrefixCnt + 1;
        break;
      case ModX:
      case ModY:
        BAsmCode[PrefixCnt] = 0x50 + Code;
        CodeLen = PrefixCnt + 1;
        break;
      default:
        if (AdrType != ModNone)
        {
          BAsmCode[PrefixCnt] = Code + ((AdrPart - 8) << 4);
          memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
          CodeLen = PrefixCnt + 1 + AdrCnt;
        }
    }
  }
}

static void DecodeMUL(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else
  {
    DecodeAdr(&ArgStr[2], MModA);
    if (AdrType != ModNone)
    {
      DecodeAdr(&ArgStr[1], MModX | MModY);
      if (AdrType != ModNone)
      {
        BAsmCode[PrefixCnt] = 0x42;
        CodeLen = PrefixCnt + 1;
      }
    }
  }
}

static void DecodeBRES_BSET(Word Code)
{
  if (!ChkArgCnt(2, 2));
  else if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else if (*ArgStr[2].Str != '#') WrError(ErrNum_InvAddrMode);
  else
  {
    Boolean OK;
    int Bit = EvalStrIntExpressionOffs(&ArgStr[2], 1, UInt3, &OK);
    if (OK)
    {
      DecodeAdr(&ArgStr[1], MModAbs8 | MModIAbs8);
      if (AdrType != ModNone)
      {
        BAsmCode[PrefixCnt] = Code + (Bit << 1);
        memcpy(BAsmCode + 1 + PrefixCnt, AdrVals, AdrCnt);
        CodeLen = PrefixCnt + 1 + AdrCnt;
      }
    }
  }
}

static void DecodeBTJF_BTJT(Word Code)
{
  if (!ChkArgCnt(3, 3));
  else if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else if (*ArgStr[2].Str != '#') WrError(ErrNum_InvAddrMode);
  else
  {
    Boolean OK;
    int Bit = EvalStrIntExpressionOffs(&ArgStr[2], 1, UInt3, &OK);
    if (OK)
    {
      DecodeAdr(&ArgStr[1], MModAbs8 + MModIAbs8);
      if (AdrType != ModNone)
      {
        Integer AdrInt;

        BAsmCode[PrefixCnt] = Code + (Bit << 1);
        memcpy(BAsmCode + 1 + PrefixCnt, AdrVals, AdrCnt);
        AdrInt = EvalStrIntExpression(&ArgStr[3], UInt16, &OK) - (EProgCounter() + PrefixCnt + 1 + AdrCnt);
        if (OK)
        {
          if ((!SymbolQuestionable) && ((AdrInt < -128) || (AdrInt > 127))) WrError(ErrNum_JmpDistTooBig);
          else
          {
            BAsmCode[PrefixCnt + 1 + AdrCnt] = AdrInt & 0xff;
            CodeLen = PrefixCnt + 1 + AdrCnt + 1;
          }
        }
      }
    }
  }
}

static void DecodeJP_CALL(Word Code)
{
  if (!ChkArgCnt(1, 1));
  else if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else
  {
    LongInt Mask;

    Mask = MModAbs8 | MModAbs16 | MModIX | MModIX8 | MModIX16 | MModIY |
           MModIY8 | MModIY16 | MModIAbs8 | MModIAbs16 | MModIXAbs8 | MModIXAbs16 |
           MModIYAbs8 | MModIYAbs16;
    DecodeAdr(&ArgStr[1], Mask);
    if (AdrType != ModNone)
    {
      BAsmCode[PrefixCnt] = Code + (AdrPart << 4);
      memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
      CodeLen = PrefixCnt + 1 + AdrCnt;
    }
  }
}

static void DecodeRel(Word Code)
{
  if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else if (!ChkArgCnt(1, 1));
  else if (*ArgStr[1].Str == '[')
  {
    DecodeAdr(&ArgStr[1], MModIAbs8);
    if (AdrType != ModNone)
    {
      BAsmCode[PrefixCnt] = Code;
      memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
      CodeLen = PrefixCnt + 1 + AdrCnt;
    }
  }
  else
  {
    Boolean OK;
    Integer AdrInt = EvalStrIntExpression(&ArgStr[1], UInt16, &OK) - (EProgCounter() + 2);
    if (OK)
    {
      if ((!SymbolQuestionable) && ((AdrInt < -128) || (AdrInt > 127))) WrError(ErrNum_JmpDistTooBig);
      else
      {
        BAsmCode[0] = Code;
        BAsmCode[1] = AdrInt & 0xff;
        CodeLen = 2;
      }
    }
  }
}

/*--------------------------------------------------------------------------*/

static void AddFixed(char *NName, Byte NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeFixed);
}

static void AddAri(char *NName, Word NCode, Boolean NMay)
{
  AddInstTable(InstTable, NName, NCode | (NMay << 8), DecodeAri);
}

static void AddRMW(char *NName, Byte NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeRMW);
}

static void AddRel(char *NName, Byte NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeRel);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(201);
  AddInstTable(InstTable, "LD", 0, DecodeLD);
  AddInstTable(InstTable, "PUSH", 4, DecodePUSH_POP);
  AddInstTable(InstTable, "POP", 0, DecodePUSH_POP);
  AddInstTable(InstTable, "CP", 0, DecodeCP);
  AddInstTable(InstTable, "MUL", 0, DecodeMUL);
  AddInstTable(InstTable, "BRES", 0x11, DecodeBRES_BSET);
  AddInstTable(InstTable, "BSET", 0x10, DecodeBRES_BSET);
  AddInstTable(InstTable, "BTJF", 0x01, DecodeBTJF_BTJT);
  AddInstTable(InstTable, "BTJT", 0x00, DecodeBTJF_BTJT);
  AddInstTable(InstTable, "JP", 0x0c, DecodeJP_CALL);
  AddInstTable(InstTable, "CALL", 0x0d, DecodeJP_CALL);

  AddFixed("HALT" , 0x8e); AddFixed("IRET" , 0x80); AddFixed("NOP"  , 0x9d);
  AddFixed("RCF"  , 0x98); AddFixed("RET"  , 0x81); AddFixed("RIM"  , 0x9a);
  AddFixed("RSP"  , 0x9c); AddFixed("SCF"  , 0x99); AddFixed("SIM"  , 0x9b);
  AddFixed("TRAP" , 0x83); AddFixed("WFI"  , 0x8f);

  AddAri("ADC" , 0x09, True ); AddAri("ADD" , 0x0b, True ); AddAri("AND" , 0x04, True );
  AddAri("BCP" , 0x05, True ); AddAri("OR"  , 0x0a, True ); AddAri("SBC" , 0x02, True );
  AddAri("SUB" , 0x00, True ); AddAri("XOR" , 0x08, True );

  AddRMW("CLR" , 0x0f); AddRMW("CPL" , 0x03); AddRMW("DEC" , 0x0a);
  AddRMW("INC" , 0x0c); AddRMW("NEG" , 0x00); AddRMW("RLC" , 0x09);
  AddRMW("RRC" , 0x06); AddRMW("SLA" , 0x08); AddRMW("SLL" , 0x08);
  AddRMW("SRA" , 0x07); AddRMW("SRL" , 0x04); AddRMW("SWAP", 0x0e);
  AddRMW("TNZ" , 0x0d);

  AddRel("CALLR", 0xad); AddRel("JRA"  , 0x20); AddRel("JRC"  , 0x25);
  AddRel("JREQ" , 0x27); AddRel("JRF"  , 0x21); AddRel("JRH"  , 0x29);
  AddRel("JRIH" , 0x2f); AddRel("JRIL" , 0x2e); AddRel("JRM"  , 0x2d);
  AddRel("JRMI" , 0x2b); AddRel("JRNC" , 0x24); AddRel("JRNE" , 0x26);
  AddRel("JRNH" , 0x28); AddRel("JRNM" , 0x2c); AddRel("JRPL" , 0x2a);
  AddRel("JRT"  , 0x20); AddRel("JRUGE", 0x24); AddRel("JRUGT", 0x22);
  AddRel("JRULE", 0x23); AddRel("JRULT", 0x25);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*--------------------------------------------------------------------------*/

static void MakeCode_ST7(void)
{
  CodeLen = 0; DontPrint = False; OpSize = eSymbolSize16Bit; PrefixCnt = 0;

  /* zu ignorierendes */

  if (Memo("")) return;

  /* Attribut verarbeiten */

  if (!DecodeMoto16AttrSize(*AttrPart.Str, &OpSize, False))
    return;

  /* Pseudoanweisungen */

  if (DecodeMotoPseudo(True)) return;
  if (DecodeMoto16Pseudo(OpSize,True)) return;

  if (!LookupInstTable(InstTable, OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownOpcode, &OpPart);
}

static Boolean IsDef_ST7(void)
{
  return False;
}

static void SwitchFrom_ST7(void)
{
  DeinitFields();
}

static void SwitchTo_ST7(void)
{
  TurnWords = False; ConstMode = ConstModeMoto; SetIsOccupied = False;

  PCSymbol = "PC"; HeaderID = 0x33; NOPCode = 0x9d;
  DivideChars = ","; HasAttrs = True; AttrChars = ".";

  ValidSegs = 1 << SegCode;
  Grans[SegCode] = 1; ListGrans[SegCode] = 1; SegInits[SegCode] = 0;
  SegLimits[SegCode] = 0xffff;

  MakeCode = MakeCode_ST7;
  IsDef = IsDef_ST7;
  SwitchFrom = SwitchFrom_ST7;
  InitFields();
  AddMoto16PseudoONOFF();

  SetFlag(&DoPadding, DoPaddingName, False);
}

void codest7_init(void)
{
  CPUST7 = AddCPU("ST7", SwitchTo_ST7);
}


