/* codecop4.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegeneratormodul COP4-Familie                                           */
/*                                                                           */
/*****************************************************************************/
/* $Id: codecop4.c,v 1.2 2006/04/09 12:40:11 alfred Exp $                   *
 *****************************************************************************
 * $Log: codecop4.c,v $
 * Revision 1.2  2006/04/09 12:40:11  alfred
 * - unify COP pseudo instructions
 *
 * Revision 1.1  2006/04/06 20:26:54  alfred
 * - add COP4
 *
 *****************************************************************************/

#include "stdinc.h"

#include "bpemu.h"
#include "asmdef.h"
#include "asmpars.h"
#include "asmsub.h"
#include "asmitree.h"
#include "headids.h"
#include "codevars.h"
#include "intpseudo.h"
#include "natpseudo.h"

#include "codecop4.h"

static CPUVar CPUCOP410;

/*---------------------------------------------------------------------------*/
/* Code Generators */

static void DecodeFixed(Word Index)
{
  if (ArgCnt != 0) WrError(1110);
  else
  {
    if (Hi(Index))
      BAsmCode[CodeLen++] = Hi(Index);
    BAsmCode[CodeLen++] = Lo(Index);
  }
}

static void DecodeSK(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Byte Bit;
    Boolean OK;

    Bit = EvalIntExpression(ArgStr[1], UInt2, &OK);
    if (OK)
    {
      if (Index)
        BAsmCode[CodeLen++] = Index;
      BAsmCode[CodeLen++] = 0x01 | ((Bit & 1) << 4) | (Bit & 2);
    }
  }
}

static void DecodeImm(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Byte Val;
    Boolean OK;

    Val = EvalIntExpression(ArgStr[1], Int4, &OK);
    if (OK)
    {
      if (Hi(Index))
        BAsmCode[CodeLen++] = Hi(Index);
      BAsmCode[CodeLen++] = Lo(Index) | (Val & 0x0f);
    }
  }
}

static void DecodeJmp(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Word Addr;
    Boolean OK;
 
    Addr = EvalIntExpression(ArgStr[1], UInt9, &OK);
    if (OK)
    {
      BAsmCode[CodeLen++] = Index | Hi(Addr);
      BAsmCode[CodeLen++] = Lo(Addr);
    }
  }
}
 
static void DecodeReg(Word Index)
{
  if (ArgCnt != 1) WrError(1110);  
  else
  {
    Byte Reg;
    Boolean OK;

    Reg = EvalIntExpression(ArgStr[1], UInt2, &OK);   
    if (OK)
    {
      BAsmCode[CodeLen++] = Index | ((Reg & 3) << 4);;
    }
  }
}

static void DecodeRMB(Word Index)
{
  if (ArgCnt != 1) WrError(1110);  
  else
  {
    Byte Reg;
    Boolean OK;
    static Byte Vals[4] = { 0x4c, 0x45, 0x42, 0x43 };
 
    Reg = EvalIntExpression(ArgStr[1], UInt2, &OK);
    if (OK)
    {
      BAsmCode[CodeLen++] = Vals[Reg & 3];
    }
  }
}

static void DecodeSMB(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {   
    Byte Reg;
    Boolean OK;
    static Byte Vals[4] = { 0x4d, 0x47, 0x46, 0x4b };
 
    Reg = EvalIntExpression(ArgStr[1], UInt2, &OK);
    if (OK)
    {
      BAsmCode[CodeLen++] = Vals[Reg & 3];                
    }
  }  
}    

static void DecodeXAD(Word Index)
{
  if (ArgCnt != 2) WrError(1110);
  else
  {   
    Byte Reg1, Reg2;
    Boolean OK;
 
    Reg1 = EvalIntExpression(ArgStr[1], UInt4, &OK);
    if (OK)
    {
      Reg2 = EvalIntExpression(ArgStr[2], UInt4, &OK);
      if (OK)
      {
        BAsmCode[CodeLen++] = 0x20 | Reg1;
        BAsmCode[CodeLen++] = 0xb0 | Reg2;
      }
    }
  }  
}    
     
static void DecodeLBI(Word Index)
{
  if (ArgCnt != 2) WrError(1110);
  else
  {   
    Byte Reg, Val;
    Boolean OK;
 
    Reg = EvalIntExpression(ArgStr[1], UInt2, &OK);
    if (OK)
    {
      Val = EvalIntExpression(ArgStr[2], UInt4, &OK);
      if (OK)
      {
        Val = (Val - 1) & 0x0f;
        BAsmCode[CodeLen++] = (Reg << 4) | Val;
      }
    }
  }  
}    
     
static void DecodeJSRP(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Word Addr;
    Boolean OK;
 
    FirstPassUnknown = FALSE;
    Addr = EvalIntExpression(ArgStr[1], UInt9, &OK);
    if (FirstPassUnknown)
      Addr = 2 << 6;
    if (OK)
    {
      if (((Addr >> 6) != 2) || (Addr == 0xbf)) WrError(1905);
      else if ((EProgCounter() >> 7) == 1) WrError(1900);
      else
        BAsmCode[CodeLen++] = 0x80 | (Addr & 0x3f);
    }
  }   
}     

static void DecodeJP(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Word Addr, CurrPC;
    Boolean OK;
 
    FirstPassUnknown = FALSE;
    Addr = EvalIntExpression(ArgStr[1], UInt9, &OK);
    if (FirstPassUnknown)
      Addr = EProgCounter() & (~0x1f);
    if (OK)
    {
      CurrPC = EProgCounter();
      if ((Addr & 0x3f) == 0x3f)
        WrError(1905);
      if (((CurrPC >> 7) == 1) && ((Addr >> 7) == 1))
        BAsmCode[CodeLen++] = 0x80 | (Addr & 0x7f);
      else if ((CurrPC >> 6) == (Addr >> 6))
        BAsmCode[CodeLen++] = 0xc0 | (Addr & 0x3f);
      else
        WrError(1905);
    }
  }   
}     

/*---------------------------------------------------------------------------*/
/* Code Table Handling */

static void InitFields(void)
{
  InstTable = CreateInstTable(103);

  AddInstTable(InstTable, "ASC"  , 0x30, DecodeFixed);
  AddInstTable(InstTable, "ADD"  , 0x31, DecodeFixed);
  AddInstTable(InstTable, "CLRA" , 0x00, DecodeFixed);
  AddInstTable(InstTable, "COMP" , 0x40, DecodeFixed);
  AddInstTable(InstTable, "NOP"  , 0x44, DecodeFixed);
  AddInstTable(InstTable, "RC"   , 0x32, DecodeFixed);
  AddInstTable(InstTable, "SC"   , 0x22, DecodeFixed);
  AddInstTable(InstTable, "XOR"  , 0x02, DecodeFixed);
  AddInstTable(InstTable, "JID"  , 0xff, DecodeFixed);
  AddInstTable(InstTable, "RET"  , 0x48, DecodeFixed);
  AddInstTable(InstTable, "RETSK", 0x49, DecodeFixed);
  AddInstTable(InstTable, "CAMQ" , 0x333c, DecodeFixed);
  AddInstTable(InstTable, "LQID" , 0xbf, DecodeFixed);
  AddInstTable(InstTable, "CAB"  , 0x50, DecodeFixed);
  AddInstTable(InstTable, "CBA"  , 0x4e, DecodeFixed);
  AddInstTable(InstTable, "SKC"  , 0x20, DecodeFixed);
  AddInstTable(InstTable, "SKE"  , 0x21, DecodeFixed);
  AddInstTable(InstTable, "SKGZ" , 0x3321, DecodeFixed);
  AddInstTable(InstTable, "ING"  , 0x332a, DecodeFixed);
  AddInstTable(InstTable, "INL"  , 0x332e, DecodeFixed);
  AddInstTable(InstTable, "OBD"  , 0x333e, DecodeFixed);
  AddInstTable(InstTable, "OMG"  , 0x333a, DecodeFixed);
  AddInstTable(InstTable, "XAS"  , 0x4f, DecodeFixed);

  AddInstTable(InstTable, "SKGBZ", 0x33, DecodeSK);
  AddInstTable(InstTable, "SKMBZ", 0x00, DecodeSK);

  AddInstTable(InstTable, "AISC" , 0x50, DecodeImm);
  AddInstTable(InstTable, "STII" , 0x70, DecodeImm);
  AddInstTable(InstTable, "LEI"  , 0x3360, DecodeImm);

  AddInstTable(InstTable, "JMP"  , 0x60, DecodeJmp);
  AddInstTable(InstTable, "JSR"  , 0x68, DecodeJmp);

  AddInstTable(InstTable, "LD"   , 0x05, DecodeReg);
  AddInstTable(InstTable, "X"    , 0x06, DecodeReg);
  AddInstTable(InstTable, "XDS"  , 0x07, DecodeReg);
  AddInstTable(InstTable, "XIS"  , 0x04, DecodeReg);

  AddInstTable(InstTable, "RMB"  , 0x00, DecodeRMB);
  AddInstTable(InstTable, "SMB"  , 0x00, DecodeSMB);

  AddInstTable(InstTable, "XAD"  , 0x00, DecodeXAD);

  AddInstTable(InstTable, "LBI"  , 0x00, DecodeLBI);  

  AddInstTable(InstTable, "JSRP"  , 0x00, DecodeJSRP);

  AddInstTable(InstTable, "JP"    , 0x00, DecodeJP);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*---------------------------------------------------------------------------*/

static void MakeCode_COP4(void)
{
  Boolean BigFlag;

  CodeLen = 0; DontPrint = False;

  /* zu ignorierendes */

  if (*OpPart == '\0') return;

  /* pseudo instructions */

  if (DecodeNatPseudo(&BigFlag)) return;

  if (DecodeIntelPseudo(BigFlag)) return;

  /* machine instructions */

  if (!LookupInstTable(InstTable, OpPart))
    WrXError(1200, OpPart);
}

static void SwitchFrom_COP4(void)
{
  DeinitFields();
}

static Boolean IsDef_COP4(void)
{
  return False;
}

static void SwitchTo_COP4(void)
{
  PFamilyDescr pDescr;

  pDescr = FindFamilyByName("COP4");

  TurnWords = False; ConstMode = ConstModeC; SetIsOccupied = False;

  PCSymbol = "."; HeaderID = pDescr->Id;
  NOPCode = 0x44;
  DivideChars = ","; HasAttrs = False;

  ValidSegs = (1 << SegCode);
  Grans[SegCode] = 1; ListGrans[SegCode] = 1; SegInits[SegCode] = 0;
  SegLimits[SegCode] = 0x1ff;

  MakeCode = MakeCode_COP4; IsDef = IsDef_COP4;
  SwitchFrom = SwitchFrom_COP4; InitFields();
}

void codecop4_init(void)
{
  CPUCOP410 = AddCPU("COP410", SwitchTo_COP4);
}
