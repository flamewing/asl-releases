/* codeh8_5.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* AS-Codegenerator H8/500                                                   */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "motpseudo.h"
#include "codevars.h"
#include "errmsg.h"

#include "codeh8_5.h"

#define OneOrderCount 13
#define OneRegOrderCount 3
#define RegEAOrderCount 9
#define TwoRegOrderCount 3


#define ModNone (-1)
#define ModReg 0
#define MModReg (1 << ModReg)
#define ModIReg 1
#define MModIReg (1 << ModIReg)
#define ModDisp8 2
#define MModDisp8 (1 << ModDisp8)
#define ModDisp16 3
#define MModDisp16 (1 << ModDisp16)
#define ModPredec 4
#define MModPredec (1 << ModPredec)
#define ModPostInc 5
#define MModPostInc (1 << ModPostInc)
#define ModAbs8 6
#define MModAbs8 (1 << ModAbs8)
#define ModAbs16 7
#define MModAbs16 (1 << ModAbs16)
#define ModImm 8
#define MModImm (1 << ModImm)

#define MModAll (MModReg|MModIReg|MModDisp8|MModDisp16|MModPredec|MModPostInc|MModAbs8|MModAbs16|MModImm)
#define MModNoImm (MModAll-MModImm)


typedef struct
{
  char *Name;
  Word Code;
  Byte SizeMask;
  tSymbolSize DefSize;
} OneOrder;


static CPUVar CPU532,CPU534,CPU536,CPU538;

static tSymbolSize OpSize;
static char *Format;
static ShortInt AdrMode;
static Byte AdrByte,FormatCode;
static Byte AdrVals[3];
static Byte AbsBank;

static ShortInt Adr2Mode;
static Byte Adr2Byte,Adr2Cnt;
static Byte Adr2Vals[3];

static LongInt Reg_DP,Reg_EP,Reg_TP,Reg_BR;

static OneOrder *OneOrders;
static OneOrder *OneRegOrders;
static OneOrder *RegEAOrders;
static OneOrder *TwoRegOrders;

#define ASSUMEH8_5Count 4
static ASSUMERec ASSUMEH8_5s[ASSUMEH8_5Count] =
{
  {"DP", &Reg_DP, 0, 0xff, -1, NULL},
  {"EP", &Reg_EP, 0, 0xff, -1, NULL},
  {"TP", &Reg_TP, 0, 0xff, -1, NULL},
  {"BR", &Reg_BR, 0, 0xff, -1, NULL}
};

/*-------------------------------------------------------------------------*/
/* Adressparsing */

static void SetOpSize(tSymbolSize NSize)
{
  if (OpSize == eSymbolSizeUnknown) OpSize = NSize;
  else if (OpSize != NSize) WrError(ErrNum_UndefOpSizes);
}

static Boolean DecodeReg(char *Asc, Byte *pErg)
{
  if (!as_strcasecmp(Asc, "SP")) *pErg = 7;
  else if (!as_strcasecmp(Asc, "FP")) *pErg=6;
  else if ((strlen(Asc) == 2) && (mytoupper(*Asc) == 'R') && (Asc[1] >= '0') && (Asc[1] <= '7'))
    *pErg = Asc[1] - '0';
  else
    return False;
  return True;
}

static Boolean DecodeRegList(char *Asc, Byte *pErg)
{
  String Part;
  Byte Reg1, Reg2, z;
  char *p;

  if (IsIndirect(Asc))
  {
    Asc++; Asc[strlen(Asc) - 1] = '\0';
    KillBlanks(Asc);
  }

  *pErg = 0;
  while (*Asc)
  {
    p = QuotPos(Asc, ',');
    if (!p)
    {
      strmaxcpy(Part, Asc, STRINGSIZE); *Asc = '\0';
    }
    else
    {
      *p = '\0'; strmaxcpy(Part, Asc, STRINGSIZE); strmov(Asc, p + 1);
    }
    if (DecodeReg(Part,&Reg1)) *pErg |= (1 << Reg1);
    else
    {
      p = strchr(Part,'-');
      if (!p)
        return False;
      *p = '\0'; 
      if (!DecodeReg(Part, &Reg1)) return False;
      if (!DecodeReg(p + 1, &Reg2)) return False;
      if (Reg1 > Reg2) Reg2 += 8;
      for (z = Reg1; z <= Reg2; z++) *pErg |= (1 << (z & 7));
    }
  }

  return True;
}

static Boolean DecodeCReg(char *Asc, Byte *pErg)
{
  if (!as_strcasecmp(Asc, "SR"))
  {
    *pErg = 0; SetOpSize(eSymbolSize16Bit);
  }
  else if (!as_strcasecmp(Asc, "CCR"))
  {
    *pErg = 1; SetOpSize(eSymbolSize8Bit);
  }
  else if (!as_strcasecmp(Asc, "BR"))
  {
    *pErg = 3; SetOpSize(eSymbolSize8Bit);
  }
  else if (!as_strcasecmp(Asc, "EP"))
  {
    *pErg = 4; SetOpSize(eSymbolSize8Bit);
  }
  else if (!as_strcasecmp(Asc, "DP"))
  {
    *pErg = 5; SetOpSize(eSymbolSize8Bit);
  }
  else if (!as_strcasecmp(Asc, "TP"))
  {
    *pErg = 7; SetOpSize(eSymbolSize8Bit);
  }
  else
    return False;
  return True;
}

static void SplitDisp(tStrComp *pArg, tSymbolSize *pSize)
{
  int l = strlen(pArg->Str);

  if ((l > 2) && !strcmp(pArg->Str + l - 2, ":8"))
  {
    StrCompShorten(pArg, 2);
    *pSize = eSymbolSize8Bit;
  }
  else if ((l > 3) && !strcmp(pArg->Str + l - 3, ":16"))
  {
    StrCompShorten(pArg, 3);
    *pSize = eSymbolSize16Bit;
  }
}

static void DecideAbsolute(LongInt Value, ShortInt Size, Boolean Unknown, Word Mask)
{
  LongInt Base;

  if (Size == eSymbolSizeUnknown)
  {
    if (((Value >> 8) == Reg_BR) && (Mask & MModAbs8)) Size = eSymbolSize8Bit;
    else Size = eSymbolSize16Bit;
  }

  switch (Size)
  {
    case eSymbolSize8Bit:
      if (Unknown) Value = (Value & 0xff) | (Reg_BR << 8);
      if ((Value >> 8) != Reg_BR) WrError(ErrNum_InAccPage);
      AdrMode = ModAbs8; AdrByte = 0x05;
      AdrVals[0] = Value & 0xff; AdrCnt = 1;
      break;
    case eSymbolSize16Bit:
      if (Maximum)
      {
        Base = AbsBank;
        Base <<= 16;
      }
      else
        Base = 0;
      if (Unknown) Value = (Value & 0xffff) | Base;
      if ((Value >> 16) != (Base >> 16)) WrError(ErrNum_InAccPage);
      AdrMode = ModAbs16; AdrByte = 0x15;
      AdrVals[0] = (Value >> 8) & 0xff;
      AdrVals[1] = Value & 0xff;
      AdrCnt = 2;
      break;
  }
}

static void ChkAdr(Word Mask)
{
  if ((AdrMode != ModNone) && (!(Mask & (1 << AdrMode))))
  {
    WrError(ErrNum_InvAddrMode); AdrMode = ModNone; AdrCnt = 0;
  }
}

static void DecodeAdr(tStrComp *pArg, Word Mask)
{
  Word AdrWord;
  Boolean OK, Unknown;
  tSymbolFlags Flags;
  LongInt DispAcc;
  Byte HReg;
  tSymbolSize DispSize;
  ShortInt RegPart;
  char *p;

  AdrMode = ModNone; AdrCnt = 0;

  /* einfaches Register ? */

  if (DecodeReg(pArg->Str, &AdrByte))
  {
    AdrMode = ModReg; AdrByte |= 0xa0; ChkAdr(Mask); return;
  }

  /* immediate ? */

  if (*pArg->Str == '#')
  {
    switch (OpSize)
    {
      case eSymbolSizeUnknown:
        OK = False; WrError(ErrNum_UndefOpSizes);
        break;
      case eSymbolSize8Bit:
        AdrVals[0] = EvalStrIntExpressionOffs(pArg, 1, Int8, &OK);
        break;
      case eSymbolSize16Bit:
        AdrWord = EvalStrIntExpressionOffs(pArg, 1, Int16, &OK);
        AdrVals[0] = Hi(AdrWord); AdrVals[1] = Lo(AdrWord);
        break;
      default:
        OK = False;
        break;
    }
    if (OK)
    {
      AdrMode = ModImm; AdrByte = 0x04; AdrCnt = OpSize + 1;
    }
    ChkAdr(Mask); return;
  }

  /* indirekt ? */

  if (*pArg->Str == '@')
  {
    tStrComp Arg;

    StrCompRefRight(&Arg, pArg, 1);
    if (IsIndirect(Arg.Str))
    {
      StrCompIncRefLeft(&Arg, 1);
      StrCompShorten(&Arg, 1);
    }

    /* Predekrement ? */

    if ((*Arg.Str == '-') && (DecodeReg(Arg.Str + 1, &AdrByte)))
    {
      AdrMode = ModPredec; AdrByte |= 0xb0;
    }

    /* Postinkrement ? */

    else if ((*Arg.Str) && (Arg.Str[strlen(Arg.Str) - 1] == '+'))
    {
      StrCompShorten(&Arg, 1);
      if (DecodeReg(Arg.Str, &AdrByte))
      {
        AdrMode = ModPostInc; AdrByte |= 0xc0;
      }
    }

    /* zusammengesetzt */

    else
    {
      tStrComp Remainder;

      DispAcc = 0; DispSize = eSymbolSizeUnknown; RegPart = -1; OK = True; Unknown = False;
      do
      {
        p = QuotPos(Arg.Str, ',');
        if (p)
          StrCompSplitRef(&Arg, &Remainder, &Arg, p);
        if (DecodeReg(Arg.Str, &HReg))
        {
          if (RegPart != -1)
          {
            WrError(ErrNum_InvAddrMode); OK = False;
          }
          else
            RegPart = HReg;
        }
        else
        {
          SplitDisp(&Arg, &DispSize);
          DispAcc += EvalStrIntExpressionOffsWithFlags(&Arg, !!(*Arg.Str == '#'), Int32, &OK, &Flags);
          if (mFirstPassUnknown(Flags)) Unknown = True;
        }
        if (p)
          Arg = Remainder;
      }
      while (p && OK);
      if (OK)
      {
        if (RegPart == -1) DecideAbsolute(DispAcc, DispSize, Unknown, Mask);
        else if (DispAcc == 0)
        {
          switch (DispSize)
          {
            case eSymbolSizeUnknown:
              AdrMode = ModIReg; AdrByte = 0xd0 | RegPart;
              break;
            case eSymbolSize8Bit:
              AdrMode = ModDisp8; AdrByte = 0xe0 | RegPart;
              AdrVals[0] = 0; AdrCnt = 1;
              break;
            case eSymbolSize16Bit:
              AdrMode = ModDisp16; AdrByte = 0xf0 | RegPart;
              AdrVals[0] = 0; AdrVals[1] = 0; AdrCnt = 2;
              break;
            default:
              break;
          }
        }
        else
        {
          if (DispSize == eSymbolSizeUnknown)
          {
            if ((DispAcc >= -128) && (DispAcc < 127)) DispSize = eSymbolSize8Bit;
            else DispSize = eSymbolSize16Bit;
          }
          switch (DispSize)
          {
            case eSymbolSize8Bit:
              if (Unknown) DispAcc &= 0x7f;
              if (ChkRange(DispAcc, -128, 127))
              {
                AdrMode = ModDisp8; AdrByte = 0xe0 | RegPart;
                AdrVals[0] = DispAcc & 0xff; AdrCnt = 1;
              }
              break;
            case eSymbolSize16Bit:
              if (Unknown) DispAcc &= 0x7fff;
              if (ChkRange(DispAcc, -0x8000l, 0xffffl))
              {
                AdrMode = ModDisp16; AdrByte = 0xf0 | RegPart;
                AdrVals[1] = DispAcc & 0xff;
                AdrVals[0] = (DispAcc >> 8) & 0xff;
                AdrCnt = 2;
              }
              break;
            default:
              break;
          }
        }
      }
    }
    ChkAdr(Mask); return;
  }

  /* absolut */

  DispSize = eSymbolSizeUnknown; SplitDisp(pArg, &DispSize);
  DispAcc = EvalStrIntExpressionWithFlags(pArg, UInt24, &OK, &Flags);
  DecideAbsolute(DispAcc, DispSize, mFirstPassUnknown(Flags), Mask);

  ChkAdr(Mask);
}

static LongInt ImmVal(void)
{
  LongInt t;

  switch (OpSize)
  {
    case eSymbolSize8Bit:
      t = AdrVals[0]; if (t > 127) t -= 256;
      break;
    case eSymbolSize16Bit:
      t = (((Word)AdrVals[0]) << 8) + AdrVals[1];
      if (t > 0x7fff) t -= 0x10000;
      break;
    default:
      t = 0; WrError(ErrNum_InternalError);
  }
  return t;
}

/*-------------------------------------------------------------------------*/

static Boolean CheckFormat(const char *FSet)
{
  const char *p;

  if (!strcmp(Format, " "))
    FormatCode = 0;
  else
  {
    p = strchr(FSet, *Format);
    if (!p)
    {
      WrError(ErrNum_InvFormat);
      return False;
    }
    else
      FormatCode = p - FSet + 1;
  }
  return True;
}

static void CopyAdr(void)
{
  Adr2Mode = AdrMode;
  Adr2Byte = AdrByte;
  Adr2Cnt = AdrCnt;
  memcpy(Adr2Vals, AdrVals, AdrCnt);
}

/*-------------------------------------------------------------------------*/
/* Instruction Decoders */

static void DecodeFixed(Word Code)
{
  if (!ChkArgCnt(0, 0));
  else if (OpSize != eSymbolSizeUnknown) WrError(ErrNum_UseLessAttr);
  else if (strcmp(Format, " ")) WrError(ErrNum_InvFormat);
  else
  {
    CodeLen = 0;
    if (Hi(Code) != 0)
      BAsmCode[CodeLen++] = Hi(Code);
    BAsmCode[CodeLen++] = Lo(Code);
  }
}

static void DecodeMOV(Word Dummy)
{
  UNUSED(Dummy);

  if (!ChkArgCnt(2, 2));
  else if (CheckFormat("GEIFLS"))
  {
    if (OpSize == eSymbolSizeUnknown)
      SetOpSize((FormatCode == 2) ? eSymbolSize8Bit : eSymbolSize16Bit);
    if ((OpSize != eSymbolSize8Bit) && (OpSize != eSymbolSize16Bit)) WrError(ErrNum_InvOpSize);
    else
    {
      DecodeAdr(&ArgStr[2], MModNoImm);
      if (AdrMode != ModNone)
      {
        CopyAdr();
        DecodeAdr(&ArgStr[1], MModAll);
        if (AdrMode != ModNone)
        {
          if (FormatCode == 0)
          {
            if ((AdrMode == ModImm) && (Adr2Mode == ModReg)) FormatCode = 2 + OpSize;
            else if ((AdrMode == ModReg) && (Adr2Byte == 0xe6)) FormatCode = 4;
            else if ((Adr2Mode == ModReg) && (AdrByte == 0xe6)) FormatCode = 4;
            else if ((AdrMode == ModReg) && (Adr2Mode == ModAbs8)) FormatCode = 6;
            else if ((AdrMode == ModAbs8) && (Adr2Mode == ModReg)) FormatCode = 5;
            else FormatCode = 1;
          }
          switch (FormatCode)
          {
            case 1:
              if ((AdrMode == ModReg) && (Adr2Mode == ModReg))
              {
                BAsmCode[0] = AdrByte | (OpSize << 3);
                BAsmCode[1] = 0x80 | (Adr2Byte & 7);
                CodeLen = 2;
              }
              else if (AdrMode == ModReg)
              {
                BAsmCode[0] = Adr2Byte | (OpSize << 3);
                memcpy(BAsmCode + 1, Adr2Vals, Adr2Cnt);
                BAsmCode[1 + Adr2Cnt] = 0x90 | (AdrByte & 7);
                CodeLen = 2 + Adr2Cnt;
              }
              else if (Adr2Mode == ModReg)
              {
                BAsmCode[0] = AdrByte | (OpSize << 3);
                memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                BAsmCode[1 + AdrCnt] = 0x80 | (Adr2Byte & 7);
                CodeLen = 2 + AdrCnt;
              }
              else if (AdrMode == ModImm)
              {
                BAsmCode[0] = Adr2Byte | (OpSize << 3);
                memcpy(BAsmCode + 1, Adr2Vals, Adr2Cnt);
                if ((OpSize == eSymbolSize8Bit) || ((ImmVal() >= -128) && (ImmVal() < 127)))
                {
                  BAsmCode[1 + Adr2Cnt] = 0x06;
                  BAsmCode[2 + Adr2Cnt] = AdrVals[OpSize];
                  CodeLen = 3 + Adr2Cnt;
                }
                else
                {
                  BAsmCode[1 + Adr2Cnt] = 0x07;
                  memcpy(BAsmCode + 2 + Adr2Cnt, AdrVals, AdrCnt);
                  CodeLen=2 + Adr2Cnt + AdrCnt;
                }
              }
              else WrError(ErrNum_InvAddrMode);
              break;
            case 2:
              if ((AdrMode != ModImm) || (Adr2Mode != ModReg)) WrError(ErrNum_InvAddrMode);
              else if (OpSize != eSymbolSize8Bit) WrError(ErrNum_InvOpSize);
              else
              {
                BAsmCode[0] = 0x50 | (Adr2Byte & 7);
                memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                CodeLen = 1 + AdrCnt;
              }
              break;
            case 3:
              if ((AdrMode != ModImm) || (Adr2Mode != ModReg)) WrError(ErrNum_InvAddrMode);
              else if (OpSize != eSymbolSize16Bit) WrError(ErrNum_InvOpSize);
              else
              {
                BAsmCode[0] = 0x58 | (Adr2Byte & 7);
                memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                CodeLen = 1 + AdrCnt;
              }
              break;
            case 4:
              if ((AdrMode == ModReg) && (Adr2Byte == 0xe6))
              {
                BAsmCode[0] = 0x90 | (OpSize << 3) | (AdrByte & 7);
                memcpy(BAsmCode + 1, Adr2Vals, Adr2Cnt);
                CodeLen =1 + Adr2Cnt;
              }
              else if ((Adr2Mode == ModReg) && (AdrByte == 0xe6))
              {
                BAsmCode[0] = 0x80 | (OpSize << 3) | (Adr2Byte & 7);
                memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                CodeLen = 1 + AdrCnt;
              }
              else WrError(ErrNum_InvAddrMode);
              break;
            case 5:
              if ((AdrMode != ModAbs8) || (Adr2Mode != ModReg)) WrError(ErrNum_InvAddrMode);
              else
              {
                BAsmCode[0] = 0x60 | (OpSize << 3) | (Adr2Byte & 7);
                memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                CodeLen = 1 + AdrCnt;
              }
              break;
            case 6:
              if ((Adr2Mode != ModAbs8) || (AdrMode != ModReg)) WrError(ErrNum_InvAddrMode);
              else
              {
                BAsmCode[0] = 0x70 | (OpSize << 3) | (AdrByte & 7);
                memcpy(BAsmCode + 1, Adr2Vals, Adr2Cnt);
                CodeLen = 1 + Adr2Cnt;
              }
              break;
          }
        }
      }
    }
  }
}

static void DecodeLDC_STC(Word IsSTC_16)
{
  Byte HReg;
  int CRegIdx = 2, AdrIdx = 1;

  if (!ChkArgCnt(2, 2));
  else if (strcmp(Format," ")) WrError(ErrNum_InvFormat);
  else
  {
    if (IsSTC_16)
    {
      CRegIdx = 1;
      AdrIdx = 2;
    }
    if (!DecodeCReg(ArgStr[CRegIdx].Str, &HReg)) WrStrErrorPos(ErrNum_InvCtrlReg, &ArgStr[2]);
    else
    {
      DecodeAdr(&ArgStr[AdrIdx], IsSTC_16 ? MModNoImm : MModAll);
      if (AdrMode != ModNone)
      {
        BAsmCode[0] = AdrByte | (OpSize << 3);
        memcpy(BAsmCode + 1, AdrVals, AdrCnt);
        BAsmCode[1 + AdrCnt] = 0x88 | IsSTC_16 | HReg;
        CodeLen = 2 + AdrCnt;
      }
    }
  }
}

static void DecodeLDM(Word Dummy)
{
  UNUSED(Dummy);

  if (OpSize == eSymbolSizeUnknown) OpSize = eSymbolSize16Bit;
  if (!ChkArgCnt(2, 2));
  else if (OpSize != eSymbolSize16Bit) WrError(ErrNum_InvOpSize);
  else if (strcmp(Format, " ")) WrError(ErrNum_InvFormat);
  else if (!DecodeRegList(ArgStr[2].Str, BAsmCode + 1)) WrError(ErrNum_InvRegList);
  else
  {
    DecodeAdr(&ArgStr[1], MModPostInc);
    if (AdrMode != ModNone)
    {
      if ((AdrByte & 7) != 7) WrError(ErrNum_InvAddrMode);
      else
      {
        BAsmCode[0] = 0x02; CodeLen = 2;
      }
    }
  }
}

static void DecodeSTM(Word Dummy)
{
  UNUSED(Dummy);

  if (OpSize == eSymbolSizeUnknown) OpSize = eSymbolSize16Bit;
  if (!ChkArgCnt(2, 2));
  else if (OpSize != eSymbolSize16Bit) WrError(ErrNum_InvOpSize);
  else if (strcmp(Format, " ")) WrError(ErrNum_InvFormat);
  else if (!DecodeRegList(ArgStr[1].Str, BAsmCode + 1)) WrError(ErrNum_InvRegList);
  else
  {
    DecodeAdr(&ArgStr[2], MModPredec);
    if (AdrMode != ModNone)
    {
      if ((AdrByte & 7) != 7) WrError(ErrNum_InvAddrMode);
      else
      {
        BAsmCode[0] = 0x12; CodeLen = 2;
      }
    }
  }
}

static void DecodeMOVTPE_MOVFPE(Word IsMOVTPE_16)
{
  Byte HReg;
  int RegIdx = 2, AdrIdx = 1;

  if (ChkArgCnt(2, 2)
   && CheckFormat("G"))
  {
    if (IsMOVTPE_16)
    {
      RegIdx = 1;
      AdrIdx = 2;
    }
    if (OpSize == eSymbolSizeUnknown) SetOpSize(eSymbolSize8Bit);
    if (OpSize != eSymbolSize8Bit) WrError(ErrNum_InvOpSize);
    else if (!DecodeReg(ArgStr[RegIdx].Str, &HReg)) WrError(ErrNum_InvAddrMode);
    else
    {
      DecodeAdr(&ArgStr[AdrIdx], MModNoImm & (~MModReg));
      if (AdrMode != ModNone)
      {
        BAsmCode[0] = AdrByte | (OpSize << 3);
        memcpy(BAsmCode + 1, AdrVals, AdrCnt);
        BAsmCode[1 + AdrCnt] = 0;
        BAsmCode[2 + AdrCnt] = 0x80 | HReg | IsMOVTPE_16;
        CodeLen =3 + AdrCnt;
      }
    }
  }
}

static void DecodeADD_SUB(Word IsSUB_16)
{
  LongInt AdrLong;

  if (ChkArgCnt(2, 2)
   && CheckFormat("GQ"))
  {
    if (OpSize == eSymbolSizeUnknown) SetOpSize(eSymbolSize16Bit);
    if ((OpSize != eSymbolSize8Bit) && (OpSize != eSymbolSize16Bit)) WrError(ErrNum_InvOpSize);
    else
    {
      DecodeAdr(&ArgStr[2], MModNoImm);
      if (AdrMode != ModNone)
      {
        CopyAdr();
        DecodeAdr(&ArgStr[1], MModAll);
        if (AdrMode != ModNone)
        {
          AdrLong = ImmVal();
          if (FormatCode == 0)
          {
            if ((AdrMode == ModImm) && (abs(AdrLong) >= 1) && (abs(AdrLong) <= 2)) FormatCode = 2;
            else FormatCode = 1;
          }
          switch (FormatCode)
          {
            case 1:
              if (Adr2Mode != ModReg) WrError(ErrNum_InvAddrMode);
              else
              {
                BAsmCode[0] = AdrByte | (OpSize << 3);
                memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                BAsmCode[1 + AdrCnt] = 0x20 | IsSUB_16 | (Adr2Byte & 7);
                CodeLen = 2 + AdrCnt;
              }
              break;
            case 2:
              if (ChkRange(AdrLong, -2, 2))
              {
                if (AdrLong == 0) WrError(ErrNum_UnderRange);
                else
                {
                  if (IsSUB_16) AdrLong = (-AdrLong);
                  BAsmCode[0] = Adr2Byte | (OpSize << 3);
                  memcpy(BAsmCode + 1, Adr2Vals, Adr2Cnt);
                  BAsmCode[1 + Adr2Cnt] = 0x08 | (abs(AdrLong) - 1);
                  if (AdrLong < 0) BAsmCode[1 + Adr2Cnt] |= 4;
                  CodeLen = 2 + Adr2Cnt;
                }
              }
              break;
          }
        }
      }
    }
  }
}

static void DecodeCMP(Word Dummy)
{
  UNUSED(Dummy);

  if (ChkArgCnt(2, 2)
   && CheckFormat("GEI"))
  {
    if (OpSize == eSymbolSizeUnknown)
     SetOpSize((FormatCode == 2) ? eSymbolSize8Bit : eSymbolSize16Bit);
    if ((OpSize != 0) && (OpSize != 1)) WrError(ErrNum_InvOpSize);
    else
    {
      DecodeAdr(&ArgStr[2], MModNoImm);
      if (AdrMode != ModNone)
      {
        CopyAdr();
        DecodeAdr(&ArgStr[1], MModAll);
        if (AdrMode != ModNone)
        {
          if (FormatCode == 0)
          {
            if ((AdrMode == ModImm) && (Adr2Mode == ModReg)) FormatCode = 2 + OpSize;
            else FormatCode = 1;
          }
          switch (FormatCode)
          {
            case 1:
              if (Adr2Mode == ModReg)
              {
                BAsmCode[0] = AdrByte | (OpSize << 3);
                memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                BAsmCode[1 + AdrCnt] = 0x70 | (Adr2Byte & 7);
                CodeLen = 2 + AdrCnt;
              }
              else if (AdrMode == ModImm)
              {
                BAsmCode[0] = Adr2Byte | (OpSize << 3);
                memcpy(BAsmCode + 1,Adr2Vals, Adr2Cnt);
                BAsmCode[1 + Adr2Cnt] = 0x04 | OpSize;
                memcpy(BAsmCode + 2 + Adr2Cnt, AdrVals, AdrCnt);
                CodeLen = 2 + AdrCnt + Adr2Cnt;
              }
              else WrError(ErrNum_InvAddrMode);
              break;
            case 2:
              if ((AdrMode != ModImm) || (Adr2Mode != ModReg)) WrError(ErrNum_InvAddrMode);
              else if (OpSize != eSymbolSize8Bit) WrError(ErrNum_InvOpSize);
              else
              {
                BAsmCode[0] = 0x40 | (Adr2Byte & 7);
                memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                CodeLen = 1 + AdrCnt;
              }
              break;
             case 3:
               if ((AdrMode != ModImm) || (Adr2Mode != ModReg)) WrError(ErrNum_InvAddrMode);
               else if (OpSize != eSymbolSize16Bit) WrError(ErrNum_InvOpSize);
               else
               {
                 BAsmCode[0] = 0x48 + (Adr2Byte & 7);
                 memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                 CodeLen = 1 + AdrCnt;
               }
               break;
          }
        }
      }
    }
  }
}

static void DecodeRegEA(Word Index)
{
  Byte HReg;
  OneOrder *pOrder = RegEAOrders + Index;

  if (ChkArgCnt(2, 2)
   && CheckFormat("G"))
  {
    if (OpSize == eSymbolSizeUnknown) SetOpSize(pOrder->DefSize);
    if (!((1 << OpSize) & pOrder->SizeMask)) WrError(ErrNum_InvOpSize);
    else if (!DecodeReg(ArgStr[2].Str, &HReg)) WrError(ErrNum_InvAddrMode);
    else
    {
      DecodeAdr(&ArgStr[1], MModAll);
      if (AdrMode != ModNone)
      {
        BAsmCode[0] = AdrByte | (OpSize << 3);
        memcpy(BAsmCode + 1, AdrVals, AdrCnt);
        BAsmCode[1 + AdrCnt] = pOrder->Code | HReg;
        CodeLen = 2 + AdrCnt;
      }
    }
  }
}

static void DecodeTwoReg(Word Index)
{
  Byte HReg;
  OneOrder *pOrder = TwoRegOrders + Index;

  if (!ChkArgCnt(2, 2));
  else if (strcmp(Format," ")) WrError(ErrNum_InvFormat);
  else if (!DecodeReg(ArgStr[1].Str, &HReg)) WrError(ErrNum_InvAddrMode);
  else if (!DecodeReg(ArgStr[2].Str, &AdrByte)) WrError(ErrNum_InvAddrMode);
  else
  {
    if (OpSize == eSymbolSizeUnknown) SetOpSize(pOrder->DefSize);
    if (!((1 << OpSize) & pOrder->SizeMask)) WrError(ErrNum_InvOpSize);
    else
    {
      BAsmCode[0] = 0xa0 | HReg | (OpSize << 3);
      if (Hi(pOrder->Code))
      {
        BAsmCode[1] = Lo(pOrder->Code);
        BAsmCode[2] = Hi(pOrder->Code) | AdrByte;
        CodeLen = 3;
      }
      else
      {
        BAsmCode[1] = pOrder->Code | AdrByte;
        CodeLen = 2;
      }
    }
  }
}

static void DecodeLog(Word Code)
{
  Byte HReg;

  if (!ChkArgCnt(2, 2));
  else if (!DecodeCReg(ArgStr[2].Str, &HReg)) WrStrErrorPos(ErrNum_InvCtrlReg, &ArgStr[2]);
  else
  {
    DecodeAdr(&ArgStr[1], MModImm);
    if (AdrMode != ModNone)
    {
      BAsmCode[0] = AdrByte | (OpSize << 3);
      memcpy(BAsmCode + 1,AdrVals, AdrCnt);
      BAsmCode[1 + AdrCnt] = Code | HReg;
      CodeLen = 2 + AdrCnt;
    }
  }
}

static void DecodeOne(Word Index)
{
  OneOrder *pOrder = OneOrders + Index;

  if (!ChkArgCnt(1, 1));
  else if (CheckFormat("G"))
  {
    if (OpSize == eSymbolSizeUnknown) SetOpSize(pOrder->DefSize);
    if (!((1 << OpSize) & pOrder->SizeMask)) WrError(ErrNum_InvOpSize);
    else
    {
      DecodeAdr(&ArgStr[1], MModNoImm);
      if (AdrMode != ModNone)
      {
        BAsmCode[0] = AdrByte | (OpSize << 3);
        memcpy(BAsmCode+1, AdrVals, AdrCnt);
        BAsmCode[1 + AdrCnt] = pOrder->Code;
        CodeLen = 2 + AdrCnt;
      }
    }
  }
}

static void DecodeOneReg(Word Index)
{
  Byte HReg;
  OneOrder *pOrder = OneRegOrders + Index;

  if (!ChkArgCnt(1, 1));
  else if (strcmp(Format, " ")) WrError(ErrNum_InvFormat);
  else if (!DecodeReg(ArgStr[1].Str, &HReg)) WrError(ErrNum_InvAddrMode);
  else
  {
    if (OpSize == -1) SetOpSize(pOrder->DefSize);
    if (!((1 << OpSize) & pOrder->SizeMask)) WrError(ErrNum_InvOpSize);
    else
    {
      BAsmCode[0] = 0xa0 | HReg | (OpSize << 3);
      BAsmCode[1] = pOrder->Code;
      CodeLen = 2;
    }
  }
}

static void DecodeBit(Word Code)
{
  Boolean OK;
  Byte HReg;

  if (ChkArgCnt(2, 2))
  {
    if (OpSize == eSymbolSizeUnknown) OpSize = eSymbolSize8Bit;
    if ((OpSize != 0) && (OpSize != 1)) WrError(ErrNum_InvOpSize);
    else
    {
      DecodeAdr(&ArgStr[2], MModNoImm);
      if (AdrMode != ModNone)
      {
        if (DecodeReg(ArgStr[1].Str, &HReg))
        {
          OK = True; HReg += 8;
        }
        else
        {
          HReg = EvalStrIntExpressionOffs(&ArgStr[1], !!(*ArgStr[1].Str == '#'), (OpSize == eSymbolSize8Bit) ? UInt3 : UInt4, &OK);
          if (OK) HReg |= 0x80;
        }
        if (OK)
        {
          BAsmCode[0] = AdrByte | (OpSize << 3);
          memcpy(BAsmCode + 1, AdrVals, AdrCnt);
          BAsmCode[1 + AdrCnt] = Code | HReg;
          CodeLen=2 + AdrCnt;
        }
      }
    }
  }
}

static void DecodeRel(Word Code)
{
  Boolean OK;
  tSymbolFlags Flags;
  LongInt AdrLong;

  if (!ChkArgCnt(1, 1));
  else if (strcmp(Format," ")) WrError(ErrNum_InvFormat);
  else
  {
    AdrLong = EvalStrIntExpressionWithFlags(&ArgStr[1], UInt24, &OK, &Flags);
    if (OK)
    {
      if (!ChkSamePage(AdrLong, EProgCounter(), 16, Flags));
      else if ((EProgCounter() & 0xffff) >= 0xfffc) WrError(ErrNum_NotFromThisAddress);
      else
      {
        AdrLong -= EProgCounter() + 2;
        if (AdrLong > 0x7fff) AdrLong -= 0x10000;
        else if (AdrLong < -0x8000l) AdrLong += 0x10000;
        if (OpSize == eSymbolSizeUnknown)
        {
          if ((AdrLong <= 127) && (AdrLong >= -128)) OpSize = eSymbolSizeFloat32Bit;
          else OpSize = eSymbolSize32Bit;
        }
        switch (OpSize)
        {
          case eSymbolSize32Bit:
            AdrLong--;
            BAsmCode[0] = Code | 0x10;
            BAsmCode[1] = (AdrLong >> 8) & 0xff;
            BAsmCode[2] = AdrLong & 0xff;
            CodeLen = 3;
            break;
          case eSymbolSizeFloat32Bit:
            if (((AdrLong < -128) || (AdrLong > 127)) && !mSymbolQuestionable(Flags)) WrError(ErrNum_JmpDistTooBig);
            else
            {
              BAsmCode[0] = Code;
              BAsmCode[1] = AdrLong & 0xff;
              CodeLen = 2;
            }
            break;
          default:
           WrError(ErrNum_InvOpSize);
        }
      }
    }
  }
}

static void DecodeJMP_JSR(Word IsJSR_8)
{
  if (ChkArgCnt(1, 1)
   && CheckFormat("G"))
  {
    AbsBank = EProgCounter() >> 16;
    DecodeAdr(&ArgStr[1], MModIReg | MModReg | MModDisp8 | MModDisp16 | MModAbs16);
    switch (AdrMode)
    {
      case ModReg:
      case ModIReg:
        BAsmCode[0] = 0x11; BAsmCode[1] = 0xd0 | IsJSR_8 | (AdrByte & 7);
        CodeLen = 2;
        break;
      case ModDisp8:
      case ModDisp16:
        BAsmCode[0] = 0x11; BAsmCode[1] = AdrByte | IsJSR_8;
        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
        CodeLen = 2 + AdrCnt;
        break;
      case ModAbs16:
        BAsmCode[0] = 0x10 | IsJSR_8; memcpy(BAsmCode + 1, AdrVals, AdrCnt);
        CodeLen = 1 + AdrCnt;
        break;
    }
  }
}

static void DecodePJMP_PJSR(Word IsPJMP)
{
  if (!ChkArgCnt(1, 1));
  else if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else if (strcmp(Format," ")) WrError(ErrNum_InvFormat);
  else if (!Maximum) WrError(ErrNum_OnlyInMaxmode);
  else
  {
    tStrComp *pArg = &ArgStr[1];
    unsigned ArgOffs = !!(*pArg->Str == '@');
    Byte HReg;

    if (DecodeReg(pArg->Str + ArgOffs, &HReg))
    {
      BAsmCode[0] = 0x11; BAsmCode[1] = 0xc0 | ((1 - IsPJMP) << 3) | HReg;
      CodeLen = 2;
    }
    else
    {
      Boolean OK;
      LongInt AdrLong = EvalStrIntExpressionOffs(pArg, ArgOffs, UInt24, &OK);

      if (OK)
      {
        BAsmCode[0] = 0x03 | (IsPJMP << 4);
        BAsmCode[1] = (AdrLong >> 16) & 0xff;
        BAsmCode[2] = (AdrLong >> 8) & 0xff;
        BAsmCode[3] = AdrLong & 0xff;
        CodeLen = 4;
      }
    }
  }
}

static void DecodeSCB(Word Code)
{
  Byte HReg;
  LongInt AdrLong;
  Boolean OK;
  tSymbolFlags Flags;

  if (!ChkArgCnt(2, 2));
  else if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else if (strcmp(Format," ")) WrError(ErrNum_InvFormat);
  else if (!DecodeReg(ArgStr[1].Str, &HReg)) WrError(ErrNum_InvAddrMode);
  else
  {
    AdrLong = EvalStrIntExpressionWithFlags(&ArgStr[2], UInt24, &OK, &Flags);
    if (OK)
    {
      if (!ChkSamePage(AdrLong, EProgCounter(), 16, Flags));
      else if ((EProgCounter() & 0xffff) >= 0xfffc) WrError(ErrNum_NotFromThisAddress);
      else
      {
        AdrLong -= EProgCounter() + 3;
        if (!mSymbolQuestionable(Flags) && ((AdrLong > 127) || (AdrLong < -128))) WrError(ErrNum_JmpDistTooBig);
        else
        {
          BAsmCode[0] = Code;
          BAsmCode[1] = 0xb8 | HReg;
          BAsmCode[2] = AdrLong & 0xff;
          CodeLen = 3;
        }
      }
    }
  }
}

static void DecodePRTD_RTD(Word IsPRTD)
{
  tSymbolSize HSize;
  Integer AdrInt;

  if (!ChkArgCnt(1, 1));
  else if (strcmp(Format," ")) WrError(ErrNum_InvFormat);
  else if (*ArgStr[1].Str != '#') WrError(ErrNum_OnlyImmAddr);
  else
  {
    tStrComp Arg;
    Boolean OK;
    tSymbolFlags Flags;

    StrCompRefRight(&Arg, &ArgStr[1], 1);
    HSize = eSymbolSizeUnknown; SplitDisp(&Arg, &HSize);
    if (HSize != eSymbolSizeUnknown) SetOpSize(HSize);
    AdrInt = EvalStrIntExpressionWithFlags(&Arg, SInt16, &OK, &Flags);
    if (mFirstPassUnknown(Flags)) AdrInt &= 127;
    if (OK)
    {
      if (OpSize == eSymbolSizeUnknown)
      {
        if ((AdrInt < 127) && (AdrInt > -128)) OpSize = eSymbolSize8Bit;
        else OpSize = eSymbolSize16Bit;
      }
      if (IsPRTD) BAsmCode[0] = 0x11;
      switch (OpSize)
      {
        case eSymbolSize8Bit:
          if (ChkRange(AdrInt, -128, 127))
          {
            BAsmCode[IsPRTD] = 0x14;
            BAsmCode[1 + IsPRTD] = AdrInt & 0xff;
            CodeLen = 2 + IsPRTD;
          }
          break;
        case eSymbolSize16Bit:
          BAsmCode[IsPRTD] = 0x1c;
          BAsmCode[1 + IsPRTD] = (AdrInt >> 8) & 0xff;
          BAsmCode[2 + IsPRTD] = AdrInt & 0xff;
          CodeLen = 3 + IsPRTD;
          break;
        default:
          WrError(ErrNum_InvOpSize);
      }
    }
  }
}

static void DecodeLINK(Word Dummy)
{
  UNUSED(Dummy);

  if (!ChkArgCnt(2, 2));
  else if (strcmp(Format," ")) WrError(ErrNum_InvFormat);
  else
  {
    DecodeAdr(&ArgStr[1], MModReg);
    if (AdrMode != ModNone)
    {
      if ((AdrByte & 7) != 6) WrError(ErrNum_InvAddrMode);
      else if (*ArgStr[2].Str != '#') WrError(ErrNum_OnlyImmAddr);
      else
      {
        tStrComp Arg;
        tSymbolSize HSize;
        Integer AdrInt;
        Boolean OK;
        tSymbolFlags Flags;

        StrCompRefRight(&Arg, &ArgStr[2], 1);
        HSize = eSymbolSizeUnknown; SplitDisp(&Arg, &HSize);
        if (HSize != eSymbolSizeUnknown) SetOpSize(HSize);
        AdrInt = EvalStrIntExpressionWithFlags(&Arg, SInt16, &OK, &Flags);
        if (mFirstPassUnknown(Flags)) AdrInt &= 127;
        if (OK)
        {
          if (OpSize == eSymbolSizeUnknown)
          {
            if ((AdrInt < 127) && (AdrInt > -128)) OpSize = eSymbolSize8Bit;
            else OpSize = eSymbolSize16Bit;
          }
          switch (OpSize)
          {
            case eSymbolSize8Bit:
              if (ChkRange(AdrInt, -128, 127))
              {
                BAsmCode[0] = 0x17;
                BAsmCode[1] = AdrInt & 0xff;
                CodeLen = 2;
              }
              break;
            case eSymbolSize16Bit:
              BAsmCode[0] = 0x1f;
              BAsmCode[1] = (AdrInt >> 8) & 0xff;
              BAsmCode[2] = AdrInt & 0xff;
              CodeLen = 3;
              break;
            default:
              WrError(ErrNum_InvOpSize);
          }
        }
      }
    }
  }
}

static void DecodeUNLK(Word Dummy)
{
  UNUSED(Dummy);

  if (!ChkArgCnt(1, 1));
  else if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else if (strcmp(Format," ")) WrError(ErrNum_InvFormat);
  else
  {
    DecodeAdr(&ArgStr[1], MModReg);
    if (AdrMode != ModNone)
    {
      if ((AdrByte & 7) != 6) WrError(ErrNum_InvAddrMode);
      else
      {
        BAsmCode[0] = 0x0f; CodeLen = 1;
      }
    }
  }
}

static void DecodeTRAPA(Word Dummy)
{
  UNUSED(Dummy);

  if (!ChkArgCnt(1, 1));
  else if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else if (strcmp(Format," ")) WrError(ErrNum_InvFormat);
  else if (*ArgStr[1].Str != '#') WrError(ErrNum_OnlyImmAddr);
  else
  {
    Boolean OK;

    BAsmCode[1] = 0x10 | EvalStrIntExpressionOffs(&ArgStr[1], 1, UInt4, &OK);
    if (OK)
    {
      BAsmCode[0] = 0x08; CodeLen = 2;
    }
  }
}

/*-------------------------------------------------------------------------*/
/* dynamische Belegung/Freigabe Codetabellen */

static void AddFixed(const char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeFixed);
}

static void AddRel(const char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeRel);
}

static void AddOne(const char *NName, Word NCode, Byte NMask, tSymbolSize NDef)
{
  if (InstrZ>=OneOrderCount) exit(255);
  OneOrders[InstrZ].Code = NCode;
  OneOrders[InstrZ].SizeMask = NMask;
  OneOrders[InstrZ].DefSize = NDef;
  AddInstTable(InstTable, NName, InstrZ++, DecodeOne);
}

static void AddOneReg(const char *NName, Word NCode, Byte NMask, tSymbolSize NDef)
{
  if (InstrZ>=OneRegOrderCount) exit(255);
  OneRegOrders[InstrZ].Code=NCode;
  OneRegOrders[InstrZ].SizeMask = NMask;
  OneRegOrders[InstrZ].DefSize = NDef;
  AddInstTable(InstTable, NName, InstrZ++, DecodeOneReg);
}

static void AddRegEA(const char *NName, Word NCode, Byte NMask, tSymbolSize NDef)
{
  if (InstrZ >= RegEAOrderCount) exit(255);
  RegEAOrders[InstrZ].Code = NCode;
  RegEAOrders[InstrZ].SizeMask = NMask;
  RegEAOrders[InstrZ].DefSize = NDef;
  AddInstTable(InstTable, NName, InstrZ++, DecodeRegEA);
}

static void AddTwoReg(const char *NName, Word NCode, Byte NMask, tSymbolSize NDef)
{
  if (InstrZ >= TwoRegOrderCount) exit(255);
  TwoRegOrders[InstrZ].Code = NCode;
  TwoRegOrders[InstrZ].SizeMask = NMask;
  TwoRegOrders[InstrZ].DefSize = NDef;
  AddInstTable(InstTable, NName, InstrZ++, DecodeTwoReg);
}

static void AddLog(const char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeLog);
}

static void AddBit(const char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeBit);
}

static void InitFields(void)
{
  Format = (char*)malloc(sizeof(char) * STRINGSIZE);

  InstTable = CreateInstTable(302);

  AddFixed("NOP"  , 0x0000); AddFixed("PRTS"   , 0x1119);
  AddFixed("RTE"  , 0x000a); AddFixed("RTS"    , 0x0019);
  AddFixed("SLEEP", 0x001a); AddFixed("TRAP/VS", 0x0009);

  AddInstTable(InstTable, "MOV", 0, DecodeMOV);
  AddInstTable(InstTable, "LDC", 0, DecodeLDC_STC);
  AddInstTable(InstTable, "STC", 16, DecodeLDC_STC);
  AddInstTable(InstTable, "LDM", 0, DecodeLDM);
  AddInstTable(InstTable, "STM", 0, DecodeSTM);
  AddInstTable(InstTable, "MOVTPE", 16, DecodeMOVTPE_MOVFPE);
  AddInstTable(InstTable, "MOVFPE", 0, DecodeMOVTPE_MOVFPE);
  AddInstTable(InstTable, "ADD", 0, DecodeADD_SUB);
  AddInstTable(InstTable, "SUB", 16, DecodeADD_SUB);
  AddInstTable(InstTable, "CMP", 0, DecodeCMP);
  AddInstTable(InstTable, "JMP", 0, DecodeJMP_JSR);
  AddInstTable(InstTable, "JSR", 8, DecodeJMP_JSR);
  AddInstTable(InstTable, "PJMP", 1, DecodePJMP_PJSR);
  AddInstTable(InstTable, "PJSR", 0, DecodePJMP_PJSR);
  AddInstTable(InstTable, "SCB/F", 0x01, DecodeSCB);
  AddInstTable(InstTable, "SCB/NE", 0x06, DecodeSCB);
  AddInstTable(InstTable, "SCB/EQ", 0x07, DecodeSCB);
  AddInstTable(InstTable, "RTD", 0, DecodePRTD_RTD);
  AddInstTable(InstTable, "PRTD", 1, DecodePRTD_RTD);
  AddInstTable(InstTable, "LINK", 0, DecodeLINK);
  AddInstTable(InstTable, "UNLK", 0, DecodeUNLK);
  AddInstTable(InstTable, "TRAPA", 0, DecodeTRAPA);

  AddRel("BRA",0x20); AddRel("BT" ,0x20); AddRel("BRN",0x21);
  AddRel("BF" ,0x21); AddRel("BHI",0x22); AddRel("BLS",0x23);
  AddRel("BCC",0x24); AddRel("BHS",0x24); AddRel("BCS",0x25);
  AddRel("BLO",0x25); AddRel("BNE",0x26); AddRel("BEQ",0x27);
  AddRel("BVC",0x28); AddRel("BVS",0x29); AddRel("BPL",0x2a);
  AddRel("BMI",0x2b); AddRel("BGE",0x2c); AddRel("BLT",0x2d);
  AddRel("BGT",0x2e); AddRel("BLE",0x2f); AddRel("BSR",0x0e);

  InstrZ = 0; OneOrders = (OneOrder *) malloc(sizeof(OneOrder) * OneOrderCount);
  AddOne("CLR"  ,0x13,3,eSymbolSize16Bit); AddOne("NEG"  ,0x14,3,eSymbolSize16Bit);
  AddOne("NOT"  ,0x15,3,eSymbolSize16Bit); AddOne("ROTL" ,0x1c,3,eSymbolSize16Bit);
  AddOne("ROTR" ,0x1d,3,eSymbolSize16Bit); AddOne("ROTXL",0x1e,3,eSymbolSize16Bit);
  AddOne("ROTXR",0x1f,3,eSymbolSize16Bit); AddOne("SHAL" ,0x18,3,eSymbolSize16Bit);
  AddOne("SHAR" ,0x19,3,eSymbolSize16Bit); AddOne("SHLL" ,0x1a,3,eSymbolSize16Bit);
  AddOne("SHLR" ,0x1b,3,eSymbolSize16Bit); AddOne("TAS"  ,0x17,1,eSymbolSize8Bit);
  AddOne("TST"  ,0x16,3,eSymbolSize16Bit);

  InstrZ = 0; OneRegOrders = (OneOrder *) malloc(sizeof(OneOrder) * OneRegOrderCount);
  AddOneReg("EXTS",0x11,1,eSymbolSize8Bit); AddOneReg("EXTU",0x12,1,eSymbolSize8Bit);
  AddOneReg("SWAP",0x10,1,eSymbolSize8Bit);

  InstrZ = 0; RegEAOrders = (OneOrder *) malloc(sizeof(OneOrder) * RegEAOrderCount);
  AddRegEA("ADDS" ,0x28,3,eSymbolSize16Bit); AddRegEA("ADDX" ,0xa0,3,eSymbolSize16Bit);
  AddRegEA("AND"  ,0x50,3,eSymbolSize16Bit); AddRegEA("DIVXU",0xb8,3,eSymbolSize16Bit);
  AddRegEA("MULXU",0xa8,3,eSymbolSize16Bit); AddRegEA("OR"   ,0x40,3,eSymbolSize16Bit);
  AddRegEA("SUBS" ,0x38,3,eSymbolSize16Bit); AddRegEA("SUBX" ,0xb0,3,eSymbolSize16Bit);
  AddRegEA("XOR"  ,0x60,3,eSymbolSize16Bit);

  InstrZ = 0; TwoRegOrders = (OneOrder *) malloc(sizeof(OneOrder) * TwoRegOrderCount);
  AddTwoReg("DADD",0xa000,1,eSymbolSize8Bit); AddTwoReg("DSUB",0xb000,1,eSymbolSize8Bit);
  AddTwoReg("XCH" ,0x90,2,eSymbolSize16Bit);

  AddLog("ANDC",0x58); AddLog("ORC",0x48); AddLog("XORC",0x68);

  AddBit("BCLR",0x50); AddBit("BNOT",0x60);
  AddBit("BSET",0x40); AddBit("BTST",0x70);
}

static void DeinitFields(void)
{
  free(Format);
  free(OneOrders);
  free(OneRegOrders);
  free(RegEAOrders);
  free(TwoRegOrders);
  DestroyInstTable(InstTable);
}

static Boolean DecodeAttrPart_H8_5(void)
{
  char *p;

  /* Formatangabe abspalten */

  switch (AttrSplit)
  {
    case '.':
      p = strchr(AttrPart.Str, ':');
      if (p)
      {
        if (p < AttrPart.Str + strlen(AttrPart.Str) - 1)
          strmaxcpy(Format, p + 1, STRINGSIZE - 1);
        else
          strcpy(Format, " ");
        *p = '\0';
      }
      else
        strcpy(Format, " ");
      break;
    case ':':
      p = strchr(AttrPart.Str, '.');
      if (!p)
      {
        strmaxcpy(Format, AttrPart.Str, STRINGSIZE - 1);
        *AttrPart.Str = '\0';
      }
      else
      {
        *p = '\0';
        if (p == AttrPart.Str)
          strcpy(Format, " ");
        else
          strmaxcpy(Format, AttrPart.Str, STRINGSIZE - 1);
        strcpy(AttrPart.Str, p + 1);
      }
      break;
    default:
      strcpy(Format, " ");
  }

  NLS_UpString(Format);

  if (*AttrPart.Str)
  {
    if (!DecodeMoto16AttrSize(*AttrPart.Str, &AttrPartOpSize, False))
      return False;
  }
  return True;
}

static void MakeCode_H8_5(void)
{
  CodeLen = 0; DontPrint = False; AbsBank = Reg_DP;

  /* zu ignorierendes */

  if (Memo("")) return;

  OpSize = eSymbolSizeUnknown;
  if (*AttrPart.Str)
    SetOpSize(AttrPartOpSize);

  if (DecodeMoto16Pseudo(OpSize,True)) return;

  /* Sonderfaelle */

  if (!LookupInstTable(InstTable, OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
}

static Boolean ChkPC_H8_5(LargeWord Addr)
{
  if (ActPC == SegCode)
    return (Addr < (Maximum ? 0x1000000u : 0x10000u));
  else
    return False;
}

static Boolean IsDef_H8_5(void)
{
  return False;
}

static void SwitchFrom_H8_5(void)
{
  DeinitFields(); ClearONOFF();
}

static void InitCode_H8_5(void)
{
  Reg_DP = -1;
  Reg_EP = -1;
  Reg_TP = -1;
  Reg_BR = -1;
}

static void SwitchTo_H8_5(void)
{
  TurnWords = True; ConstMode = ConstModeMoto;

  PCSymbol = "*"; HeaderID = 0x69; NOPCode = 0x00;
  DivideChars = ","; HasAttrs = True; AttrChars = ".:";

  ValidSegs = 1 << SegCode;
  Grans[SegCode] = 1; ListGrans[SegCode] = 1; SegInits[SegCode] = 0;

  DecodeAttrPart = DecodeAttrPart_H8_5;
  MakeCode = MakeCode_H8_5;
  ChkPC = ChkPC_H8_5;
  IsDef = IsDef_H8_5;
  SwitchFrom = SwitchFrom_H8_5;
  InitFields();
  AddONOFF("MAXMODE", &Maximum, MaximumName, False);
  AddMoto16PseudoONOFF();

  pASSUMERecs = ASSUMEH8_5s;
  ASSUMERecCnt = ASSUMEH8_5Count;

  SetFlag(&DoPadding, DoPaddingName, False);
}

void codeh8_5_init(void)
{
  CPU532 = AddCPU("HD6475328", SwitchTo_H8_5);
  CPU534 = AddCPU("HD6475348", SwitchTo_H8_5);
  CPU536 = AddCPU("HD6475368", SwitchTo_H8_5);
  CPU538 = AddCPU("HD6475388", SwitchTo_H8_5);

  AddInitPassProc(InitCode_H8_5);
}
