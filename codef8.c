/* codef8.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Code Generator Fairchild F8                                               */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <string.h>
#include <ctype.h>

#include "bpemu.h"
#include "strutil.h"
#include "chunks.h"
#include "headids.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "intpseudo.h"
#include "codevars.h"
#include "errmsg.h"

#include "codef8.h"

#define Flg_IO (1 << 8)
#define Flg_Code (1 << 9)
#define Flg_CMOS (1 << 10)

static CPUVar CPU3870, CPU3872, CPU3873, CPU3874, CPU3875, CPU3876,
              CPU38C70;

/*---------------------------------------------------------------------------*/

static Boolean DecodeReg(const tStrComp *pArg, Byte *pResult)
{
  /* KU/KL/QU/QL are addressed via dedicated LR instructions! */
  static const char *pRegNames[] = { "J", "HU", "HL", "S", "I", "D", NULL };
  int z;
  Boolean OK;

  for (z = 0; pRegNames[z]; z++)
    if (!strcasecmp(pRegNames[z], pArg->Str))
    {
      *pResult = z + 9;
      return True;
    }
  *pResult = EvalStrIntExpression(pArg, UInt4, &OK);
  return OK;
}

static Boolean DecodeAux(const char *pArg, Byte *pResult)
{
  if (strlen(pArg) != 2)
    return False;
  *pResult = 0;

  if (toupper(pArg[0]) == 'Q')
    *pResult |= 2;
  else if (toupper(pArg[0]) != 'K')
    return False;
  if (toupper(pArg[1]) == 'L')
    *pResult |= 1;
  else if (toupper(pArg[1]) != 'U')
    return False;
  return True;
}

static Boolean ArgPair(const char *pArg1, const char *pArg2)
{
  return !strcasecmp(ArgStr[1].Str, pArg1) && !strcasecmp(ArgStr[2].Str, pArg2);
}

/*---------------------------------------------------------------------------*/

static void DecodeFixed(Word Code)
{
  Boolean NeedCMOS = !!(Code & Flg_CMOS);

  if (!ChkArgCnt(0, 0));
  else if (NeedCMOS && (MomCPU != CPU38C70)) WrError(ErrNum_InstructionNotSupported);
  else
  {
    BAsmCode[0] = Code;
    CodeLen = 1;
  }
}

static void DecodeImm8(Word Code)
{
  Boolean IsIO = !!(Code & Flg_IO);

  if (ChkArgCnt(1, 1))
  {
    Boolean OK;

    BAsmCode[1] = EvalStrIntExpression(&ArgStr[1], IsIO ? UInt8 : Int8, &OK);
    if (OK)
    {
      if (IsIO)
        ChkSpace(SegIO);
      BAsmCode[0] = Lo(Code);
      CodeLen = 2;
    }
  }
}

static void DecodeImm4(Word Code)
{
  Boolean IsIO = !!(Code & Flg_IO);

  if (ChkArgCnt(1, 1))
  {
    Boolean OK;

    BAsmCode[0] = Lo(Code) | EvalStrIntExpression(&ArgStr[1], UInt4, &OK);
    if (OK)
    {
      CodeLen = 1;
      if (IsIO)
        ChkSpace(SegIO);
    }
  }
}

static void DecodeImm3(Word Code)
{
  if (ChkArgCnt(1, 1))
  {
    Boolean OK;

    BAsmCode[0] = Code | EvalStrIntExpression(&ArgStr[1], UInt3, &OK);
    if (OK)
      CodeLen = 1;
  }
}

static void DecodeImm16(Word Code)
{
  Boolean IsCode = !!(Code & Flg_Code);

  if (ChkArgCnt(1, 1))
  {
    Boolean OK;
    Word Arg;

    Arg = EvalStrIntExpression(&ArgStr[1], IsCode ? UInt16 : Int16, &OK);
    if (OK)
    {
      BAsmCode[0] = Code;
      BAsmCode[1] = Hi(Arg);
      BAsmCode[2] = Lo(Arg);
      CodeLen = 3;
      if (IsCode)
        ChkSpace(SegCode);
    }
  }
}

static void DecodeOneReg(Word Code)
{
  if (ChkArgCnt(1, 1) && DecodeReg(&ArgStr[1], &BAsmCode[0]))
  {
    BAsmCode[0] |= Code;
    CodeLen = 1;
  }
}

static void DecodeBranchCore(const tStrComp *pArg, Word Code)
{
  Boolean OK;
  LongInt Dist;
  
  Dist = EvalStrIntExpression(pArg, UInt16, &OK) - (EProgCounter() + 1);
  if (OK)
  {
    if (!SymbolQuestionable && ((Dist < -128) || (Dist > 127))) WrStrErrorPos(ErrNum_JmpDistTooBig, pArg);
    else
    {
      BAsmCode[0] = Code;
      BAsmCode[1] = Dist & 0xff;
      CodeLen = 2;
    }
  } 
}

static void DecodeBranch(Word Code)
{
  if (ChkArgCnt(1, 1))
    DecodeBranchCore(&ArgStr[1], Code);
}

static void DecodeGenBranch(Word Code)
{
  if (ChkArgCnt(2, 2))
  {
    Boolean OK;

    /* BT does not include OVF flag in its condition mask, only values 0..7! */

    Code |= EvalStrIntExpression(&ArgStr[1], (Code == 0x90) ? UInt4 : UInt3, &OK);
    if (OK)
      DecodeBranchCore(&ArgStr[2], Code);
  }
}

static void DecodeLR(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(2,2))
    return;

  if (!strcasecmp(ArgStr[1].Str, "A"))
  {
    if (DecodeAux(ArgStr[2].Str, &BAsmCode[0]))
    {
      CodeLen = 1;
    }
    else if (!strcasecmp(ArgStr[2].Str, "IS"))
    {
      BAsmCode[0] = 0x0a;
      CodeLen = 1;
    }
    else if (DecodeReg(&ArgStr[2], &BAsmCode[0]))
    {
      BAsmCode[0] |= 0x40;
      CodeLen = 1;
    }
  }
  else if (!strcasecmp(ArgStr[2].Str, "A"))
  {
    if (DecodeAux(ArgStr[1].Str, &BAsmCode[0]))
    {
      BAsmCode[0] |= 4;
      CodeLen = 1;
    }
    else if (!strcasecmp(ArgStr[1].Str, "IS"))
      BAsmCode[CodeLen++] = 0x0b;
    else if (DecodeReg(&ArgStr[1], &BAsmCode[0]))
    {
      BAsmCode[0] |= 0x50;
      CodeLen = 1;
    }
  }
  else if (ArgPair("DC", "Q"))
    BAsmCode[CodeLen++] = 0x0f;
  else if (ArgPair("DC", "H"))
    BAsmCode[CodeLen++] = 0x10;
  else if (ArgPair("Q", "DC"))
    BAsmCode[CodeLen++] = 0x0e;
  else if (ArgPair("H", "DC"))
    BAsmCode[CodeLen++] = 0x11;
  else if (ArgPair("K", "P"))
    BAsmCode[CodeLen++] = 0x08;
  else if (ArgPair("P", "K"))
    BAsmCode[CodeLen++] = 0x09;
  else if (ArgPair("P0", "Q"))
    BAsmCode[CodeLen++] = 0x0d;
  else if (ArgPair("J", "W"))
    BAsmCode[CodeLen++] = 0x1e;
  else if (ArgPair("W", "J"))
    BAsmCode[CodeLen++] = 0x1d;
  else
    WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[1]);
}

static void DecodeShift(Word Code)
{
  if (!ArgCnt)
    BAsmCode[CodeLen++] = Code;
  else if (ChkArgCnt(1,1))
  {
    Boolean OK;
    Byte Cnt;

    FirstPassUnknown = False;
    Cnt = EvalStrIntExpression(&ArgStr[1], UInt3, &OK);
    if (OK)
    {
      if (Cnt == 4)
        BAsmCode[CodeLen++] = Code + 2;
      else if (FirstPassUnknown || (Cnt == 1))
        BAsmCode[CodeLen++] = Code;
      else
        WrStrErrorPos(ErrNum_OverRange, &ArgStr[1]);
    }
  }
}

static void DecodePORT(Word Code)
{
  UNUSED(Code);

  CodeEquate(SegIO, 0, 0xff);
}

/*---------------------------------------------------------------------------*/

static void AddFixed(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeFixed);
}

static void AddImm8(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeImm8);
}

static void AddImm4(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeImm4);
}

static void AddImm3(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeImm3);
}

static void AddImm16(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeImm16);
}

static void AddOneReg(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeOneReg);
}

static void AddBranch(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeBranch);
}

static void AddGenBranch(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeGenBranch);
}

static void AddShift(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeShift);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(101);

  AddFixed("ADC", 0x8e);
  AddFixed("AM" , 0x88);
  AddFixed("AMD", 0x89);
  AddFixed("CLR", 0x70);
  AddFixed("CM" , 0x8d);
  AddFixed("COM", 0x18);
  AddFixed("DI" , 0x1a);
  AddFixed("EI" , 0x1b);
  AddFixed("HET", Flg_CMOS | 0x2e); /* guess */
  AddFixed("HAL", Flg_CMOS | 0x2f); /* guess */
  AddFixed("INC", 0x1f);
  AddFixed("LM" , 0x16);
  AddFixed("LNK", 0x19);
  AddFixed("NM" , 0x8a);
  AddFixed("NOP", NOPCode);
  AddFixed("OM" , 0x8b);
  AddFixed("PK" , 0x0c);
  AddFixed("POP", 0x1c);
  AddFixed("ST" , 0x17);
  AddFixed("XDC", 0x2c);
  AddFixed("XM" , 0x8c);

  AddImm8("AI" , 0x24);
  AddImm8("CI" , 0x25);
  AddImm8("LI" , 0x20);
  AddImm8("NI" , 0x21);
  AddImm8("IN" , Flg_IO | 0x26);
  AddImm8("OI" , 0x22);
  AddImm8("OUT", Flg_IO | 0x27);
  AddImm8("XI" , 0x23);

  AddImm4("LIS" , 0x70);
  AddImm4("INS" , Flg_IO | 0xa0);
  AddImm4("OUTS", Flg_IO | 0xb0);

  AddImm3("LISL", 0x68);
  AddImm3("LISU", 0x60);

  AddImm16("DCI", 0x2a);
  AddImm16("JMP", Flg_Code | 0x29);
  AddImm16("PI" , Flg_Code | 0x28);

  AddOneReg("AS"  , 0xc0);
  AddOneReg("ASD" , 0xd0);
  AddOneReg("DS"  , 0x30);
  AddOneReg("NS"  , 0xf0);
  AddOneReg("XS"  , 0xe0);

  AddBranch("BC"  , 0x82);
  AddBranch("BM"  , 0x91);
  AddBranch("BNC" , 0x92);
  AddBranch("BNO" , 0x98);
  AddBranch("BNZ" , 0x94);
  AddBranch("BP"  , 0x81);
  AddBranch("BR"  , 0x90);
  AddBranch("BR7" , 0x8f);
  AddBranch("BZ"  , 0x84);
  AddGenBranch("BF", 0x90);
  AddGenBranch("BT", 0x80);

  AddShift("SL", 0x13);
  AddShift("SR", 0x12);

  AddInstTable(InstTable, "LR", 0, DecodeLR);
  AddInstTable(InstTable, "PORT", 0, DecodePORT);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*---------------------------------------------------------------------------*/

static void MakeCode_F8(void)
{
  CodeLen = 0; DontPrint = False;

  /* zu ignorierendes */

  if (Memo("")) return;

  /* Pseudoanweisungen - DS is a machine instruction on F8 */

  if (!Memo("DS"))
  {
    if (DecodeIntelPseudo(True)) return;
  }

  if (!LookupInstTable(InstTable, OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
}

static void SwitchFrom_F8(void)
{
  DeinitFields();
}

static Boolean IsDef_F8(void)
{
  return Memo("PORT");
}

static void SwitchTo_F8(void)
{
  PFamilyDescr Descr;

  TurnWords = False;
  ConstMode = ConstModeIntel;
  SetIsOccupied = False;

  Descr = FindFamilyByName("F8");
  PCSymbol = "$";
  HeaderID = Descr->Id;
  NOPCode = 0x2b;
  DivideChars = ",";
  HasAttrs = False;

  ValidSegs = (1 << SegCode) | (1 << SegData) | (1 << SegIO);
  Grans[SegCode] = 1; ListGrans[SegCode] = 1; SegLimits[SegCode] = 0xfff;
  Grans[SegData] = 1; ListGrans[SegData] = 1; SegLimits[SegData] = 0x3f;
  Grans[SegIO  ] = 1; ListGrans[SegIO  ] = 1; SegLimits[SegIO  ] = 0xff;

  MakeCode = MakeCode_F8;
  SwitchFrom = SwitchFrom_F8;
  IsDef = IsDef_F8;
  InitFields();
}

void codef8_init(void)
{
  CPU3870 = AddCPU("MK3870", SwitchTo_F8);
  CPU3872 = AddCPU("MK3872", SwitchTo_F8);
  CPU3873 = AddCPU("MK3873", SwitchTo_F8);
  CPU3874 = AddCPU("MK3874", SwitchTo_F8);
  CPU3875 = AddCPU("MK3875", SwitchTo_F8);
  CPU3876 = AddCPU("MK3876", SwitchTo_F8);
  CPU38C70 = AddCPU("MK38C70", SwitchTo_F8);
}
