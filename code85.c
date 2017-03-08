/* code85.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator 8080/8085                                                   */
/*                                                                           */
/* Historie: 24.10.1996 Grundsteinlegung                                     */
/*            2. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: code85.c,v 1.10 2016/11/24 22:42:26 alfred Exp $                      */
/***************************************************************************** 
 * $Log: code85.c,v $
 * Revision 1.10  2016/11/24 22:42:26  alfred
 * - silence warning
 *
 * Revision 1.9  2016/10/07 20:39:07  alfred
 * - add Z80SYNTAX instruction
 *
 * Revision 1.8  2016/10/07 19:35:29  alfred
 * - first version of Z80 syntax ibn 8080/8085 target
 *
 * Revision 1.7  2016/04/26 14:13:50  alfred
 * - complain about wrong register names for PUSH/POP
 *
 * Revision 1.6  2014/12/07 19:14:00  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.5  2014/08/10 13:27:53  alfred
 * - rework to current style
 *
 * Revision 1.4  2007/11/24 22:48:05  alfred
 * - some NetBSD changes
 *
 * Revision 1.3  2005/09/08 16:53:42  alfred
 * - use common PInstTable
 *
 * Revision 1.2  2004/05/29 11:33:01  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 * Revision 1.1  2003/11/06 02:49:22  alfred
 * - recreated
 *
 * Revision 1.2  2002/11/09 13:27:12  alfred
 * - added hash table search, added undocumented 8085 instructions
 *
 *****************************************************************************/

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
#include "codepseudo.h" 
#include "intpseudo.h"
#include "asmitree.h"
#include "codevars.h"

/*--------------------------------------------------------------------------------------------------*/

typedef enum
{
  ModNone = 0xff,
  ModReg8 = 0,
  ModReg16 = 1,
  ModIReg16 = 2,
  ModAbs = 3,
  ModImm = 4,
} tAdrMode;

#define MModReg8 (1 << ModReg8)
#define MModReg16 (1 << ModReg16)
#define MModIReg16 (1 << ModIReg16)
#define MModAbs (1 << ModAbs)
#define MModImm (1 << ModImm)

static CPUVar CPU8080, CPU8085, CPU8085U;
static tAdrMode AdrMode;
static Byte AdrVals[2], OpSize;

static const char *Z80SyntaxName = "Z80SYNTAX";
static Boolean AllowZ80Syntax;

/*---------------------------------------------------------------------------*/

static const Byte AccReg = 7;

static Boolean DecodeReg8(const char *Asc, Byte *Erg)
{
  static const char *RegNames = "BCDEHLMA";
  const char *p;

  if (strlen(Asc) != 1) return False;
  else
  {
    p = strchr(RegNames, mytoupper(*Asc));
    if (!p) return False;
    else
    {
      *Erg = p - RegNames;
      return True;
    }
  }
}

static const Byte DEReg = 1;
static const Byte HLReg = 2;
static const Byte SPReg = 3;

static Boolean DecodeReg16(char *pAsc, Boolean OnlyZ80Names, Byte *pResult)
{
  static const char *RegNames[8] = {"B", "D", "H", "SP", "BC", "DE", "HL", "SP"};

  for (*pResult = OnlyZ80Names ? 4 : 0; (*pResult) < 8; (*pResult)++)
    if (!strcasecmp(pAsc, RegNames[*pResult]))
    {
      *pResult &= 3;
      break;
    }

  return ((*pResult) < 4);
}

static const char *pConditions[] =
{
  "NZ", "Z", "NC", "C", "PO", "PE", "P", "M",
};
static const int ConditionCnt = sizeof(pConditions) / sizeof(*pConditions);

static Boolean DecodeCondition(const char *pAsc, Byte *pResult)
{
  for (*pResult = 0; *pResult < ConditionCnt; (*pResult)++)
    if (!strcasecmp(pAsc, pConditions[*pResult]))
      return True;
  return False;
}

static void DecodeAdr_Z80(char *pAsc, Word Mask)
{
  Boolean OK;
  int l = strlen(pAsc);

  AdrMode = ModNone; AdrCnt = 0;

  if (DecodeReg8(pAsc, &AdrVals[0]) && (AdrVals[0] != 6))
  {
    AdrMode = ModReg8;
    goto AdrFound;
  }    

  if ((l == 2) && DecodeReg16(pAsc, True, &AdrVals[0]))
  {
    AdrMode = ModReg16;
    OpSize = 1;
    goto AdrFound;
  }

  if (IsIndirect(pAsc))
  {
    pAsc++; l -= 2; pAsc[l] = '\0';

    if (DecodeReg16(pAsc, True, &AdrVals[0]))
    {
      AdrMode = ModIReg16;
      goto AdrFound;
    }
    else
    {
      Word Addr = EvalIntExpression(pAsc, UInt16, &OK);

      if (OK)
      {
        AdrVals[0] = Lo(Addr);
        AdrVals[1] = Hi(Addr);
        AdrMode = ModAbs;
      }
    }
  }
  else if (OpSize)
  {
    Word Val = EvalIntExpression(pAsc, Int16, &OK);

    if (OK)
    {
      AdrVals[0] = Lo(Val);
      AdrVals[1] = Hi(Val);
      AdrMode = ModImm;
    }
  }
  else
  {
    AdrVals[0] = EvalIntExpression(pAsc, Int8, &OK);
    if (OK)
      AdrMode = ModImm;
  }

AdrFound:

  if ((AdrMode != ModNone) && (!(Mask & (1 << AdrMode))))
  {
    WrError(1350);
    AdrMode = ModNone; AdrCnt = 0;
  }
}

/*---------------------------------------------------------------------------*/

/* Anweisungen ohne Operanden */

static void DecodeFixed(Word Code)
{
  if (ArgCnt != 0) WrError(1110);
  else
  {
    CodeLen = 1;
    BAsmCode[0] = Code;
  }
}

static void DecodeFixed_Z80(Word Code)
{
  if (!AllowZ80Syntax) WrError(1500);
  else if (ArgCnt != 0) WrError(1110);
  else
  {
    CodeLen = 1;
    BAsmCode[0] = Code;
  }
}

/* ein 16-Bit-Operand */

static void DecodeOp16(Word Code)
{
  Boolean OK;
  Word AdrWord;

  if (ArgCnt != 1) WrError(1110);
  else
  {
    AdrWord = EvalIntExpression(ArgStr[1], Int16, &OK);
    if (OK)
    {
      CodeLen = 3;
      BAsmCode[0] = Code;
      BAsmCode[1] = Lo(AdrWord);
      BAsmCode[2] = Hi(AdrWord);
      ChkSpace(SegCode);
    }
  }
}

static void DecodeOp8(Word Code)
{
  Boolean OK;
  Byte AdrByte;

  if (ArgCnt!=1) WrError(1110);
  else
  {
    AdrByte = EvalIntExpression(ArgStr[1], Int8, &OK);
    if (OK)
    {
      CodeLen = 2;
      BAsmCode[0] = Lo(Code);
      BAsmCode[1] = AdrByte;
    }
  }
}

static void DecodeALU(Word Code)
{
  Byte Reg;

  if (ArgCnt != 1) WrError(1110);
  else if (!DecodeReg8(ArgStr[1], &Reg)) WrXError(1980, ArgStr[1]);
  else
  {
    CodeLen = 1;
    BAsmCode[0] = Code + Reg;
  }
}

static void DecodeMOV(Word Index)
{
  Byte Dest;

  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else if (!DecodeReg8(ArgStr[1], &Dest)) WrXError(1980, ArgStr[1]);
  else if (!DecodeReg8(ArgStr[2], BAsmCode + 0)) WrXError(1980, ArgStr[2]);
  else
  {
    BAsmCode[0] += 0x40 + (Dest << 3);
    if (BAsmCode[0] == 0x76)
      WrError(1760);
    else
      CodeLen = 1;
  }
}

static void DecodeMVI(Word Index)
{
  Boolean OK;
  Byte Reg;

  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    BAsmCode[1] = EvalIntExpression(ArgStr[2], Int8, &OK);
    if (OK)
    {
      if (!DecodeReg8(ArgStr[1], &Reg)) WrXError(1980, ArgStr[1]);
      else
      {
        BAsmCode[0] = 0x06 + (Reg << 3);
        CodeLen = 2;
      }
    }
  }
}

static void DecodeLXI(Word Index)
{
  Boolean OK;
  Word AdrWord;
  Byte Reg;

  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    AdrWord = EvalIntExpression(ArgStr[2], Int16, &OK);
    if (OK)
    {
      if (!DecodeReg16(ArgStr[1], False, &Reg)) WrXError(1980, ArgStr[1]);
      else
      {
        BAsmCode[0] = 0x01 + (Reg << 4);
        BAsmCode[1] = Lo(AdrWord);
        BAsmCode[2] = Hi(AdrWord);
        CodeLen = 3;
      }
    }
  }
}

static void DecodeLDAX_STAX(Word Index)
{
  Byte Reg;

  if (ArgCnt != 1) WrError(1110);
  else if (!DecodeReg16(ArgStr[1], False, &Reg)) WrXError(1980, ArgStr[1]);
  else 
  {
    switch (Reg)
    {
      case 3:                             /* SP */
        WrError(1135);
        break;
      case 2:                             /* H --> MOV A,M oder M,A */
        CodeLen = 1;
        BAsmCode[0] = 0x77 + (Index * 7);
        break;
      default:
        CodeLen = 1;
        BAsmCode[0] = 0x02 + (Reg << 4) + (Index << 3);
        break;
    }
  }
}

static void DecodePUSH_POP(Word Index)
{
  Byte Reg;
  Boolean OK;

  if (ArgCnt != 1) WrError(1110);
  else
  {
    if ((!strcasecmp(ArgStr[1], "PSW"))
     || (AllowZ80Syntax && (!strcasecmp(ArgStr[1], "AF"))))
    {
      Reg = 3;
      OK = TRUE;
    }
    else if (DecodeReg16(ArgStr[1], False, &Reg))
      OK = (Reg != 3);
    else
      OK = FALSE;
    if (!OK) WrXError(1980, ArgStr[1]);
    else
    {
      CodeLen = 1;
      BAsmCode[0] = 0xc1 + (Reg << 4) + Index;
    }
  }
}

static void DecodeRST(Word Index)
{
  Byte AdrByte;
  Boolean OK;

  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else if ((MomCPU >= CPU8085U) && (!strcasecmp(ArgStr[1], "V")))
  {
    CodeLen = 1;
    BAsmCode[0] = 0xcb; 
  }
  else
  {
    AdrByte = EvalIntExpression(ArgStr[1], AllowZ80Syntax ? UInt6 : UInt3, &OK);
    if (FirstPassUnknown)
      AdrByte = 0;
    if (OK)
    {
      if (AdrByte < 8)
        BAsmCode[CodeLen++] = 0xc7 + (AdrByte << 3);
      else if (AdrByte & 7)
        WrError(1320);
      else
        BAsmCode[CodeLen++] = 0xc7 + (AdrByte & 0x38);
    }
  }
}

static void DecodeINR_DCR(Word Index)
{
  Byte Reg;

  if (ArgCnt != 1) WrError(1110);
  else if (!DecodeReg8(ArgStr[1], &Reg)) WrXError(1980, ArgStr[1]);
  else
  {
    CodeLen = 1;
    BAsmCode[0] = 0x04 + (Reg << 3) + Index;
  }
}

static void DecodeINX_DCX(Word Index)
{
  Byte Reg;

  if (ArgCnt != 1) WrError(1110);
  else if (!DecodeReg16(ArgStr[1], False, &Reg)) WrXError(1980, ArgStr[1]);
  else
  {
    CodeLen = 1;
    BAsmCode[0] = 0x03 + (Reg << 4) + Index;
  }
}

static void DecodeDAD(Word Index)
{
  Byte Reg;

  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else if (!DecodeReg16(ArgStr[1], False, &Reg)) WrXError(1980, ArgStr[1]);
  else
  {
    CodeLen = 1;
    BAsmCode[0] = 0x09 + (Reg << 4);
  }
}

static void DecodeDSUB(Word Index) 
{
  Byte Reg;

  UNUSED(Index);

  if (!ArgCnt)
    strcpy(ArgStr[++ArgCnt], "B");

  if (ArgCnt != 1) WrError(1110);
  else if (MomCPU < CPU8085U) WrError(1500);
  else if (!DecodeReg16(ArgStr[1], False, &Reg)) WrXError(1980, ArgStr[1]);
  else if (Reg != 0) WrXError(1980, ArgStr[1]);
  else
  {   
    CodeLen = 1;
    BAsmCode[0] = 0x08;
  }
}

static void DecodeLHLX_SHLX(Word Index) 
{
  Byte Reg;

  UNUSED(Index);

  if (!ArgCnt)
    strcpy(ArgStr[++ArgCnt], "D");

  if (ArgCnt != 1) WrError(1110);
  else if (MomCPU < CPU8085U) WrError(1500);
  else if (!DecodeReg16(ArgStr[1], False, &Reg)) WrXError(1980, ArgStr[1]);
  else if (Reg != 1) WrXError(1980, ArgStr[1]);
  else
  {   
    CodeLen = 1;
    BAsmCode[0] = Index ? 0xed: 0xd9;
  }
}

static void DecodeLD(Word Code)
{
  Byte HVals[2];
  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else if (!AllowZ80Syntax) WrError(1500);
  else
  {
    DecodeAdr_Z80(ArgStr[1], MModReg8 | MModIReg16 | MModAbs | MModReg16);
    switch (AdrMode)
    {
      case ModReg8:
        HVals[0] = AdrVals[0];
        DecodeAdr_Z80(ArgStr[2], MModReg8 | MModIReg16 | (HVals[0] == AccReg ? MModAbs : 0) | MModImm);
        switch (AdrMode)
        {
          case ModReg8:
            BAsmCode[CodeLen++] = 0x40 | (HVals[0] << 3) | AdrVals[0];
            break;
          case ModIReg16:
            if (AdrVals[0] == HLReg)
              BAsmCode[CodeLen++] = 0x46 | (HVals[0] << 3);
            else if ((HVals[0] == AccReg) && (AdrVals[0] != SPReg))
              BAsmCode[CodeLen++] = 0x0a | (AdrVals[0] << 4);
            else
              WrError(1350);
            break;
          case ModAbs:
            BAsmCode[CodeLen++] = 0x3a;
            BAsmCode[CodeLen++] = AdrVals[0];
            BAsmCode[CodeLen++] = AdrVals[1];
            break;
          case ModImm:
            BAsmCode[CodeLen++] = 0x06 | (HVals[0] << 3);
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
          default:
            break;
        }
        break;
      case ModReg16:
        HVals[0] = AdrVals[0];
        DecodeAdr_Z80(ArgStr[2], MModImm | (HVals[0] == HLReg ? MModAbs : 0) | (HVals[0] == SPReg ? MModReg16 : 0));
        switch (AdrMode)
        {
          case ModImm:
            BAsmCode[CodeLen++] = 0x01 | (HVals[0] << 4);
            BAsmCode[CodeLen++] = AdrVals[0];
            BAsmCode[CodeLen++] = AdrVals[1];
            break;
          case ModAbs:
            BAsmCode[CodeLen++] = 0x2a;
            BAsmCode[CodeLen++] = AdrVals[0];
            BAsmCode[CodeLen++] = AdrVals[1];
            break;
          case ModReg16:
            if (AdrVals[0] == HLReg)
              BAsmCode[CodeLen++] = 0xf9;
            else
              WrError(1350);
            break;
          default:
            break;
        }
        break;
      case ModIReg16:
        HVals[0] = AdrVals[0];
        DecodeAdr_Z80(ArgStr[2], MModReg8 | (HVals[0] == HLReg ? MModImm : 0));
        switch (AdrMode)
        {
          case ModReg8:
            if (HVals[0] == HLReg)
              BAsmCode[CodeLen++] = 0x70 | AdrVals[0];
            else if ((AdrVals[0] == AccReg) && (HVals[0] != SPReg))
              BAsmCode[CodeLen++] = 0x02 | (HVals[0] << 4);
            else
              WrError(1350);
            break;
          case ModImm:
            BAsmCode[CodeLen++] = 0x36;
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
          default:
            break;
        }
        break;
      case ModAbs:
        HVals[0] = AdrVals[0];
        HVals[1] = AdrVals[1];
        DecodeAdr_Z80(ArgStr[2], MModReg8 | MModReg16);
        switch (AdrMode)
        {
          case ModReg8:
            if (AdrVals[0] != AccReg) WrError(1350);
            else
            {
              BAsmCode[CodeLen++] = 0x32;
              BAsmCode[CodeLen++] = HVals[0];
              BAsmCode[CodeLen++] = HVals[1];
            }
            break;
          case ModReg16:
            if (AdrVals[0] != HLReg) WrError(1350);
            else
            {
              BAsmCode[CodeLen++] = 0x22;
              BAsmCode[CodeLen++] = HVals[0];
              BAsmCode[CodeLen++] = HVals[1];
            }
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
  }
}

static void DecodeEX(Word Code)
{
  Byte HVal;

  UNUSED(Code);   

  if (ArgCnt != 2) WrError(1110);
  else if (!AllowZ80Syntax) WrError(1500);
  else
  {
    DecodeAdr_Z80(ArgStr[1], MModReg16 | MModIReg16);
    switch (AdrMode)
    {
      case ModReg16:
        HVal = AdrVals[0];
        DecodeAdr_Z80(ArgStr[2], MModReg16 | MModIReg16);
        switch (AdrMode)
        {
          case ModReg16:
            if (((HVal == DEReg) && (AdrVals[0] == HLReg))
             || ((HVal == HLReg) && (AdrVals[0] == DEReg)))
              BAsmCode[CodeLen++] = 0xeb;
            else
              WrError(1350);
            break;
          case ModIReg16:
            if ((HVal == HLReg) && (AdrVals[0] == SPReg))
              BAsmCode[CodeLen++] = 0xe3;
            else
              WrError(1350);
            break;
          default:
            break;
        }
        break;
      case ModIReg16:
        HVal = AdrVals[0];
        DecodeAdr_Z80(ArgStr[2], MModReg16);
        switch (AdrMode)
        {
          case ModReg16:
            if ((HVal == SPReg) && (AdrVals[0] == HLReg))
              BAsmCode[CodeLen++] = 0xe3;
            else
              WrError(1350);
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
  }
}

static void DecodeADD(Word Code)
{
  UNUSED(Code);

  if (ArgCnt == 1) /* 8080 style - 8 bit register src only */
  {
    Byte Reg;

    if (!DecodeReg8(ArgStr[1], &Reg)) WrXError(1980, ArgStr[1]);
    else
      BAsmCode[CodeLen++] = 0x80 | Reg;
  }
  else if ((ArgCnt == 2) && (AllowZ80Syntax))
  {
    DecodeAdr_Z80(ArgStr[1], MModReg8 | MModReg16);
    switch (AdrMode)
    {
      case ModReg8:
        if (AdrVals[0] != AccReg) WrError(1350);
        else
        {
          DecodeAdr_Z80(ArgStr[2], MModReg8 | MModIReg16 | MModImm);
          switch (AdrMode)
          {
            case ModReg8:
              BAsmCode[CodeLen++] = 0x80 | AdrVals[0];
              break;
            case ModIReg16:
              if (AdrVals[0] != HLReg) WrError(1350);
              else
                BAsmCode[CodeLen++] = 0x86;
              break;
            case ModImm:
              BAsmCode[CodeLen++] = 0xc6;
              BAsmCode[CodeLen++] = AdrVals[0];
              break;
            default:
              break;
          }
        }
        break;
      case ModReg16:
        if (AdrVals[0] != HLReg) WrError(1350);
        else
        {   
          DecodeAdr_Z80(ArgStr[2], MModReg16);
          switch (AdrMode)
          {
            case ModReg16: 
              BAsmCode[CodeLen++] = 0x09 | (AdrVals[0] << 4);
              break;
            default:
              break;
          }
        }  
        break;
      default:
        break;
    } 
  }
  else
    WrError(1110);
}

static void DecodeADC(Word Code)
{
  UNUSED(Code);

  if (ArgCnt == 1) /* 8080 style - 8 bit register src only */
  {
    Byte Reg;

    if (!DecodeReg8(ArgStr[1], &Reg)) WrXError(1980, ArgStr[1]);
    else
      BAsmCode[CodeLen++] = 0x88 | Reg;
  }
  else if ((ArgCnt == 2) && (AllowZ80Syntax))
  {
    DecodeAdr_Z80(ArgStr[1], MModReg8);
    switch (AdrMode)
    {
      case ModReg8:
        if (AdrVals[0] != AccReg) WrError(1350);
        else
        {
          DecodeAdr_Z80(ArgStr[2], MModReg8 | MModIReg16 | MModImm);
          switch (AdrMode)
          {
            case ModReg8:
              BAsmCode[CodeLen++] = 0x88 | AdrVals[0];
              break;
            case ModIReg16:
              if (AdrVals[0] != HLReg) WrError(1350);
              else
                BAsmCode[CodeLen++] = 0x8e;
              break;
            case ModImm:
              BAsmCode[CodeLen++] = 0xce;
              BAsmCode[CodeLen++] = AdrVals[0];
              break;
            default:
              break;
          }
        }
        break;
      default:
        break;
    } 
  }
  else
    WrError(1110);
}

static void DecodeSUB(Word Code)
{
  Byte Reg;

  UNUSED(Code);

  if ((ArgCnt < 1) || (ArgCnt > 2))
  {
    WrError(1110);
    return;
  }

  if (ArgCnt == 2) /* optional Z80 style */
  {
    if (!AllowZ80Syntax)
    {
      WrError(1110);
      return;
    }
    DecodeAdr_Z80(ArgStr[1], MModReg8);

    switch (AdrMode)
    {
      case ModNone:
        return;
      case ModReg8:
        if (AdrVals[0] == AccReg)
          break; /* conditional */
      default:
        WrError(1350);
        return;
    }
  }

  if (DecodeReg8(ArgStr[ArgCnt], &Reg)) /* 8080 style incl. M, Z80 style excl. (HL) */
  {
    BAsmCode[CodeLen++] = 0x90 | Reg;
    return;
  }

  /* rest is Z80 style ( (HL) or immediate) */

  if (!AllowZ80Syntax)
  {
    WrError(1350);
    return;
  }

  DecodeAdr_Z80(ArgStr[ArgCnt], MModImm | MModIReg16);
  switch (AdrMode)
  {
    case ModIReg16:
      if (AdrVals[0] != HLReg) WrError(1350);
      else
        BAsmCode[CodeLen++] = 0x96;
      break;
    case ModImm:
      BAsmCode[CodeLen++] = 0xd6;
      BAsmCode[CodeLen++] = AdrVals[0];
      break;
    default:
      break;
  }
}

static void DecodeALU8_Z80(Word Code)
{
  if (!AllowZ80Syntax)
  {
    WrError(1500);
    return;
  }

  if ((ArgCnt < 1) || (ArgCnt > 2))
  {
    WrError(1110);
    return;
  }

  if (ArgCnt == 2) /* A as dest */
  {
    DecodeAdr_Z80(ArgStr[1], MModReg8);

    switch (AdrMode)
    {
      case ModNone:
        return;
      case ModReg8:
        if (AdrVals[0] == AccReg)
          break; /* conditional */
      default:
        WrError(1350);
        return;
    }
  }

  DecodeAdr_Z80(ArgStr[ArgCnt], MModImm | MModIReg16 | MModReg8);
  switch (AdrMode)
  {
    case ModReg8:
      BAsmCode[CodeLen++] = 0x80 | (Code << 3) | AdrVals[0];
      break;
    case ModIReg16:
      if (AdrVals[0] != HLReg) WrError(1350);
      else
        BAsmCode[CodeLen++] = 0x86 | (Code << 3);
      break;
    case ModImm:
      BAsmCode[CodeLen++] = 0xc6 | (Code << 3);
      BAsmCode[CodeLen++] = AdrVals[0];
      break;
    default:
      break;
  }
}

static void DecodeINCDEC(Word Code)
{
  if (!AllowZ80Syntax) WrError(1500);
  else if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr_Z80(ArgStr[1], MModReg8 | MModReg16 | MModIReg16);
    switch (AdrMode)
    {
      case ModReg8:
        BAsmCode[CodeLen++] = 0x04 | Code | (AdrVals[0] << 3);
        break;
      case ModReg16:
        BAsmCode[CodeLen++] = 0x03 | (Code << 3) | (AdrVals[0] << 4);
        break;
      case ModIReg16:
        if (AdrVals[0] != HLReg) WrError(1350);
        else
          BAsmCode[CodeLen++] = 0x34 | Code;
      default:
        break;
    }
  }
}

static void DecodeCP(Word Code)
{
  UNUSED(Code);

  if ((ArgCnt < 1) || (ArgCnt > 2))
  {
    WrError(1110);
    return;
  }

  if (ArgCnt == 2) /* A as dest */
  {
    if (!AllowZ80Syntax)
    {
      WrError(1110);
      return;
    }

    DecodeAdr_Z80(ArgStr[1], MModReg8);

    switch (AdrMode)
    {
      case ModNone:
        return;
      case ModReg8:
        if (AdrVals[0] == AccReg)
          break; /* conditional */
      default:
        WrError(1350);
        return;
    }
  }

  /* with one argument, treat immediate argument as 16-bit address for call-on-positive! */

  OpSize = (ArgCnt == 1) ? 1 : 0;
  DecodeAdr_Z80(ArgStr[ArgCnt], MModImm | (AllowZ80Syntax ? (MModIReg16 | MModReg8) : 0));
  switch (AdrMode)
  {
    case ModReg8:
      BAsmCode[CodeLen++] = 0xb8 | AdrVals[0];
      break;
    case ModIReg16:
      if (AdrVals[0] != HLReg) WrError(1350);
      else
        BAsmCode[CodeLen++] = 0xbe;
      break;
    case ModImm:
      if (1 == ArgCnt) /* see comment above */
      {
        BAsmCode[CodeLen++] = 0xf4;
        BAsmCode[CodeLen++] = AdrVals[0];
        BAsmCode[CodeLen++] = AdrVals[1];
      }
      else
      {
        BAsmCode[CodeLen++] = 0xfe;
        BAsmCode[CodeLen++] = AdrVals[0];
      }
      break;
    default:
      break;
  }
}

static void DecodeJP(Word Code)
{
  Byte Condition;
  UNUSED(Code);

  if ((ArgCnt < 1) || (ArgCnt > 2))
  {
    WrError(1110);
    return;
  }

  if (ArgCnt == 2) /* Z80-style with condition */
  {
    if (!AllowZ80Syntax)
    {
      WrError(1110);
      return;
    }
    if (!DecodeCondition(ArgStr[1], &Condition))
    {
      WrXError(1360, ArgStr[1]);
      return;
    }
  }
  else
   Condition = 0xff;

  OpSize = 1;
  DecodeAdr_Z80(ArgStr[ArgCnt], MModImm | (((ArgCnt == 1) && AllowZ80Syntax) ? MModIReg16 : 0));
  switch (AdrMode)
  {
    case ModIReg16:
      if (AdrVals[0] != HLReg) WrError(1350);
      else
        BAsmCode[CodeLen++] = 0xe9;
      break;
    case ModImm:
      BAsmCode[CodeLen++] = (1 == ArgCnt) ? 0xf2 : 0xc2 | (Condition << 3);
      BAsmCode[CodeLen++] = AdrVals[0];
      BAsmCode[CodeLen++] = AdrVals[1];
    default:
      break;
  }
}

static void DecodeCALL(Word Code)
{
  Byte Condition;
  UNUSED(Code);

  if ((ArgCnt < 1) || (ArgCnt > 2))
  {
    WrError(1110);
    return;
  }

  if (ArgCnt == 2) /* Z80-style with condition */
  {
    if (!AllowZ80Syntax)
    {
      WrError(1110);
      return;
    }
    if (!DecodeCondition(ArgStr[1], &Condition))
    {
      WrXError(1360, ArgStr[1]);
      return;
    }
  }
  else
   Condition = 0xff;

  OpSize = 1;
  DecodeAdr_Z80(ArgStr[ArgCnt], MModImm);
  switch (AdrMode)
  {
    case ModImm:
      BAsmCode[CodeLen++] = (1 == ArgCnt) ? 0xcd : 0xc4 | (Condition << 3);
      BAsmCode[CodeLen++] = AdrVals[0];
      BAsmCode[CodeLen++] = AdrVals[1];
    default:
      break;
  }
}

static void DecodeRET(Word Code)
{
  Byte Condition;
  UNUSED(Code);

  if (ArgCnt > 1)
  {
    WrError(1110);
    return;
  }

  if (ArgCnt == 1) /* Z80-style with condition */
  {
    if (!AllowZ80Syntax)
    {
      WrError(1110);
      return;
    }
    if (!DecodeCondition(ArgStr[1], &Condition)) WrXError(1360, ArgStr[1]);
    else
      BAsmCode[CodeLen++] = 0xc0 | (Condition << 3);
  }
  else
   BAsmCode[CodeLen++] = 0xc9;
}

static void DecodeINOUT(Word Code)
{
  Boolean OK;

  if ((ArgCnt < 1) || (ArgCnt > 2))
  {
    WrError(1110);
    return;
  }
  if (ArgCnt == 2) /* Z80-style with A */
  {
    if (!AllowZ80Syntax)
    {
      WrError(1110);
      return;
    }
    DecodeAdr_Z80(ArgStr[Code == 0xdb ? 1 : 2], MModReg8);
    if ((AdrMode != ModNone) && (AdrVals[0] != AccReg))
    {
      WrError(1350);
      AdrMode = ModNone;
    }
    if (AdrMode == ModNone)
      return;
  }
  BAsmCode[1] = EvalIntExpression(ArgStr[((Code == 0xdb) && (ArgCnt == 2)) ? 2 : 1], UInt8, &OK);
  if (OK)
  {
    ChkSpace(SegCode);
    BAsmCode[0] = Lo(Code);
    CodeLen = 2;
  }
}

static void DecodePORT(Word Index)
{
  UNUSED(Index);
              
  CodeEquate(SegIO, 0, 0xff);
}

/*--------------------------------------------------------------------------------------------------------*/

static void AddFixed(char *NName, CPUVar NMinCPU, Byte NCode)
{
  if (MomCPU >= NMinCPU)
    AddInstTable(InstTable, NName, NCode, DecodeFixed);
}

static void AddOp16(char *NName, CPUVar NMinCPU, Byte NCode)
{
  if (MomCPU >= NMinCPU)
    AddInstTable(InstTable, NName, NCode, DecodeOp16);
}

static void AddOp8(char *NName, CPUVar NMinCPU, Word NCode)
{
  if (MomCPU >= NMinCPU)
    AddInstTable(InstTable, NName, NCode, DecodeOp8);
}                         

static void AddALU(char *NName, Byte NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeALU);
}           

static void InitFields(void)
{
  InstTable = CreateInstTable(201);

  AddInstTable(InstTable, "MOV" , 0, DecodeMOV);
  AddInstTable(InstTable, "MVI" , 0, DecodeMVI);
  AddInstTable(InstTable, "LXI" , 0, DecodeLXI);
  AddInstTable(InstTable, "STAX", 0, DecodeLDAX_STAX);
  AddInstTable(InstTable, "LDAX", 1, DecodeLDAX_STAX);
  AddInstTable(InstTable, "SHLX", 0, DecodeLHLX_SHLX);
  AddInstTable(InstTable, "LHLX", 1, DecodeLHLX_SHLX);
  AddInstTable(InstTable, "PUSH", 4, DecodePUSH_POP);
  AddInstTable(InstTable, "POP" , 0, DecodePUSH_POP);
  AddInstTable(InstTable, "RST" , 0, DecodeRST);
  AddInstTable(InstTable, "INR" , 0, DecodeINR_DCR);
  AddInstTable(InstTable, "DCR" , 1, DecodeINR_DCR);
  AddInstTable(InstTable, "INX" , 0, DecodeINX_DCX);
  AddInstTable(InstTable, "DCX" , 8, DecodeINX_DCX);
  AddInstTable(InstTable, "DAD" , 0, DecodeDAD);
  AddInstTable(InstTable, "DSUB", 0, DecodeDSUB);
  AddInstTable(InstTable, "PORT", 0, DecodePORT);

  AddFixed("XCHG", CPU8080 , 0xeb); AddFixed("XTHL", CPU8080 , 0xe3);
  AddFixed("SPHL", CPU8080 , 0xf9); AddFixed("PCHL", CPU8080 , 0xe9);
  /* RET needs special handling */  AddFixed("RC"  , CPU8080 , 0xd8);
  AddFixed("RNC" , CPU8080 , 0xd0); AddFixed("RZ"  , CPU8080 , 0xc8);
  AddFixed("RNZ" , CPU8080 , 0xc0); AddFixed("RP"  , CPU8080 , 0xf0);
  AddFixed("RM"  , CPU8080 , 0xf8); AddFixed("RPE" , CPU8080 , 0xe8);
  AddFixed("RPO" , CPU8080 , 0xe0); AddFixed("RLC" , CPU8080 , 0x07);
  AddFixed("RRC" , CPU8080 , 0x0f); AddFixed("RAL" , CPU8080 , 0x17);
  AddFixed("RAR" , CPU8080 , 0x1f); AddFixed("CMA" , CPU8080 , 0x2f);
  AddFixed("STC" , CPU8080 , 0x37); AddFixed("CMC" , CPU8080 , 0x3f);
  AddFixed("DAA" , CPU8080 , 0x27); AddFixed("EI"  , CPU8080 , 0xfb);
  AddFixed("DI"  , CPU8080 , 0xf3); AddFixed("NOP" , CPU8080 , 0x00);
  AddFixed("HLT" , CPU8080 , 0x76); AddFixed("RIM" , CPU8085 , 0x20);
  AddFixed("SIM" , CPU8085 , 0x30); AddFixed("ARHL", CPU8085U, 0x10);
  AddFixed("RDEL", CPU8085U, 0x18); 

  AddInstTable(InstTable, "CPL", 0x2f, DecodeFixed_Z80);
  AddInstTable(InstTable, "SCF", 0x37, DecodeFixed_Z80);
  AddInstTable(InstTable, "CCF", 0x3f, DecodeFixed_Z80);
  AddInstTable(InstTable, "RLCA", 0x07, DecodeFixed_Z80);
  AddInstTable(InstTable, "RRCA", 0x0f, DecodeFixed_Z80);
  AddInstTable(InstTable, "RLA", 0x17, DecodeFixed_Z80);
  AddInstTable(InstTable, "RRA", 0x1f, DecodeFixed_Z80);

  AddOp16("STA" , CPU8080 , 0x32); AddOp16("LDA" , CPU8080 , 0x3a);
  AddOp16("SHLD", CPU8080 , 0x22); AddOp16("LHLD", CPU8080 , 0x2a);
  AddOp16("JMP" , CPU8080 , 0xc3); AddOp16("JC"  , CPU8080 , 0xda);
  AddOp16("JNC" , CPU8080 , 0xd2); AddOp16("JZ"  , CPU8080 , 0xca);
  AddOp16("JNZ" , CPU8080 , 0xc2); /* JP needs special handling */
  AddOp16("JM"  , CPU8080 , 0xfa); AddOp16("JPE" , CPU8080 , 0xea);
  AddOp16("JPO" , CPU8080 , 0xe2); /* CALL needs special handling */
  AddOp16("CC"  , CPU8080 , 0xdc); AddOp16("CNC" , CPU8080 , 0xd4);
  AddOp16("CZ"  , CPU8080 , 0xcc); AddOp16("CNZ" , CPU8080 , 0xc4);
  /* CP needs special handling */  AddOp16("CM"  , CPU8080 , 0xfc);
  AddOp16("CPE" , CPU8080 , 0xec); AddOp16("CPO" , CPU8080 , 0xe4);
  AddOp16("JNX5", CPU8085U, 0xdd); AddOp16("JX5" , CPU8085U, 0xfd);

  AddOp8("ADI" , CPU8080 , 0xc6); AddOp8("ACI" , CPU8080 , 0xce);
  AddOp8("SUI" , CPU8080 , 0xd6); AddOp8("SBI" , CPU8080 , 0xde);
  AddOp8("ANI" , CPU8080 , 0xe6); AddOp8("XRI" , CPU8080 , 0xee);
  AddOp8("ORI" , CPU8080 , 0xf6); AddOp8("CPI" , CPU8080 , 0xfe);
  AddOp8("LDHI", CPU8085U, 0x28); AddOp8("LDSI", CPU8085U, 0x38);

  AddALU("SBB" , 0x98);
  AddALU("ANA" , 0xa0); AddALU("XRA" , 0xa8);
  AddALU("ORA" , 0xb0); AddALU("CMP" , 0xb8);

  AddInstTable(InstTable, "LD", 0, DecodeLD);
  AddInstTable(InstTable, "EX", 0, DecodeEX);
  AddInstTable(InstTable, "ADD", 0, DecodeADD);
  AddInstTable(InstTable, "ADC", 0, DecodeADC);
  AddInstTable(InstTable, "SUB", 0, DecodeSUB);
  AddInstTable(InstTable, "SBC", 3, DecodeALU8_Z80);
  AddInstTable(InstTable, "INC", 0, DecodeINCDEC);
  AddInstTable(InstTable, "DEC", 1, DecodeINCDEC);
  AddInstTable(InstTable, "AND", 4, DecodeALU8_Z80);
  AddInstTable(InstTable, "XOR", 5, DecodeALU8_Z80);
  AddInstTable(InstTable, "OR" , 6, DecodeALU8_Z80);
  AddInstTable(InstTable, "CP" , 0, DecodeCP);
  AddInstTable(InstTable, "JP" , 0, DecodeJP);
  AddInstTable(InstTable, "CALL", 0, DecodeCALL);
  AddInstTable(InstTable, "RET", 0, DecodeRET);
  AddInstTable(InstTable, "IN", 0xdb, DecodeINOUT);
  AddInstTable(InstTable, "OUT", 0xd3, DecodeINOUT);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*--------------------------------------------------------------------------------------------------------*/

static void MakeCode_85(void)
{
  CodeLen = 0;
  DontPrint = False;
  OpSize = 0;

  /* zu ignorierendes */

  if (Memo("")) return;

  /* Pseudoanweisungen */

  if (DecodeIntelPseudo(False)) return;

  /* suchen */

  if (!LookupInstTable(InstTable, OpPart))
    WrXError(1200,OpPart);
}

static Boolean IsDef_85(void)
{
  return (Memo("PORT"));
}

static void SwitchFrom_85(void)
{
  DeinitFields();
}

static void SwitchTo_85(void)
{
  TurnWords = False;
  ConstMode = ConstModeIntel;
  SetIsOccupied = False;

  PCSymbol = "$";
  HeaderID = 0x41;
  NOPCode = 0x00;
  DivideChars = ",";
  HasAttrs = False;

  ValidSegs = (1 << SegCode) | (1 << SegIO);
  Grans[SegCode] = 1; ListGrans[SegCode] = 1; SegInits[SegCode] = 0;
  SegLimits[SegCode] = 0xffff;
  Grans[SegIO  ] = 1; ListGrans[SegIO  ] = 1; SegInits[SegIO  ] = 0;
  SegLimits[SegIO  ] = 0xff;

  MakeCode = MakeCode_85;
   IsDef = IsDef_85;
  SwitchFrom = SwitchFrom_85;
  InitFields();
  AddONOFF(Z80SyntaxName , &AllowZ80Syntax , Z80SyntaxName , False);
}

void code85_init(void)
{
  CPU8080 = AddCPU("8080", SwitchTo_85);
  CPU8085 = AddCPU("8085", SwitchTo_85);
  CPU8085U = AddCPU("8085UNDOC", SwitchTo_85);
}
