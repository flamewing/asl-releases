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
/* $Id: code85.c,v 1.5 2014/08/10 13:27:53 alfred Exp $                      */
/***************************************************************************** 
 * $Log: code85.c,v $
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
#include "codepseudo.h" 
#include "intpseudo.h"
#include "asmitree.h"
#include "codevars.h"

/*--------------------------------------------------------------------------------------------------*/

static CPUVar CPU8080, CPU8085, CPU8085U;

/*---------------------------------------------------------------------------*/

static Boolean DecodeReg8(char *Asc, Byte *Erg)
{
  static const char *RegNames = "BCDEHLMA";
  char *p;

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

static Boolean DecodeReg16(char *Asc, Byte *Erg)
{
  static const char *RegNames[4] = {"B", "D", "H", "SP"};

  for (*Erg = 0; (*Erg) < 4; (*Erg)++)
    if (strcasecmp(Asc, RegNames[*Erg]) == 0)
      break;

  return ((*Erg) < 4);
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
      if (Hi(Code))
        ChkSpace(SegIO);
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
      if (!DecodeReg16(ArgStr[1], &Reg)) WrXError(1980, ArgStr[1]);
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
  else if (!DecodeReg16(ArgStr[1], &Reg)) WrXError(1980, ArgStr[1]);
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
    OK = TRUE;
    if (!strcasecmp(ArgStr[1], "PSW"))
      Reg = 3;
    else if (DecodeReg16(ArgStr[1], &Reg))
    {
      if (Reg == 3)
        OK = FALSE;
    } 
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
    AdrByte = EvalIntExpression(ArgStr[1], UInt3, &OK);
    if (OK)
    {
      CodeLen = 1;
      BAsmCode[0] = 0xc7 + (AdrByte << 3);
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
  else if (!DecodeReg16(ArgStr[1], &Reg)) WrXError(1980, ArgStr[1]);
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
  else if (!DecodeReg16(ArgStr[1], &Reg)) WrXError(1980, ArgStr[1]);
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
  else if (!DecodeReg16(ArgStr[1], &Reg)) WrXError(1980, ArgStr[1]);
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
  else if (!DecodeReg16(ArgStr[1], &Reg)) WrXError(1980, ArgStr[1]);
  else if (Reg != 1) WrXError(1980, ArgStr[1]);
  else
  {   
    CodeLen = 1;
    BAsmCode[0] = Index ? 0xed: 0xd9;
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
  InstTable = CreateInstTable(103);

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
  AddFixed("RET" , CPU8080 , 0xc9); AddFixed("RC"  , CPU8080 , 0xd8);
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

  AddOp16("STA" , CPU8080 , 0x32); AddOp16("LDA" , CPU8080 , 0x3a);
  AddOp16("SHLD", CPU8080 , 0x22); AddOp16("LHLD", CPU8080 , 0x2a);
  AddOp16("JMP" , CPU8080 , 0xc3); AddOp16("JC"  , CPU8080 , 0xda);
  AddOp16("JNC" , CPU8080 , 0xd2); AddOp16("JZ"  , CPU8080 , 0xca);
  AddOp16("JNZ" , CPU8080 , 0xc2); AddOp16("JP"  , CPU8080 , 0xf2);
  AddOp16("JM"  , CPU8080 , 0xfa); AddOp16("JPE" , CPU8080 , 0xea);
  AddOp16("JPO" , CPU8080 , 0xe2); AddOp16("CALL", CPU8080 , 0xcd);
  AddOp16("CC"  , CPU8080 , 0xdc); AddOp16("CNC" , CPU8080 , 0xd4);
  AddOp16("CZ"  , CPU8080 , 0xcc); AddOp16("CNZ" , CPU8080 , 0xc4);
  AddOp16("CP"  , CPU8080 , 0xf4); AddOp16("CM"  , CPU8080 , 0xfc);
  AddOp16("CPE" , CPU8080 , 0xec); AddOp16("CPO" , CPU8080 , 0xe4);
  AddOp16("JNX5", CPU8085U, 0xdd); AddOp16("JX5" , CPU8085U, 0xfd);

  AddOp8("IN"  , CPU8080 , 0x01db); AddOp8("OUT" , CPU8080 , 0x01d3);
  AddOp8("ADI" , CPU8080 , 0xc6); AddOp8("ACI" , CPU8080 , 0xce);
  AddOp8("SUI" , CPU8080 , 0xd6); AddOp8("SBI" , CPU8080 , 0xde);
  AddOp8("ANI" , CPU8080 , 0xe6); AddOp8("XRI" , CPU8080 , 0xee);
  AddOp8("ORI" , CPU8080 , 0xf6); AddOp8("CPI" , CPU8080 , 0xfe);
  AddOp8("LDHI", CPU8085U, 0x28); AddOp8("LDSI", CPU8085U, 0x38);

  AddALU("ADD" , 0x80); AddALU("ADC" , 0x88);
  AddALU("SUB" , 0x90); AddALU("SBB" , 0x98);
  AddALU("ANA" , 0xa0); AddALU("XRA" , 0xa8);
  AddALU("ORA" , 0xb0); AddALU("CMP" , 0xb8);
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
}

void code85_init(void)
{
  CPU8080 = AddCPU("8080", SwitchTo_85);
  CPU8085 = AddCPU("8085", SwitchTo_85);
  CPU8085U = AddCPU("8085UNDOC", SwitchTo_85);
}
