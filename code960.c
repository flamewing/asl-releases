/* code960.c */
/*****************************************************************************/
/* Makroassembler AS                                                         */
/*                                                                           */
/* Codegenerator i960-Familie                                                */
/*                                                                           */
/* Historie: 24. 8.1998 angelegt                                             */
/*           25. 8.1998 Register-Format                                      */
/*           27. 8.1998 Reg-Struktur hat getrennte Operandentypen            */
/*                      Fixed+COBR-Anweisungen                               */
/*           28. 8.1998 CTRL-Anweisungen                                     */
/*                      MEM-Anweisungen                                      */
/*                      MODPC privilegiert gemacht                           */
/*                      Sonderregisternamen                                  */
/*                      nicht ausgerichtete Instruktionen anmeckern          */
/*           29. 8.1998 Abfrage Id per headerid-Modul                        */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*           23. 1.1999 Unsauberkeit in Zuweisung (-1 an unsigned) beseitigt */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*           14. 1.2001 silenced warnings about unused parameters            */
/*                                                                           */
/*****************************************************************************/
/* $Id: code960.c,v 1.11 2017/06/07 19:33:12 alfred Exp $                     */
/*****************************************************************************
 * $Log: code960.c,v $
 * Revision 1.11  2017/06/07 19:33:12  alfred
 * - correct SCANBIT -> SPANBIT instruction
 *
 * Revision 1.10  2014/12/07 19:14:00  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.9  2014/12/05 11:58:16  alfred
 * - collapse STDC queries into one file
 *
 * Revision 1.8  2014/11/05 15:47:15  alfred
 * - replace InitPass callchain with registry
 *
 * Revision 1.7  2014/07/12 14:45:17  alfred
 * - reworked to current style
 *
 * Revision 1.6  2010/04/17 13:14:23  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.5  2007/11/24 22:48:06  alfred
 * - some NetBSD changes
 *
 * Revision 1.4  2005/09/08 16:53:42  alfred
 * - use common PInstTable
 *
 * Revision 1.3  2005/05/21 16:35:05  alfred
 * - removed variables available globally
 *
 * Revision 1.2  2004/05/29 11:33:02  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 * Revision 1.1  2003/11/06 02:49:22  alfred
 * - recreated
 *
 * Revision 1.2  2002/08/14 18:43:49  alfred
 * - warn null allocation, remove some warnings
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "endian.h"
#include "strutil.h"
#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "asmitree.h"
#include "codevars.h"            
#include "intpseudo.h"
#include "headids.h"
#include "intconsts.h"

/*--------------------------------------------------------------------------*/

enum
{
  ModNone = -1,
  ModReg = 0,
  ModFReg = 1,
  ModImm = 2
};

#define MModReg (1 << ModReg)
#define MModFReg (1 << ModFReg)
#define MModImm (1 << ModImm)

#define FixedOrderCnt 13
#define RegOrderCnt 116
#define CobrOrderCnt 24
#define CtrlOrderCnt 11
#define MemOrderCnt 20
#define SpecRegCnt 4

typedef enum
{
  NoneOp, IntOp, LongOp, QuadOp, SingleOp, DoubleOp, ExtOp, OpCnt
} OpType;

static LongWord OpMasks[OpCnt] =
{
  0xffffffff, 0, 1, 3, 0, 1, 3
};

typedef struct
{
  LongWord Code;
} FixedOrder;

typedef struct
{
  LongWord Code;
  Boolean HasSrc;
} CobrOrder;

typedef struct
{
  LongWord Code;
  OpType Src1Type, Src2Type, DestType;
  Boolean Imm1, Imm2, Privileged;
} RegOrder;

typedef struct
{
  LongWord Code;
  OpType Type;
  ShortInt RegPos;
} MemOrder;

typedef struct
{
  char *Name;
  LongWord Code;
} SpecReg;

static FixedOrder *FixedOrders;
static RegOrder *RegOrders;
static CobrOrder *CobrOrders;
static FixedOrder *CtrlOrders;
static MemOrder *MemOrders;
static SpecReg *SpecRegs;

static CPUVar CPU80960;

/*--------------------------------------------------------------------------*/

static Boolean ChkAdr(int AMode, Byte Mask, LongWord *Erg, LongWord *Mode)
{
  UNUSED(Erg);

  if (!(Mask & (1 << AMode)))
  {
    WrError(1350);
    return False;
  }
  else
  {
    *Mode = (AMode != ModReg);
    return True;
  }
}

static Boolean DecodeIReg(char *Asc, LongWord *Erg)
{
  int z;
  char *end;

  for (z = 0; z < SpecRegCnt; z++)
   if (!strcasecmp(Asc, SpecRegs[z].Name))
   {
     *Erg = SpecRegs[z].Code;
     return True;
   }

  if ((mytoupper(*Asc) == 'G') || (mytoupper(*Asc) == 'R'))
  {
    *Erg = strtol(Asc + 1, &end, 10);
    if ((*end == '\0') && (*Erg <= 15))
    {
      if (mytoupper(*Asc) == 'G')
        *Erg += 16;
      return TRUE;
    }
  }

  return False;
}

static Boolean DecodeAdr(char *Asc, Byte Mask, OpType Type, LongWord *Erg, LongWord *Mode)
{
  char *end;
  Double FVal;
  Boolean OK;

  *Mode = ModNone;
  *Erg = 0;

  if (DecodeIReg(Asc, Erg))
  {
    if ((*Erg) & OpMasks[Type])
    {
      WrXError(1760, Asc);
      return False;
    }
    else
      return ChkAdr(ModReg, Mask, Erg, Mode);
  }

  if (!strncasecmp(Asc, "FP", 2))
  {
    *Erg = strtol(Asc + 2, &end, 10);
    if ((*end == '\0') && (*Erg <= 3))
      return ChkAdr(ModFReg, Mask, Erg, Mode);
  }

  if (Type != IntOp)
  {
    FirstPassUnknown = False;
    FVal = EvalFloatExpression(Asc, Float64, &OK);
    if (OK)
    {
      if (FirstPassUnknown)
        FVal=0.0;
      if (FVal == 0.0)
        *Erg = 16;
      else if (FVal == 1.0)
        *Erg = 22;
      else
      {
        WrError(1320);
        OK = False;
      }
      if (OK)
        return ChkAdr(ModImm, Mask, Erg, Mode);
    }
  }
  else
  {
    *Erg = EvalIntExpression(Asc, UInt5, &OK);
    if (OK)
      return ChkAdr(ModImm, Mask, Erg, Mode);
  }
  return False;
}

#define NOREG 33
#define IPREG 32

static int AddrError(int Num)
{
  WrError(Num);
  return -1;
}

static int AddrXError(int Num, char *Param)
{
  WrXError(Num, Param);
  return -1;
}

static int DecodeMem(char *Asc, LongWord *Erg, LongWord *Ext)
{
  LongInt DispAcc;
  LongWord Base, Index, Scale, Mode;
  Boolean Done;
  int l, Scale2;
  char *p, *p2, *end;
  Boolean OK;

  Base = Index = NOREG;
  Scale = 0;

  /* Register abhobeln */

  Done = FALSE;
  do
  {
    l = strlen(Asc);
    if (l == 0)
      Done = True;
    else switch (Asc[l-1])
    {
      case ']':
        if (Index != NOREG) return AddrError(1350);
        for (p = Asc + l - 1; p >= Asc; p--)
          if (*p == '[')
            break;
        if (p < Asc) return AddrError(1300);
        *(p++) = '\0';
        Asc[l - 1] = '\0';
        p2 = strchr(p, '*');
        if (p2)
        {
          *p2 = '\0';
          Scale2 = strtol(p2 + 1, &end, 10);
          if (*end != '\0') return AddrError(1350);
          for (Scale = 0; Scale < 5; Scale++, Scale2 = Scale2 >> 1)
            if (Odd(Scale2))
              break;
          if (Scale2 != 1) return AddrError(1350);
        }
        if (!DecodeIReg(p, &Index))
          return AddrXError(1445, p);
        break;
      case ')':
        if (Base != NOREG) return AddrError(1350);
        for (p = Asc + l - 1; p >= Asc; p--)
          if (*p == '(')
            break;
        if (p < Asc) return AddrError(1300);
        *(p++) = '\0';
        Asc[l - 1] = '\0';
        if (!strcasecmp(p, "IP"))
          Base = IPREG;
        else if (!DecodeIReg(p, &Base)) return AddrXError(1445, p);
        break;
      default:
        Done = True;
    }
  }
  while (!Done);
  
  DispAcc = EvalIntExpression(Asc, Int32, &OK);

  if (Base == IPREG)
  {
    DispAcc -= EProgCounter() + 8;
    if (Index != NOREG) return AddrError(1350);
    else
    {
      *Erg = (5 << 10);
      *Ext = DispAcc;
      return 1;
    }
  }
  else if ((Index == NOREG) && (DispAcc >= 0) && (DispAcc<=4095))
  {
    *Erg = DispAcc;
    if (Base != NOREG)
      *Erg += 0x2000 + (Base << 14);
    return 0;
  }
  else
  {
    Mode = (Ord(DispAcc != 0) << 3) + 4 + (Ord(Index != NOREG) << 1) + Ord(Base != NOREG);
    if ((Mode & 9) == 0)
      Mode += 8;
    if (Mode == 5)
      Mode--;
    *Erg = (Mode << 10);
    if (Base != NOREG)
      *Erg += Base << 14;
    if (Index != NOREG)
      *Erg += Index + (Scale << 7);
    if (Mode < 8) return 0;
    else
    {
      *Ext = DispAcc;
      return 1;
    }
  }
}

/*--------------------------------------------------------------------------*/

static void DecodeFixed(Word Index)
{
  FixedOrder *Op = FixedOrders + Index;
   
  if (ArgCnt != 0) WrError(1110);
  else
  {
    DAsmCode[0] = Op->Code;
    CodeLen = 4;
  }
}

static void DecodeReg(Word Index)
{
  RegOrder *Op = RegOrders + Index;
  LongWord DReg = 0, DMode = 0;
  LongWord S1Reg = 0, S1Mode = 0;
  LongWord S2Reg = 0, S2Mode = 0;

  if ((Op->DestType != NoneOp) && (ArgCnt == 1 + Ord(Op->Src2Type != NoneOp))) 
  {
    strcpy(ArgStr[ArgCnt + 1], ArgStr[ArgCnt]);
    ArgCnt++;
  }

  if (ArgCnt != 1 + Ord(Op->Src2Type != NoneOp) + Ord(Op->DestType != NoneOp)) WrError(1110);
  else if (((Op->DestType >= SingleOp) || (Op->Src1Type >= SingleOp)) && (!FPUAvail)) WrXError(1200,OpPart);
  else if (((Op->DestType == NoneOp) || (DecodeAdr(ArgStr[ArgCnt], MModReg | (Op->DestType >= SingleOp ? MModFReg :0 ), Op->DestType, &DReg, &DMode)))
        && (DecodeAdr(ArgStr[1], MModReg | (Op->Src1Type >= SingleOp ? MModFReg : 0) | (Op->Imm1 ? MModImm : 0 ), Op->Src1Type, &S1Reg, &S1Mode))
        && ((Op->Src2Type == NoneOp) || (DecodeAdr(ArgStr[2], MModReg | (Op->Src2Type >= SingleOp ? MModFReg : 0) | (Op->Imm2 ? MModImm : 0), Op->Src2Type, &S2Reg, &S2Mode))))
  {
    DAsmCode[0] = ((Op->Code & 0xff0) << 20)
                + ((Op->Code & 0xf) << 7)
                + (S1Reg)
                + (S2Reg << 14)
                + (DReg << 19)
                + (S1Mode << 11)
                + (S2Mode << 12)
                + (DMode << 13);
    CodeLen = 4;
    if ((Op->Privileged) && (!SupAllowed)) WrError(50);
  }
}

static void DecodeCobr(Word Index)
{
  CobrOrder *Op = CobrOrders + Index;
  LongWord S1Reg, S1Mode;
  LongWord S2Reg = 0, S2Mode = 0;
  LongInt AdrInt;
  Boolean OK;

  if (ArgCnt != 1 + 2 * Ord(Op->HasSrc)) WrError(1110);
  else if ((DecodeAdr(ArgStr[1], MModReg | (Op->HasSrc ? MModImm : 0), IntOp, &S1Reg, &S1Mode))
        && ((!Op->HasSrc) || (DecodeAdr(ArgStr[2], MModReg, IntOp, &S2Reg, &S2Mode))))
  {
    FirstPassUnknown = False;
    OK = True;
    AdrInt = (Op->HasSrc) ? EvalIntExpression(ArgStr[3], UInt32, &OK) - EProgCounter() : 0;
    if (FirstPassUnknown)
      AdrInt &= (~3);
    if (OK)
    {
      if (AdrInt & 3) WrError(1325);
      else if ((!SymbolQuestionable) && ((AdrInt < -4096) || (AdrInt > 4090))) WrError(1370);
      else
      {
        DAsmCode[0] = (Op->Code << 24)
                    + (S1Reg << 19)
                    + (S2Reg << 14)
                    + (S1Mode << 13)
                    + (AdrInt & 0x1ffc);
        CodeLen = 4;
      }
    }
  }
}

static void DecodeCtrl(Word Index)
{
  FixedOrder *Op = CtrlOrders + Index;
  LongInt AdrInt;
  Boolean OK;

  if (ArgCnt != 1) WrError(1110);
  else
  {
    FirstPassUnknown = False;
    AdrInt = EvalIntExpression(ArgStr[1], UInt32, &OK) - EProgCounter();
    if (FirstPassUnknown) AdrInt &= (~3);
    if (OK)
    {
      if (AdrInt & 3) WrError(1325);
      else if ((!SymbolQuestionable) && ((AdrInt < -8388608) || (AdrInt > 8388604))) WrError(1370);
      else
      {
        DAsmCode[0] = (Op->Code << 24) + (AdrInt & 0xfffffc);
        CodeLen = 4;
      }
    }
  }
}

static void DecodeMemO(Word Index)
{
  MemOrder *Op = MemOrders + Index;
  LongWord Reg = 0, Mem = 0;
  int MemType;
  ShortInt MemPos = (Op->RegPos > 0) ? 3 - Op->RegPos : 1;

  if (ArgCnt != 1 + Ord(Op->RegPos > 0)) WrError(1110);
  else if ((Op->RegPos > 0) && (!DecodeIReg(ArgStr[Op->RegPos], &Reg))) WrXError(1445, ArgStr[Op->RegPos]);
  else if (Reg & OpMasks[Op->Type]) WrXError(1445, ArgStr[Op->RegPos]);
  else if ((MemType = DecodeMem(ArgStr[MemPos],&Mem,DAsmCode+1)) >= 0)
  {
    DAsmCode[0] = (Op->Code << 24) + (Reg << 19) + Mem;
    CodeLen = (1 + MemType) << 2;
  }
}

static Boolean DecodePseudo(void)
{
  int z;
  Boolean OK;
  LongWord Size;

  if (Memo("WORD"))
  {
    if (ArgCnt < 1) WrError(1110);
    else
    {
      OK = True;
      z = 1;
      while ((z <= ArgCnt) && (OK))
      {
        DAsmCode[z - 1] = EvalIntExpression(ArgStr[z], Int32, &OK);
        z++;
      }
      if (OK)
        CodeLen = 4 * ArgCnt;
    }
    return True;
  }

  if (Memo("SPACE"))
  {
    if (ArgCnt != 1) WrError(1110);
    else
    {
      FirstPassUnknown = False;
      Size = EvalIntExpression(ArgStr[1], UInt16, &OK);
      if (FirstPassUnknown) WrError(1820);
      if ((OK) && (!FirstPassUnknown))
      {
        DontPrint = True;
        if (!Size) WrError(290);
        CodeLen = Size;
        BookKeeping();
      }
    }
    return True;
  }

  return False;
}

/*--------------------------------------------------------------------------*/

static void MakeCode_960(void)
{
  CodeLen = 0;
  DontPrint = False;

  /* Nullanweisung */

  if (Memo(""))
    return;

  /* Pseudoanweisungen */

  if (DecodeIntelPseudo(False))
    return;

  if (DecodePseudo())
    return;

  /* Befehlszaehler nicht ausgerichtet? */

  if (EProgCounter() & 3)
    WrError(180);

  /* CPU-Anweisungen */

  if (!LookupInstTable(InstTable, OpPart))
    WrXError(1200, OpPart);
}

/*--------------------------------------------------------------------------*/

static void AddFixed(char *NName, LongWord NCode)
{
  if (InstrZ >= FixedOrderCnt) exit(255);
  FixedOrders[InstrZ].Code = NCode;
  AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
}

static void AddReg(char *NName, LongWord NCode,
                   OpType NSrc1, OpType NSrc2, OpType NDest,
                   Boolean NImm1, Boolean NImm2, Boolean NPriv)
{
  if (InstrZ >= RegOrderCnt) exit(255);
  RegOrders[InstrZ].Code = NCode;
  RegOrders[InstrZ].Src1Type = NSrc1;
  RegOrders[InstrZ].Src2Type = NSrc2;
  RegOrders[InstrZ].DestType = NDest;
  RegOrders[InstrZ].Imm1 = NImm1;
  RegOrders[InstrZ].Imm2 = NImm2;
  RegOrders[InstrZ].Privileged = NPriv;
  AddInstTable(InstTable, NName, InstrZ++, DecodeReg);
}

static void AddCobr(char *NName, LongWord NCode, Boolean NHas)
{
  if (InstrZ >= CobrOrderCnt) exit(255);
  CobrOrders[InstrZ].Code = NCode;
  CobrOrders[InstrZ].HasSrc = NHas;
  AddInstTable(InstTable, NName, InstrZ++, DecodeCobr);
}

static void AddCtrl(char *NName, LongWord NCode)
{
  if (InstrZ >= CtrlOrderCnt) exit(255);
  CtrlOrders[InstrZ].Code = NCode;
  AddInstTable(InstTable, NName, InstrZ++, DecodeCtrl);
}

static void AddMem(char *NName, LongWord NCode, OpType NType, int NPos)
{
  if (InstrZ >= MemOrderCnt) exit(255);
  MemOrders[InstrZ].Code = NCode;
  MemOrders[InstrZ].Type = NType;
  MemOrders[InstrZ].RegPos = NPos;
  AddInstTable(InstTable, NName, InstrZ++, DecodeMemO);
}

static void AddSpecReg(char *NName, LongWord NCode)
{
  if (InstrZ >= SpecRegCnt) exit(255);
  SpecRegs[InstrZ].Code = NCode;
  SpecRegs[InstrZ++].Name = NName;
}

static void InitFields(void)
{
  InstTable = CreateInstTable(301);
  
  FixedOrders = (FixedOrder*) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ = 0;
  AddFixed("FLUSHREG", 0x66000680);
  AddFixed("FMARK"   , 0x66000600);
  AddFixed("MARK"    , 0x66000580);
  AddFixed("RET"     , 0x0a000000);
  AddFixed("SYNCF"   , 0x66000780);
  AddFixed("FAULTNO" , 0x18000000);
  AddFixed("FAULTG"  , 0x19000000);
  AddFixed("FAULTE"  , 0x1a000000);
  AddFixed("FAULTGE" , 0x1b000000);
  AddFixed("FAULTL"  , 0x1c000000);
  AddFixed("FAULTNE" , 0x1d000000);
  AddFixed("FAULTLE" , 0x1e000000);
  AddFixed("FAULTO"  , 0x1f000000);

  RegOrders = (RegOrder*) malloc(sizeof(RegOrder)*RegOrderCnt); InstrZ = 0;
      /*  Name       OpCode Src1Type  Src2Type  DestType  Imm1   Imm2 */
  AddReg("ADDC"    , 0x5b0, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("ADDI"    , 0x591, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("ADDO"    , 0x590, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("ADDR"    , 0x78f, SingleOp, SingleOp, SingleOp, True , True , False);
  AddReg("ADDRL"   , 0x79f, DoubleOp, DoubleOp, DoubleOp, True , True , False);
  AddReg("ALTERBIT", 0x58f, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("AND"     , 0x581, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("ANDNOT"  , 0x582, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("ATADD"   , 0x612, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("ATANR"   , 0x680, SingleOp, SingleOp, SingleOp, True , True , False);
  AddReg("ATANRL"  , 0x690, DoubleOp, DoubleOp, DoubleOp, True , True , False);
  AddReg("ATMOD"   , 0x610, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("CALLS"   , 0x660, IntOp   , NoneOp  , NoneOp  , True , False, False);
  AddReg("CHKBIT"  , 0x5ae, IntOp   , IntOp   , NoneOp  , True , True , False);
  AddReg("CLASSR"  , 0x68f, SingleOp, NoneOp  , NoneOp  , True , False, False);
  AddReg("CLASSRL" , 0x69f, DoubleOp, NoneOp  , NoneOp  , True , False, False);
  AddReg("CLRBIT"  , 0x58c, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("CMPDECI" , 0x5a7, IntOp   , IntOp   , IntOp   , True , False, False);
  AddReg("CMPDECO" , 0x5a6, IntOp   , IntOp   , IntOp   , True , False, False);
  AddReg("CMPI"    , 0x5a1, IntOp   , IntOp   , NoneOp  , True , True , False);
  AddReg("CMPO"    , 0x5a0, IntOp   , IntOp   , NoneOp  , True , True , False);
  AddReg("CMPINCI" , 0x5a5, IntOp   , IntOp   , IntOp   , True , False, False);
  AddReg("CMPINCO" , 0x5a4, IntOp   , IntOp   , IntOp   , True , False, False);
  AddReg("CMPOR"   , 0x684, SingleOp, SingleOp, NoneOp  , True , True , False);
  AddReg("CMPORL"  , 0x694, DoubleOp, DoubleOp, NoneOp  , True , True , False);
  AddReg("CMPR"    , 0x685, SingleOp, SingleOp, NoneOp  , True , True , False);
  AddReg("CMPRL"   , 0x695, DoubleOp, DoubleOp, NoneOp  , True , True , False);
  AddReg("CONCMPI" , 0x5a3, IntOp   , IntOp   , NoneOp  , True , True , False);
  AddReg("CONCMPO" , 0x5a2, IntOp   , IntOp   , NoneOp  , True , True , False);
  AddReg("COSR"    , 0x68d, SingleOp, NoneOp  , SingleOp, True , False, False);
  AddReg("COSRL"   , 0x69d, DoubleOp, NoneOp  , DoubleOp, True , False, False);
  AddReg("CPYSRE"  , 0x6e2, SingleOp, SingleOp, SingleOp, True , True , False);
  AddReg("CPYRSRE" , 0x6e3, SingleOp, SingleOp, SingleOp, True , True , False);
  AddReg("CVTIR"   , 0x674, IntOp,    NoneOp  , SingleOp, True , False, False);
  AddReg("CVTILR"  , 0x675, LongOp,   NoneOp  , DoubleOp, True , False, False);
  AddReg("CVTRI"   , 0x6c0, SingleOp, NoneOp  , IntOp   , True , False, False);
  AddReg("CVTRIL"  , 0x6c1, SingleOp, NoneOp  , LongOp  , True , False, False);
  AddReg("CVTZRI"  , 0x6c2, IntOp   , NoneOp  , IntOp   , True , False, False);
  AddReg("CVTZRIL" , 0x6c2, LongOp  , NoneOp  , LongOp  , True , False, False);
      /*  Name       OpCode Src1Type  Src2Type  DestType  Imm1   Imm2 */
  AddReg("DADDC"   , 0x642, IntOp   , IntOp   , IntOp   , False, False, False);
  AddReg("DIVI"    , 0x74b, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("DIVO"    , 0x70b, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("DIVR"    , 0x78b, SingleOp, SingleOp, SingleOp, True , True , False);
  AddReg("DIVRL"   , 0x79b, DoubleOp, DoubleOp, DoubleOp, True , True , False);
  AddReg("DMOVT"   , 0x644, IntOp   , NoneOp  , IntOp   , False, False, False);
  AddReg("DSUBC"   , 0x643, IntOp   , IntOp   , IntOp   , False, False, False);
  AddReg("EDIV"    , 0x671, IntOp   , LongOp  , LongOp  , True , True , False);
  AddReg("EMUL"    , 0x670, IntOp   , IntOp   , LongOp  , True , True , False);
  AddReg("EXPR"    , 0x689, SingleOp, NoneOp  , SingleOp, True , False, False);
  AddReg("EXPRL"   , 0x699, DoubleOp, NoneOp  , DoubleOp, True , False, False);
  AddReg("EXTRACT" , 0x651, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("LOGBNR"  , 0x68a, SingleOp, NoneOp  , SingleOp, True , False, False);
  AddReg("LOGBNRL" , 0x69a, DoubleOp, NoneOp  , DoubleOp, True , False, False);
  AddReg("LOGEPR"  , 0x681, SingleOp, SingleOp, SingleOp, True , True , False);
  AddReg("LOGEPRL" , 0x691, DoubleOp, DoubleOp, DoubleOp, True , True , False);
  AddReg("LOGR"    , 0x682, SingleOp, SingleOp, SingleOp, True , True , False);
  AddReg("LOGRL"   , 0x692, DoubleOp, DoubleOp, DoubleOp, True , True , False);
  AddReg("MODAC"   , 0x645, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("MODI"    , 0x749, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("MODIFY"  , 0x650, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("MODPC"   , 0x655, IntOp   , IntOp   , IntOp   , True , True , True );
  AddReg("MODTC"   , 0x654, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("MOV"     , 0x5cc, IntOp   , NoneOp  , IntOp   , True , False, False);
  AddReg("MOVL"    , 0x5dc, LongOp  , NoneOp  , LongOp  , True , False, False);
  AddReg("MOVT"    , 0x5ec, QuadOp  , NoneOp  , QuadOp  , True , False, False);
  AddReg("MOVQ"    , 0x5fc, QuadOp  , NoneOp  , QuadOp  , True , False, False);
  AddReg("MOVR"    , 0x6c9, SingleOp, NoneOp  , SingleOp, True , False, False);
  AddReg("MOVRL"   , 0x6d9, DoubleOp, NoneOp  , DoubleOp, True , False, False);
  AddReg("MOVRE"   , 0x6e1, ExtOp   , NoneOp  , ExtOp   , True , False, False);
  AddReg("MULI"    , 0x741, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("MULO"    , 0x701, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("MULR"    , 0x78c, SingleOp, SingleOp, SingleOp, True , True , False);
  AddReg("MULRL"   , 0x79c, DoubleOp, DoubleOp, DoubleOp, True , True , False);
  AddReg("NAND"    , 0x58e, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("NOR"     , 0x588, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("NOT"     , 0x58a, IntOp   , NoneOp  , IntOp   , True , False, False);
  AddReg("NOTAND"  , 0x584, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("NOTBIT"  , 0x580, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("NOTOR"   , 0x58d, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("OR"      , 0x587, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("ORNOT"   , 0x58b, IntOp   , IntOp   , IntOp   , True , True , False);
      /*  Name       OpCode TwoSrc HDest  Float     Imm1   Imm2 */
  AddReg("REMI"    , 0x748, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("REMO"    , 0x708, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("REMR"    , 0x683, SingleOp, SingleOp, SingleOp, True , True , False);
  AddReg("REMRL"   , 0x693, DoubleOp, DoubleOp, DoubleOp, True , True , False);
  AddReg("ROTATE"  , 0x59d, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("ROUNDR"  , 0x68b, SingleOp, NoneOp  , SingleOp, True , False, False);
  AddReg("ROUNDRL" , 0x69b, DoubleOp, NoneOp  , DoubleOp, True , False, False);
  AddReg("SCALER"  , 0x677, IntOp   , SingleOp, SingleOp, True , True , False);
  AddReg("SCALERL" , 0x676, IntOp   , DoubleOp, DoubleOp, True , True , False);
  AddReg("SCANBIT" , 0x641, IntOp   , NoneOp  , IntOp   , True , False, False);
  AddReg("SCANBYTE", 0x5ac, IntOp   , NoneOp  , IntOp   , True , False, False);
  AddReg("SETBIT"  , 0x583, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("SHLO"    , 0x59c, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("SHRO"    , 0x598, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("SHLI"    , 0x59e, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("SHRI"    , 0x59B, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("SHRDI"   , 0x59a, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("SINR"    , 0x68c, SingleOp, NoneOp  , SingleOp, True , False, False);
  AddReg("SINRL"   , 0x69c, DoubleOp, NoneOp  , DoubleOp, True , False, False);
  AddReg("SPANBIT" , 0x640, IntOp   , NoneOp  , IntOp   , True , False, False);
  AddReg("SQRTR"   , 0x688, SingleOp, NoneOp  , SingleOp, True , False, False);
  AddReg("SQRTRL"  , 0x698, DoubleOp, NoneOp  , DoubleOp, True , False, False);
  AddReg("SUBC"    , 0x5b2, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("SUBI"    , 0x593, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("SUBO"    , 0x592, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("SUBR"    , 0x78d, SingleOp, SingleOp, SingleOp, True , True , False);
  AddReg("SUBRL"   , 0x79d, DoubleOp, DoubleOp, DoubleOp, True , True , False);
  AddReg("SYNLD"   , 0x615, IntOp   , NoneOp  , IntOp   , False, False, False);
  AddReg("SYNMOV"  , 0x600, IntOp   , NoneOp  , IntOp   , False, False, False);
  AddReg("SYNMOVL" , 0x601, IntOp   , NoneOp  , IntOp   , False, False, False);
  AddReg("SYNMOVQ" , 0x602, IntOp   , NoneOp  , IntOp   , False, False, False);
  AddReg("TANR"    , 0x68e, SingleOp, NoneOp  , SingleOp, True , False, False);
  AddReg("TANRL"   , 0x69e, DoubleOp, NoneOp  , DoubleOp, True , False, False);
  AddReg("XOR"     , 0x589, IntOp   , IntOp   , IntOp   , True , True , False);
  AddReg("XNOR"    , 0x589, IntOp   , IntOp   , IntOp   , True , True , False);

  CobrOrders = (CobrOrder*) malloc(sizeof(CobrOrder)*CobrOrderCnt); InstrZ = 0;
  AddCobr("BBC"    , 0x30, True ); AddCobr("BBS"    , 0x37, True );
  AddCobr("CMPIBE" , 0x3a, True ); AddCobr("CMPOBE" , 0x32, True );
  AddCobr("CMPIBNE", 0x3d, True ); AddCobr("CMPOBNE", 0x35, True );
  AddCobr("CMPIBL" , 0x3c, True ); AddCobr("CMPOBL" , 0x34, True );
  AddCobr("CMPIBLE", 0x3e, True ); AddCobr("CMPOBLE", 0x36, True );
  AddCobr("CMPIBG" , 0x39, True ); AddCobr("CMPOBG" , 0x31, True );
  AddCobr("CMPIBGE", 0x3b, True ); AddCobr("CMPOBGE", 0x33, True );
  AddCobr("CMPIBO" , 0x3f, True ); AddCobr("CMPIBNO", 0x38, True );
  AddCobr("TESTE"  , 0x22, False); AddCobr("TESTNE" , 0x25, False);
  AddCobr("TESTL"  , 0x24, False); AddCobr("TESTLE" , 0x26, False);
  AddCobr("TESTG"  , 0x21, False); AddCobr("TESTGE" , 0x23, False);
  AddCobr("TESTO"  , 0x27, False); AddCobr("TESTNO" , 0x27, False);

  CtrlOrders = (FixedOrder*) malloc(sizeof(FixedOrder)*CtrlOrderCnt); InstrZ = 0;
  AddCtrl("B"   , 0x08); AddCtrl("CALL", 0x09);
  AddCtrl("BAL" , 0x0b); AddCtrl("BNO" , 0x19);
  AddCtrl("BG"  , 0x11); AddCtrl("BE"  , 0x12);
  AddCtrl("BGE" , 0x13); AddCtrl("BL"  , 0x14);
  AddCtrl("BNE" , 0x15); AddCtrl("BLE" , 0x16);
  AddCtrl("BO"  , 0x17);

  MemOrders = (MemOrder*) malloc(sizeof(MemOrder)*MemOrderCnt); InstrZ = 0;
  AddMem("LDOB" , 0x80, IntOp   , 2);
  AddMem("STOB" , 0x82, IntOp   , 1);
  AddMem("BX"   , 0x84, IntOp   , 0);
  AddMem("BALX" , 0x85, IntOp   , 2);
  AddMem("CALLX", 0x86, IntOp   , 0);
  AddMem("LDOS" , 0x88, IntOp   , 2);
  AddMem("STOS" , 0x8a, IntOp   , 1);
  AddMem("LDA"  , 0x8c, IntOp   , 2);
  AddMem("LD"   , 0x90, IntOp   , 2);
  AddMem("ST"   , 0x92, IntOp   , 1);
  AddMem("LDL"  , 0x98, LongOp  , 2);
  AddMem("STL"  , 0x9a, LongOp  , 1);
  AddMem("LDT"  , 0xa0, QuadOp  , 2);
  AddMem("STT"  , 0xa2, QuadOp  , 1);
  AddMem("LDQ"  , 0xb0, QuadOp  , 2);
  AddMem("STQ"  , 0xb2, QuadOp  , 1);
  AddMem("LDIB" , 0xc0, IntOp   , 2);
  AddMem("STIB" , 0xc2, IntOp   , 1);
  AddMem("LDIS" , 0xc8, IntOp   , 2);
  AddMem("STIS" , 0xca, IntOp   , 1);

  SpecRegs = (SpecReg*) malloc(sizeof(SpecReg)*SpecRegCnt); InstrZ = 0;
  AddSpecReg("FP" , 31); AddSpecReg("PFP", 0);
  AddSpecReg("SP" ,  1); AddSpecReg("RIP", 2);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
  free(FixedOrders);
  free(RegOrders);
  free(CobrOrders);
  free(CtrlOrders);
  free(SpecRegs);
}

/*--------------------------------------------------------------------------*/

static Boolean IsDef_960(void)
{
  return False;
}

static void InitPass_960(void)
{
  SetFlag(&FPUAvail, FPUAvailName, False);
}

static void SwitchFrom_960(void)
{
  DeinitFields();
  ClearONOFF();
}

static void SwitchTo_960(void)
{
  PFamilyDescr FoundId;

  TurnWords = False;
  ConstMode = ConstModeIntel;
  SetIsOccupied = False;

  FoundId = FindFamilyByName("i960");
  if (!FoundId)
    exit(255);
  PCSymbol = "$";
  HeaderID = FoundId->Id;
  NOPCode = 0x000000000;
  DivideChars = ",";
  HasAttrs = False;

  ValidSegs=(1 << SegCode);
  Grans[SegCode] = 1;
  ListGrans[SegCode] = 4;
  SegInits[SegCode] = 0;
  SegLimits[SegCode] = INTCONST_ffffffff;

  MakeCode = MakeCode_960;
  IsDef = IsDef_960;
  SwitchFrom=SwitchFrom_960;
  AddONOFF("FPU"     , &FPUAvail  , FPUAvailName  , False);
  AddONOFF("SUPMODE" , &SupAllowed, SupAllowedName, False);

  InitFields();
}

void code960_init(void)
{
  CPU80960 = AddCPU("80960", SwitchTo_960);
  
  AddInitPassProc(InitPass_960);
}
