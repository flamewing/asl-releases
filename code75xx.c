/* code75xx.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator NEC 75xx                                                    */
/*                                                                           */
/* Historie: 2013-03-09 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/
/* $Id: code75xx.c,v 1.1 2013-03-09 16:15:08 alfred Exp $                    */
/*****************************************************************************
 * $Log: code75xx.c,v $
 * Revision 1.1  2013-03-09 16:15:08  alfred
 * - add NEC 75xx
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "strutil.h"
#include "headids.h"
#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"  
#include "codepseudo.h"
#include "intpseudo.h"
#include "codevars.h"

static CPUVar CPU7500;

/*-------------------------------------------------------------------------*/
/* code generation */

static void PutCode(Word Code)
{
  CodeLen = 0;
  if (Hi(Code))
    BAsmCode[CodeLen++] = Hi(Code);
  BAsmCode[CodeLen++] = Lo(Code);
}

static void DecodeFixed(Word Index)
{
  if (ArgCnt != 0) WrError(1110);
  else
    PutCode(Index);
}

static void DecodeImm4(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;
    Word Value;

    Value = EvalIntExpression(ArgStr[1], Int4, &OK);
    if (OK)
      PutCode(Index | (Value & 15));
  }
}

static void DecodeImm5(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;
    Word Value;

    Value = EvalIntExpression(ArgStr[1], Int5, &OK);
    if (OK)
      PutCode(Index | (Value & 31));
  }
}

static void DecodeImm2(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;
    Word Value;

    Value = EvalIntExpression(ArgStr[1], UInt2, &OK);
    if (OK)
      PutCode(Index | (Value & 3));
  }
}

static void DecodePR(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else if (!strcasecmp(ArgStr[1], "HL-"))
    PutCode(Index | 0);
  else if (!strcasecmp(ArgStr[1], "HL+"))
    PutCode(Index | 1);
  else if (!strcasecmp(ArgStr[1], "HL"))
    PutCode(Index | 2);
  else
    WrXError(1135, ArgStr[1]);
}

static void DecodeDataMem(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;
    Word Value;
 
    Value = EvalIntExpression(ArgStr[1], UInt6, &OK);
    if (OK)
    {
      PutCode(Index | (Value & 0x3f));
      ChkSpace(SegData);
    }
  }
}

static void DecodeAbs(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;
    Word Address;

    Address = EvalIntExpression(ArgStr[1], UInt10, &OK);
    if (OK)
    {
      PutCode(Index | (Address & 0x3ff));
      ChkSpace(SegCode);
    }
  }
}

static void DecodeJCP(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;
    Word Address;

    FirstPassUnknown = FALSE;
    Address = EvalIntExpression(ArgStr[1], UInt10, &OK);
    if (OK)
    {
      Word NextAddr = (EProgCounter() + 1) & 0x3ff;

      if ((!FirstPassUnknown) && ((NextAddr & 0x3c0) != (Address & 0x3c0))) WrError(1910);
      else
        PutCode(Index | (Address & 0x3f));
      ChkSpace(SegCode);
    }
  }
}

static void DecodeCAL(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;
    Word Address;
 
    FirstPassUnknown = FALSE;
    Address = EvalIntExpression(ArgStr[1], UInt10, &OK);
    if (FirstPassUnknown)
      Address = (Address & 0x1c7) | 0x100;
    if (OK)
    {
      if ((Address & 0x338) != 0x100) WrError(1320);
      else
        PutCode(Index | ((Address & 0x0c0) >> 3) | (Address & 7));
      ChkSpace(SegCode);
    }
  }
}

/*-------------------------------------------------------------------------*/
/* dynamische Codetabellenverwaltung */

static void AddFixed(char *NewName, Word NewCode)
{
  AddInstTable(InstTable, NewName, NewCode, DecodeFixed);
}

static void AddImm4(char *NewName, Word NewCode)
{
  AddInstTable(InstTable, NewName, NewCode, DecodeImm4);
}

static void AddImm2(char *NewName, Word NewCode)
{
  AddInstTable(InstTable, NewName, NewCode, DecodeImm2);
}

static void AddImm5(char *NewName, Word NewCode)
{
  AddInstTable(InstTable, NewName, NewCode, DecodeImm5);
}

static void AddPR(char *NewName, Word NewCode)
{
  AddInstTable(InstTable, NewName, NewCode, DecodePR);
}

static void AddDataMem(char *NewName, Word NewCode)
{
  AddInstTable(InstTable, NewName, NewCode, DecodeDataMem);
}

static void AddAbs(char *NewName, Word NewCode)
{
  AddInstTable(InstTable, NewName, NewCode, DecodeAbs);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(103);
  AddFixed("ST"    , 0x57);
  AddFixed("XAL"   , 0x7b);
  AddFixed("ASC"   , 0x7d);
  AddFixed("ACSC"  , 0x7c);
  AddFixed("EXL"   , 0x7e);
  AddFixed("CMA"   , 0x7f);
  AddFixed("RC"    , 0x78);
  AddFixed("SC"    , 0x79);
  AddFixed("ILS"   , 0x59);
  AddFixed("DLS"   , 0x58);
  AddFixed("RT"    , 0x53);
  AddFixed("RTS"   , 0x5b);
  AddFixed("TAMSP" , 0x3f31);
  AddFixed("SKC"   , 0x5a);
  AddFixed("SKAEM" , 0x5f);
  AddFixed("TAMSIO", 0x3f3e);
  AddFixed("TSIOAM", 0x3f3a);
  AddFixed("SIO"   , 0x3f33);
  AddFixed("TIMER" , 0x3f32);
  AddFixed("TCNTAM", 0x3f3b);
  AddFixed("IPL"   , 0x70);
  AddFixed("OPL"   , 0x72);
  AddFixed("RPBL"  , 0x5c);
  AddFixed("SPBL"  , 0x5d);
  AddFixed("HALT"  , 0x3f36);
  AddFixed("STOP"  , 0x3f37);
  AddFixed("NOP"  , 0x00);

  AddImm4("LAI", 0x10);
  AddImm4("STII", 0x40);
  AddImm4("AISC", 0x00);
  AddImm4("SKAEI", 0x3f60);

  AddImm2("LHI", 0x28);
  AddImm2("RMB", 0x68);
  AddImm2("SMB", 0x6c);
  AddImm2("SKABT", 0x74);
  AddImm2("SKMBT", 0x64);
  AddImm2("SKMBF", 0x60);
  AddImm2("SKI", 0x3d40);

  AddImm5("LHLI", 0xc0);

  AddPR("LAM", 0x50);
  AddPR("XAM", 0x54);

  AddDataMem("IDRS", 0x3d00);
  AddDataMem("DDRS", 0x3c00);

  AddAbs("JMP" , 0x2000);
  AddAbs("CALL", 0x3000);
  AddInstTable(InstTable, "JCP", 0x80, DecodeJCP);
  AddInstTable(InstTable, "CAL", 0xe0, DecodeCAL);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*-------------------------------------------------------------------------*/

static void MakeCode_75xx(void)
{
  CodeLen = 0; DontPrint = False;

  /* zu ignorierendes */

  if (Memo("")) return;

  /* Pseudoanweisungen */

  if (DecodeIntelPseudo(True)) return;

  if (!LookupInstTable(InstTable,OpPart))
    WrXError(1200, OpPart);
}

static Boolean IsDef_75xx(void)
{
  return False;
}

static void SwitchFrom_75xx(void)
{
  DeinitFields();
}

static void SwitchTo_75xx(void)
{
  TurnWords = False; ConstMode = ConstModeIntel; SetIsOccupied = False;

  PCSymbol = "PC"; HeaderID = FindFamilyByName("75xx")->Id; NOPCode = 0x00;
  DivideChars = ","; HasAttrs = False;

  ValidSegs = (1 << SegCode) | (1 << SegData);
  Grans[SegCode] = 1; ListGrans[SegCode] = 1; SegInits[SegCode] = 0;
  Grans[SegData] = 1; ListGrans[SegData] = 1; SegInits[SegData] = 0;
  SegLimits[SegData] = 0x3f;
  SegLimits[SegCode] = 0x3ff;

  MakeCode = MakeCode_75xx; IsDef = IsDef_75xx;
  SwitchFrom = SwitchFrom_75xx; InitFields();
}

void code75xx_init(void) 
{
  CPU7500 = AddCPU("7500",SwitchTo_75xx);
}
