/* code807c.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator National INS807X.c                                          */
/*                                                                           */
/*****************************************************************************/
/* $Id: code807x.c,v 1.6 2003/05/02 21:23:11 alfred Exp $                   *
 *****************************************************************************
 * $Log: code807x.c,v $
 * Revision 1.6  2003/05/02 21:23:11  alfred
 * - strlen() updates
 *
 * Revision 1.5  2003/03/30 12:50:21  alfred
 * - fixed some warnings
 *
 * Revision 1.4  2003/03/23 11:07:24  alfred
 * - documented 807x support
 *
 * Revision 1.3  2003/03/19 21:00:47  alfred
 * - first half done
 *
 * Revision 1.2  2003/03/16 19:22:25  alfred
 * - LD works
 *
 *****************************************************************************/

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
#include "codepseudo.h"
#include "codevars.h"

/*---------------------------------------------------------------------------*/

#define FixedOrderCnt 10
#define ShiftOrderCnt 5
#define BranchOrderCnt 5

#define ModNone (-1)
#define ModReg 0
#define MModReg (1 << ModReg)
#define ModImm 1
#define MModImm (1 << ModImm)
#define ModMem 2
#define MModMem (1 << ModMem)
#define ModAcc 3
#define MModAcc (1 << ModAcc)
#define ModE 4
#define MModE (1 << ModE)
#define ModS 5
#define MModS (1 << ModS)
#define ModT 6
#define MModT (1 << ModT)

typedef struct
        {
          Byte Code;
        } FixedOrder;

typedef struct
        {
          Byte Code, Code16;
        } ShiftOrder;

static PInstTable InstTable;
static FixedOrder *FixedOrders, *BranchOrders;
static ShiftOrder *ShiftOrders;

static int OpSize, AdrMode;
static Byte AdrVals[2], AdrPart;

static CPUVar CPU8070;

/*---------------------------------------------------------------------------*/

static Boolean SetOpSize(int NewSize)
{
  Boolean Result = True;

  if (OpSize == -1)
    OpSize = NewSize;
  else if (OpSize != NewSize)
  {
    WrError(1131);
    Result = False;
  }

  return Result;  
}

static Boolean GetReg16(char *Asc, Byte *AdrPart)
{
  int z;
  static char *Reg16Names[4] = { "PC", "SP", "P2", "P3" };

  for (z = 0; z < 4; z++)
    if (!strcasecmp(Asc, Reg16Names[z]))
    {
      *AdrPart = z;
      return True;
    }
  return False;
}

static void DecodeAdr(int Index, Byte Mask)
{
  int Cnt;
  Word TmpVal;
  LongInt Addr;
  Boolean OK;

  AdrMode = ModNone;

  Cnt = ArgCnt - Index + 1;
  if (!Cnt)
  {
    WrError(1110);
    goto AdrFound;
  }

  /* accumulator ? */

  if (!strcasecmp(ArgStr[Index], "A"))
  {
    if (SetOpSize(0))
      AdrMode = ModAcc;
    goto AdrFound;
  }

  if (!strcasecmp(ArgStr[Index], "EA"))
  {
    if (SetOpSize(1))
      AdrMode = ModAcc;
    goto AdrFound;
  }

  if (!strcasecmp(ArgStr[Index], "E"))
  {
    if (SetOpSize(0))
      AdrMode = ModE;
    goto AdrFound;
  }

  if (!strcasecmp(ArgStr[Index], "S"))
  {
    if (SetOpSize(0))
      AdrMode = ModS;
    goto AdrFound;
  }

  if (!strcasecmp(ArgStr[Index], "T"))
  {
    if (SetOpSize(1))
      AdrMode = ModT;
    goto AdrFound;
  }

  /* register ? */

  if (GetReg16(ArgStr[Index], &AdrPart))
  {
    if (SetOpSize(1))  
      AdrMode = ModReg;
    goto AdrFound;
  }

  /* immediate? */

  if (*ArgStr[Index] == '#')
  {
    switch (OpSize)
    {
      case 0:
        AdrVals[0] = EvalIntExpression(ArgStr[Index] + 1, Int8, &OK);
        break;
      case 1:
        TmpVal = EvalIntExpression(ArgStr[Index] + 1, Int16, &OK);
        if (OK)
        {
          AdrVals[0] = Lo(TmpVal); AdrVals[1] = Hi(TmpVal);
        }
        break;
      default:
        WrError(1132);
        OK = False;
    }
    if (OK)
    {
      AdrCnt = OpSize + 1; AdrMode = ModImm; AdrPart = 4;
    }
    goto AdrFound;
  }

  if (Cnt == 1)
  {
    AdrVals[0] = EvalIntExpression(ArgStr[Index], UInt8, &OK);
    if (OK)
    {
      AdrMode = ModMem; AdrPart = 5; AdrCnt = 1;
    }
    goto AdrFound;
  }

  else
  {
    char *pDist = ArgStr[Index];
    Boolean Incr = False;

    if (*pDist == '@')
    {
      pDist++;
      Incr = True;
    }

    OK = GetReg16(ArgStr[Index + 1], &AdrPart);
    if (!OK) WrXError(1445, ArgStr[Index + 1]);
    else if ((Incr) && (AdrPart < 2)) WrXError(1445, ArgStr[Index + 1]);
    else
    {
      if (Incr)
        AdrPart |= 4;
      FirstPassUnknown = False;
      Addr = EvalIntExpression(pDist, Int16, &OK);
      if (OK)
      {
        if (!AdrPart)
          Addr -= (EProgCounter() + 2);
        if ((!FirstPassUnknown) && (!SymbolQuestionable) && ((Addr < - 128) || (Addr > 127))) WrError(1320);
        else
        {
          AdrMode = ModMem;
          AdrVals[0] = Addr & 0xff;
          AdrCnt = 1;
        }
      }
    }
  }

AdrFound:
  if ((AdrMode != ModNone) && ((Mask & (1 << AdrMode)) == 0))
  {
    WrError(1350);
    AdrMode = ModNone;
  }
}

/*---------------------------------------------------------------------------*/

static void DecodeFixed(Word Index)
{
  FixedOrder *POrder = FixedOrders + Index;

  if (ArgCnt != 0) WrError(1110);
  else
  {
    BAsmCode[0] = POrder->Code;
    CodeLen = 1;
  }
}

static void DecodeLD(Word Index)
{
  UNUSED(Index);

  if ((ArgCnt < 1) || (ArgCnt > 3)) WrError(1110);
  else
  {
    DecodeAdr(1, MModAcc | MModReg | MModT | MModE);
    switch (AdrMode)
    {
      case ModAcc:
        if (OpSize == 0)
        {
          DecodeAdr(2, MModMem | MModImm | MModS | MModE);
          switch (AdrMode)
          {
            case ModMem:
            case ModImm:
              BAsmCode[0] = 0xc0 | AdrPart;
              memcpy(BAsmCode + 1, AdrVals, AdrCnt);
              CodeLen = 1 + AdrCnt;
              break;
            case ModS:
              BAsmCode[0] = 0x06;
              CodeLen = 1;
              break;
            case ModE:
              BAsmCode[0] = 0x40;
              CodeLen = 1;
              break;
          }
        }
        else
        {
          DecodeAdr(2, MModMem | MModImm | MModReg | MModT);
          switch (AdrMode)
          {
            case ModMem:
            case ModImm:
              BAsmCode[0] = 0x80 | AdrPart;
              memcpy(BAsmCode + 1, AdrVals, AdrCnt);
              CodeLen = 1 + AdrCnt;
              break;
            case ModReg:
              BAsmCode[0] = 0x30 | AdrPart;
              CodeLen = 1;
              break;
            case ModT:
              BAsmCode[0] = 0x0b;
              CodeLen = 1;
              break;
          }
        }
        break;
      case ModReg:
        BAsmCode[0] = AdrPart;
        DecodeAdr(2, MModAcc | MModImm);
        switch (AdrMode)
        {
          case ModAcc:
            BAsmCode[0] |= 0x44;
            CodeLen = 1;
            break;
          case ModImm:
            if (!BAsmCode[0]) WrError(1350);
            else
            {
              BAsmCode[0] |= 0x24;
              memcpy(BAsmCode + 1, AdrVals, AdrCnt);
              CodeLen = 1 + AdrCnt;
            }
            break;
        }
        break;
      case ModT:
        DecodeAdr(2, MModAcc | MModMem | MModImm);
        switch (AdrMode)
        {
          case ModAcc:
            BAsmCode[0] = 0x09;
            CodeLen = 1;
            break;
          case ModMem:
          case ModImm:
            BAsmCode[0] = 0xa0 | AdrPart;
            memcpy(BAsmCode + 1, AdrVals, AdrCnt);
            CodeLen = 1 + AdrCnt;
            break;
        }
        break;
      case ModE:
        DecodeAdr(2, MModAcc);
        switch (AdrMode)
        {
          case ModAcc:
            BAsmCode[0] = 0x48;
            CodeLen = 1;
            break;
        }
    }
  }
}

static void DecodeST(Word Index)
{
  UNUSED(Index);

  if ((ArgCnt < 1) || (ArgCnt > 3)) WrError(1110);
  else
  {   
    DecodeAdr(1, MModAcc);
    switch (AdrMode)
    {
      case ModAcc:
        DecodeAdr(2, MModMem);
        switch (AdrMode)
        {
          case ModMem:
            BAsmCode[0] = 0x88 | ((1 - OpSize) << 6) | AdrPart;
            memcpy(BAsmCode + 1, AdrVals, AdrCnt);
            CodeLen = 1 + AdrCnt;
            break;
        }
        break;
    }
  }
}

static void DecodeXCH(Word Index)
{
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(1, MModE | MModAcc | MModReg);
    switch (AdrMode)
    {
      case ModE:
        DecodeAdr(2, MModAcc);
        if (AdrMode == ModAcc)
          BAsmCode[CodeLen++] = 0x01;
        break;
      case ModAcc:
        DecodeAdr(2, MModE | MModReg);
        switch (AdrMode)
        {
          case ModE:
            BAsmCode[CodeLen++] = 0x01;
            break;
          case ModReg:
            BAsmCode[CodeLen++] = 0x4c | AdrPart;
            break;
        }
        break;
      case ModReg:
        BAsmCode[0] = 0x4c | AdrPart;
        DecodeAdr(2, MModAcc);
        if (AdrMode == ModAcc)
          CodeLen = 1;
        break;
    }
  }
}

static void DecodePLI(Word Index)
{
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(1, MModReg);
    if (AdrMode == ModReg)
    {
      if (AdrPart < 2) WrError(1350);
      else
      {
        BAsmCode[0] = 0x20 | AdrPart;
        DecodeAdr(2, MModImm);
        if (AdrMode == ModImm)
        {
          memcpy(BAsmCode + 1, AdrVals, AdrCnt);
          CodeLen = 1 + AdrCnt;
        }
      }
    }
  }
}

static void DecodeADDSUB(Word Index)
{
  if ((ArgCnt < 1) || (ArgCnt > 3)) WrError(1110);
  else
  {   
    DecodeAdr(1, MModAcc);
    switch (AdrMode)
    {
      case ModAcc:
        /* EA <-> E implicitly give operand size conflict, we don't have to check it here */

        DecodeAdr(2, MModMem | MModImm | MModE);
        switch (AdrMode)
        {
          case ModMem:
          case ModImm:
            BAsmCode[0] = Index | ((1 - OpSize) << 6) | AdrPart;
            memcpy(BAsmCode + 1, AdrVals, AdrCnt);
            CodeLen = 1 + AdrCnt;
            break;
          case ModE:
            BAsmCode[0] = Index - 0x40;
            CodeLen = 1;
            break;
        }
    }
  }
}

static void DecodeMulDiv(Word Index)
{
  if (ArgCnt != 2) WrError(1110);
  else
  {   
    OpSize = 1;
    DecodeAdr(1, MModAcc);
    if (AdrMode == ModAcc)
    {
      DecodeAdr(2, MModT);
      if (AdrMode == ModT)
      {
        BAsmCode[0] = Index;
        CodeLen = 1;
      }
    }
  }
}

static void DecodeLogic(Word Index)
{
  if ((ArgCnt < 1) || (ArgCnt > 3)) WrError(1110);
  else
  {
    OpSize = 0;
    DecodeAdr(1, MModAcc | MModS);
    switch (AdrMode)
    {
      case ModAcc:
        DecodeAdr(2, MModMem | MModImm | MModE);
        switch (AdrMode)
        {
          case ModMem:
          case ModImm:
            BAsmCode[0] = Index | AdrPart;
            memcpy(BAsmCode + 1, AdrVals, AdrCnt);
            CodeLen = 1 + AdrCnt;
            break;
          case ModE:
            BAsmCode[0] = Index - 0x80;
            CodeLen = 1;
            break;
        }
        break;
      case ModS:
        if (Index == 0xe0) WrError(1350);
        else
        {
          DecodeAdr(2, MModImm);
          if (AdrMode == ModImm)
          {
            BAsmCode[0] = (Index == 0xd0) ? 0x39 : 0x3b;
            BAsmCode[1] = *AdrVals;
            CodeLen = 2;
          }
        }
        break;
    }
  }
}

static void DecodeShift(Word Index)
{
  ShiftOrder *POrder = ShiftOrders + Index;

  if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr(1, MModAcc);
    if (AdrMode == ModAcc)
    {
      if ((OpSize == 1) && (!POrder->Code16)) WrError(1350);
      else
        BAsmCode[CodeLen++] = OpSize ? POrder->Code16 : POrder->Code;
    }
  }
}

static void DecodeStack(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr(1, MModAcc | MModReg);
    switch (AdrMode)
    {
      case ModAcc:
        BAsmCode[0] = 0x0a - (OpSize << 1);
        if (Index)
          BAsmCode[0] = (BAsmCode[0] | 0x30) ^ 2;
        CodeLen++;
        break;
      case ModReg:
        if ((AdrPart == 1) || ((Index) && (AdrPart == 0))) WrError(1350);
        else
          BAsmCode[CodeLen++] = 0x54 | Index | AdrPart;
        break;
    }
  }
}

static void DecodeIDLD(Word Index)
{
  if ((ArgCnt < 2) || (ArgCnt > 3)) WrError(1110);
  else
  {
    DecodeAdr(1, MModAcc);
    if (AdrMode == ModAcc)
    {
      if (OpSize == 1) WrError(1350);
      else
      {
        DecodeAdr(2, MModMem);
        if (AdrMode == ModMem)
        {
          BAsmCode[0] = 0x90 | Index | AdrPart;
          memcpy(BAsmCode + 1, AdrVals, AdrCnt);
          CodeLen = 1 + AdrCnt;
        }
      }
    }
  }
}

static void DecodeJMPJSR(Word Index)
{
  Word Address;
  Boolean OK;

  if (ArgCnt != 1) WrError(1110);
  else
  {
    Address = EvalIntExpression(ArgStr[1], UInt16, &OK);
    if (OK)
    {
      BAsmCode[CodeLen++] = Index;
      BAsmCode[CodeLen++] = Lo(Address);
      BAsmCode[CodeLen++] = Hi(Address);
    }
  }
}

static void DecodeCALL(Word Index)
{
  Boolean OK;

  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    BAsmCode[0] = 0x10 | EvalIntExpression(ArgStr[1], UInt4, &OK);
    if (OK)
      CodeLen = 1;
  }
}

static void DecodeBranch(Word Index)
{
  FixedOrder *POrder = BranchOrders + Index;

  /* allow both syntaxes for PC-relative addressing */

  if (ArgCnt == 1)
    strcpy(ArgStr[++ArgCnt], "PC");

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(1, MModMem);
    if (AdrMode == ModMem)
    {
      if ((AdrPart == 1) || (AdrPart > 3)) WrError(1350);
      else if ((POrder->Code < 0x60) && (AdrPart != 0))  WrError(1350);
      else
      {
        BAsmCode[0] = POrder->Code | AdrPart;
        memcpy(BAsmCode + 1, AdrVals, AdrCnt);
        CodeLen = 1 + AdrCnt;
      }
    }
  }
}

/*---------------------------------------------------------------------------*/

static void AddFixed(char *NName, Byte NCode)
{
  if (InstrZ >= FixedOrderCnt)
    exit(0);
  FixedOrders[InstrZ].Code = NCode;
  AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
}

static void AddBranch(char *NName, Byte NCode)
{
  if (InstrZ >= BranchOrderCnt)
    exit(0);
  BranchOrders[InstrZ].Code = NCode;
  AddInstTable(InstTable, NName, InstrZ++, DecodeBranch);
}

static void AddShift(char *NName, Byte NCode, Byte NCode16)
{
  if (InstrZ >= ShiftOrderCnt)
    exit(0);
  ShiftOrders[InstrZ].Code = NCode;
  ShiftOrders[InstrZ].Code16 = NCode16;
  AddInstTable(InstTable, NName, InstrZ++, DecodeShift);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(53);

  FixedOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * FixedOrderCnt); InstrZ = 0;
  AddFixed("RET", 0x5c);
  AddFixed("SSM", 0x2e);
  AddFixed("NOP", 0x00);

  ShiftOrders = (ShiftOrder*) malloc(sizeof(ShiftOrder) * ShiftOrderCnt); InstrZ = 0;
  AddShift("SR" , 0x3c, 0x0c);
  AddShift("SRL", 0x3d, 0x00);
  AddShift("RR" , 0x3e, 0x00);
  AddShift("RRL", 0x3f, 0x00);
  AddShift("SL" , 0x0e, 0x0f);

  AddInstTable(InstTable, "LD",     0, DecodeLD);
  AddInstTable(InstTable, "ST",     0, DecodeST);
  AddInstTable(InstTable, "XCH",    0, DecodeXCH);
  AddInstTable(InstTable, "PLI",    0, DecodePLI);

  AddInstTable(InstTable, "ADD", 0xb0, DecodeADDSUB);
  AddInstTable(InstTable, "SUB", 0xb8, DecodeADDSUB);

  AddInstTable(InstTable, "MPY", 0x2c, DecodeMulDiv);
  AddInstTable(InstTable, "DIV", 0x0d, DecodeMulDiv);

  AddInstTable(InstTable, "AND", 0xd0, DecodeLogic);
  AddInstTable(InstTable, "OR",  0xd8, DecodeLogic);
  AddInstTable(InstTable, "XOR", 0xe0, DecodeLogic);

  AddInstTable(InstTable, "PUSH",0x00, DecodeStack);
  AddInstTable(InstTable, "POP", 0x08, DecodeStack);

  AddInstTable(InstTable, "ILD" ,0x00, DecodeIDLD);
  AddInstTable(InstTable, "DLD", 0x08, DecodeIDLD);

  AddInstTable(InstTable, "JMP" ,0x24, DecodeJMPJSR);
  AddInstTable(InstTable, "JSR", 0x20, DecodeJMPJSR);
  AddInstTable(InstTable, "CALL",   0, DecodeCALL);

  BranchOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * BranchOrderCnt); InstrZ = 0;
  AddBranch("BND", 0x2d);
  AddBranch("BRA", 0x74);
  AddBranch("BP" , 0x64);
  AddBranch("BZ" , 0x6c);
  AddBranch("BNZ", 0x7c);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
  free(FixedOrders);
  free(ShiftOrders);
}

/*---------------------------------------------------------------------------*/

static void MakeCode_807x(void) 
{
   CodeLen=0; DontPrint=False; OpSize = -1;

   /* zu ignorierendes */

   if (Memo("")) return;

   if (DecodeIntelPseudo(False)) return;

   if (!LookupInstTable(InstTable,OpPart))
     WrXError(1200,OpPart);
}

static Boolean IsDef_807x(void)
{
   return False;
}

static void SwitchFrom_807x(void)
{
   DeinitFields();
}

static void SwitchTo_807x(void)
{
   PFamilyDescr FoundDescr;

   FoundDescr = FindFamilyByName("807x");

   TurnWords = False; ConstMode = ConstModeIntel; SetIsOccupied = False;

   PCSymbol="$"; HeaderID = FoundDescr->Id; NOPCode = 0x00;
   DivideChars = ","; HasAttrs = False;

   ValidSegs = 1 << SegCode;
   Grans[SegCode] = 1; ListGrans[SegCode] = 1; SegInits[SegCode] = 0;
   SegLimits[SegCode] = 0xffff;

   MakeCode = MakeCode_807x; IsDef = IsDef_807x;
   SwitchFrom = SwitchFrom_807x; InitFields();
}

/*---------------------------------------------------------------------------*/

void code807x_init(void)
{
  CPU8070 = AddCPU("8070", SwitchTo_807x);
}
