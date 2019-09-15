/* code75k0.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator NEC 75K0                                                    */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "strutil.h"
#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"  
#include "codepseudo.h"
#include "intpseudo.h"
#include "codevars.h"
#include "errmsg.h"

#include "code75k0.h"

enum
{
  ModNone  = -1,
  ModReg4  = 0,
  ModReg8  = 1,
  ModImm  = 2 ,
  ModInd  = 3,
  ModAbs  = 4,
};

#define MModReg4 (1 << ModReg4)
#define MModReg8 (1 << ModReg8)
#define MModImm (1 << ModImm)
#define MModInd (1 << ModInd)
#define MModAbs (1 << ModAbs)

static LongInt MBSValue, MBEValue;
static Boolean MinOneIs0;
static CPUVar
   CPU75402, CPU75004, CPU75006, CPU75008,
   CPU75268, CPU75304, CPU75306, CPU75308,
   CPU75312, CPU75316, CPU75328, CPU75104,
   CPU75106, CPU75108, CPU75112, CPU75116,
   CPU75206, CPU75208, CPU75212, CPU75216,
   CPU75512, CPU75516;
static Word ROMEnd;

static ShortInt OpSize;
static Byte AdrPart;
static ShortInt AdrMode;

/*-------------------------------------------------------------------------*/
/* Untermengen von Befehlssatz abpruefen */

static void CheckCPU(CPUVar MinCPU)
{
  if (!ChkMinCPU(MinCPU))
    CodeLen = 0;
}

/*-------------------------------------------------------------------------*/
/* Adressausdruck parsen */

static Boolean SetOpSize(ShortInt NewSize)
{
  if (OpSize == -1)
    OpSize = NewSize;
  else if (NewSize != OpSize)
  {
    WrError(ErrNum_ConfOpSizes);
    return False;
  }
  return True;
}

static void ChkDataPage(Word Adr)
{
  switch (MBEValue)
  {
    case 0:
      if ((Adr > 0x7f) && (Adr < 0xf80))
        WrError(ErrNum_InAccPage);
      break;
    case 1:
      if (Hi(Adr) != MBSValue)
        WrError(ErrNum_InAccPage);
      break;
  }
}

static void CheckMBE(void)
{
  if ((MomCPU == CPU75402) && (MBEValue != 0))
  {
    MBEValue = 0;
    WrError(ErrNum_InvCtrlReg);
  }
}

static void DecodeAdr(const tStrComp *pArg, Byte Mask)
{
  static char *RegNames = "XAHLDEBC";

  char *p;
  int pos, ArgLen = strlen(pArg->Str);
  Boolean OK;
  String s;

  AdrMode = ModNone;

  /* Register ? */

  memcpy(s, pArg->Str, 2);
  s[2] = '\0';
  NLS_UpString(s);
  p = strstr(RegNames, s);

  if (p)
  {
    pos = p - RegNames;

    /* 8-Bit-Register ? */

    if (ArgLen == 1)
    {
      AdrPart = pos ^ 1;
      if (SetOpSize(0))
      {
        if ((AdrPart > 4) && !ChkMinCPUExt(CPU75004, ErrNum_AddrModeNotSupported));
        else
          AdrMode = ModReg4;
      }
      goto chk;
    }

    /* 16-Bit-Register ? */

    if ((ArgLen == 2) && (!Odd(pos)))
    {
      AdrPart = pos;
      if (SetOpSize(1))
      {
        if ((AdrPart > 2) && !ChkMinCPUExt(CPU75004, ErrNum_AddrModeNotSupported));
        else
          AdrMode = ModReg8;
      }
      goto chk;
    }

    /* 16-Bit-Schattenregister ? */

    if ((ArgLen == 3) && ((pArg->Str[2] == '\'') || (pArg->Str[2] == '`')) && (!Odd(pos)))
    {
      AdrPart = pos + 1;
      if (SetOpSize(1))
      {
        if (ChkMinCPUExt(CPU75104, ErrNum_AddrModeNotSupported))
          AdrMode = ModReg8;
      }
      goto chk;
    }
  }

  /* immediate? */

  if (*pArg->Str == '#')
  {
    if ((OpSize == -1) && (MinOneIs0))
      OpSize = 0;
    FirstPassUnknown = False;
    switch (OpSize)
    {
      case -1:
        WrError(ErrNum_UndefOpSizes);
        OK = False;
        break;
      case 0:
        AdrPart = EvalStrIntExpressionOffs(pArg, 1, Int4, &OK) & 15;
        break;
      case 1:
        AdrPart = EvalStrIntExpressionOffs(pArg, 1, Int8, &OK);
        break;
    }
    if (OK)
      AdrMode = ModImm;
    goto chk;
  }

  /* indirekt ? */

  if (*pArg->Str == '@')
  {
    tStrComp Arg;

    StrCompRefRight(&Arg, pArg, 1);
    if (!strcasecmp(Arg.Str, "HL")) AdrPart = 1;
    else if (!strcasecmp(Arg.Str, "HL+")) AdrPart = 2;
    else if (!strcasecmp(Arg.Str, "HL-")) AdrPart = 3;
    else if (!strcasecmp(Arg.Str, "DE")) AdrPart = 4;
    else if (!strcasecmp(Arg.Str, "DL")) AdrPart = 5;
    else
      AdrPart = 0;
    if (AdrPart != 0)
    {
      if ((AdrPart != 1) && !ChkMinCPUExt(CPU75004, ErrNum_AddrModeNotSupported));
      else if (((AdrPart == 2) || (AdrPart == 3)) && !ChkMinCPUExt(CPU75104, ErrNum_AddrModeNotSupported));
      else
        AdrMode = ModInd;
      goto chk;
    }
  }

  /* absolut */

  FirstPassUnknown = False;
  pos = EvalStrIntExpression(pArg, UInt12, &OK);
  if (OK)
  {
    AdrPart = Lo(pos);
    AdrMode = ModAbs;
    ChkSpace(SegData);
    if (!FirstPassUnknown)
      ChkDataPage(pos);
  }

chk:
  if ((AdrMode != ModNone) && (!(Mask & (1 << AdrMode))))
  {
    WrError(ErrNum_InvAddrMode);
    AdrMode = ModNone;
  }
}

/* Bit argument coding:

   aaaa01bbaaaaaaaa -> aaaaaaaaaaaa.bb
           11bbxxxx -> 0ffxh.bb
           10bbxxxx -> 0fbxh.bb
           00bbxxxx -> @h+x.bb
           0100xxxx -> 0fc0h+(x*4).@l
 */

/*!------------------------------------------------------------------------
 * \fn     DissectBit_75K0(char *pDest, int DestSize, LargeWord Inp)
 * \brief  dissect compact storage of bit (field) into readable form for listing
 * \param  pDest destination for ASCII representation
 * \param  DestSize destination buffer size
 * \param  Inp compact storage
 * ------------------------------------------------------------------------ */

static void DissectBit_75K0(char *pDest, int DestSize, LargeWord Inp)
{
  if (Hi(Inp))
    as_snprintf(pDest, DestSize, "%03.*u%s.%c",
                ListRadixBase, (unsigned)(((Inp >> 4) & 0xf00) + Lo(Inp)), GetIntelSuffix(ListRadixBase),
                '0' + (Hi(Inp) & 3));
  else switch ((Inp >> 6) & 3)
  {
    case 0:
      as_snprintf(pDest, DestSize, "@%c+%0.*u%s.%c",
                  HexStartCharacter + ('H' - 'A'),
                  ListRadixBase, (unsigned)(Inp & 0x0f), GetIntelSuffix(ListRadixBase),
                  '0' + ((Inp >> 4) & 3));
      break;
    case 1:
      as_snprintf(pDest, DestSize, "%03.*u%s.@%c",
                  ListRadixBase, (unsigned)(0xfc0 + ((Inp & 0x0f) << 2)), GetIntelSuffix(ListRadixBase),
                  HexStartCharacter + ('L' - 'A'));
      break;
    case 2:
      as_snprintf(pDest, DestSize, "%03.*u%s.%c",
                  ListRadixBase, (unsigned)(0xfb0 + (Inp & 15)), GetIntelSuffix(ListRadixBase),
                  '0' + ((Inp >> 4) & 3));
      break;
    case 3:
      as_snprintf(pDest, DestSize, "%03.*u%s.%c",
                  ListRadixBase, (unsigned)(0xff0 + (Inp & 15)), GetIntelSuffix(ListRadixBase),
                  '0' + ((Inp >> 4) & 3));
      break;
  }
}

static Boolean DecodeBitAddr(const tStrComp *pArg, Word *Erg)
{
  char *p;
  int Num;
  Boolean OK;
  Word Adr;
  tStrComp AddrPart, BitPart;

  p = QuotPos(pArg->Str, '.');
  if (!p)
  {
    *Erg = EvalStrIntExpression(pArg, Int16, &OK);
    if (Hi(*Erg) != 0)
      ChkDataPage(((*Erg >> 4) & 0xf00) + Lo(*Erg));
    return OK;
  }

  StrCompSplitRef(&AddrPart, &BitPart, pArg, p);

  if (!strcasecmp(BitPart.Str, "@L"))
  {
    FirstPassUnknown = False;
    Adr = EvalStrIntExpression(&AddrPart, UInt12, &OK);
    if (FirstPassUnknown)
      Adr = (Adr & 0xffc) | 0xfc0;
    if (OK)
    {
      ChkSpace(SegData);
      if ((Adr & 3) != 0) WrError(ErrNum_NotAligned);
      else if (Adr < 0xfc0) WrError(ErrNum_UnderRange);
      else if (ChkMinCPUExt(CPU75004, ErrNum_AddrModeNotSupported))
      {
        *Erg = 0x40 + ((Adr & 0x3c) >> 2);
        return True;
      }
    }
  }
  else
  {
    Num = EvalStrIntExpression(&BitPart, UInt2, &OK);
    if (OK)
    {
      if (!strncasecmp(AddrPart.Str, "@H", 2))
      {
        Adr = EvalStrIntExpressionOffs(&AddrPart, 2, UInt4, &OK);
        if (OK)
        {
          if (ChkMinCPUExt(CPU75004, ErrNum_AddrModeNotSupported))
          {
            *Erg = (Num << 4) + Adr;
            return True;
          }
        }
      }
      else
      {
        FirstPassUnknown = False;
        Adr = EvalStrIntExpression(&AddrPart, UInt12, &OK);
        if (FirstPassUnknown)
          Adr = (Adr | 0xff0);
        if (OK)
        {
          ChkSpace(SegData);
          if ((Adr >= 0xfb0) && (Adr < 0xfc0))
            *Erg = 0x80 + (Num << 4) + (Adr & 15);
          else if (Adr >= 0xff0)
            *Erg = 0xc0 + (Num << 4) + (Adr & 15);
          else
            *Erg = 0x400 + (((Word)Num) << 8) + Lo(Adr) + (Hi(Adr) << 12);
          return True;
        }
      }
    }
  }
  return False;
}

static Boolean DecodeIntName(char *Asc, Byte *Erg)
{
  Word HErg;
  Byte LPart;
  String Asc_N;

  strmaxcpy(Asc_N, Asc, STRINGSIZE);
  NLS_UpString(Asc_N);
  Asc = Asc_N;

  if (MomCPU <= CPU75402)
    LPart = 0;
  else if (MomCPU < CPU75004)
    LPart = 1;
  else if (MomCPU < CPU75104)
    LPart = 2;
  else
    LPart = 3;
       if (!strcmp(Asc, "IEBT"))   HErg = 0x000;
  else if (!strcmp(Asc, "IEW"))    HErg = 0x102;
  else if (!strcmp(Asc, "IETPG"))  HErg = 0x203;
  else if (!strcmp(Asc, "IET0"))   HErg = 0x104;
  else if (!strcmp(Asc, "IECSI"))  HErg = 0x005;
  else if (!strcmp(Asc, "IECSIO")) HErg = 0x205;
  else if (!strcmp(Asc, "IE0"))    HErg = 0x006;
  else if (!strcmp(Asc, "IE2"))    HErg = 0x007;
  else if (!strcmp(Asc, "IE4"))    HErg = 0x120;
  else if (!strcmp(Asc, "IEKS"))   HErg = 0x123;
  else if (!strcmp(Asc, "IET1"))   HErg = 0x224;
  else if (!strcmp(Asc, "IE1"))    HErg = 0x126;
  else if (!strcmp(Asc, "IE3"))    HErg = 0x227;
  else
    HErg = 0xfff;
  if (HErg == 0xfff)
    return False;
  else if (Hi(HErg) > LPart)
    return False;
  else
  {
    *Erg = Lo(HErg);
    return True;
  }
}

static void PutCode(Word Code)
{
  BAsmCode[0] = Lo(Code);
  if (Hi(Code) == 0) CodeLen = 1;
  else
  {
    BAsmCode[1] = Hi(Code); CodeLen = 2;
  }
}

static Word PageNum(Word Inp)
{
  return ((Inp >> 12) & 15);
}

/*-------------------------------------------------------------------------*/
/* Instruction Decoders */

static void DecodeFixed(Word Code)
{
  if (ChkArgCnt(0, 0))
    PutCode(Code);
}

static void DecodeMOV(Word Code)
{
  Byte HReg;

  UNUSED(Code);

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(&ArgStr[1], MModReg4 | MModReg8 | MModInd | MModAbs);
    switch (AdrMode)
    {
      case ModReg4:
        HReg = AdrPart;
        DecodeAdr(&ArgStr[2], MModReg4 | MModInd | MModAbs | MModImm);
        switch (AdrMode)
        {
          case ModReg4:
            if (HReg == 0)
            {
              PutCode(0x7899 + (((Word)AdrPart) << 8)); CheckCPU(CPU75004);
            }
            else if (AdrPart == 0)
            {
              PutCode(0x7099 + (((Word)HReg) << 8)); CheckCPU(CPU75004);
            }
            else WrError(ErrNum_InvAddrMode);
            break;
          case ModInd:
            if (HReg != 0) WrError(ErrNum_InvAddrMode);
            else PutCode(0xe0 + AdrPart);
            break;
          case ModAbs:
            if (HReg != 0) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[0] = 0xa3; BAsmCode[1] = AdrPart; CodeLen = 2;
            }
            break;
          case ModImm:
            if (HReg == 0) PutCode(0x70 + AdrPart);
            else
            {
              PutCode(0x089a + (((Word)AdrPart) << 12) + (((Word)HReg) << 8));
              CheckCPU(CPU75004);
            }
            break;
        }
        break;
      case ModReg8:
        HReg = AdrPart;
        DecodeAdr(&ArgStr[2], MModReg8 | MModAbs | MModInd | MModImm);
        switch (AdrMode)
        {
          case ModReg8:
            if (HReg == 0)
            {
              PutCode(0x58aa + (((Word)AdrPart) << 8)); CheckCPU(CPU75004);
            }
            else if (AdrPart == 0)
            {
              PutCode(0x50aa + (((Word)HReg) << 8)); CheckCPU(CPU75004);
            }
            else WrError(ErrNum_InvAddrMode);
            break;
          case ModAbs:
            if (HReg != 0) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[0] = 0xa2; BAsmCode[1] = AdrPart; CodeLen = 2;
              if ((!FirstPassUnknown) && (Odd(AdrPart))) WrError(ErrNum_AddrNotAligned);
            }
            break;
          case ModInd:
            if ((HReg != 0) || (AdrPart != 1)) WrError(ErrNum_InvAddrMode);
            else
            {
              PutCode(0x18aa); CheckCPU(CPU75004);
            }
            break;
          case ModImm:
            if (Odd(HReg)) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[0] = 0x89 + HReg; BAsmCode[1] = AdrPart; CodeLen = 2;
            }
            break;
        }
        break;
      case ModInd:
        if (AdrPart != 1) WrError(ErrNum_InvAddrMode);
        else
        {
          DecodeAdr(&ArgStr[2], MModReg4 | MModReg8);
          switch (AdrMode)
          {
            case ModReg4:
              if (AdrPart != 0) WrError(ErrNum_InvAddrMode);
              else
              {
                PutCode(0xe8); CheckCPU(CPU75004);
              }
              break;
            case ModReg8:
              if (AdrPart != 0) WrError(ErrNum_InvAddrMode);
              else
              {
                PutCode(0x10aa); CheckCPU(CPU75004);
              }
              break;
          }
        }
        break;
      case ModAbs:
        HReg = AdrPart;
        DecodeAdr(&ArgStr[2], MModReg4 | MModReg8);
        switch (AdrMode)
        {
          case ModReg4:
            if (AdrPart != 0) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[0] = 0x93; BAsmCode[1] = HReg; CodeLen = 2;
            }
            break; 
          case ModReg8:
            if (AdrPart != 0) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[0] = 0x92; BAsmCode[1] = HReg; CodeLen = 2;
              if ((!FirstPassUnknown) && (Odd(HReg))) WrError(ErrNum_AddrNotAligned);
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

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(&ArgStr[1], MModReg4 | MModReg8 | MModAbs | MModInd);
    switch (AdrMode)
    {
      case ModReg4:
        HReg = AdrPart;
        DecodeAdr(&ArgStr[2], MModReg4 | MModAbs | MModInd);
        switch (AdrMode)
        {
          case ModReg4:
            if (HReg == 0) PutCode(0xd8 + AdrPart);
            else if (AdrPart == 0) PutCode(0xd8 + HReg);
            else WrError(ErrNum_InvAddrMode);
            break; 
          case ModAbs:
            if (HReg != 0) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[0] = 0xb3; BAsmCode[1] = AdrPart; CodeLen = 2;
            }
            break;
          case ModInd:
            if (HReg != 0) WrError(ErrNum_InvAddrMode);
            else PutCode(0xe8 + AdrPart);
            break;
        }
        break;
      case ModReg8:
        HReg = AdrPart;
        DecodeAdr(&ArgStr[2], MModReg8 | MModAbs | MModInd);
        switch (AdrMode)
        {
          case ModReg8:
            if (HReg == 0)
            {
              PutCode(0x40aa + (((Word)AdrPart) << 8)); CheckCPU(CPU75004);
            }
            else if (AdrPart == 0)
            {
              PutCode(0x40aa + (((Word)HReg) << 8)); CheckCPU(CPU75004);
            }
            else WrError(ErrNum_InvAddrMode);
            break;
          case ModAbs:
            if (HReg != 0) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[0] = 0xb2; BAsmCode[1] = AdrPart; CodeLen = 2;
              if ((FirstPassUnknown) && (Odd(AdrPart))) WrError(ErrNum_AddrNotAligned);
            }
            break;
          case ModInd:
            if ((AdrPart != 1) || (HReg != 0)) WrError(ErrNum_InvAddrMode);
            else
            {
              PutCode(0x11aa); CheckCPU(CPU75004);
            }
            break;
        }
        break;
      case ModAbs:
        HReg = AdrPart;
        DecodeAdr(&ArgStr[2], MModReg4 | MModReg8);
        switch (AdrMode)
        {
          case ModReg4:
            if (AdrPart != 0) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[0] = 0xb3; BAsmCode[1] = HReg; CodeLen = 2;
            }
            break;
          case ModReg8:
            if (AdrPart != 0) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[0] = 0xb2; BAsmCode[1] = HReg; CodeLen = 2;
              if ((FirstPassUnknown) && (Odd(HReg))) WrError(ErrNum_AddrNotAligned);
            }
            break;
        }
        break;
      case ModInd:
        HReg = AdrPart;
        DecodeAdr(&ArgStr[2], MModReg4 | MModReg8);
        switch (AdrMode)
        { 
          case ModReg4:
            if (AdrPart != 0) WrError(ErrNum_InvAddrMode);
            else PutCode(0xe8 + HReg);
            break;
          case ModReg8:
            if ((AdrPart != 0) || (HReg != 1)) WrError(ErrNum_InvAddrMode);
            else
            {
              PutCode(0x11aa);
              CheckCPU(CPU75004);
            }
            break;
        }
        break;
    }
  }
}

static void DecodeMOVT(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else if (strcasecmp(ArgStr[1].Str, "XA")) WrError(ErrNum_InvAddrMode);
  else if (!strcasecmp(ArgStr[2].Str, "@PCDE"))
  {
    PutCode(0xd4);
    CheckCPU(CPU75004);
  }
  else if (!strcasecmp(ArgStr[2].Str, "@PCXA"))
    PutCode(0xd0);
  else
    WrError(ErrNum_InvAddrMode);
}

static void DecodePUSH_POP(Word Code)
{
  if (!ChkArgCnt(1, 1));
  else if (!strcasecmp(ArgStr[1].Str, "BS"))
  {
    PutCode(0x0699 + (Code << 8)); CheckCPU(CPU75004);
  }
  else
  {
    DecodeAdr(&ArgStr[1], MModReg8);
    switch (AdrMode)
    {
      case ModReg8:
        if (Odd(AdrPart)) WrError(ErrNum_InvAddrMode);
        else PutCode(0x48 + Code + AdrPart);
        break;
    }
  }
}

static void DecodeIN_OUT(Word IsIN)
{
  if (ChkArgCnt(2, 2))
  {
    const tStrComp *pPortArg = IsIN ? &ArgStr[2] : &ArgStr[1],
                   *pRegArg = IsIN ? &ArgStr[1] : &ArgStr[2];

    if (strncasecmp(pPortArg->Str, "PORT", 4)) WrError(ErrNum_InvAddrMode);
    else
    {
      Boolean OK;

      BAsmCode[1] = 0xf0 + EvalStrIntExpressionOffs(pPortArg, 4, UInt4, &OK);
      if (OK)
      {
        DecodeAdr(pRegArg, MModReg8 | MModReg4);
        switch (AdrMode)
        {
          case ModReg4:
            if (AdrPart != 0) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[0] = 0x93 + (IsIN << 4); CodeLen = 2;
            }
            break;
          case ModReg8:
            if (AdrPart != 0) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[0] = 0x92 + (IsIN << 4); CodeLen = 2;
              CheckCPU(CPU75004);
            }
            break;
        }
      }
    }
  }
}

static void DecodeADDS(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(&ArgStr[1], MModReg4 | MModReg8);
    switch (AdrMode)
    {
      case ModReg4:
        if (AdrPart != 0) WrError(ErrNum_InvAddrMode);
        else
        {
          DecodeAdr(&ArgStr[2], MModImm | MModInd);
          switch (AdrMode)
          {
            case ModImm: 
              PutCode(0x60 + AdrPart); break;
            case ModInd:
              if (AdrPart == 1) PutCode(0xd2); else WrError(ErrNum_InvAddrMode);
              break;
          }
        }
        break;
      case ModReg8:
        if (AdrPart == 0)
        {
          DecodeAdr(&ArgStr[2], MModReg8 | MModImm);
          switch (AdrMode)
          {
            case ModReg8:
              PutCode(0xc8aa + (((Word)AdrPart) << 8));
              CheckCPU(CPU75104);
              break;
            case ModImm:
              BAsmCode[0] = 0xb9; BAsmCode[1] = AdrPart;
              CodeLen = 2;
              CheckCPU(CPU75104);
              break;
          }
        }
        else if (strcasecmp(ArgStr[2].Str, "XA")) WrError(ErrNum_InvAddrMode);
        else
        {
          PutCode(0xc0aa + (((Word)AdrPart) << 8));
          CheckCPU(CPU75104);
        }
        break;
    }
  }
}

static void DecodeAri(Word Code)
{
  if (ChkArgCnt(2, 2))
  {
   DecodeAdr(&ArgStr[1], MModReg4 | MModReg8);
    switch (AdrMode)
    {
      case ModReg4:
        if (AdrPart != 0) WrError(ErrNum_InvAddrMode);
        else
        {
          DecodeAdr(&ArgStr[2], MModInd);
          switch (AdrMode)
          {
            case ModInd:
              if (AdrPart == 1)
              {
                BAsmCode[0] = 0xa8;
                if (Code == 0) BAsmCode[0]++;
                if (Code == 2) BAsmCode[0] += 0x10;
                CodeLen = 1;
                if (Code != 0)
                  CheckCPU(CPU75004);
              }
              else
                WrError(ErrNum_InvAddrMode);
             break;
          }
        }
        break;
      case ModReg8:
        if (AdrPart == 0)
        {
          DecodeAdr(&ArgStr[2], MModReg8);
          switch (AdrMode)
          {
            case ModReg8:
              PutCode(0xc8aa + ((Code + 1) << 12) + (((Word)AdrPart) << 8));
              CheckCPU(CPU75104);
              break;
          }
        }
        else if (strcasecmp(ArgStr[2].Str, "XA")) WrError(ErrNum_InvAddrMode);
        else
        {
          PutCode(0xc0aa + ((Code + 1) << 12) + (((Word)AdrPart) << 8));
          CheckCPU(CPU75104);
        }
        break;
    }
  }
}

static void DecodeLog(Word Code)
{
  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(&ArgStr[1], MModReg4 | MModReg8);
    switch (AdrMode)
    {
      case ModReg4:
        if (AdrPart != 0) WrError(ErrNum_InvAddrMode);
        else
        {
          DecodeAdr(&ArgStr[2], MModImm | MModInd);
          switch (AdrMode)
          {
            case ModImm:
              PutCode(0x2099 + (((Word)AdrPart & 15) << 8) + ((Code + 1) << 12));
              CheckCPU(CPU75004);
              break;
            case ModInd:
              if (AdrPart == 1)
                PutCode(0x80 + ((Code + 1) << 4));
              else
                WrError(ErrNum_InvAddrMode);
              break;
          }
        }
        break;
      case ModReg8:
        if (AdrPart == 0)
        {
          DecodeAdr(&ArgStr[2], MModReg8);
          switch (AdrMode)
          {
            case ModReg8:
              PutCode(0x88aa + (((Word)AdrPart) << 8) + ((Code + 1) << 12));
              CheckCPU(CPU75104);
              break;
          }
        }
        else if (strcasecmp(ArgStr[2].Str, "XA")) WrError(ErrNum_InvAddrMode);
        else
        {
          PutCode(0x80aa + (((Word)AdrPart) << 8) + ((Code + 1) << 12));
          CheckCPU(CPU75104);
        }
        break;
    }
  }
}

static void DecodeLog1(Word Code)
{
  Word BVal;

  if (!ChkArgCnt(2, 2));
  else if (strcasecmp(ArgStr[1].Str, "CY")) WrError(ErrNum_InvAddrMode);
  else if (DecodeBitAddr(&ArgStr[2], &BVal))
  {
    if (Hi(BVal) != 0) WrError(ErrNum_InvAddrMode);
    else
    {
      BAsmCode[0] = 0xac + ((Code & 1) << 1) + ((Code & 2) << 3);
      BAsmCode[1] = BVal;
      CodeLen = 2;
    }
  }
}

static void DecodeINCS(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(1, 1))
  {
    DecodeAdr(&ArgStr[1], MModReg4 | MModReg8 | MModInd | MModAbs);
    switch (AdrMode)
    {
      case ModReg4:
        PutCode(0xc0 + AdrPart);
        break;
      case ModReg8:
        if ((AdrPart < 1) || (Odd(AdrPart))) WrError(ErrNum_InvAddrMode);
        else
        {
          PutCode(0x88 + AdrPart);
          CheckCPU(CPU75104);
        }
        break;
      case ModInd:
        if (AdrPart == 1)
        {
          PutCode(0x0299);
          CheckCPU(CPU75004);
        }
        else
          WrError(ErrNum_InvAddrMode);
        break;
      case ModAbs:
        BAsmCode[0] = 0x82;
        BAsmCode[1] = AdrPart;
        CodeLen = 2;
        break;
    }
  }
}

static void DecodeDECS(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(1, 1))
  {
    DecodeAdr(&ArgStr[1], MModReg4 | MModReg8);
    switch (AdrMode)
    {
      case ModReg4:
        PutCode(0xc8 + AdrPart);
        break;
      case ModReg8:
        PutCode(0x68aa + (((Word)AdrPart) << 8));
        CheckCPU(CPU75104);
        break;
    }
  }
}

static void DecodeSKE(Word Code)
{
  Byte HReg;

  UNUSED(Code);

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(&ArgStr[1], MModReg4 | MModReg8 | MModInd);
    switch (AdrMode)
    {
      case ModReg4:
        HReg = AdrPart;
        DecodeAdr(&ArgStr[2], MModImm | MModInd | MModReg4);
        switch (AdrMode)
        {
          case ModReg4:
            if (HReg == 0)
            {
              PutCode(0x0899 + (((Word)AdrPart) << 8));
              CheckCPU(CPU75004);
            }
            else if (AdrPart == 0)
            {
              PutCode(0x0899 + (((Word)HReg) << 8));
              CheckCPU(CPU75004);
            }
            else WrError(ErrNum_InvAddrMode);
            break;
          case ModImm:
            BAsmCode[0] = 0x9a;
            BAsmCode[1] = (AdrPart << 4) + HReg;
            CodeLen = 2;
            break;
          case ModInd:
            if ((AdrPart == 1) && (HReg == 0))
              PutCode(0x80);
            else
              WrError(ErrNum_InvAddrMode);
            break;
        }
        break;
      case ModReg8:
        HReg = AdrPart;
        DecodeAdr(&ArgStr[2], MModInd | MModReg8);
        switch (AdrMode)
        {
          case ModReg8:
            if (HReg == 0)
            {
              PutCode(0x48aa + (((Word)AdrPart) << 8));
              CheckCPU(CPU75104);
            }
            else if (AdrPart == 0)
            {
              PutCode(0x48aa + (((Word)HReg) << 8));
              CheckCPU(CPU75104);
            }
            else WrError(ErrNum_InvAddrMode);
            break;
          case ModInd:
            if (AdrPart == 1)
            {
              PutCode(0x19aa);
              CheckCPU(CPU75104);
            }
            else
              WrError(ErrNum_InvAddrMode);
            break;
        }
        break;
      case ModInd:
        if (AdrPart != 1) WrError(ErrNum_InvAddrMode);
        else
        {
          MinOneIs0 = True;
          DecodeAdr(&ArgStr[2], MModImm | MModReg4 | MModReg8);
          switch (AdrMode)
          {
            case ModImm:
              PutCode(0x6099 + (((Word)AdrPart) << 8));
              CheckCPU(CPU75004);
              break;
            case ModReg4:
              if (AdrPart == 0)
                PutCode(0x80);
              else
                WrError(ErrNum_InvAddrMode);
              break;
            case ModReg8:
              if (AdrPart == 0)
              {
                PutCode(0x19aa);
                CheckCPU(CPU75004);
              }
              else
                WrError(ErrNum_InvAddrMode);
              break;
          }
        }
        break;
    }
  }
}

static void DecodeAcc(Word Code)
{
  if (!ChkArgCnt(1, 1));
  else if (strcasecmp(ArgStr[1].Str, "A")) WrError(ErrNum_InvAddrMode);
  else 
    PutCode(Code);
}

static void DecodeMOV1(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(2, 2))
  {
    Boolean OK = True;
    Word BVal;

    if (!strcasecmp(ArgStr[1].Str, "CY"))
      Code = 0xbd;
    else if (!strcasecmp(ArgStr[2].Str, "CY"))
      Code = 0x9b;
    else OK = False;
    if (!OK) WrError(ErrNum_InvAddrMode);
    else if (DecodeBitAddr(&ArgStr[((Code >> 2) & 3) - 1], &BVal))
    {
      if (Hi(BVal) != 0) WrError(ErrNum_InvAddrMode);
      else
      {
        BAsmCode[0] = Code;
        BAsmCode[1] = BVal;
        CodeLen = 2;
        CheckCPU(CPU75104);
      }
    }
  }
}

static void DecodeSET1_CLR1(Word Code)
{
  Word BVal;

  if (!ChkArgCnt(1, 1));
  else if (!strcasecmp(ArgStr[1].Str, "CY"))
    PutCode(0xe6 + Code);
  else if (DecodeBitAddr(&ArgStr[1], &BVal))
  {
    if (Hi(BVal) != 0)
    {
      BAsmCode[0] = 0x84 + Code + (Hi(BVal & 0x300) << 4);
      BAsmCode[1] = Lo(BVal);
      CodeLen = 2;
    }
    else
    {
      BAsmCode[0] = 0x9c + Code;
      BAsmCode[1] = BVal;
      CodeLen = 2;
    }
  }
}

static void DecodeSKT_SKF(Word Code)
{
  Word BVal;

  if (!ChkArgCnt(1, 1));
  else if (!strcasecmp(ArgStr[1].Str, "CY"))
  {
    if (Code)
      PutCode(0xd7);
    else
      WrError(ErrNum_InvAddrMode);
  }
  else if (DecodeBitAddr(&ArgStr[1], &BVal))
  {
    if (Hi(BVal) != 0)
    {
      BAsmCode[0] = 0x86 + Code + (Hi(BVal & 0x300) << 4);
      BAsmCode[1] = Lo(BVal);
      CodeLen = 2;
    }
    else
    {
      BAsmCode[0] = 0xbe + Code;
      BAsmCode[1] = BVal;
      CodeLen = 2;
    }
  }
}

static void DecodeNOT1(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(1, 1));
  else if (strcasecmp(ArgStr[1].Str, "CY")) WrError(ErrNum_InvAddrMode);
  else
    PutCode(0xd6);
}

static void DecodeSKTCLR(Word Code)
{
  Word BVal;

  UNUSED(Code);

  if (!ChkArgCnt(1, 1));
  else if (DecodeBitAddr(&ArgStr[1], &BVal))
  {
    if (Hi(BVal) != 0) WrError(ErrNum_InvAddrMode);
    else
    {
      BAsmCode[0] = 0x9f;
      BAsmCode[1] = BVal;
      CodeLen = 2;
    }
  }
}

static void DecodeBR(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(1, 1));
  else if (!strcasecmp(ArgStr[1].Str, "PCDE"))
  {
    PutCode(0x0499);
    CheckCPU(CPU75004);
  }
  else if (!strcasecmp(ArgStr[1].Str, "PCXA"))
  {
    BAsmCode[0] = 0x99;
    BAsmCode[1] = 0x00;
    CodeLen = 2;
    CheckCPU(CPU75104);
  }
  else
  {
    Integer AdrInt, Dist;
    Boolean OK, BrRel, BrLong;
    unsigned Offset = 0;

    BrRel = False;
    BrLong = False;
    if (ArgStr[1].Str[Offset] == '$')
    {
      BrRel = True;
      Offset++;
    }
    else if (ArgStr[1].Str[Offset] == '!')
    {
      BrLong = True;
      Offset++;
    }
    AdrInt = EvalStrIntExpressionOffs(&ArgStr[1], Offset, UInt16, &OK);
    if (OK)
    {
      Dist = AdrInt-EProgCounter();
      if ((BrRel) || ((Dist <= 16) && (Dist >= -15) && (Dist != 0)))
      {
        if (Dist > 0)
        {
          Dist--;
          if ((Dist > 15) && (!SymbolQuestionable)) WrError(ErrNum_JmpDistTooBig);
          else
            PutCode(0x00 + Dist);
        }
        else
        {
          if ((Dist < -15) && (!SymbolQuestionable)) WrError(ErrNum_JmpDistTooBig);
          else
            PutCode(0xf0 + 15 + Dist);
        }
      }
      else if ((!BrLong) && (PageNum(AdrInt) == PageNum(EProgCounter())) && ((EProgCounter() & 0xfff) < 0xffe))
      {
        BAsmCode[0] = 0x50 + ((AdrInt >> 8) & 15);
        BAsmCode[1] = Lo(AdrInt);
        CodeLen = 2;
      }
      else
      {
        BAsmCode[0] = 0xab;
        BAsmCode[1] = Hi(AdrInt & 0x3fff);
        BAsmCode[2] = Lo(AdrInt);
        CodeLen = 3;
        CheckCPU(CPU75004);
      }
      ChkSpace(SegCode);
    }
  }
}

static void DecodeBRCB(Word Code)
{
  UNUSED(Code);  

  if (ChkArgCnt(1, 1))
  {
    Boolean OK;
    Integer AdrInt;

    FirstPassUnknown = False;
    AdrInt = EvalStrIntExpression(&ArgStr[1], UInt16, &OK);
    if (OK)
    {
      if (!ChkSamePage(AdrInt, EProgCounter(), 12));
      else if ((EProgCounter() & 0xfff) >= 0xffe) WrError(ErrNum_NotFromThisAddress);
      else
      {
        BAsmCode[0] = 0x50 + ((AdrInt >> 8) & 15);
        BAsmCode[1] = Lo(AdrInt);
        CodeLen = 2;
        ChkSpace(SegCode);
      }
    }
  }
}

static void DecodeCALL(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(1, 1))
  {
    Boolean OK;
    unsigned BrLong;
    Integer AdrInt;

    BrLong = !!(*ArgStr[1].Str == '!');
    FirstPassUnknown = False;
    AdrInt = EvalStrIntExpressionOffs(&ArgStr[1], BrLong, UInt16, &OK);
    if (FirstPassUnknown) AdrInt &= 0x7ff;
    if (OK)
    {
      if ((BrLong) || (AdrInt > 0x7ff))
      {
        BAsmCode[0] = 0xab;
        BAsmCode[1] = 0x40 + Hi(AdrInt & 0x3fff);
        BAsmCode[2] = Lo(AdrInt);
        CodeLen = 3;
        CheckCPU(CPU75004);
      }
      else
      {
        BAsmCode[0] = 0x40 + Hi(AdrInt & 0x7ff);
        BAsmCode[1] = Lo(AdrInt);
        CodeLen = 2;
      }
      ChkSpace(SegCode);
    }
  }
}

static void DecodeCALLF(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(1, 1))
  {
    Integer AdrInt;
    Boolean OK;

    AdrInt = EvalStrIntExpressionOffs(&ArgStr[1], !!(*ArgStr[1].Str == '!'), UInt11, &OK);
    if (OK)
    {
      BAsmCode[0] = 0x40 + Hi(AdrInt);
      BAsmCode[1] = Lo(AdrInt);
      CodeLen = 2;
      ChkSpace(SegCode);
    }
  }
}

static void DecodeGETI(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(1, 1))
  {
    Boolean OK;

    BAsmCode[0] = EvalStrIntExpression(&ArgStr[1], UInt6, &OK);
    CodeLen = Ord(OK);
    CheckCPU(CPU75004);
  }
}

static void DecodeEI_DI(Word Code)
{
  Byte HReg;

  if (ArgCnt == 0)
    PutCode(0xb29c + Code);
  else if (!ChkArgCnt(1, 1));
  else if (DecodeIntName(ArgStr[1].Str, &HReg))
    PutCode(0x989c + Code + (((Word)HReg) << 8));
  else
    WrError(ErrNum_InvCtrlReg);
}

static void DecodeSEL(Word Code)
{
  Boolean OK;

  UNUSED(Code);

  BAsmCode[0] = 0x99;
  if (!ChkArgCnt(1, 1));
  else if (!strncasecmp(ArgStr[1].Str, "RB", 2))
  {
    BAsmCode[1] = 0x20 + EvalStrIntExpressionOffs(&ArgStr[1], 2, UInt2, &OK);
    if (OK)
    {
      CodeLen = 2;
      CheckCPU(CPU75104);
    }
  }
  else if (!strncasecmp(ArgStr[1].Str, "MB", 2))
  {
    BAsmCode[1] = 0x10 + EvalStrIntExpressionOffs(&ArgStr[1], 2, UInt4, &OK);
    if (OK)
    {
      CodeLen = 2;
      CheckCPU(CPU75004);
    }
  }
  else
    WrError(ErrNum_InvAddrMode);
}

static void DecodeSFR(Word Code)
{
  UNUSED(Code);

  CodeEquate(SegData, 0, 0xfff);
}

static void DecodeBIT(Word Code)
{
  Word BErg;

  UNUSED(Code);

  if (ChkArgCnt(1, 1))
  {
    FirstPassUnknown = False;
    if (DecodeBitAddr(&ArgStr[1], &BErg))
      if (!FirstPassUnknown)
      {
        PushLocHandle(-1);
        EnterIntSymbol(&LabPart, BErg, SegBData, False);
        *ListLine = '=';
        DissectBit_75K0(ListLine + 1,  STRINGSIZE- 1, BErg);
        PopLocHandle();
      }
  }
}

/*-------------------------------------------------------------------------*/
/* dynamische Codetabellenverwaltung */

static void AddFixed(char *NewName, Word NewCode)
{
  AddInstTable(InstTable, NewName, NewCode, DecodeFixed);
}

static void InitFields(void)
{
  Boolean Err;

  ROMEnd = ConstLongInt(MomCPUName + 3, &Err, 10);
  if (ROMEnd > 2)
    ROMEnd %= 10;
  ROMEnd = (ROMEnd << 10) - 1;

  InstTable = CreateInstTable(103);
  AddInstTable(InstTable, "MOV", 0, DecodeMOV);
  AddInstTable(InstTable, "XCH", 0, DecodeXCH);
  AddInstTable(InstTable, "MOVT", 0, DecodeMOVT);
  AddInstTable(InstTable, "PUSH", 1, DecodePUSH_POP);
  AddInstTable(InstTable, "POP", 0, DecodePUSH_POP);
  AddInstTable(InstTable, "IN", 1, DecodeIN_OUT);
  AddInstTable(InstTable, "OUT", 0, DecodeIN_OUT);
  AddInstTable(InstTable, "ADDS", 0, DecodeADDS);
  AddInstTable(InstTable, "INCS", 0, DecodeINCS);
  AddInstTable(InstTable, "DECS", 0, DecodeDECS);
  AddInstTable(InstTable, "SKE", 0, DecodeSKE);
  AddInstTable(InstTable, "RORC", 0x98, DecodeAcc);
  AddInstTable(InstTable, "NOT", 0x5f99, DecodeAcc);
  AddInstTable(InstTable, "MOV1", 0, DecodeMOV1);
  AddInstTable(InstTable, "SET1", 1, DecodeSET1_CLR1);
  AddInstTable(InstTable, "CLR1", 0, DecodeSET1_CLR1);
  AddInstTable(InstTable, "SKT", 1, DecodeSKT_SKF);
  AddInstTable(InstTable, "SKF", 0, DecodeSKT_SKF);
  AddInstTable(InstTable, "NOT1", 0, DecodeNOT1);
  AddInstTable(InstTable, "SKTCLR", 0, DecodeSKTCLR);
  AddInstTable(InstTable, "BR", 0, DecodeBR);
  AddInstTable(InstTable, "BRCB", 0, DecodeBRCB);
  AddInstTable(InstTable, "CALL", 0, DecodeCALL);
  AddInstTable(InstTable, "CALLF", 0, DecodeCALLF);
  AddInstTable(InstTable, "GETI", 0, DecodeGETI);
  AddInstTable(InstTable, "EI", 1, DecodeEI_DI);  
  AddInstTable(InstTable, "DI", 0, DecodeEI_DI);
  AddInstTable(InstTable, "SEL", 0, DecodeSEL);
  AddInstTable(InstTable, "SFR", 0, DecodeSFR);
  AddInstTable(InstTable, "BIT", 0, DecodeBIT);

  AddFixed("RET" , 0x00ee);
  AddFixed("RETS", 0x00e0);
  AddFixed("RETI", 0x00ef);
  AddFixed("HALT", 0xa39d);
  AddFixed("STOP", 0xb39d);
  AddFixed("NOP" , 0x0060);

  InstrZ = 0;
  AddInstTable(InstTable, "ADDC", InstrZ++, DecodeAri);
  AddInstTable(InstTable, "SUBS", InstrZ++, DecodeAri);
  AddInstTable(InstTable, "SUBC", InstrZ++, DecodeAri);

  InstrZ = 0;
  AddInstTable(InstTable, "AND", InstrZ++, DecodeLog);
  AddInstTable(InstTable, "OR" , InstrZ++, DecodeLog);
  AddInstTable(InstTable, "XOR", InstrZ++, DecodeLog);

  InstrZ = 0;
  AddInstTable(InstTable, "AND1", InstrZ++, DecodeLog1);
  AddInstTable(InstTable, "OR1" , InstrZ++, DecodeLog1);
  AddInstTable(InstTable, "XOR1", InstrZ++, DecodeLog1);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*-------------------------------------------------------------------------*/

static void MakeCode_75K0(void)
{
  CodeLen = 0;
  DontPrint = False;
  OpSize = -1;
  MinOneIs0 = False;

  /* zu ignorierendes */

  if (Memo(""))
    return;

  /* Pseudoanweisungen */

  if (DecodeIntelPseudo(True))
    return;

  if (!LookupInstTable(InstTable, OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
}

static void InitCode_75K0(void)
{
  MBSValue = 0; MBEValue = 0;
}

static Boolean IsDef_75K0(void)
{
  return ((Memo("SFR")) || (Memo("BIT")));
}

static void SwitchFrom_75K0(void)
{
  DeinitFields();
}

static ASSUMERec ASSUME75s[] =
{
  {"MBS", &MBSValue, 0, 0x0f, 0x10, NULL},
  {"MBE", &MBEValue, 0, 0x01, 0x01, CheckMBE}
};
#define ASSUME75Count (sizeof(ASSUME75s) / sizeof(*ASSUME75s))

static void SwitchTo_75K0(void)
{
  TurnWords = False; ConstMode = ConstModeIntel;

  PCSymbol = "PC"; HeaderID = 0x7b; NOPCode = 0x60;
  DivideChars = ","; HasAttrs = False;

  ValidSegs = (1 << SegCode)|(1 << SegData);
  Grans[SegCode] = 1; ListGrans[SegCode] = 1; SegInits[SegCode] = 0;
  Grans[SegData] = 1; ListGrans[SegData] = 1; SegInits[SegData] = 0;
  SegLimits[SegData] = 0xfff;

  pASSUMERecs = ASSUME75s;
  ASSUMERecCnt = ASSUME75Count;

  MakeCode = MakeCode_75K0; IsDef = IsDef_75K0;
  SwitchFrom = SwitchFrom_75K0; InitFields();
  DissectBit = DissectBit_75K0;
  SegLimits[SegCode] = ROMEnd;
}

void code75k0_init(void) 
{
  CPU75402 = AddCPU("75402", SwitchTo_75K0);
  CPU75004 = AddCPU("75004", SwitchTo_75K0);
  CPU75006 = AddCPU("75006", SwitchTo_75K0);
  CPU75008 = AddCPU("75008", SwitchTo_75K0);
  CPU75268 = AddCPU("75268", SwitchTo_75K0);
  CPU75304 = AddCPU("75304", SwitchTo_75K0);
  CPU75306 = AddCPU("75306", SwitchTo_75K0);
  CPU75308 = AddCPU("75308", SwitchTo_75K0);
  CPU75312 = AddCPU("75312", SwitchTo_75K0);
  CPU75316 = AddCPU("75316", SwitchTo_75K0);
  CPU75328 = AddCPU("75328", SwitchTo_75K0);
  CPU75104 = AddCPU("75104", SwitchTo_75K0);
  CPU75106 = AddCPU("75106", SwitchTo_75K0);
  CPU75108 = AddCPU("75108", SwitchTo_75K0);
  CPU75112 = AddCPU("75112", SwitchTo_75K0);
  CPU75116 = AddCPU("75116", SwitchTo_75K0);
  CPU75206 = AddCPU("75206", SwitchTo_75K0);
  CPU75208 = AddCPU("75208", SwitchTo_75K0);
  CPU75212 = AddCPU("75212", SwitchTo_75K0);
  CPU75216 = AddCPU("75216", SwitchTo_75K0);
  CPU75512 = AddCPU("75512", SwitchTo_75K0);
  CPU75516 = AddCPU("75516", SwitchTo_75K0);

  AddInitPassProc(InitCode_75K0);
}
