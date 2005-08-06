/* codemic8.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator LatticeMico8                                                */
/*                                                                           */
/*****************************************************************************/
/* $Id: codemic8.c,v 1.3 2005/08/06 14:19:28 alfred Exp $                   */
/*****************************************************************************
 * $Log: codemic8.c,v $
 * Revision 1.3  2005/08/06 14:19:28  alfred
 * - assure long unsigned constants on 16-bit-platforms
 *
 * Revision 1.2  2005/08/06 13:35:11  alfred
 * - added INC/DEC
 *
 * Revision 1.1  2005/07/30 13:57:02  alfred
 * - add LatticeMico8
 *
 *****************************************************************************/

#include "stdinc.h"
#include "stdio.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "strutil.h"
#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "intpseudo.h"
#include "codevars.h"
#include "headids.h"

#include "codepseudo.h"
#include "codemic8.h"

#define ALUOrderCnt 18
#define FixedOrderCnt 9
#define BranchOrderCnt 10
#define MemOrderCnt 4
#define RegOrderCnt 2

typedef struct
        {
          LongWord Code;
        } FixedOrder;

typedef struct
        {
          LongWord Code;
          Byte Space;
        } MemOrder;

static PInstTable InstTable;     

static FixedOrder *FixedOrders, *ALUOrders, *BranchOrders, *RegOrders;
static MemOrder *MemOrders;

static CPUVar CPUMico8;

/*--------------------------------------------------------------------------
 * Address Expression Parsing
 *--------------------------------------------------------------------------*/

static Boolean IsWReg(char *Asc, LongWord *pErg)
{
  Boolean OK;
  char *s;

  if (FindRegDef(Asc, &s)) Asc = s;

  if ((strlen(Asc) < 2) || (toupper(*Asc) != 'R')) 
    return False;

  *pErg = ConstLongInt(Asc + 1, &OK);
  if (!OK)
    return False;

  return (*pErg < 32);
}

/*--------------------------------------------------------------------------
 * Code Handlers
 *--------------------------------------------------------------------------*/

static void DecodePort(Word Index)
{
  UNUSED(Index);

  CodeEquate(SegIO, 0, SegLimits[SegIO]);
}

static void DecodeRegDef(Word Index)
{
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else AddRegDef(LabPart, ArgStr[1]);
}

static void DecodeFixed(Word Index)
{
  FixedOrder *pOrder = FixedOrders + Index;

  if (ArgCnt != 0) WrError(1110);
  else
  {
    DAsmCode[0] = pOrder->Code;
    CodeLen = 1;
  }
}

static void DecodeALU(Word Index)
{
  FixedOrder *pOrder = ALUOrders + Index;
  LongWord SReg, DReg;

  if (ArgCnt != 2) WrError(1110);
  else if (!IsWReg(ArgStr[1], &DReg)) WrXError(1445, ArgStr[1]);
  else if (!IsWReg(ArgStr[2], &SReg)) WrXError(1445, ArgStr[2]);
  else
  {
    DAsmCode[0] = pOrder->Code | (DReg << 8) | (SReg << 3);
    CodeLen = 1;
  }
}

static void DecodeALUI(Word Index)
{
  FixedOrder *pOrder = ALUOrders + Index;
  LongWord Src, DReg;
  Boolean OK;

  if (ArgCnt != 2) WrError(1110);
  else if (!IsWReg(ArgStr[1], &DReg)) WrXError(1445, ArgStr[1]);
  else
  {
    Src = EvalIntExpression(ArgStr[2], Int8, &OK);
    if (OK)
    {
      DAsmCode[0] = pOrder->Code | (1 << 13) | (DReg << 8) | (Src & 0xff);
      CodeLen = 1;
    }
  }
}

static void DecodeBranch(Word Index)
{
  FixedOrder *pOrder = BranchOrders + Index;
  LongInt Dest;
  Boolean OK;

  if (ArgCnt != 1) WrError(1110);
  else
  {   
    Dest = EvalIntExpression(ArgStr[1], UInt10, &OK);
    if (OK)
    {
      Dest -= EProgCounter();
      if (((Dest < -512) || (Dest > 511)) && (!SymbolQuestionable)) WrError(1370);
      else
      {
        DAsmCode[0] = pOrder->Code | (Dest & 0x3ff);
        CodeLen = 1;
      }
    }
  }  
}    
     
static void DecodeMem(Word Index)
{
  MemOrder *pOrder = MemOrders + Index;
  LongWord DReg, Src;
  Boolean OK;

  if (ArgCnt != 2) WrError(1110);
  else if (!IsWReg(ArgStr[1], &DReg)) WrXError(1445, ArgStr[1]);
  else
  {
    Src = EvalIntExpression(ArgStr[2], UInt5, &OK);
    if (OK)
    {
      ChkSpace(pOrder->Space);
      DAsmCode[0] = pOrder->Code | (DReg << 8) | ((Src & 0x1f) << 3);
      CodeLen = 1;
    }
  }
}

static void DecodeReg(Word Index)
{
  FixedOrder *pOrder = RegOrders + Index;
  LongWord Reg;

  if (ArgCnt != 1) WrError(1110);
  else if (!IsWReg(ArgStr[1], &Reg)) WrXError(1445, ArgStr[1]);
  {
    DAsmCode[0] = pOrder->Code | (Reg << 8);
    CodeLen = 1;
  }
}

/*--------------------------------------------------------------------------
 * Instruction Table Handling
 *--------------------------------------------------------------------------*/

static void AddFixed(char *NName, LongWord NCode)
{
  if (InstrZ >= FixedOrderCnt)
    exit(255);

  FixedOrders[InstrZ].Code = NCode;
  AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
}
 
static void AddALU(char *NName, char *NImmName, LongWord NCode)
{
  if (InstrZ >= ALUOrderCnt)
    exit(255);

  ALUOrders[InstrZ].Code = NCode;
  AddInstTable(InstTable, NName, InstrZ, DecodeALU);
  if (NImmName)
    AddInstTable(InstTable, NImmName, InstrZ, DecodeALUI);
  InstrZ++;
}

static void AddBranch(char *NName, LongWord NCode)
{
  if (InstrZ >= BranchOrderCnt)
    exit(255);

  BranchOrders[InstrZ].Code = NCode;
  AddInstTable(InstTable, NName, InstrZ++, DecodeBranch);
}
 
static void AddMem(char *NName, LongWord NCode, Byte NSpace)
{
  if (InstrZ >= MemOrderCnt)
    exit(255);

  MemOrders[InstrZ].Code = NCode;
  MemOrders[InstrZ].Space = NSpace;
  AddInstTable(InstTable, NName, InstrZ++, DecodeMem);
}      
 
static void AddReg(char *NName, LongWord NCode)
{
  if (InstrZ >= RegOrderCnt)
    exit(255);

  RegOrders[InstrZ].Code = NCode;
  AddInstTable(InstTable, NName, InstrZ++, DecodeReg);
}      
       
static void InitFields(void)
{
  InstTable = CreateInstTable(97);

  InstrZ = 0;
  FixedOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * FixedOrderCnt);
  AddFixed("CLRC"  , 0x2c000);
  AddFixed("SETC"  , 0x2c001);
  AddFixed("CLRZ"  , 0x2c002);
  AddFixed("SETZ"  , 0x2c003);
  AddFixed("CLRI"  , 0x2c004);
  AddFixed("SETI"  , 0x2c005);
  AddFixed("RET"   , 0x3a000);
  AddFixed("IRET"  , 0x3a001);
  AddFixed("NOP"   , 0x10000);

  InstrZ = 0;
  ALUOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * ALUOrderCnt);
  AddALU("ADD"    , "ADDI"  ,   2UL << 14);
  AddALU("ADDC"   , "ADDIC" ,   3UL << 14);
  AddALU("SUB"    , "SUBI"  ,   0UL << 14);
  AddALU("SUBC"   , "SUBIC" ,   1UL << 14);
  AddALU("MOV"    , "MOVI"  ,   4UL << 14);
  AddALU("AND"    , "ANDI"  ,   5UL << 14);
  AddALU("OR"     , "ORI"   ,   6UL << 14);
  AddALU("XOR"    , "XORI"  ,   7UL << 14);
  AddALU("CMP"    , "CMPI"  ,   8UL << 14);
  AddALU("TEST"   , "TESTI" ,   9UL << 14);
  AddALU("ROR"    , NULL    , (10UL << 14) | 0);
  AddALU("RORC"   , NULL    , (10UL << 14) | 1);
  AddALU("ROL"    , NULL    , (10UL << 14) | 2);
  AddALU("ROLC"   , NULL    , (10UL << 14) | 3);
  AddALU("IMPORTI", NULL    , (15UL << 14) | 3);
  AddALU("EXPORTI", NULL    , (15UL << 14) | 2);
  AddALU("LSPI"   , NULL    , (15UL << 14) | 7);
  AddALU("SSPI"   , NULL    , (15UL << 14) | 6);

  InstrZ = 0;
  RegOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * RegOrderCnt); 
  AddReg("INC"    , (2UL << 14)  | (1UL << 13) | 1);
  AddReg("DEC"    , (0UL << 14)  | (1UL << 13) | 1);

  InstrZ = 0;
  BranchOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * BranchOrderCnt);
  AddBranch("BZ"    , 0x32000);
  AddBranch("BNZ"   , 0x32400);
  AddBranch("BC"    , 0x32800);
  AddBranch("BNC"   , 0x32c00);
  AddBranch("B"     , 0x33000);
  AddBranch("CALLZ" , 0x36000);
  AddBranch("CALLNZ", 0x36400);
  AddBranch("CALLC" , 0x36800);  
  AddBranch("CALLNC", 0x36c00);
  AddBranch("CALL"  , 0x37000);

  InstrZ = 0;
  MemOrders = (MemOrder*) malloc(sizeof(MemOrder) * MemOrderCnt);
  AddMem("IMPORT" , (15UL << 14) | 1, SegIO);
  AddMem("EXPORT" , (15UL << 14) | 0, SegIO);
  AddMem("LSP"    , (15UL << 14) | 5, SegData);
  AddMem("SSP"    , (15UL << 14) | 4, SegData);

  AddInstTable(InstTable, "REG", 0, DecodeRegDef);
  AddInstTable(InstTable, "PORT", 0, DecodePort);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
  free(FixedOrders);
  free(ALUOrders);
  free(BranchOrders);
  free(MemOrders);
  free(RegOrders);
}

/*--------------------------------------------------------------------------
 * Semipublic Functions
 *--------------------------------------------------------------------------*/

static Boolean IsDef_Mico8(void)
{
   return (Memo("REG")) || (Memo("PORT")); 
}

static void SwitchFrom_Mico8(void)
{
   DeinitFields();
}

static void MakeCode_Mico8(void)
{
  CodeLen = 0; DontPrint = False;

  /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodeIntelPseudo(True)) return;

   if (NOT LookupInstTable(InstTable, OpPart))
     WrXError(1200, OpPart);
}

static void SwitchTo_Mico8(void)
{
   PFamilyDescr FoundDescr;

   FoundDescr = FindFamilyByName("Mico8");

   TurnWords = True; ConstMode = ConstModeC; SetIsOccupied = False;

   PCSymbol = "$"; HeaderID = FoundDescr->Id;

   /* NOP = mov R0,R0 */

   NOPCode = 0x10000;
   DivideChars = ","; HasAttrs = False;

   ValidSegs = (1 << SegCode) | (1 << SegData) | (1 << SegXData) | (1 << SegIO);
   Grans[SegCode] = 4; ListGrans[SegCode] = 4; SegInits[SegCode] = 0;
   SegLimits[SegCode] = 0x3ff;
   Grans[SegData] = 1; ListGrans[SegData] = 1; SegInits[SegData] = 0;
   SegLimits[SegData] = 0x1f;
   Grans[SegXData] = 1; ListGrans[SegXData] = 1; SegInits[SegXData] = 0;
   SegLimits[SegXData] = 0xff;
   Grans[SegIO] = 1; ListGrans[SegIO] = 1; SegInits[SegIO] = 0;
   SegLimits[SegIO] = 0xff;

   MakeCode = MakeCode_Mico8; IsDef = IsDef_Mico8;
   SwitchFrom = SwitchFrom_Mico8; InitFields();
}

/*--------------------------------------------------------------------------
 * Initialization
 *--------------------------------------------------------------------------*/

void codemico8_init(void)
{
   CPUMico8 = AddCPU("Mico8", SwitchTo_Mico8);
}
