/* codexgate.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator XGATE-Kern                                                  */
/*****************************************************************************/
/* $Id: codexgate.c,v 1.6 2010/04/17 13:14:24 alfred Exp $                   */
/*****************************************************************************
 * $Log: codexgate.c,v $
 * Revision 1.6  2010/04/17 13:14:24  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.5  2007/11/24 22:48:08  alfred
 * - some NetBSD changes
 *
 * Revision 1.4  2007/06/28 20:27:31  alfred
 * - silence some warnings on recent GNU C versions
 *
 * Revision 1.3  2006/08/05 18:06:43  alfred
 * - silence some compiler warnings
 *
 * Revision 1.2  2005/09/12 18:35:21  alfred
 * - add load/store
 *
 * Revision 1.1  2005/09/11 18:10:51  alfred
 * - added XGATE
 *
 *****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "nls.h"
#include "endian.h"
#include "strutil.h"
#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "headids.h"
#include "codepseudo.h"
#include "motpseudo.h"
#include "asmitree.h"
#include "codevars.h"
#include "errmsg.h"

#include "codexgate.h"

/*--------------------------------------------------------------------------*/
/* Variables */

#define FixedOrderCnt 2

static CPUVar CPUXGate;

/*--------------------------------------------------------------------------*/
/* Address Decoders */

static Boolean DecodeReg(char *pAsc, Word *pResult)
{
  Boolean Result;

  if ((strlen(pAsc) != 2) || (mytoupper(*pAsc) != 'R') || (!myisdigit(pAsc[1])))
  {
    *pResult = 0;
    Result = False;
  }
  else
  {
    *pResult =  pAsc[1]- '0';
    Result = *pResult <= 7;
  }
  return Result;
}

static Boolean DecodeArgReg(int Index, Word *pReg)
{
  Boolean Result = DecodeReg(ArgStr[Index], pReg);

  if (!Result)
    WrXErrorPos(ErrNum_InvReg, ArgStr[Index], &ArgStrPos[Index]);
  return Result;
}

/*--------------------------------------------------------------------------*/
/* Instruction Decoders */

static void DecodeFixed(Word Index)
{
  if (ChkArgCnt(0, 0))
  {
    WAsmCode[0] = Index;
    CodeLen = 2;
  }
}

static void DecodeBranch(Word Index)
{
  LongInt Dist;
  Boolean OK;

  if (ChkArgCnt(1, 1))
  {   
    FirstPassUnknown = False;
    Dist = EvalIntExpression(ArgStr[1], UInt16, &OK) - (EProgCounter() + 2);
    if (OK)
    {
      if ((!SymbolQuestionable) && (Dist & 1)) WrError(1325);
      else if ((!SymbolQuestionable) && ((Dist < -512) || (Dist > 510))) WrError(1325);
      else
      {
        WAsmCode[0] = Index | ((Dist >> 1) & 0x01ff);
        CodeLen = 2;
      }
    }
  }
}

static void DecodeBRA(Word Index)
{
  LongInt Dist;
  Boolean OK;  

  UNUSED(Index);

  if (ChkArgCnt(1, 1))
  {   
    FirstPassUnknown = False;
    Dist = EvalIntExpression(ArgStr[1], UInt16, &OK) - (EProgCounter() + 2);
    if (OK)
    {
      if ((!SymbolQuestionable) && (Dist & 1)) WrError(1325);
      else if ((!SymbolQuestionable) && ((Dist < -1024) || (Dist > 1022))) WrError(1325);
      else
      {   
        WAsmCode[0] = 0x3c00 | ((Dist >> 1) & 0x03ff);
        CodeLen = 2;
      }
    }  
  }    
}      
       
static void DecodeShift(Word Index)
{
  Word DReg, SReg;
  Boolean OK;

  if (!ChkArgCnt(2, 2));
  else if (!DecodeArgReg(1, &DReg));
  else if (*ArgStr[2] == '#')
  {
    SReg = EvalIntExpression(ArgStr[2] + 1, UInt4, &OK);
    if (OK)
    {
      WAsmCode[0] = 0x0808 | Index | (DReg << 8) | (SReg << 4);
      CodeLen = 2;
    }
  }
  else if (DecodeArgReg(2, &SReg))
  {
    WAsmCode[0] = 0x0810 | Index | (DReg << 8) | (SReg << 5);
    CodeLen = 2;
  }
}

static void DecodeAriImm(Word Index)
{
  Word DReg, SReg1, SReg2;
  Boolean OK;

  if (!ChkArgCnt(2, 3));
  else if (!DecodeArgReg(1, &DReg));
  else if (ArgCnt == 2)
  {
    if (*ArgStr[2] == '#')
    {
      SReg1 = EvalIntExpression(ArgStr[2] + 1, Int16, &OK);
      if (OK)
      {
        WAsmCode[0] = 0x8000 | (Index << 12) | (DReg << 8) | Lo(SReg1);
        WAsmCode[1] = 0x8800 | (Index << 12) | (DReg << 8) | Hi(SReg1);
        CodeLen = 4;
      }
    }
    else if (DecodeArgReg(2, &SReg1))
    {
      WAsmCode[0] = 0x1000 | ((Index & 4) << 9) | (Index & 3) | (DReg << 8) | (DReg << 5) | (SReg1 << 2);
      CodeLen = 2;
    }
  }
  else if (DecodeArgReg(2, &SReg1) && DecodeArgReg(3, &SReg2))
  {
    WAsmCode[0] = 0x1000 | ((Index & 4) << 9) | (Index & 3) | (DReg << 8) | (SReg1 << 5) | (SReg2 << 2);
    CodeLen = 2;
  }
}

static void DecodeImm8(Word Index)
{
  Word DReg, Src;
  Boolean OK;

  if (!ChkArgCnt(2, 2));
  else if (!DecodeArgReg(1, &DReg));
  else if (*ArgStr[2] != '#') WrError(1120);
  else
  {
    Src = EvalIntExpression(ArgStr[2] + 1, Int8, &OK);
    if (OK)
    {
      WAsmCode[0] = Index | (DReg << 8) | (Src & 0xff);
      CodeLen = 2;
    }
  }
}

static void DecodeReg3(Word Index)
{
  Word DReg, SReg1, SReg2;

  if (ChkArgCnt(3, 3)
   && DecodeArgReg(1, &DReg)
   && DecodeArgReg(2, &SReg1)
   && DecodeArgReg(3, &SReg2))
  {
    WAsmCode[0] = Index | (DReg << 8) | (SReg1 << 5) | (SReg2 << 2);
    CodeLen = 2;
  }
}

static void DecodeReg23(Word Index)
{
  Word DReg, SReg1, SReg2;

  if (!ChkArgCnt(2, 3));
  else if (!DecodeArgReg(1, &DReg));
  else if (!DecodeArgReg(2, &SReg1));
  else if (ArgCnt == 2)
  {
    WAsmCode[0] = Index | (DReg << 8) | (DReg << 5) | (SReg1 << 2);
    CodeLen = 2;
  }
  else if (DecodeArgReg(3, &SReg2))
  {
    WAsmCode[0] = Index | (DReg << 8) | (SReg1 << 5) | (SReg2 << 2);
    CodeLen = 2;
  }
}

static void DecodeCPC(Word Index)
{
  Word DReg, SReg;

  if (ChkArgCnt(2, 3)
   && DecodeArgReg(1, &DReg)
   && DecodeArgReg(2, &SReg))
  {
    WAsmCode[0] = Index | (DReg << 5) | (SReg << 2);
    CodeLen = 2;
  }
}

static void DecodeMOV(Word Index)
{
  Word DReg, SReg;

  if (ChkArgCnt(2, 3)
   && DecodeArgReg(1, &DReg)
   && DecodeArgReg(2, &SReg))
  {
    WAsmCode[0] = Index | (DReg << 8) | (SReg << 2);
    CodeLen = 2;
  }
}

static void DecodeBFFFO(Word Index)
{
  Word DReg, SReg;

  if (ChkArgCnt(2, 3)
   && DecodeArgReg(1, &DReg)
   && DecodeArgReg(2, &SReg))
  {
    WAsmCode[0] = Index | (DReg << 8) | (SReg << 5);
    CodeLen = 2;
  }
}

static void DecodeReg12(Word Index)
{
  Word DReg, SReg;

  if (!ChkArgCnt(1, 2));
  else if (!DecodeArgReg(1, &DReg));
  else if (ArgCnt == 1)
  {
    WAsmCode[0] = Index | (DReg << 8) | (DReg << 2);
    CodeLen = 2;
  }
  else if (DecodeArgReg(2, &SReg))
  {
    WAsmCode[0] = Index | (DReg << 8) | (SReg << 2);
    CodeLen = 2;
  }
}

static void DecodeReg1(Word Index)
{
  Word Reg;

  if (ChkArgCnt(1, 1) && DecodeArgReg(1, &Reg))
  {   
    WAsmCode[0] = Index | (Reg << 8);
    CodeLen = 2;
  }
}

static void DecodeTST(Word Index)
{
  Word Reg;

  if (ChkArgCnt(1, 1) && DecodeArgReg(1, &Reg))
  {   
    WAsmCode[0] = Index | (Reg << 5);
    CodeLen = 2;
  }
}

static void DecodeSem(Word Index)
{
  Word Reg;
  Boolean OK;

  if (!ChkArgCnt(1, 1));
  else if (*ArgStr[1] == '#')
  {
    Reg = EvalIntExpression(ArgStr[1] + 1, UInt3, &OK);
    if (OK)
    {
      WAsmCode[0] = Index | (Reg << 8);
      CodeLen = 2;
    }
  }
  else if (DecodeArgReg(1, &Reg))
  {   
    WAsmCode[0] = Index | (Reg << 8) | 1;
    CodeLen = 2;
  }
}

static void DecodeSIF(Word Index)
{
  Word Reg;

  UNUSED(Index);

  if (ArgCnt == 0)
  {
    WAsmCode[0] = 0x0300;
    CodeLen = 2;
  }
  else if (ChkArgCnt(0, 1) && DecodeArgReg(1, &Reg))
  {
    WAsmCode[0] = 0x00f7 | (Reg << 8);
    CodeLen = 2;
  }
}

static void DecodeTFR(Word Index)
{
  Word Reg;
  int RegIdx = 0;

  UNUSED(Index);

  if (ChkArgCnt(2, 2))
  {
    Boolean OK = True;

    if (!strcasecmp(ArgStr[2], "CCR"))
    {
      WAsmCode[0] = 0x00f8;
      RegIdx = 1;
    }
    else if (!strcasecmp(ArgStr[1], "CCR"))
    {
      WAsmCode[0] = 0x00f9;
      RegIdx = 2;
    }
    else if (!strcasecmp(ArgStr[2], "PC"))
    {
      WAsmCode[0] = 0x00fa;
      RegIdx = 1;
    }
    else
      OK = False;
    
    if (!OK) WrError(1320);
    else if (DecodeArgReg(RegIdx, &Reg))
    {
      WAsmCode[0] |= (Reg << 8);
      CodeLen = 2;
    }
  }
}

static void DecodeCmp(Word Index)
{
  Word DReg, Src;
  Boolean OK;

  UNUSED(Index);  

  if (ChkArgCnt(2, 2) && DecodeArgReg(1, &DReg))
  {
    if (*ArgStr[2] == '#')
    {
      Src = EvalIntExpression(ArgStr[2] + 1, Int16, &OK);
      if (OK)
      {
        WAsmCode[0] = 0xd000 | (DReg << 8) | Lo(Src);
        WAsmCode[1] = 0xd800 | (DReg << 8) | Hi(Src);
        CodeLen = 4;
      }
    }
    else if (DecodeArgReg(2, &Src))
    {
      WAsmCode[0] = 0x1800 | (DReg << 5) | (Src << 2);
      CodeLen = 2;
    }
  }
}

static void DecodeMem(Word Code)
{
  Word DReg;

  if (!ChkArgCnt(2, 2));
  else if (!DecodeArgReg(1, &DReg));
  else if (*ArgStr[2] == '#')
  {
    if (!Memo("LDW")) WrError(1350);
    else
    {
      Word Val;
      Boolean OK;

      Val = EvalIntExpression(ArgStr[2] + 1, Int16, &OK);
      if (OK)
      {
        WAsmCode[0] = 0xf000 | (DReg << 8) | Lo(Val);
        WAsmCode[1] = 0xf800 | (DReg << 8) | Hi(Val);
        CodeLen = 4;
      }
    }
  }
  else if (!IsIndirect(ArgStr[2])) WrError(1350);
  else
  {
    int l = strlen(ArgStr[2]) - 2;
    char *pPos;
    Word Base, Index;
    Boolean OK;

    /* remove parentheses */

    memmove(ArgStr[2], ArgStr[2] + 1, l); ArgStr[2][l] = '\0';

    /* base present? */

    pPos = strchr(ArgStr[2], ',');
    if (pPos)
    {
      *pPos = '\0';
      KillBlanks(ArgStr[2]);
      OK = DecodeReg(ArgStr[2], &Base);
      if (!OK)
        WrXErrorPos(ErrNum_InvReg, ArgStr[2], &ArgStrPos[2]);
      strmov(ArgStr[2], pPos + 1);
    }
    else
    {
      Base = 0;
      OK = True;
    }

    /* go on with index? */

    if (OK)
    {
      KillPrefBlanksPos(ArgStr[2], &ArgStrPos[2]);
      KillPostBlanksPos(ArgStr[2], &ArgStrPos[2]);

      if (*ArgStr[2] == '#')
        Index = EvalIntExpression(ArgStr[2] + 1, UInt5, &OK);
      else if (*ArgStr[2] == '-')
      {
        Code |= 0x2000;
        OK = DecodeReg(ArgStr[2] + 1, &Index);
        if (OK)
          Index = (Index << 2) | 2;
        else
          WrXErrorPos(ErrNum_InvReg, ArgStr[2] + 1, &ArgStrPos[2]);
      }
      else if (((l = strlen(ArgStr[2])) > 1) && (ArgStr[2][l - 1] == '+'))
      {
        Code |= 0x2000;
        ArgStr[2][l - 1] = '\0'; ArgStrPos[2].Len--;
        OK = DecodeReg(ArgStr[2], &Index);
        if (OK)
          Index = (Index << 2) | 1;
        else
          WrXErrorPos(ErrNum_InvReg, ArgStr[2], &ArgStrPos[2]);
      }
      else
      {
        Code |= 0x2000;
        OK = DecodeReg(ArgStr[2], &Index);
        if (OK)
          Index = (Index << 2);
        else
          WrXErrorPos(ErrNum_InvReg, ArgStr[2], &ArgStrPos[2]);
      }

      if (OK)
      {
        WAsmCode[0] = Code | (DReg << 8) | (Base << 5) | Index;
        CodeLen = 2;
      }
    }
  }
}

/*--------------------------------------------------------------------------*/
/* Dynamic Code Table Handling */

static void InitFields(void)
{
  InstTable = CreateInstTable(103);

  AddInstTable(InstTable, "NOP", NOPCode, DecodeFixed);
  AddInstTable(InstTable, "BRK", 0x0000 , DecodeFixed);
  AddInstTable(InstTable, "RTS", 0x0200 , DecodeFixed);

  AddInstTable(InstTable, "BCC", 0x2000 , DecodeBranch);
  AddInstTable(InstTable, "BCS", 0x2200 , DecodeBranch);
  AddInstTable(InstTable, "BEQ", 0x2600 , DecodeBranch);
  AddInstTable(InstTable, "BGE", 0x3400 , DecodeBranch);
  AddInstTable(InstTable, "BGT", 0x3800 , DecodeBranch);
  AddInstTable(InstTable, "BHI", 0x3000 , DecodeBranch);
  AddInstTable(InstTable, "BHS", 0x2000 , DecodeBranch);
  AddInstTable(InstTable, "BLE", 0x3a00 , DecodeBranch);
  AddInstTable(InstTable, "BLO", 0x2200 , DecodeBranch);
  AddInstTable(InstTable, "BLS", 0x3200 , DecodeBranch);
  AddInstTable(InstTable, "BLT", 0x3600 , DecodeBranch);
  AddInstTable(InstTable, "BMI", 0x2a00 , DecodeBranch);
  AddInstTable(InstTable, "BNE", 0x2400 , DecodeBranch);
  AddInstTable(InstTable, "BPL", 0x2800 , DecodeBranch);
  AddInstTable(InstTable, "BVC", 0x2c00 , DecodeBranch);
  AddInstTable(InstTable, "BVS", 0x2e00 , DecodeBranch);

  AddInstTable(InstTable, "BRA", 0      , DecodeBRA   );

  AddInstTable(InstTable, "ASR", 0x0001 , DecodeShift);
  AddInstTable(InstTable, "CSL", 0x0002 , DecodeShift);
  AddInstTable(InstTable, "CSR", 0x0003 , DecodeShift);
  AddInstTable(InstTable, "LSL", 0x0004 , DecodeShift);
  AddInstTable(InstTable, "LSR", 0x0005 , DecodeShift);
  AddInstTable(InstTable, "ROL", 0x0006 , DecodeShift);
  AddInstTable(InstTable, "ROR", 0x0007 , DecodeShift);

  AddInstTable(InstTable, "ADD" , 6, DecodeAriImm);
  AddInstTable(InstTable, "AND" , 0, DecodeAriImm);
  AddInstTable(InstTable, "OR"  , 2, DecodeAriImm);
  AddInstTable(InstTable, "SUB" , 4, DecodeAriImm);
  AddInstTable(InstTable, "XNOR", 3, DecodeAriImm);

  AddInstTable(InstTable, "ADDH" , 0xe800, DecodeImm8);
  AddInstTable(InstTable, "ADDL" , 0xe000, DecodeImm8);
  AddInstTable(InstTable, "ANDH" , 0x8800, DecodeImm8);
  AddInstTable(InstTable, "ANDL" , 0x8000, DecodeImm8);
  AddInstTable(InstTable, "BITH" , 0x9800, DecodeImm8);
  AddInstTable(InstTable, "BITL" , 0x9000, DecodeImm8);
  AddInstTable(InstTable, "CMPL" , 0xd000, DecodeImm8);
  AddInstTable(InstTable, "CPCH" , 0xd800, DecodeImm8);
  AddInstTable(InstTable, "ORH"  , 0xa800, DecodeImm8);
  AddInstTable(InstTable, "ORL"  , 0xa000, DecodeImm8);
  AddInstTable(InstTable, "SUBH" , 0xc800, DecodeImm8);
  AddInstTable(InstTable, "SUBL" , 0xc000, DecodeImm8);
  AddInstTable(InstTable, "XNORH", 0xb800, DecodeImm8);
  AddInstTable(InstTable, "XNORL", 0xb000, DecodeImm8);
  AddInstTable(InstTable, "LDH"  , 0xf800, DecodeImm8);
  AddInstTable(InstTable, "LDL"  , 0xf000, DecodeImm8);

  AddInstTable(InstTable, "BFEXT" , 0x6003, DecodeReg3);
  AddInstTable(InstTable, "BFINS" , 0x6803, DecodeReg3);
  AddInstTable(InstTable, "BFINSI", 0x7003, DecodeReg3);
  AddInstTable(InstTable, "BFINSX", 0x7803, DecodeReg3);

  AddInstTable(InstTable, "ADC"  , 0x1803, DecodeReg23);
  AddInstTable(InstTable, "SBC"  , 0x1801, DecodeReg23); 

  AddInstTable(InstTable, "CPC"  , 0x1801, DecodeCPC);
  AddInstTable(InstTable, "MOV"  , 0x1002, DecodeMOV);
  AddInstTable(InstTable, "BFFFO", 0x0810, DecodeBFFFO);

  AddInstTable(InstTable, "COM"  , 0x1003, DecodeReg12);
  AddInstTable(InstTable, "NEG"  , 0x1800, DecodeReg12);

  AddInstTable(InstTable, "JAL"  , 0x00f6, DecodeReg1);
  AddInstTable(InstTable, "PAR"  , 0x00f5, DecodeReg1);
  AddInstTable(InstTable, "SEX"  , 0x00f4, DecodeReg1);

  AddInstTable(InstTable, "TST"  , 0x1800, DecodeTST);

  AddInstTable(InstTable, "CSEM" , 0x00f0, DecodeSem);
  AddInstTable(InstTable, "SSEM" , 0x00f2, DecodeSem);

  AddInstTable(InstTable, "SIF"  , 0     , DecodeSIF);

  AddInstTable(InstTable, "TFR"  , 0     , DecodeTFR);

  AddInstTable(InstTable, "CMP"  , 0     , DecodeCmp);

  AddInstTable(InstTable, "LDB"  , 0x4000, DecodeMem);
  AddInstTable(InstTable, "LDW"  , 0x4800, DecodeMem); 
  AddInstTable(InstTable, "STB"  , 0x5000, DecodeMem);
  AddInstTable(InstTable, "STW"  , 0x5800, DecodeMem);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*--------------------------------------------------------------------------*/
/* Callbacks */

static void MakeCode_XGATE(void)
{
  CodeLen = 0;

  DontPrint = False;

  /* Nullanweisung */

  if ((*OpPart.Str == '\0') && (ArgCnt == 0))
    return;

  /* Pseudoanweisungen */

  if (DecodeMotoPseudo(True))
    return;

  /* Befehlszaehler ungerade ? */

  if (Odd(EProgCounter())) WrError(180);

  /* alles aus der Tabelle */

  if (!LookupInstTable(InstTable,OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownOpcode, &OpPart);
}

static Boolean IsDef_XGATE(void)
{
  return Memo("REG");
}

static void SwitchFrom_XGATE(void)
{
  DeinitFields();
}

static void SwitchTo_XGATE(void)
{
  PFamilyDescr pDescr;

  TurnWords = True; ConstMode = ConstModeMoto; SetIsOccupied = False;

  pDescr = FindFamilyByName("XGATE");
  PCSymbol = "*"; HeaderID = pDescr->Id; NOPCode = 0x0100;
  DivideChars = ","; HasAttrs = False;

  ValidSegs = (1 << SegCode);
  Grans[SegCode] = 1; ListGrans[SegCode] = 2; SegInits[SegCode] = 0;
  SegLimits[SegCode] = 0xffffl;

  MakeCode = MakeCode_XGATE; IsDef = IsDef_XGATE;

  SwitchFrom = SwitchFrom_XGATE; InitFields();
}

/*--------------------------------------------------------------------------*/
/* Initialisierung */  

void codexgate_init(void)
{
  CPUXGate = AddCPU("XGATE", SwitchTo_XGATE);
}
