/* code2650.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Signetics 2650                                              */
/*                                                                           */
/*****************************************************************************/
/* $Id: code2650.c,v 1.6 2014/12/07 19:13:59 alfred Exp $
 *****************************************************************************
 * $Log: code2650.c,v $
 * Revision 1.6  2014/12/07 19:13:59  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.5  2013-02-14 20:48:57  alfred
 * - allow UN as condition
 *
 * Revision 1.4  2012-12-31 11:21:29  alfred
 * - add Dx pseudo instructions to 2650 target
 *
 * Revision 1.3  2007/11/24 22:48:03  alfred
 * - some NetBSD changes
 *
 * Revision 1.2  2006/03/05 18:07:42  alfred
 * - remove double InstTable variable
 *
 * Revision 1.1  2005/12/09 14:48:06  alfred
 * - added 2650
 * 
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "chunks.h"
#include "bpemu.h"
#include "strutil.h"

#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "codevars.h" 
#include "headids.h"
#include "intpseudo.h"
#include "errmsg.h"

#include "code2650.h"

/*--------------------------------------------------------------------------*/
/* Local Variables */

static CPUVar CPU2650;

/*--------------------------------------------------------------------------*/
/* Expression Parsers */

static Boolean DecodeReg(const char *pAsc, Byte *pRes)
{
  Boolean Result;

  Result = ((strlen(pAsc) == 2) && (mytoupper(pAsc[0]) == 'R') && (pAsc[1] >= '0') && (pAsc[1] <= '3'));
  if (Result)
    *pRes = pAsc[1] - '0';
  return Result;
}

static Boolean DecodeCondition(const char *pAsc, Byte *pRes)
{
  Boolean Result = TRUE;

  if (!strcasecmp(pAsc, "EQ"))
    *pRes = 0;
  else if (!strcasecmp(pAsc, "GT"))
    *pRes = 1;
  else if (!strcasecmp(pAsc, "LT"))
    *pRes = 2;
  else if ((!strcasecmp(pAsc, "ALWAYS")) || (!strcasecmp(pAsc, "UN")))
    *pRes = 3;
  else
    Result = FALSE;

  return Result;
}

/*--------------------------------------------------------------------------*/
/* Instruction Decoders */

static void DecodeFixed(Word Index)
{
  if (ChkArgCnt(0, 0))
  {
    BAsmCode[0] = Index; CodeLen = 1;
  }
}

static void DecodeOneReg(Word Index)
{
  Byte Reg;

  if (!ChkArgCnt(1, 1));
  else if (!DecodeReg(ArgStr[1], &Reg)) WrXErrorPos(ErrNum_InvReg, ArgStr[1], &ArgStrPos[1]);
  else
  {
    BAsmCode[0] = Index | Reg; CodeLen = 1;
  }
}

static void DecodeImm(Word Index)
{
  Boolean OK;

  if (ChkArgCnt(1, 1))
  {
    BAsmCode[1] = EvalIntExpression(ArgStr[1], Int8, &OK);
    if (OK)
    {
      BAsmCode[0] = Index; CodeLen = 2;
    }
  }
}

static void DecodeRegImm(Word Index)
{
  Byte Reg;
  Boolean OK;

  if (!ChkArgCnt(2, 2));
  else if (!DecodeReg(ArgStr[1], &Reg)) WrXErrorPos(ErrNum_InvReg, ArgStr[1], &ArgStrPos[1]);
  else
  {
    BAsmCode[1] = EvalIntExpression(ArgStr[2], Int8, &OK);
    if (OK)
    {
      BAsmCode[0] = Index | Reg; CodeLen = 2;
    }
  }
}

static void DecodeRegAbs(Word Index)
{
  Byte DReg, IReg;
  Word AbsVal;
  Boolean OK, IndFlag;

  if (!ChkArgCnt(2, 4));
  else if (!DecodeReg(ArgStr[1], &DReg)) WrXErrorPos(ErrNum_InvReg, ArgStr[1], &ArgStrPos[1]);
  else
  {
    IndFlag = *ArgStr[2] == '*';
    AbsVal = EvalIntExpression(ArgStr[2] + IndFlag, UInt13, &OK);
    if (OK)
    {
      BAsmCode[0] = Index;
      BAsmCode[1] = Hi(AbsVal);
      BAsmCode[2] = Lo(AbsVal);
      if (IndFlag)
        BAsmCode[1] |= 0x80;
      if (ArgCnt == 2)
      {
        BAsmCode[0] |= DReg;
        CodeLen = 3;
      }
      else
      {
        if (!DecodeReg(ArgStr[3], &IReg)) WrXErrorPos(ErrNum_InvReg, ArgStr[3], &ArgStrPos[3]);
        else if (DReg != 0) WrError(1350);
        else
        {
          BAsmCode[0] |= IReg;
          if (ArgCnt == 3)
          {
            BAsmCode[1] |= 0x60;
            CodeLen = 3;
          }
          else if (!strcmp(ArgStr[4], "-"))
          {
            BAsmCode[1] |= 0x40;
            CodeLen = 3;
          }
          else if (!strcmp(ArgStr[4], "+"))
          {
            BAsmCode[1] |= 0x20;
            CodeLen = 3;
          }
          else
            WrError(1350);
        }
      }
    }
  }
}

static void DecodeRegRel(Word Index)
{
  Byte Reg;
  Boolean IndFlag, OK;
  int Dist;

  if (!ChkArgCnt(2, 2));
  else if (!DecodeReg(ArgStr[1], &Reg)) WrXErrorPos(ErrNum_InvReg, ArgStr[1], &ArgStrPos[1]);
  else
  {
    BAsmCode[0] = Index | Reg;
    IndFlag = *ArgStr[2] == '*';
    Dist = EvalIntExpression(ArgStr[2] + IndFlag, UInt13, &OK) - (EProgCounter() + 2);
    if (OK)
    {
      if (((Dist < - 64) || (Dist > 63)) && (!SymbolQuestionable)) WrError(1370);
      else
      {
        BAsmCode[1] = Dist & 0x7f;
        if (IndFlag)
          BAsmCode[1] |= 0x80;
        CodeLen = 2;
      }
    }
  }
}

static void DecodeCondAbs(Word Index)
{
  Byte Cond;
  Word Address;
  Boolean OK, IndFlag;
  
  if (!ChkArgCnt(2, 2));
  else if (!DecodeCondition(ArgStr[1], &Cond)) WrXErrorPos(ErrNum_InvReg, ArgStr[1], &ArgStrPos[1]);
  else
  {
    IndFlag = *ArgStr[2] == '*';
    Address = EvalIntExpression(ArgStr[2] + IndFlag, UInt13, &OK);
    if (OK)
    {
      BAsmCode[0] = Index | Cond;
      BAsmCode[1] = Hi(Address);
      if (IndFlag)
        BAsmCode[1] |= 0x80;
      BAsmCode[2] = Lo(Address);
      CodeLen = 3;
    }
  }
}

static void DecodeCondRel(Word Index)
{
  Byte Cond;
  Boolean IndFlag, OK;
  int Dist;

  if (!ChkArgCnt(2, 2));
  else if (!DecodeCondition(ArgStr[1], &Cond)) WrXErrorPos(ErrNum_InvReg, ArgStr[1], &ArgStrPos[1]);
  else
  {
    BAsmCode[0] = Index | Cond;
    IndFlag = *ArgStr[2] == '*';
    Dist = EvalIntExpression(ArgStr[2] + IndFlag, UInt13, &OK) - (EProgCounter() + 2);
    if (OK)
    {
      if (((Dist < - 64) || (Dist > 63)) && (!SymbolQuestionable)) WrError(1370);
      else
      {
        BAsmCode[1] = Dist & 0x7f;
        if (IndFlag)
          BAsmCode[1] |= 0x80;
        CodeLen = 2;
      }
    }
  }
}

static void DecodeRegAbs2(Word Index)
{
  Byte Reg;
  Word AbsVal;
  Boolean IndFlag, OK;

  if (!ChkArgCnt(2, 2));
  else if (!DecodeReg(ArgStr[1], &Reg)) WrXErrorPos(ErrNum_InvReg, ArgStr[1], &ArgStrPos[1]);
  else
  {
    BAsmCode[0] = Index | Reg;
    IndFlag = *ArgStr[2] == '*';
    AbsVal = EvalIntExpression(ArgStr[2] + IndFlag, UInt13, &OK);
    if (OK)
    {
      BAsmCode[1] = Hi(AbsVal);
      if (IndFlag)
        BAsmCode[1] |= 0x80;
      BAsmCode[2] = Lo(AbsVal);
      CodeLen = 3;
    }
  }
}

static void DecodeBrAbs(Word Index)
{
  Byte Reg = 3;
  Word AbsVal;
  Boolean IndFlag, OK;

  if (!ChkArgCnt(1, 2));
  else if ((ArgCnt == 2) && (!DecodeReg(ArgStr[2], &Reg))) WrXErrorPos(ErrNum_InvReg, ArgStr[2], &ArgStrPos[2]);
  else if (Reg != 3) WrError(1350);
  else
  {
    BAsmCode[0] = Index | Reg;
    IndFlag = *ArgStr[1] == '*';
    AbsVal = EvalIntExpression(ArgStr[1] + IndFlag, UInt13, &OK);
    if (OK)
    {
      BAsmCode[1] = Hi(AbsVal);
      if (IndFlag)
        BAsmCode[1] |= 0x80;
      BAsmCode[2] = Lo(AbsVal);
      CodeLen = 3;
    }
  }
}

static void DecodeCond(Word Index)
{
  Byte Cond;
  
  if (!ChkArgCnt(1, 1));
  else if (!DecodeCondition(ArgStr[1], &Cond)) WrXErrorPos(ErrNum_UndefCond, ArgStr[1], &ArgStrPos[1]);
  else
  {
    BAsmCode[0] = Index | Cond;
    CodeLen = 1;
  }
}

static void DecodeZero(Word Index)
{
  Boolean IndFlag, OK;

  if (ChkArgCnt(1, 1))
  {
    BAsmCode[0] = Index;
    IndFlag = *ArgStr[1] == '*';
    BAsmCode[1] = EvalIntExpression(ArgStr[1] + IndFlag, UInt7, &OK);
    if (OK)
    {
      if (IndFlag)
        BAsmCode[1] |= 0x80; 
      CodeLen = 2;
    }
  }
}

/*--------------------------------------------------------------------------*/
/* Code Table Handling */

static void AddFixed(char *pName, Word Code)
{
  AddInstTable(InstTable, pName, Code, DecodeFixed);
}

static void AddOneReg(char *pName, Word Code)
{
  AddInstTable(InstTable, pName, Code, DecodeOneReg);
}

static void AddImm(char *pName, Word Code)
{
  AddInstTable(InstTable, pName, Code, DecodeImm);
}

static void AddRegImm(char *pName, Word Code)
{
  AddInstTable(InstTable, pName, Code, DecodeRegImm);
}

static void AddRegAbs(char *pName, Word Code)
{
  AddInstTable(InstTable, pName, Code, DecodeRegAbs);
}

static void AddRegRel(char *pName, Word Code)
{
  AddInstTable(InstTable, pName, Code, DecodeRegRel);
}

static void AddCondAbs(char *pName, Word Code)
{
  AddInstTable(InstTable, pName, Code, DecodeCondAbs);
}

static void AddCondRel(char *pName, Word Code)
{
  AddInstTable(InstTable, pName, Code, DecodeCondRel);
}

static void AddRegAbs2(char *pName, Word Code)
{
  AddInstTable(InstTable, pName, Code, DecodeRegAbs2);
}
   
static void AddBrAbs(char *pName, Word Code)
{
  AddInstTable(InstTable, pName, Code, DecodeBrAbs);
}  

static void AddCond(char *pName, Word Code)
{
  AddInstTable(InstTable, pName, Code, DecodeCond);
}

static void AddZero(char *pName, Word Code)
{
  AddInstTable(InstTable, pName, Code, DecodeZero);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(203);

  AddFixed("NOP", 0xc0);
  AddFixed("HALT", 0x40);
  AddFixed("LPSL", 0x93);
  AddFixed("LPSU", 0x92);
  AddFixed("SPSL", 0x13);
  AddFixed("SPSU", 0x12);

  AddOneReg("ADDZ", 0x80);
  AddOneReg("ANDZ", 0x40);
  AddOneReg("COMZ", 0xe0);
  AddOneReg("DAR", 0x94);
  AddOneReg("EORZ", 0x20);
  AddOneReg("IORZ", 0x60);
  AddOneReg("LODZ", 0x00);
  AddOneReg("REDC", 0x30);
  AddOneReg("REDD", 0x70);
  AddOneReg("RRL", 0xd0);
  AddOneReg("RRR", 0x50);
  AddOneReg("STRZ", 0xc0);
  AddOneReg("SUBZ", 0xa0);
  AddOneReg("WRTC", 0xb0);
  AddOneReg("WRTD", 0xf0);

  AddImm("CPSL", 0x75);
  AddImm("CPSU", 0x74);
  AddImm("PPSL", 0x77);
  AddImm("PPSU", 0x76);
  AddImm("TPSL", 0xb5);
  AddImm("TPSU", 0xb4);

  AddRegImm("ADDI", 0x84);
  AddRegImm("ANDI", 0x44);
  AddRegImm("COMI", 0xe4);
  AddRegImm("EORI", 0x24);
  AddRegImm("IORI", 0x64);
  AddRegImm("LODI", 0x04);
  AddRegImm("REDE", 0x54);
  AddRegImm("SUBI", 0xa4);
  AddRegImm("TMI", 0xf4);
  AddRegImm("WRTE", 0xd4);

  AddRegAbs("ADDA", 0x8c);
  AddRegAbs("ANDA", 0x4c);
  AddRegAbs("COMA", 0xec);
  AddRegAbs("EORA", 0x2c);
  AddRegAbs("IORA", 0x6c);
  AddRegAbs("LODA", 0x0c);
  AddRegAbs("STRA", 0xcc);
  AddRegAbs("SUBA", 0xac);

  AddRegRel("ADDR", 0x88);
  AddRegRel("ANDR", 0x48);
  AddRegRel("BDRR", 0xf8);
  AddRegRel("BIRR", 0xd8);
  AddRegRel("BRNR", 0x58);
  AddRegRel("BSNR", 0x78);
  AddRegRel("COMR", 0xe8);
  AddRegRel("EORR", 0x28);
  AddRegRel("IORR", 0x68);
  AddRegRel("LODR", 0x08);
  AddRegRel("STRR", 0xc8);
  AddRegRel("SUBR", 0xa8);

  AddCondAbs("BCFA", 0x9c);
  AddCondAbs("BCTA", 0x1c);
  AddCondAbs("BSFA", 0xbc);
  AddCondAbs("BSTA", 0x3c);

  AddCondRel("BCFR", 0x98);
  AddCondRel("BCTR", 0x18);
  AddCondRel("BSFR", 0xb8);
  AddCondRel("BSTR", 0x38);

  AddRegAbs2("BDRA", 0xfc);
  AddRegAbs2("BIRA", 0xdc);
  AddRegAbs2("BRNA", 0x5c);
  AddRegAbs2("BSNA", 0x7c);

  AddBrAbs("BSXA", 0xbf);
  AddBrAbs("BXA", 0x9f);

  AddCond("RETC", 0x14);
  AddCond("RETE", 0x34);

  AddZero("ZBRR", 0x9b);
  AddZero("ZBSR", 0xbb);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*--------------------------------------------------------------------------*/
/* Callbacks */

static void MakeCode_2650(void)
{
  char *pPos;

  CodeLen = 0;

  DontPrint = False;

  /* Nullanweisung */

  if ((*OpPart.Str == '\0') && (ArgCnt == 0))
    return;

  /* Pseudoanweisungen */

  if (DecodeIntelPseudo(False)) return;

  /* try to split off first (register) operand from instruction */

  pPos = strchr(OpPart.Str, ',');
  if (pPos)
  {
    int ArgC;

    for (ArgC = ArgCnt; ArgC >= 1; ArgC--)
      strcpy(ArgStr[ArgC + 1], ArgStr[ArgC]);
    strcpy(ArgStr[1], pPos + 1);
    *pPos = '\0';
    ArgCnt++;
  }

  /* alles aus der Tabelle */

  if (!LookupInstTable(InstTable, OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownOpcode, &OpPart);
}

static Boolean IsDef_2650(void)
{
  return FALSE;
}

static void SwitchFrom_2650(void)
{
  DeinitFields();
}

static void SwitchTo_2650(void)
{
  PFamilyDescr pDescr;

  TurnWords = False; ConstMode = ConstModeMoto; SetIsOccupied = False;

  pDescr = FindFamilyByName("2650");
  PCSymbol = "$"; HeaderID = pDescr->Id; NOPCode = 0xc0;
  DivideChars = ","; HasAttrs = False;

  ValidSegs = (1 << SegCode);
  Grans[SegCode] = 1; ListGrans[SegCode] = 1; SegInits[SegCode] = 0;
  SegLimits[SegCode] = 0x1fffl;

  MakeCode = MakeCode_2650; IsDef = IsDef_2650;

  SwitchFrom = SwitchFrom_2650; InitFields();
}

/*--------------------------------------------------------------------------*/
/* Initialisierung */

void code2650_init(void)
{
  CPU2650 = AddCPU("2650", SwitchTo_2650);
}
