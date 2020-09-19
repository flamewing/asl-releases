/* code8008.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Intel 8008                                                  */
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
#include "headids.h"
#include "codevars.h"
#include "intpseudo.h"
#include "errmsg.h"

#include "code8008.h"

/*---------------------------------------------------------------------------*/
/* Variablen */

static CPUVar CPU8008, CPU8008New;

static const char RegNames[] = "ABCDEHLM";

/*---------------------------------------------------------------------------*/
/* Parser */

static Boolean DecodeReg(char *Asc, Byte *pErg)
{
  const char *p;

  if (strlen(Asc) != 1) return False;
 
  p = strchr(RegNames, as_toupper(*Asc));
  if (p != NULL) *pErg = p - RegNames;
  return (p != NULL);
}

/*---------------------------------------------------------------------------*/
/* Hilfsdekoder */

static void DecodeFixed(Word Index)
{
  if (ChkArgCnt(0, 0))
  {
    BAsmCode[0] = Index;
    CodeLen = 1;
  }
}

static void DecodeImm(Word Index)
{
  if (ChkArgCnt(1, 1))
  {
    Boolean OK;

    BAsmCode[1] = EvalStrIntExpression(&ArgStr[1], Int8, &OK);
    if (OK)
    {
      BAsmCode[0] = Index;
      CodeLen = 2;
    }
  }
}

static void DecodeJmp(Word Index)
{
  if (ChkArgCnt(1, 1))
  {
    tEvalResult EvalResult;
    Word AdrWord;

    AdrWord = EvalStrIntExpressionWithResult(&ArgStr[1], UInt14, &EvalResult);
    if (EvalResult.OK)
    {
      BAsmCode[0] = Index;
      BAsmCode[1] = Lo(AdrWord);
      BAsmCode[2] = Hi(AdrWord) & 0x3f;
      CodeLen = 3;
      ChkSpace(SegCode, EvalResult.AddrSpaceMask);
    }
  }
}

static void DecodeRST(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(1, 1))
  {
    Word AdrWord;
    tEvalResult EvalResult;

    AdrWord = EvalStrIntExpressionWithResult(&ArgStr[1], UInt14, &EvalResult);
    if (mFirstPassUnknown(EvalResult.Flags)) AdrWord &= 0x38;
    if (EvalResult.OK)
    {
      if (ChkRange(AdrWord, 0, 0x38))
      {
        if (AdrWord < 8)
        {
          BAsmCode[0] = 0x05 | (AdrWord << 3);
          CodeLen = 1;
          ChkSpace(SegCode, EvalResult.AddrSpaceMask);
        }
        else if ((AdrWord & 7) != 0) WrError(ErrNum_NotAligned);
        else
        {
          BAsmCode[0] = AdrWord + 0x05;
          CodeLen = 1;
          ChkSpace(SegCode, EvalResult.AddrSpaceMask);
        }
      }
    }
  }
}

static void DecodeINP(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(1, 1))
  {
    tEvalResult EvalResult;

    BAsmCode[0] = 0x41 | (EvalStrIntExpressionWithResult(&ArgStr[1], UInt3, &EvalResult) << 1);
    if (EvalResult.OK)
    {
      CodeLen = 1;
      ChkSpace(SegIO, EvalResult.AddrSpaceMask);
    }
  }
}

static void DecodeOUT(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(1, 1))
  {
    Byte Addr;
    tEvalResult EvalResult;

    Addr = EvalStrIntExpressionWithResult(&ArgStr[1], UInt5, &EvalResult);
    if (mFirstPassUnknown(EvalResult.Flags))
      Addr |= 0x08;
    if (EvalResult.OK)
    {
      if (Addr < 8) WrError(ErrNum_UnderRange);
      else
      {
        BAsmCode[0] = 0x41 | (Addr << 1);
        CodeLen = 1;
        ChkSpace(SegIO, EvalResult.AddrSpaceMask);
      }
    }
  }
}

static void DecodeMOV(Word Index)
{
  Byte SReg, DReg;

  UNUSED(Index);

  if (!ChkArgCnt(2, 2));
  else if (!DecodeReg(ArgStr[1].Str, &DReg)) WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
  else if (!DecodeReg(ArgStr[2].Str, &SReg)) WrStrErrorPos(ErrNum_InvReg, &ArgStr[2]);
  else if ((DReg == 7) && (SReg == 7)) WrError(ErrNum_InvRegPair); /* MOV M,M not allowed - asame opcode as HLT */
  else
  {
    BAsmCode[0] = 0xc0 | (DReg << 3) | SReg;
    CodeLen = 1;
  }
}

static void DecodeMVI(Word Index)
{
  Byte DReg;

  UNUSED(Index);

  if (!ChkArgCnt(2, 2));
  else if (!DecodeReg(ArgStr[1].Str, &DReg)) WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
  else
  {
    Boolean OK;

    BAsmCode[1] = EvalStrIntExpression(&ArgStr[2], Int8, &OK);
    if (OK)
    {
      BAsmCode[0] = 0x06 | (DReg << 3);
      CodeLen = 2;
    }
  }
}

static void DecodeLXI(Word Index)
{
  Byte DReg;

  UNUSED(Index);

  if (!ChkArgCnt(2, 2));
  else if (!DecodeReg(ArgStr[1].Str, &DReg)) WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
  else if ((DReg != 1) && (DReg != 3) && (DReg != 5)) WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
  else
  {
    Boolean OK;
    Word Val;

    Val = EvalStrIntExpression(&ArgStr[2], Int16, &OK);
    if (OK)
    {
      BAsmCode[2] = 0x06 | ((DReg + 1) << 3);
      BAsmCode[3] = Lo(Val);
      BAsmCode[0] = 0x06 | (DReg << 3);
      BAsmCode[1] = Hi(Val);
      CodeLen = 4;
    }
  }
}

static void DecodeSingleReg(Word Index)
{
  Byte Reg, Opcode = Lo(Index), Shift = Hi(Index) & 7;
  Boolean NoAM = (Index & 0x8000) || False;

  if (!ChkArgCnt(1, 1));
  else if (!DecodeReg(ArgStr[1].Str, &Reg)) WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
  else if (NoAM && ((Reg == 0) || (Reg == 7))) WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
  else
  {
    BAsmCode[0] = Opcode | (Reg << Shift);
    CodeLen = 1;
  }
}

/*---------------------------------------------------------------------------*/
/* Codetabellenverwaltung */

static const char FlagNames[] = "CZSP";

static void AddFixed(const char *NName, Byte NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeFixed);
}

static void AddFixeds(const char *NName, Byte NCode, int Shift, Byte RegMask)
{
  char Memo[10], *p;
  int Reg;

  strcpy(Memo, NName); p = strchr(Memo, '*');
  for (Reg = 0; Reg < 8; Reg++)
    if ((1 << Reg) & RegMask)
    {
      *p = RegNames[Reg];
      AddFixed(Memo, NCode + (Reg << Shift));
    }
}

static void AddImm(const char *NName, Byte NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeImm);
}

static void AddImms(const char *NName, Byte NCode, int Pos)
{
  char Memo[10], *p;
  int z;

  strcpy(Memo, NName); p = strchr(Memo, '*');
  for (z = 0; z < 8; z++)
  {
    *p = RegNames[z];
    AddImm(Memo, NCode + (z << Pos));
  }
}

static void AddJmp(const char *NName, Byte NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeJmp);
}

static void AddJmps(const char *NName, Byte NCode, int Pos)
{
  char Memo[10], *p;
  int z;
   
  strcpy(Memo, NName); p = strchr(Memo, '*');
  for (z = 0; z < 4; z++) 
  {
    *p = FlagNames[z];  
    AddJmp(Memo, NCode + (z << Pos));
  }
}

static void InitFields(void)
{
  Boolean New = (MomCPU == CPU8008New);

  SetDynamicInstTable(InstTable = CreateInstTable(503));

  AddFixed("HLT", 0x00);
  AddFixed("NOP", 0xc0); /* = MOV A,A */

  AddInstTable(InstTable, New ? "IN" : "INP", 0, DecodeINP);
  AddInstTable(InstTable, "OUT", 0, DecodeOUT);

  AddJmp ("JMP", 0x44);
  if (New)
  {
    AddJmp ("JNC", 0x40);
    AddJmp ("JNZ", 0x48);
    AddJmp ("JP" , 0x50);
    AddJmp ("JPO", 0x58);
    AddJmp ("JM" , 0x70);
    AddJmp ("JPE", 0x78);
  }
  else
  {
    AddJmp ("JS" , 0x70);
    AddJmp ("JP" , 0x78);
    AddJmps("JF*", 0x40, 3);
    AddJmps("JT*", 0x60, 3);
  }
  AddJmp ("JC" , 0x60);
  AddJmp ("JZ" , 0x68);

  if (New)
  {
    AddJmp ("CALL", 0x46);
    AddJmp ("CNC", 0x42);
    AddJmp ("CNZ", 0x4a);
    AddJmp ("CP" , 0x52);
    AddJmp ("CPO", 0x5a);
    AddJmp ("CM" , 0x72);
    AddJmp ("CPE", 0x7a);
  }
  else
  {
    AddJmp ("CAL", 0x46);   
    AddJmp ("CS" , 0x72);
    AddJmp ("CP" , 0x7a);
    AddJmps("CF*", 0x42, 3);
    AddJmps("CT*", 0x62, 3);
  }
  AddJmp ("CC" , 0x62);
  AddJmp ("CZ" , 0x6a);

  AddFixed("RET", 0x07);
  if (New)
  {
    AddFixed("RNC", 0x03);
    AddFixed("RNZ", 0x0b);
    AddFixed("RP" , 0x13);
    AddFixed("RPO", 0x1b);
    AddFixed("RM" , 0x33);
    AddFixed("RPE", 0x3b);
  }
  else
  {
    AddFixed("RFC", 0x03);
    AddFixed("RFZ", 0x0b);
    AddFixed("RFS", 0x13);
    AddFixed("RFP", 0x1b);
    AddFixed("RS" , 0x33);
    AddFixed("RP" , 0x3b);
    AddFixed("RTC", 0x23); AddFixed("RTZ", 0x2b);
    AddFixed("RTS", 0x33); AddFixed("RTP", 0x3b);
  }
  AddFixed("RC" , 0x23);
  AddFixed("RZ" , 0x2b);

  AddInstTable(InstTable, "RST", 0, DecodeRST);

  if (New)
    AddInstTable(InstTable, "MOV", 0, DecodeMOV);
  else
  {
    AddFixeds("L*A", 0xc0, 3, 0xff);
    AddFixeds("L*B", 0xc1, 3, 0xff);
    AddFixeds("L*C", 0xc2, 3, 0xff);
    AddFixeds("L*D", 0xc3, 3, 0xff);
    AddFixeds("L*E", 0xc4, 3, 0xff);
    AddFixeds("L*H", 0xc5, 3, 0xff);
    AddFixeds("L*L", 0xc6, 3, 0xff);
    AddFixeds("L*M", 0xc7, 3, 0x7f); /* forbid LMM - would be opcode for HLT */
  }

  if (New)
  {
    AddInstTable(InstTable, "MVI", 0, DecodeMVI);
    AddInstTable(InstTable, "LXI", 0, DecodeLXI);
  }
  else
    AddImms("L*I", 0x06, 3);

  if (New)
  {
    AddInstTable(InstTable, "ADD", 0x0080, DecodeSingleReg);
    AddInstTable(InstTable, "ADC", 0x0088, DecodeSingleReg);
    AddInstTable(InstTable, "SUB", 0x0090, DecodeSingleReg);
    AddInstTable(InstTable, "SBB", 0x0098, DecodeSingleReg);
    AddInstTable(InstTable, "ANA", 0x00a0, DecodeSingleReg);
    AddInstTable(InstTable, "XRA", 0x00a8, DecodeSingleReg);
    AddInstTable(InstTable, "ORA", 0x00b0, DecodeSingleReg);
    AddInstTable(InstTable, "CMP", 0x00b8, DecodeSingleReg);
    AddInstTable(InstTable, "INR", 0x8300, DecodeSingleReg);
    AddInstTable(InstTable, "DCR", 0x8301, DecodeSingleReg);
  }
  else
  {
    AddFixeds("AD*", 0x80, 0, 0xff);
    AddFixeds("AC*", 0x88, 0, 0xff);
    AddFixeds("SU*", 0x90, 0, 0xff);
    AddFixeds("SB*", 0x98, 0, 0xff);
    AddFixeds("NR*", 0xa0, 0, 0xff);
    AddFixeds("ND*", 0xa0, 0, 0xff);
    AddFixeds("XR*", 0xa8, 0, 0xff);
    AddFixeds("OR*", 0xb0, 0, 0xff);
    AddFixeds("CP*", 0xb8, 0, 0xff);
    AddFixeds("IN*", 0x00, 3, 0x7e); /* no INA/INM */
    AddFixeds("DC*", 0x01, 3, 0x7e); /* no DCA/DCM */
  }

  AddImm ("ADI", 0x04);
  AddImm ("ACI", 0x0c);
  AddImm ("SUI", 0x14);
  AddImm ("SBI", 0x1c);
  AddImm (New ? "ANI" : "NDI", 0x24);
  AddImm ("XRI", 0x2c);
  AddImm ("ORI", 0x34);
  AddImm ("CPI", 0x3c);

  AddFixed ("RLC", 0x02);
  AddFixed ("RRC", 0x0a);
  AddFixed ("RAL", 0x12);
  AddFixed ("RAR", 0x1a);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*---------------------------------------------------------------------------*/
/* Callbacks */

static void MakeCode_8008(void)
{
  CodeLen = 0; DontPrint = False;

  /* zu ignorierendes */

  if (Memo("")) return;

  /* Pseudoanweisungen */

  if (DecodeIntelPseudo(False)) return;

  /* der Rest */

  if (!LookupInstTable(InstTable, OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
}

static Boolean IsDef_8008(void)
{
  return False;
}

static void SwitchFrom_8008(void)
{
  DeinitFields();
}

static void SwitchTo_8008(void)
{
  PFamilyDescr FoundDescr;

  FoundDescr = FindFamilyByName("8008");

  TurnWords = False; ConstMode = ConstModeIntel;

  PCSymbol = "$"; HeaderID = FoundDescr->Id; NOPCode = 0xc0;
  DivideChars = ","; HasAttrs = False;

  ValidSegs = (1 << SegCode) | (1 << SegIO);
  Grans[SegCode ] = 1; ListGrans[SegCode ] = 1; SegInits[SegCode ] = 0;
  SegLimits[SegCode] = 0x3fff;
  Grans[SegIO   ] = 1; ListGrans[SegIO   ] = 1; SegInits[SegIO   ] = 0;
  SegLimits[SegIO] = 7;

  MakeCode = MakeCode_8008; IsDef = IsDef_8008;
  SwitchFrom = SwitchFrom_8008;

  InitFields();
}

/*---------------------------------------------------------------------------*/
/* Initialisierung */

void code8008_init(void)
{
  CPU8008    = AddCPU("8008"   , SwitchTo_8008);
  CPU8008New = AddCPU("8008NEW", SwitchTo_8008);
}
