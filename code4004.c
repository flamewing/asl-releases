/* code4004.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Intel 4004                                                  */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "codevars.h"
#include "headids.h"
#include "fourpseudo.h"
#include "errmsg.h"

#include "code4004.h"

/*---------------------------------------------------------------------------*/
/* Variablen */

static CPUVar CPU4004, CPU4040;

/*---------------------------------------------------------------------------*/
/* Parser */

static Byte RegVal(const char *pInp, int l)
{
  switch (l)
  {
    case 1:
    {
      char ch = mytoupper(*pInp);
      if ((ch >='0') && (ch <= '9'))
        return ch - '0';
      else if ((ch >='A') && (ch <= 'F'))
        return ch - 'A' + 10;
      else
        return 0xff;
    }
    case 2:
    {
      unsigned Acc = 0, z;

      for (z = 0; z < 2; z++)
      {
        if (!isdigit(pInp[z]))
          return 0xff;
        Acc = (Acc * 10) + (pInp[z] - '0');
      }
      return (Acc <= 15) ? Acc : 0xff;
    }
    default:
      return 0xff;
  }
}

static Boolean DecodeRegCore(const char *pAsc, Byte *pErg, int l)
{
  if ((l < 2) || (l > 3) || (mytoupper(*pAsc) != 'R'))
    return False;

  *pErg = RegVal(pAsc + 1, l - 1);
  return (*pErg != 0xff);
}

static Boolean DecodeReg(const char *pAsc, Byte *pErg)
{
  char *s;

  if (FindRegDef(pAsc, &s))
    pAsc = s;

  return DecodeRegCore(pAsc, pErg, strlen(pAsc));
}

static Boolean DecodeRReg(char *pAsc, Byte *pErg)
{
  Byte h;
  char *s, *pPair, Save;
  int l;

  if (FindRegDef(pAsc, &s))
    pAsc = s;

  l = strlen(pAsc);

  /* syntax RnP (n even): */

  if ((l >= 3) && (l <= 4)
   && (mytoupper(pAsc[l -1]) == 'P')
   && DecodeRegCore(pAsc, pErg, l - 1))
    return !Odd(*pErg);

  /* syntax RnRn+1 */

  if ((l < 4) || (l > 6) || (mytoupper(*pAsc) != 'R'))
    return False;

  for (pPair = pAsc + 1; *pPair; pPair++)
    if (mytoupper(*pPair) == 'R')
      break;
  if ((!*pPair) || (pPair - pAsc < 2) || (pPair - pAsc >= l - 1))
    return False;

  Save = *pPair; *pPair = '\0';
  *pErg = RegVal(pAsc + 1, pPair - pAsc - 1);
  h = RegVal(pPair + 1, l - (pPair - pAsc + 1));
  *pPair = Save;
  return (*pErg != 0xff) && (h != 0xff) && (Odd(h)) && (*pErg + 1 == h);
}

/*---------------------------------------------------------------------------*/
/* Hilfsdekoder */

static void DecodeFixed(Word Code)
{
  CPUVar MinCPU = CPU4004 + (CPUVar)Hi(Code);

  if (ChkArgCnt(0, 0)
   && ChkMinCPU(MinCPU))
  {
    BAsmCode[0] = Lo(Code);
    CodeLen = 1;
  }
}

static void DecodeOneReg(Word Code)
{
  Byte Erg;

  if (!ChkArgCnt(1, 1));
  else if (!DecodeReg(ArgStr[1].Str, &Erg)) WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
  else
  {
    BAsmCode[0] = Lo(Code) + Erg;
    CodeLen = 1;
  }
}

static void DecodeOneRReg(Word Code)
{
  Byte Erg;

  if (!ChkArgCnt(1, 1));
  else if (!DecodeRReg(ArgStr[1].Str, &Erg)) WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
  else
  {
    BAsmCode[0] = Lo(Code) + Erg;
    CodeLen = 1;
  }
}

static void DecodeAccReg(Word Code)
{
  Byte Erg;

  if (!ChkArgCnt(1, 2));
  else if ((ArgCnt == 2) && (as_strcasecmp(ArgStr[1].Str, "A"))) WrError(ErrNum_InvAddrMode);
  else if (!DecodeReg(ArgStr[ArgCnt].Str, &Erg)) WrStrErrorPos(ErrNum_InvReg, &ArgStr[ArgCnt]);
  else
  {
    BAsmCode[0] = Lo(Code) + Erg;
    CodeLen = 1;
  }
}

static void DecodeImm4(Word Code)
{
  Boolean OK;

  if (ChkArgCnt(1, 1))
  {
    BAsmCode[0] = EvalStrIntExpression(&ArgStr[1], UInt4, &OK);
    if (OK)
    {
      BAsmCode[0] += Lo(Code);
      CodeLen = 1;
    }
  }
}

static void DecodeFullJmp(Word Index)
{
  Word Adr;
  Boolean OK;

  if (ChkArgCnt(1, 1))
  {
    Adr = EvalStrIntExpression(&ArgStr[1], UInt12, &OK);
    if (OK)
    {
      BAsmCode[0] = 0x40 + (Index << 4) + Hi(Adr);
      BAsmCode[1] = Lo(Adr);
      CodeLen = 2;
    }
  }
}

static void DecodeISZ(Word Index)
{
  Word Adr;
  Boolean OK;
  Byte Erg;
  UNUSED(Index);

  if (!ChkArgCnt(2, 2));
  else if (!DecodeReg(ArgStr[1].Str, &Erg)) WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
  else
  {
    FirstPassUnknown = False;
    Adr = EvalStrIntExpression(&ArgStr[2], UInt12, &OK);
    if (OK && ChkSamePage(EProgCounter() + 1, Adr, 8))
    {
      BAsmCode[0] = 0x70 + Erg;
      BAsmCode[1] = Lo(Adr);
      CodeLen = 2;
    }
  }
}

static void DecodeJCN(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(2, 2))
  {
    Boolean OK = True;
    char *pCond;

    BAsmCode[0] = 0;
    for (pCond = ArgStr[1].Str; *pCond; pCond++)
      switch (mytoupper(*pCond))
      {
        case 'Z': BAsmCode[0] |= 4; break;
        case 'C': BAsmCode[0] |= 2; break;
        case 'T': BAsmCode[0] |= 1; break;
        case 'N': BAsmCode[0] |= 8; break;
        default: OK = False;
      }
    if (!OK)
      BAsmCode[0] = EvalStrIntExpression(&ArgStr[1], UInt4, &OK);

    if (OK)
    {
      Word AdrInt;

      AdrInt = EvalStrIntExpression(&ArgStr[2], UInt12, &OK);
      if (OK)
      {
        if ((!SymbolQuestionable) && (Hi(EProgCounter() + 2) != Hi(AdrInt))) WrError(ErrNum_JmpDistTooBig);
        else
        {
          BAsmCode[0] |= 0x10;
          BAsmCode[1] = Lo(AdrInt);
          CodeLen = 2;
        }
      }
    }
  }
}

static void DecodeFIM(Word Index)
{
  Boolean OK;
  UNUSED(Index);

  if (!ChkArgCnt(2, 2));
  else if (!DecodeRReg(ArgStr[1].Str, BAsmCode)) WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
  else
  {
    BAsmCode[1] = EvalStrIntExpression(&ArgStr[2], Int8, &OK);
    if (OK)
    {
      BAsmCode[0] |= 0x20;
      CodeLen = 2;
    }
  }
}

static void DecodeDATA_4004(Word Code)
{
  UNUSED(Code); 

  DecodeDATA(Int8, Int4);
}

static void DecodeREG(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(1, 1))
    AddRegDef(LabPart.Str, ArgStr[1].Str);
}

/*---------------------------------------------------------------------------*/
/* Codetabellenverwaltung */

static void AddFixed(char *NName, Word NCode, CPUVar NMin)
{
  NCode |= ((Word)(NMin - CPU4004)) << 8;
  AddInstTable(InstTable, NName, NCode, DecodeFixed);
}

static void AddOneReg(char *NName, Byte NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeOneReg);
}

static void AddOneRReg(char *NName, Byte NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeOneRReg);
}

static void AddAccReg(char *NName, Byte NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeAccReg);
}

static void AddImm4(char *NName, Byte NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeImm4);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(101);

  AddFixed("NOP" , 0x00, CPU4004); AddFixed("WRM" , 0xe0, CPU4004);
  AddFixed("WMP" , 0xe1, CPU4004); AddFixed("WRR" , 0xe2, CPU4004);
  AddFixed("WPM" , 0xe3, CPU4004); AddFixed("WR0" , 0xe4, CPU4004);
  AddFixed("WR1" , 0xe5, CPU4004); AddFixed("WR2" , 0xe6, CPU4004);
  AddFixed("WR3" , 0xe7, CPU4004); AddFixed("SBM" , 0xe8, CPU4004);
  AddFixed("RDM" , 0xe9, CPU4004); AddFixed("RDR" , 0xea, CPU4004);
  AddFixed("ADM" , 0xeb, CPU4004); AddFixed("RD0" , 0xec, CPU4004);
  AddFixed("RD1" , 0xed, CPU4004); AddFixed("RD2" , 0xee, CPU4004);
  AddFixed("RD3" , 0xef, CPU4004); AddFixed("CLB" , 0xf0, CPU4004);
  AddFixed("CLC" , 0xf1, CPU4004); AddFixed("IAC" , 0xf2, CPU4004);
  AddFixed("CMC" , 0xf3, CPU4004); AddFixed("CMA" , 0xf4, CPU4004);
  AddFixed("RAL" , 0xf5, CPU4004); AddFixed("RAR" , 0xf6, CPU4004);
  AddFixed("TCC" , 0xf7, CPU4004); AddFixed("DAC" , 0xf8, CPU4004);
  AddFixed("TCS" , 0xf9, CPU4004); AddFixed("STC" , 0xfa, CPU4004);
  AddFixed("DAA" , 0xfb, CPU4004); AddFixed("KBP" , 0xfc, CPU4004);
  AddFixed("DCL" , 0xfd, CPU4004); AddFixed("AD0" , 0xec, CPU4004);
  AddFixed("AD1" , 0xed, CPU4004); AddFixed("AD2" , 0xee, CPU4004);
  AddFixed("AD3" , 0xef, CPU4004);

  AddFixed("HLT" , 0x01, CPU4040); AddFixed("BBS" , 0x02, CPU4040);
  AddFixed("LCR" , 0x03, CPU4040); AddFixed("OR4" , 0x04, CPU4040);
  AddFixed("OR5" , 0x05, CPU4040); AddFixed("AN6" , 0x06, CPU4040);
  AddFixed("AN7" , 0x07, CPU4040); AddFixed("DB0" , 0x08, CPU4040);
  AddFixed("DB1" , 0x09, CPU4040); AddFixed("SB0" , 0x0a, CPU4040);
  AddFixed("SB1" , 0x0b, CPU4040); AddFixed("EIN" , 0x0c, CPU4040);
  AddFixed("DIN" , 0x0d, CPU4040); AddFixed("RPM" , 0x0e, CPU4040);

  AddOneReg("INC" , 0x60);

  AddOneRReg("SRC" , 0x21);
  AddOneRReg("FIN" , 0x30);
  AddOneRReg("JIN" , 0x31);

  AddAccReg("ADD" , 0x80); AddAccReg("SUB" , 0x90);
  AddAccReg("LD"  , 0xa0); AddAccReg("XCH" , 0xb0);

  AddImm4("BBL" , 0xc0); AddImm4("LDM" , 0xd0);

  AddInstTable(InstTable,"JCN", 0, DecodeJCN);
  AddInstTable(InstTable,"JCM", 0, DecodeJCN);
  AddInstTable(InstTable,"JUN", 0, DecodeFullJmp);
  AddInstTable(InstTable,"JMS", 1, DecodeFullJmp);
  AddInstTable(InstTable,"ISZ", 0, DecodeISZ);
  AddInstTable(InstTable,"FIM", 0, DecodeFIM);

  AddInstTable(InstTable, "DS", 0, DecodeRES);
  AddInstTable(InstTable, "DATA", 0, DecodeDATA_4004);
  AddInstTable(InstTable, "REG", 0, DecodeREG);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*---------------------------------------------------------------------------*/
/* Callbacks */

static void MakeCode_4004(void)
{
  CodeLen = 0;
  DontPrint = False;

  /* zu ignorierendes */

  if (Memo(""))
    return;

  if (!LookupInstTable(InstTable, OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
}

static Boolean IsDef_4004(void)
{
  return Memo("REG");
}

static void SwitchFrom_4004(void)
{
  DeinitFields();
}

static void SwitchTo_4004(void)
{
  PFamilyDescr FoundDescr;

  FoundDescr = FindFamilyByName("4004/4040");

  TurnWords = False;
  ConstMode = ConstModeIntel;

  PCSymbol = "$";
  HeaderID = FoundDescr->Id;
  NOPCode = 0x00;
  DivideChars = ",";
  HasAttrs = False;

  ValidSegs = (1 << SegCode) | (1 << SegData);
  Grans[SegCode ] = 1; ListGrans[SegCode ] = 1; SegInits[SegCode ] = 0;
  SegLimits[SegCode] = 0xfff;
  Grans[SegData ] = 1; ListGrans[SegData ] = 1; SegInits[SegData ] = 0;
  SegLimits[SegData] = 0xff;

  MakeCode = MakeCode_4004;
  IsDef = IsDef_4004;
  SwitchFrom = SwitchFrom_4004;

  InitFields();
}

/*---------------------------------------------------------------------------*/
/* Initialisierung */

void code4004_init(void)
{
  CPU4004 = AddCPU("4004", SwitchTo_4004);
  CPU4040 = AddCPU("4040", SwitchTo_4004);
}
