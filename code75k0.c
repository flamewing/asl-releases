/* code75k0.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator NEC 75K0                                                    */
/*                                                                           */
/* Historie: 31.12.1996 Grundsteinlegung                                     */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: code75k0.c,v 1.12 2014/12/14 17:58:47 alfred Exp $                    */
/*****************************************************************************
 * $Log: code75k0.c,v $
 * Revision 1.12  2014/12/14 17:58:47  alfred
 * - remove static variables in strutil.c
 *
 * Revision 1.11  2014/11/16 13:15:07  alfred
 * - remove some superfluous semicolons
 *
 * Revision 1.10  2014/11/05 15:47:15  alfred
 * - replace InitPass callchain with registry
 *
 * Revision 1.9  2014/09/21 12:21:45  alfred
 * - compilable with Borland C again
 *
 * Revision 1.8  2014/09/09 17:07:26  alfred
 * - remove static string
 *
 * Revision 1.7  2014/09/08 20:36:21  alfred
 * - rework to current style
 *
 * Revision 1.6  2014/03/08 21:06:36  alfred
 * - rework ASSUME framework
 *
 * Revision 1.5  2010/04/17 13:14:22  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.4  2005/10/02 10:00:45  alfred
 * - ConstLongInt gets default base, correct length check on KCPSM3 registers
 *
 * Revision 1.3  2005/09/08 17:31:04  alfred
 * - add missing include
 *
 * Revision 1.2  2004/05/29 11:33:01  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 *****************************************************************************/

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
  if (MomCPU < MinCPU)
  {
    WrError(1500);
    CodeLen = 0;
  }
}

/*-------------------------------------------------------------------------*/
/* Adressausdruck parsen */

static Boolean SetOpSize(ShortInt NewSize)
{
  if (OpSize == -1)
    OpSize = NewSize;
  else if (NewSize != OpSize)
  {
    WrError(1131);
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
        WrError(110);
      break;
    case 1:
      if (Hi(Adr) != MBSValue)
        WrError(110);
      break;
  }
}

static void CheckMBE(void)
{
  if ((MomCPU == CPU75402) && (MBEValue != 0))
  {
    MBEValue = 0;
    WrError(1440);
  }
}

static void DecodeAdr(char *Asc, Byte Mask)
{
  static char *RegNames = "XAHLDEBC";

  char *p;
  int pos;
  Boolean OK;
  String s;

  AdrMode = ModNone;

  /* Register ? */

  memcpy(s, Asc, 2);
  s[2] = '\0';
  NLS_UpString(s);
  p = strstr(RegNames, s);

  if (p)
  {
    pos = p - RegNames;

    /* 8-Bit-Register ? */

    if (strlen(Asc) == 1)
    {
      AdrPart = pos ^ 1;
      if (SetOpSize(0))
      {
        if ((AdrPart > 4) && (MomCPU < CPU75004))
          WrError(1505);
        else
          AdrMode = ModReg4;
      }
      goto chk;
    }

    /* 16-Bit-Register ? */

    if ((strlen(Asc) == 2) && (!Odd(pos)))
    {
      AdrPart = pos;
      if (SetOpSize(1))
      {
        if ((AdrPart > 2) && (MomCPU < CPU75004))
          WrError(1505);
        else
          AdrMode = ModReg8;
      }
      goto chk;
    }

    /* 16-Bit-Schattenregister ? */

    if ((strlen(Asc) == 3) && ((Asc[2] == '\'') || (Asc[2] == '`')) && (!Odd(pos)))
    {
      AdrPart = pos + 1;
      if (SetOpSize(1))
      {
        if (MomCPU < CPU75104)
          WrError(1505);
        else
          AdrMode = ModReg8;
      }
      goto chk;
    }
  }

  /* immediate? */

  if (*Asc == '#')
  {
    if ((OpSize == -1) && (MinOneIs0))
      OpSize = 0;
    FirstPassUnknown = False;
    switch (OpSize)
    {
      case -1:
        WrError(1132);
        break;
      case 0:
        AdrPart = EvalIntExpression(Asc + 1, Int4, &OK) & 15;
        break;
      case 1:
        AdrPart = EvalIntExpression(Asc + 1, Int8, &OK);
        break;
    }
    if (OK)
      AdrMode = ModImm;
    goto chk;
  }

  /* indirekt ? */

  if (*Asc == '@')
  {
    strmaxcpy(s, Asc + 1, 255);
    if (!strcasecmp(s, "HL")) AdrPart = 1;
    else if (!strcasecmp(s, "HL+")) AdrPart = 2;
    else if (!strcasecmp(s, "HL-")) AdrPart = 3;
    else if (!strcasecmp(s, "DE")) AdrPart = 4;
    else if (!strcasecmp(s, "DL")) AdrPart = 5;
    else
      AdrPart = 0;
    if (AdrPart != 0)
    {
      if ((MomCPU < CPU75004) && (AdrPart != 1))
        WrError(1505);
      else if ((MomCPU < CPU75104) && ((AdrPart == 2) || (AdrPart == 3)))
        WrError(1505);
      else
        AdrMode = ModInd;
      goto chk;
    }
  }

  /* absolut */

  FirstPassUnknown = False;
  pos = EvalIntExpression(Asc, UInt12, &OK);
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
    WrError(1350);
    AdrMode = ModNone;
  }
}

static Boolean DecodeBitAddr(char *Asc, Word *Erg, char *pBName)
{
  char *p;
  int Num;
  Boolean OK;
  Word Adr;
  String bpart;

  p = QuotPos(Asc, '.');
  if (!p)
  {
    *Erg = EvalIntExpression(Asc, Int16, &OK);
    if (Hi(*Erg) != 0)
      ChkDataPage(((*Erg >> 4) & 0xf00) + Lo(*Erg));
    return OK;
  }

  *p = '\0';
  strmaxcpy(bpart, p + 1, 255);

  if (!strcasecmp(bpart, "@L"))
  {
    FirstPassUnknown = False;
    Adr = EvalIntExpression(Asc, UInt12, &OK);
    if (FirstPassUnknown)
      Adr = (Adr & 0xffc) | 0xfc0;
    if (OK)
    {
      ChkSpace(SegData);
      if ((Adr & 3) != 0) WrError(1325);
      else if (Adr < 0xfc0) WrError(1315);
      else if (MomCPU < CPU75004) WrError(1505);
      else
      {
        *Erg = 0x40 + ((Adr & 0x3c) >> 2);
        if (pBName)
        {
          HexString(pBName, STRINGSIZE, Adr, 3);
          strmaxcat(pBName, "H.@L", STRINGSIZE);
        }
        return True;
      }
    }
  }
  else
  {
    Num = EvalIntExpression(bpart, UInt2, &OK);
    if (OK)
    {
      if (!strncasecmp(Asc, "@H", 2))
      {
        Adr = EvalIntExpression(Asc + 2, UInt4, &OK);
        if (OK)
        {
          if (MomCPU < CPU75004) WrError(1505);
          else
          {
            *Erg = (Num << 4) + Adr;
            if (pBName)
            {
              char Str[30];

              HexString(Str, sizeof(Str), Adr,  1);
              sprintf(pBName, "@H%s.%c", Str, Num + '0');
            }
            return True;
          }
        }
      }
      else
      {
        FirstPassUnknown = False;
        Adr = EvalIntExpression(Asc, UInt12, &OK);
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
          if (pBName)
          {
            char Str[30];

            HexString(Str, sizeof(Str), Adr, 3);
            sprintf(pBName, "%sH.%c", Str, '0' + Num);
          }
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

  strmaxcpy(Asc_N, Asc, 255);
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

/*-------------------------------------------------------------------------*/
/* Instruction Decoders */

static void DecodeFixed(Word Code)
{
  if (ArgCnt != 0) WrError(1110);
  else
    PutCode(Code);
}

static void DecodeMOV(Word Code)
{
  Byte HReg;

  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg4 | MModReg8 | MModInd | MModAbs);
    switch (AdrMode)
    {
      case ModReg4:
        HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModReg4 | MModInd | MModAbs | MModImm);
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
            else WrError(1350);
            break;
          case ModInd:
            if (HReg != 0) WrError(1350);
            else PutCode(0xe0 + AdrPart);
            break;
          case ModAbs:
            if (HReg != 0) WrError(1350);
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
        DecodeAdr(ArgStr[2], MModReg8 | MModAbs | MModInd | MModImm);
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
            else WrError(1350);
            break;
          case ModAbs:
            if (HReg != 0) WrError(1350);
            else
            {
              BAsmCode[0] = 0xa2; BAsmCode[1] = AdrPart; CodeLen = 2;
              if ((!FirstPassUnknown) && (Odd(AdrPart))) WrError(180);
            }
            break;
          case ModInd:
            if ((HReg != 0) || (AdrPart != 1)) WrError(1350);
            else
            {
              PutCode(0x18aa); CheckCPU(CPU75004);
            }
            break;
          case ModImm:
            if (Odd(HReg)) WrError(1350);
            else
            {
              BAsmCode[0] = 0x89 + HReg; BAsmCode[1] = AdrPart; CodeLen = 2;
            }
            break;
        }
        break;
      case ModInd:
        if (AdrPart != 1) WrError(1350);
        else
        {
          DecodeAdr(ArgStr[2], MModReg4 | MModReg8);
          switch (AdrMode)
          {
            case ModReg4:
              if (AdrPart != 0) WrError(1350);
              else
              {
                PutCode(0xe8); CheckCPU(CPU75004);
              }
              break;
            case ModReg8:
              if (AdrPart != 0) WrError(1350);
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
        DecodeAdr(ArgStr[2], MModReg4 | MModReg8);
        switch (AdrMode)
        {
          case ModReg4:
            if (AdrPart != 0) WrError(1350);
            else
            {
              BAsmCode[0] = 0x93; BAsmCode[1] = HReg; CodeLen = 2;
            }
            break; 
          case ModReg8:
            if (AdrPart != 0) WrError(1350);
            else
            {
              BAsmCode[0] = 0x92; BAsmCode[1] = HReg; CodeLen = 2;
              if ((!FirstPassUnknown) && (Odd(HReg))) WrError(180);
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
    DecodeAdr(ArgStr[1], MModReg4 | MModReg8 | MModAbs | MModInd);
    switch (AdrMode)
    {
      case ModReg4:
        HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModReg4 | MModAbs | MModInd);
        switch (AdrMode)
        {
          case ModReg4:
            if (HReg == 0) PutCode(0xd8 + AdrPart);
            else if (AdrPart == 0) PutCode(0xd8 + HReg);
            else WrError(1350);
            break; 
          case ModAbs:
            if (HReg != 0) WrError(1350);
            else
            {
              BAsmCode[0] = 0xb3; BAsmCode[1] = AdrPart; CodeLen = 2;
            }
            break;
          case ModInd:
            if (HReg != 0) WrError(1350);
            else PutCode(0xe8 + AdrPart);
            break;
        }
        break;
      case ModReg8:
        HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModReg8 | MModAbs | MModInd);
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
            else WrError(1350);
            break;
          case ModAbs:
            if (HReg != 0) WrError(1350);
            else
            {
              BAsmCode[0] = 0xb2; BAsmCode[1] = AdrPart; CodeLen = 2;
              if ((FirstPassUnknown) && (Odd(AdrPart))) WrError(180);
            }
            break;
          case ModInd:
            if ((AdrPart != 1) || (HReg != 0)) WrError(1350);
            else
            {
              PutCode(0x11aa); CheckCPU(CPU75004);
            }
            break;
        }
        break;
      case ModAbs:
        HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModReg4 | MModReg8);
        switch (AdrMode)
        {
          case ModReg4:
            if (AdrPart != 0) WrError(1350);
            else
            {
              BAsmCode[0] = 0xb3; BAsmCode[1] = HReg; CodeLen = 2;
            }
            break;
          case ModReg8:
            if (AdrPart != 0) WrError(1350);
            else
            {
              BAsmCode[0] = 0xb2; BAsmCode[1] = HReg; CodeLen = 2;
              if ((FirstPassUnknown) && (Odd(HReg))) WrError(180);
            }
            break;
        }
        break;
      case ModInd:
        HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModReg4 | MModReg8);
        switch (AdrMode)
        { 
          case ModReg4:
            if (AdrPart != 0) WrError(1350);
            else PutCode(0xe8 + HReg);
            break;
          case ModReg8:
            if ((AdrPart != 0) || (HReg != 1)) WrError(1350);
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

  if (ArgCnt != 2) WrError(1110);
  else if (strcasecmp(ArgStr[1], "XA")) WrError(1350);
  else if (!strcasecmp(ArgStr[2], "@PCDE"))
  {
    PutCode(0xd4);
    CheckCPU(CPU75004);
  }
  else if (!strcasecmp(ArgStr[2], "@PCXA"))
    PutCode(0xd0);
  else
    WrError(1350);
}

static void DecodePUSH_POP(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else if (!strcasecmp(ArgStr[1], "BS"))
  {
    PutCode(0x0699 + (Code << 8)); CheckCPU(CPU75004);
  }
  else
  {
    DecodeAdr(ArgStr[1], MModReg8);
    switch (AdrMode)
    {
      case ModReg8:
        if (Odd(AdrPart)) WrError(1350);
        else PutCode(0x48 + Code + AdrPart);
        break;
    }
  }
}

static void DecodeIN_OUT(Word IsIN)
{
  if (ArgCnt != 2) WrError(1110);
  else
  {
    char *pPortArg = IsIN ? ArgStr[2] : ArgStr[1],
         *pRegArg = IsIN ? ArgStr[1] : ArgStr[2];

    if (strncasecmp(pPortArg, "PORT", 4)) WrError(1350);
    else
    {
      Boolean OK;

      BAsmCode[1] = 0xf0 + EvalIntExpression(pPortArg + 4, UInt4, &OK);
      if (OK)
      {
        DecodeAdr(pRegArg, MModReg8 | MModReg4);
        switch (AdrMode)
        {
          case ModReg4:
            if (AdrPart != 0) WrError(1350);
            else
            {
              BAsmCode[0] = 0x93 + (IsIN << 4); CodeLen = 2;
            }
            break;
          case ModReg8:
            if (AdrPart != 0) WrError(1350);
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

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg4 | MModReg8);
    switch (AdrMode)
    {
      case ModReg4:
        if (AdrPart != 0) WrError(1350);
        else
        {
          DecodeAdr(ArgStr[2], MModImm | MModInd);
          switch (AdrMode)
          {
            case ModImm: 
              PutCode(0x60 + AdrPart); break;
            case ModInd:
              if (AdrPart == 1) PutCode(0xd2); else WrError(1350);
              break;
          }
        }
        break;
      case ModReg8:
        if (AdrPart == 0)
        {
          DecodeAdr(ArgStr[2], MModReg8 | MModImm);
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
        else if (strcasecmp(ArgStr[2], "XA")) WrError(1350);
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
  if (ArgCnt != 2) WrError(1110);
  else
  {
   DecodeAdr(ArgStr[1], MModReg4 | MModReg8);
    switch (AdrMode)
    {
      case ModReg4:
        if (AdrPart != 0) WrError(1350);
        else
        {
          DecodeAdr(ArgStr[2], MModInd);
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
                WrError(1350);
             break;
          }
        }
        break;
      case ModReg8:
        if (AdrPart == 0)
        {
          DecodeAdr(ArgStr[2], MModReg8);
          switch (AdrMode)
          {
            case ModReg8:
              PutCode(0xc8aa + ((Code + 1) << 12) + (((Word)AdrPart) << 8));
              CheckCPU(CPU75104);
              break;
          }
        }
        else if (strcasecmp(ArgStr[2], "XA")) WrError(1350);
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
  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg4 | MModReg8);
    switch (AdrMode)
    {
      case ModReg4:
        if (AdrPart != 0) WrError(1350);
        else
        {
          DecodeAdr(ArgStr[2], MModImm | MModInd);
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
                WrError(1350);
              break;
          }
        }
        break;
      case ModReg8:
        if (AdrPart == 0)
        {
          DecodeAdr(ArgStr[2], MModReg8);
          switch (AdrMode)
          {
            case ModReg8:
              PutCode(0x88aa + (((Word)AdrPart) << 8) + ((Code + 1) << 12));
              CheckCPU(CPU75104);
              break;
          }
        }
        else if (strcasecmp(ArgStr[2], "XA")) WrError(1350);
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

  if (ArgCnt != 2) WrError(1110);
  else if (strcasecmp(ArgStr[1], "CY")) WrError(1350);
  else if (DecodeBitAddr(ArgStr[2], &BVal, NULL))
  {
    if (Hi(BVal) != 0) WrError(1350);
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

  if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg4 | MModReg8 | MModInd | MModAbs);
    switch (AdrMode)
    {
      case ModReg4:
        PutCode(0xc0 + AdrPart);
        break;
      case ModReg8:
        if ((AdrPart < 1) || (Odd(AdrPart))) WrError(1350);
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
          WrError(1350);
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

  if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg4 | MModReg8);
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

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg4 | MModReg8 | MModInd);
    switch (AdrMode)
    {
      case ModReg4:
        HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModImm | MModInd | MModReg4);
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
            else WrError(1350);
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
              WrError(1350);
            break;
        }
        break;
      case ModReg8:
        HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModInd | MModReg8);
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
            else WrError(1350);
            break;
          case ModInd:
            if (AdrPart == 1)
            {
              PutCode(0x19aa);
              CheckCPU(CPU75104);
            }
            else
              WrError(1350);
            break;
        }
        break;
      case ModInd:
        if (AdrPart != 1) WrError(1350);
        else
        {
          MinOneIs0 = True;
          DecodeAdr(ArgStr[2], MModImm | MModReg4 | MModReg8);
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
                WrError(1350);
              break;
            case ModReg8:
              if (AdrPart == 0)
              {
                PutCode(0x19aa);
                CheckCPU(CPU75004);
              }
              else
                WrError(1350);
              break;
          }
        }
        break;
    }
  }
}

static void DecodeAcc(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else if (strcasecmp(ArgStr[1], "A")) WrError(1350);
  else 
    PutCode(Code);
}

static void DecodeMOV1(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    Boolean OK = True;
    Word BVal;

    if (!strcasecmp(ArgStr[1], "CY"))
      Code = 0xbd;
    else if (!strcasecmp(ArgStr[2], "CY"))
      Code = 0x9b;
    else OK = False;
    if (!OK) WrError(1350);
    else if (DecodeBitAddr(ArgStr[((Code >> 2) & 3) - 1], &BVal, NULL))
    {
      if (Hi(BVal) != 0) WrError(1350);
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

  if (ArgCnt != 1) WrError(1110);
  else if (!strcasecmp(ArgStr[1], "CY"))
    PutCode(0xe6 + Code);
  else if (DecodeBitAddr(ArgStr[1], &BVal, NULL))
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

  if (ArgCnt != 1) WrError(1110);
  else if (!strcasecmp(ArgStr[1], "CY"))
  {
    if (Code)
      PutCode(0xd7);
    else
      WrError(1350);
  }
  else if (DecodeBitAddr(ArgStr[1], &BVal, NULL))
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

  if (ArgCnt != 1) WrError(1110);
  else if (strcasecmp(ArgStr[1], "CY")) WrError(1350);
  else
    PutCode(0xd6);
}

static void DecodeSKTCLR(Word Code)
{
  Word BVal;

  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else if (DecodeBitAddr(ArgStr[1], &BVal, NULL))
  {
    if (Hi(BVal) != 0) WrError(1350);
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

  if (ArgCnt != 1) WrError(1110);
  else if (!strcasecmp(ArgStr[1], "PCDE"))
  {
    PutCode(0x0499);
    CheckCPU(CPU75004);
  }
  else if (!strcasecmp(ArgStr[1], "PCXA"))
  {
    BAsmCode[0] = 0x99;
    BAsmCode[1] = 0x00;
    CodeLen = 2;
    CheckCPU(CPU75104);
  }
  else
  {
    char *pArg1 = ArgStr[1];
    Integer AdrInt, Dist;
    Boolean OK, BrRel, BrLong;

    BrRel = False;
    BrLong = False;
    if (*pArg1 == '$')
    {
      BrRel = True;
      pArg1++;
    }
    else if (*pArg1 == '!')
    {
      BrLong = True;
      pArg1++;
    }
    AdrInt = EvalIntExpression(pArg1, UInt16, &OK);
    if (OK)
    {
      Dist = AdrInt-EProgCounter();
      if ((BrRel) || ((Dist <= 16) && (Dist >= -15) && (Dist != 0)))
      {
        if (Dist > 0)
        {
          Dist--;
          if ((Dist > 15) && (!SymbolQuestionable)) WrError(1370);
          else
            PutCode(0x00 + Dist);
        }
        else
        {
          if ((Dist < -15) && (!SymbolQuestionable)) WrError(1370);
          else
            PutCode(0xf0 + 15 + Dist);
        }
      }
      else if ((!BrLong) && ((AdrInt >> 12) == (EProgCounter() >> 12)) && ((EProgCounter() & 0xfff) < 0xffe))
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

  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;
    Integer AdrInt = EvalIntExpression(ArgStr[1], UInt16, &OK);
    if (OK)
    {
      if ((AdrInt >> 12) != (EProgCounter() >> 12)) WrError(1910);
      else if ((EProgCounter() & 0xfff) >= 0xffe) WrError(1905);
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

  if (ArgCnt != 1) WrError(1110);
  else
  {
    char *pArg1 = ArgStr[1];
    Boolean BrLong, OK;
    Integer AdrInt;

    if (*pArg1 == '!')
    {
      pArg1++;
      BrLong = True;
    }
    else
      BrLong = False;
    FirstPassUnknown = False;
    AdrInt = EvalIntExpression(pArg1, UInt16, &OK);
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

  if (ArgCnt != 1) WrError(1110);
  else
  {
    char *pArg1 = ArgStr[1];
    Integer AdrInt;
    Boolean OK;

    if (*pArg1 == '!') pArg1++;
    AdrInt = EvalIntExpression(pArg1, UInt11, &OK);
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

  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;

    BAsmCode[0] = EvalIntExpression(ArgStr[1], UInt6, &OK);
    CodeLen = Ord(OK);
    CheckCPU(CPU75004);
  }
}

static void DecodeEI_DI(Word Code)
{
  Byte HReg;

  if (ArgCnt == 0)
    PutCode(0xb29c + Code);
  else if (ArgCnt != 1) WrError(1110);
  else if (DecodeIntName(ArgStr[1], &HReg))
    PutCode(0x989c + Code + (((Word)HReg) << 8));
  else
    WrError(1440);
}

static void DecodeSEL(Word Code)
{
  Boolean OK;

  UNUSED(Code);

  BAsmCode[0] = 0x99;
  if (ArgCnt != 1) WrError(1110);
  else if (!strncasecmp(ArgStr[1], "RB", 2))
  {
    BAsmCode[1] = 0x20 + EvalIntExpression(ArgStr[1] + 2, UInt2, &OK);
    if (OK)
    {
      CodeLen = 2;
      CheckCPU(CPU75104);
    }
  }
  else if (!strncasecmp(ArgStr[1], "MB", 2))
  {
    BAsmCode[1] = 0x10 + EvalIntExpression(ArgStr[1] + 2, UInt4, &OK);
    if (OK)
    {
      CodeLen = 2;
      CheckCPU(CPU75004);
    }
  }
  else
    WrError(1350);
}

static void DecodeSFR(Word Code)
{
  UNUSED(Code);

  CodeEquate(SegData, 0, 0xfff);
}

static void DecodeBIT(Word Code)
{
  Word BErg;
  String BName;

  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    FirstPassUnknown = False;
    if (DecodeBitAddr(ArgStr[1], &BErg, BName))
      if (!FirstPassUnknown)
      {
        PushLocHandle(-1);
        EnterIntSymbol(LabPart, BErg, SegNone, False);
        sprintf(ListLine, "=%s", BName);
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

  if (!LookupInstTable(InstTable, OpPart))
    WrXError(1200, OpPart);
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
  {"MBS", &MBSValue, 0, 0x0f, 0x10},
  {"MBE", &MBEValue, 0, 0x01, 0x01, CheckMBE}
};
#define ASSUME75Count (sizeof(ASSUME75s) / sizeof(*ASSUME75s))

static void SwitchTo_75K0(void)
{
  TurnWords = False; ConstMode = ConstModeIntel; SetIsOccupied = False;

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
