/* codemcore.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator MCORE-Familie                                               */
/*                                                                           */
/* Historie:  31. 1.1998 Grundsteinlegung                                    */
/*             3. 1.1999 ChkPC-Anpassung                                     */
/*             9. 3.2000 'ambigious else'-Warnungen beseitigt                */
/*           14. 1.2001 silenced warnings about unused parameters            */
/*                                                                           */
/*****************************************************************************/
/* $Id: codemcore.c,v 1.6 2014/06/10 10:27:15 alfred Exp $                   */
/*****************************************************************************
 * $Log: codemcore.c,v $
 * Revision 1.6  2014/06/10 10:27:15  alfred
 * - adapt to current style
 *
 * Revision 1.5  2007/11/24 22:48:07  alfred
 * - some NetBSD changes
 *
 * Revision 1.4  2005/09/08 16:53:43  alfred
 * - use common PInstTable
 *
 * Revision 1.3  2005/05/21 16:35:05  alfred
 * - removed variables available globally
 *
 * Revision 1.2  2004/05/29 12:04:48  alfred
 * - relocated DecodeMot(16)Pseudo into separate module
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
#include "codepseudo.h"
#include "motpseudo.h"
#include "asmitree.h"
#include "codevars.h"

/*--------------------------------------------------------------------------*/
/* Variablen */

#define FixedOrderCnt 7
#define OneRegOrderCnt 32
#define TwoRegOrderCnt 23
#define UImm5OrderCnt 13
#define LJmpOrderCnt 4
#define CRegCnt 13

typedef struct
{
  Word Code;
  Boolean Priv;
} FixedOrder;

typedef struct
{
  Word Code;
  Word Min,Ofs;
} ImmOrder;

typedef struct
{
  char *Name;
  Word Code;
} CReg;

static CPUVar CPUMCORE;
static Byte OpSize;

static FixedOrder *FixedOrders;
static FixedOrder *OneRegOrders;
static FixedOrder *TwoRegOrders;
static ImmOrder *UImm5Orders;
static FixedOrder *LJmpOrders;
static CReg *CRegs;

/*--------------------------------------------------------------------------*/
/* Hilfsdekoder */

static Boolean DecodeReg(char *Asc, Word *Erg)
{
  char *endptr, *s;

  if (FindRegDef(Asc, &s))
    Asc = s;

  if (!strcasecmp(Asc, "SP"))
  {
    *Erg = 0;
    return True;
  }

  if (!strcasecmp(Asc, "LR"))
  {
    *Erg = 15;
    return True;
  }

  if (mytoupper(*Asc) != 'R')
    return False;
  else
  {
    *Erg = strtol(Asc + 1, &endptr, 10);
    return ((*endptr == '\0') && (*Erg <= 15));
  }
}

static Boolean DecodeCReg(char *Asc, Word *Erg)
{
  char *endptr;
  int z;

  for (z = 0; z < CRegCnt; z++)
    if (!strcasecmp(Asc,CRegs[z].Name))
    {
      *Erg = CRegs[z].Code;
      return True;
    }

  if ((mytoupper(*Asc) != 'C') || (mytoupper(Asc[1]) != 'R'))
    return False;
  else
  {
    *Erg = strtol(Asc + 2, &endptr, 10);
    return ((*endptr == '\0') && (*Erg <= 31));
  }
}

static Boolean DecodeAdr(char *Asc, Word *Erg)
{
  Word Base = 0xff, Tmp;
  LongInt DispAcc = 0, DMask = (1 << OpSize) - 1, DMax = 15 << OpSize;
  Boolean OK, FirstFlag = False;
  char *Pos;

  if (!IsIndirect(Asc))
  {
    WrError(1350);
    return False;
  }

  Asc++; Asc[strlen(Asc) - 1] = '\0';
  do 
  {
    Pos = QuotPos(Asc,',');
    if (Pos)
      *Pos = '\0';
    if (DecodeReg(Asc, &Tmp))
    {
      if (Base == 0xff) Base = Tmp;
      else
      {
        WrError(1350);
        return False;
      }
    }
    else
    {
      FirstPassUnknown = FALSE;
      DispAcc += EvalIntExpression(Asc, Int32, &OK);
      if (FirstPassUnknown) FirstFlag = True;
      if (!OK)
        return False;
    }
    if (Pos)
      Asc = Pos + 1;
  }
  while (Pos);

  if (Base == 0xff)
  {
    WrError(1350);
    return False;
  }
 
  if (FirstFlag)
  {
    DispAcc -= DispAcc & DMask;
    if (DispAcc < 0) DispAcc = 0;
    if (DispAcc > DMax) DispAcc = DMax;
  }

  if ((DispAcc & DMask) != 0)
  {
    WrError(1325);
    return False;
  }
  if (!ChkRange(DispAcc, 0, DMax))
    return False;
  *Erg = Base + ((DispAcc >> OpSize) << 4);
  return True;
}

static void DecodeREG(Word Code)
{
  UNUSED(Code);

  if (ArgCnt!=1) WrError(1110);
  else AddRegDef(LabPart,ArgStr[1]);
}

static void DecodeFixed(Word Index)
{
  FixedOrder *Instr = FixedOrders + Index;

  if (*AttrPart != '\0') WrError(1100);
  else if (ArgCnt != 0) WrError(1110);
  else
  {
    if ((Instr->Priv) && (!SupAllowed)) WrError(50);
    WAsmCode[0] = Instr->Code;
    CodeLen = 2; 
  }
}

static void DecodeOneReg(Word Index)
{
  FixedOrder *Instr = OneRegOrders + Index;
  Word RegX;

  if (*AttrPart != '\0') WrError(1100);
  else if (ArgCnt != 1) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &RegX)) WrXError(1445, ArgStr[1]);
  {
    if ((Instr->Priv) && (!SupAllowed)) WrError(50);
    WAsmCode[0] = Instr->Code + RegX;
    CodeLen = 2;
  }
}

static void DecodeTwoReg(Word Index)
{
  FixedOrder *Instr = TwoRegOrders + Index;
  Word RegX, RegY;

  if (*AttrPart != '\0') WrError(1100);
  else if (ArgCnt != 2) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &RegX)) WrXError(1445, ArgStr[1]);
  else if (!DecodeReg(ArgStr[2], &RegY)) WrXError(1445, ArgStr[2]);
  {
    if ((Instr->Priv) && (!SupAllowed)) WrError(50);
    WAsmCode[0] = Instr->Code + (RegY << 4) + RegX;
    CodeLen = 2; 
  }
}

static void DecodeUImm5(Word Index)
{
  ImmOrder *Instr = UImm5Orders + Index;
  Word RegX, ImmV;
  Boolean OK;

  if (*AttrPart != '\0') WrError(1100);
  else if (ArgCnt != 2) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &RegX)) WrXError(1445, ArgStr[1]);
  else
  {
    FirstPassUnknown = False;
    ImmV = EvalIntExpression(ArgStr[2], (Instr->Ofs > 0) ? UInt6 : UInt5, &OK);
    if ((Instr->Min > 0) && (ImmV < Instr->Min))
    {
      if (FirstPassUnknown) ImmV = Instr->Min;
      else
      {
        WrError(1315); OK = False;
      }
    }
    if ((Instr->Ofs > 0) && ((ImmV < Instr->Ofs) || (ImmV > 31 + Instr->Ofs)))
    {
      if (FirstPassUnknown) ImmV = Instr->Ofs;
      else
      {
        WrError((ImmV < Instr->Ofs) ? 1315 : 1320); 
        OK = False;
      }
    }
    if (OK)
    {
      WAsmCode[0] = Instr->Code + ((ImmV - Instr->Ofs) << 4) + RegX;
      CodeLen = 2;
    }
  }
}

static void DecodeLJmp(Word Index)
{
  FixedOrder *Instr = LJmpOrders + Index;
  LongInt Dest;
  Boolean OK;
  
  if (*AttrPart != '\0') WrError(1100);
  else if (ArgCnt != 1) WrError(1110);
  else
  {
    Dest = EvalIntExpression(ArgStr[1], UInt32, &OK) - (EProgCounter() + 2);
    if (OK)
    {
      if ((!SymbolQuestionable) && (Dest & 1)) WrError(1375);
      else if ((!SymbolQuestionable) && ((Dest > 2046) || (Dest < -2048))) WrError(1370);
      else
      {
        if ((Instr->Priv) && (!SupAllowed)) WrError(50);
        WAsmCode[0] = Instr->Code + ((Dest >> 1) & 0x7ff);
        CodeLen = 2;
      }
    }
  }
}

static void DecodeSJmp(Word Index)
{
  LongInt Dest;
  Boolean OK;
  int l = 0;
  
  if (*AttrPart != '\0') WrError(1100);
  else if (ArgCnt != 1) WrError(1110);
  else if ((*ArgStr[1] != '[') || (ArgStr[1][l = strlen(ArgStr[1]) - 1] != ']')) WrError(1350);
  else
  {
    ArgStr[1][l] = '\0';
    Dest = EvalIntExpression(ArgStr[1] + 1, UInt32, &OK);
    if (OK)
    {
      if ((!SymbolQuestionable) & (Dest & 3)) WrError(1325);
      else
      {
        Dest = (Dest - (EProgCounter() + 2)) >> 2;
        if ((EProgCounter() & 3) < 2) Dest++;
        if ((!SymbolQuestionable) && ((Dest < 0) || (Dest > 255))) WrError(1370);
        else
        {
          WAsmCode[0] = 0x7000 + (Index << 8) + (Dest & 0xff);
          CodeLen = 2;
        }
      }
    }
  }
}

static void DecodeBGENI(Word Index)
{
  Word RegX, ImmV;
  Boolean OK;
  UNUSED(Index);

  if (*AttrPart != '\0') WrError(1100);
  else if (ArgCnt != 2) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &RegX)) WrXError(1445, ArgStr[1]);
  else
  {
    ImmV = EvalIntExpression(ArgStr[2], UInt5, &OK);
    if (OK)
    {
      if (ImmV > 6)
        WAsmCode[0] = 0x3200 + (ImmV << 4) + RegX;
      else
        WAsmCode[0] = 0x6000 + (1 << (4 + ImmV)) + RegX;
      CodeLen = 2;
    }
  }
}

static void DecodeBMASKI(Word Index)
{
  Word RegX, ImmV;
  Boolean OK;
  UNUSED(Index);

  if (*AttrPart != '\0') WrError(1100);
  else if (ArgCnt != 2) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &RegX)) WrXError(1445, ArgStr[1]);
  else
  {
    FirstPassUnknown = False;
    ImmV = EvalIntExpression(ArgStr[2], UInt6, &OK);
    if ((FirstPassUnknown) && ((ImmV < 1) || (ImmV > 32))) ImmV = 8;
    if (OK)
    {
      if (ChkRange(ImmV, 1, 32))
      {
        ImmV &= 31;
        if ((ImmV < 1) || (ImmV > 7))
          WAsmCode[0] = 0x2c00 + (ImmV << 4) + RegX;
        else
          WAsmCode[0] = 0x6000 + (((1 << ImmV) - 1) << 4) + RegX;
        CodeLen = 2;
      }
    }
  }
}

static void DecodeLdSt(Word Index)
{
  Word RegX, RegZ, NSize;

  if ((*AttrPart != '\0') && (Lo(Index) != 0xff)) WrError(1100);
  else if (ArgCnt != 2) WrError(1110);
  else if (OpSize > 2) WrError(1130);
  else
  {
    if (Lo(Index) != 0xff) OpSize = Lo(Index);
    if (!DecodeReg(ArgStr[1], &RegZ)) WrXError(1445, ArgStr[1]);
    else if (DecodeAdr(ArgStr[2], &RegX))
    {
      NSize = (OpSize == 2) ? 0 : OpSize + 1;
      WAsmCode[0] = 0x8000 + (NSize << 13) + (Hi(Index) << 12) + (RegZ << 8) + RegX;
      CodeLen = 2;
    }
  }
}

static void DecodeLdStm(Word Index)
{
  char *p;
  int l;
  Word RegF, RegL, RegI;

  if (*AttrPart != 0) WrError(1100);
  else if (ArgCnt != 2) WrError(1110);
  else if ((*ArgStr[2] != '(') || (ArgStr[2][l = strlen(ArgStr[2]) - 1] != ')')) WrError(1350);
  else
  {
    ArgStr[2][l] = '\0';
    if (!DecodeReg(ArgStr[2] + 1, &RegI)) WrXError(1445, ArgStr[2] + 1);
    else if (RegI != 0) WrXError(1445, ArgStr[2] + 1);
    else if (!(p = strchr(ArgStr[1], '-'))) WrError(1350);
    else if (!DecodeReg(p + 1, &RegL)) WrXError(1445, p + 1);
    else if (RegL != 15) WrXError(1445, p + 1);
    else
    {
      *p = '\0';
      if (!DecodeReg(ArgStr[1], &RegF)) WrXError(1445, ArgStr[1]);
      else if ((RegF < 1) || (RegF > 14)) WrXError(1445, ArgStr[1]);
      else
      {
        WAsmCode[0] = 0x0060 + (Index << 4) + RegF;
        CodeLen = 2;
      }
    }
  }
}

static void DecodeLdStq(Word Index)
{
  char *p;
  int l;
  Word RegF, RegL, RegX;

  if (*AttrPart != 0) WrError(1100);
  else if (ArgCnt != 2) WrError(1110);
  else if ((*ArgStr[2] != '(') || (ArgStr[2][l = strlen(ArgStr[2]) - 1] != ')')) WrError(1350);
  else
  {
    ArgStr[2][l] = '\0';
    if (!DecodeReg(ArgStr[2] + 1, &RegX)) WrXError(1445, ArgStr[2] + 1);
    else if ((RegX >= 4) && (RegX <= 7)) WrXError(1445, ArgStr[2]+1);
    else if (!(p = strchr(ArgStr[1], '-'))) WrError(1350);
    else if (!DecodeReg(p + 1, &RegL)) WrXError(1445, p + 1);
    else if (RegL != 7) WrXError(1445, p + 1);
    else
    {
      *p = '\0';
      if (!DecodeReg(ArgStr[1], &RegF)) WrXError(1445, ArgStr[1]);
      else if (RegF != 4) WrXError(1445, ArgStr[1]);
      else
      {
        WAsmCode[0] = 0x0040 + (Index << 4) + RegX;
        CodeLen = 2;
      }
    }
  }
}

static void DecodeLoopt(Word Index)
{
  Word RegY;
  LongInt Dest;
  Boolean OK;
  UNUSED(Index);

  if (*AttrPart != '\0') WrError(1100);
  else if (ArgCnt != 2) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &RegY)) WrXError(1445, ArgStr[1]);
  else
  {
    Dest = EvalIntExpression(ArgStr[2], UInt32, &OK) - (EProgCounter() + 2);
    if (OK)
    {
      if ((!SymbolQuestionable) && (Dest &1 )) WrError(1375);
      else if ((!SymbolQuestionable) && ((Dest > -2) || (Dest <- 32))) WrError(1370);
      else
      {
        WAsmCode[0] = 0x0400 + (RegY << 4) + ((Dest >> 1) & 15);
        CodeLen = 2;
      }
    }
  }
}

static void DecodeLrm(Word Index)
{
  LongInt Dest;
  Word RegZ;
  Boolean OK;
  int l = 0;
  UNUSED(Index);
  
  if (*AttrPart != '\0') WrError(1100);
  else if (ArgCnt != 2) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &RegZ)) WrXError(1445, ArgStr[1]);
  else if ((RegZ == 0) || (RegZ == 15)) WrXError(1445, ArgStr[1]);
  else if ((*ArgStr[2] != '[') || (ArgStr[2][l = strlen(ArgStr[2]) - 1] != ']')) WrError(1350);
  else
  {
    ArgStr[2][l] = '\0';
    Dest = EvalIntExpression(ArgStr[2] + 1, UInt32, &OK);
    if (OK)
    {
      if ((!SymbolQuestionable) && (Dest & 3)) WrError(1325);
      else
      {
        Dest = (Dest - (EProgCounter() + 2)) >> 2;
        if ((EProgCounter() & 3) < 2) Dest++;
        if ((!SymbolQuestionable) && ((Dest < 0) || (Dest > 255))) WrError(1370);
        else
        {
          WAsmCode[0] = 0x7000 + (RegZ << 8) + (Dest & 0xff);
          CodeLen = 2;
        }
      }
    }
  }
}

static void DecodeMcr(Word Index)
{
  Word RegX,CRegY;

  if (*AttrPart != '\0') WrError(1100);
  else if (ArgCnt != 2) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &RegX)) WrXError(1445, ArgStr[1]);
  else if (!DecodeCReg(ArgStr[2], &CRegY)) WrXError(1440, ArgStr[2]);
  else
  {
    if (!SupAllowed) WrError(50);
    WAsmCode[0] = 0x1000 + (Index << 11) + (CRegY << 4) + RegX;
    CodeLen = 2;
  }
}

static void DecodeMovi(Word Index)
{
  Word RegX, ImmV;
  Boolean OK;
  UNUSED(Index);

  if (*AttrPart != '\0') WrError(1100);
  else if (ArgCnt != 2) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &RegX)) WrXError(1445, ArgStr[1]);
  else
  {
    ImmV = EvalIntExpression(ArgStr[2], UInt7, &OK);
    if (OK)
    {
      WAsmCode[0] = 0x6000 + ((ImmV & 127) << 4) + RegX;
      CodeLen = 2;
    }
  }
}

static void DecodeTrap(Word Index)
{
  Word ImmV;
  Boolean OK;
  UNUSED(Index);

  if (*AttrPart!='\0') WrError(1100);
  else if (ArgCnt != 1) WrError(1110);
  else if (*ArgStr[1] != '#') WrError(1120);
  else
  {
    ImmV = EvalIntExpression(ArgStr[1] +1 , UInt2, &OK);
    if (OK)
    {
      WAsmCode[0] = 0x0008 + ImmV;
      CodeLen = 2;
    }
  }
}

/*--------------------------------------------------------------------------*/
/* Codetabellenverwaltung */

static void AddFixed(char *NName, Word NCode, Boolean NPriv)
{
  if (InstrZ >= FixedOrderCnt) exit(255);
  FixedOrders[InstrZ].Code = NCode;
  FixedOrders[InstrZ].Priv = NPriv;
  AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
}

static void AddOneReg(char *NName, Word NCode, Boolean NPriv)
{
  if (InstrZ >= OneRegOrderCnt) exit(255);
  OneRegOrders[InstrZ].Code = NCode;
  OneRegOrders[InstrZ].Priv = NPriv;
  AddInstTable(InstTable, NName, InstrZ++, DecodeOneReg);
}

static void AddTwoReg(char *NName, Word NCode, Boolean NPriv)
{
  if (InstrZ >= TwoRegOrderCnt) exit(255);
  TwoRegOrders[InstrZ].Code = NCode;
  TwoRegOrders[InstrZ].Priv = NPriv;
  AddInstTable(InstTable, NName, InstrZ++, DecodeTwoReg);
}

static void AddUImm5(char *NName, Word NCode, Word NMin, Word NOfs)
{
   if (InstrZ >= UImm5OrderCnt) exit(255);
   UImm5Orders[InstrZ].Code = NCode;
   UImm5Orders[InstrZ].Min = NMin;
   UImm5Orders[InstrZ].Ofs = NOfs;
   AddInstTable(InstTable, NName, InstrZ++, DecodeUImm5);
}

static void AddLJmp(char *NName, Word NCode, Boolean NPriv)
{
  if (InstrZ >= LJmpOrderCnt) exit(255);
  LJmpOrders[InstrZ].Code = NCode;
  LJmpOrders[InstrZ].Priv = NPriv;
  AddInstTable(InstTable, NName, InstrZ++, DecodeLJmp);
}

static void AddCReg(char *NName, Word NCode)
{
  if (InstrZ >= CRegCnt) exit(255);
  CRegs[InstrZ].Name = NName;
  CRegs[InstrZ++].Code = NCode;
}

static void InitFields(void)
{
  InstTable = CreateInstTable(201);

  AddInstTable(InstTable, "REG", 0, DecodeREG);

  InstrZ = 0; FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCnt);
  AddFixed("BKPT" , 0x0000, False);
  AddFixed("DOZE" , 0x0006, True );
  AddFixed("RFI"  , 0x0003, True );
  AddFixed("RTE"  , 0x0002, True );
  AddFixed("STOP" , 0x0004, True );
  AddFixed("SYNC" , 0x0001, False);
  AddFixed("WAIT" , 0x0005, True );

  InstrZ = 0; OneRegOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * OneRegOrderCnt);
  AddOneReg("ABS"   , 0x01e0, False);  AddOneReg("ASRC" , 0x3a00, False);
  AddOneReg("BREV"  , 0x00f0, False);  AddOneReg("CLRF" , 0x01d0, False);
  AddOneReg("CLRT"  , 0x01c0, False);  AddOneReg("DECF" , 0x0090, False);
  AddOneReg("DECGT" , 0x01a0, False);  AddOneReg("DECLT", 0x0180, False);
  AddOneReg("DECNE" , 0x01b0, False);  AddOneReg("DECT" , 0x0080, False);
  AddOneReg("DIVS"  , 0x3210, False);  AddOneReg("DIVU" , 0x2c10, False);
  AddOneReg("FF1"   , 0x00e0, False);  AddOneReg("INCF" , 0x00b0, False);
  AddOneReg("INCT"  , 0x00a0, False);  AddOneReg("JMP"  , 0x00c0, False);
  AddOneReg("JSR"   , 0x00d0, False);  AddOneReg("LSLC" , 0x3c00, False);
  AddOneReg("LSRC"  , 0x3e00, False);  AddOneReg("MVC"  , 0x0020, False);
  AddOneReg("MVCV"  , 0x0030, False);  AddOneReg("NOT"  , 0x01f0, False);
  AddOneReg("SEXTB" , 0x0150, False);  AddOneReg("SEXTH", 0x0170, False);
  AddOneReg("TSTNBZ", 0x0190, False);  AddOneReg("XSR"  , 0x3800, False);
  AddOneReg("XTRB0" , 0x0130, False);  AddOneReg("XTRB1", 0x0120, False);
  AddOneReg("XTRB2" , 0x0110, False);  AddOneReg("XTRB3", 0x0100, False);
  AddOneReg("ZEXTB" , 0x0140, False);  AddOneReg("ZEXTH", 0x0160, False);

  InstrZ = 0; TwoRegOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * TwoRegOrderCnt);
  AddTwoReg("ADDC" , 0x0600, False);  AddTwoReg("ADDU" , 0x1c00, False);
  AddTwoReg("AND"  , 0x1600, False);  AddTwoReg("ANDN" , 0x1f00, False);
  AddTwoReg("ASR"  , 0x1a00, False);  AddTwoReg("BGENR", 0x1300, False);
  AddTwoReg("CMPHS", 0x0c00, False);  AddTwoReg("CMPLT", 0x0d00, False);
  AddTwoReg("CMPNE", 0x0f00, False);  AddTwoReg("IXH"  , 0x1d00, False);
  AddTwoReg("IXW"  , 0x1500, False);  AddTwoReg("LSL"  , 0x1b00, False);
  AddTwoReg("LSR"  , 0x0b00, False);  AddTwoReg("MOV"  , 0x1200, False);
  AddTwoReg("MOVF" , 0x0a00, False);  AddTwoReg("MOVT" , 0x0200, False);
  AddTwoReg("MULT" , 0x0300, False);  AddTwoReg("OR"   , 0x1e00, False);
  AddTwoReg("RSUB" , 0x1400, False);  AddTwoReg("SUBC" , 0x0700, False);
  AddTwoReg("SUBU" , 0x0500, False);  AddTwoReg("TST"  , 0x0e00, False);
  AddTwoReg("XOR"  , 0x1700, False);

  InstrZ = 0; UImm5Orders = (ImmOrder *) malloc(sizeof(ImmOrder) * UImm5OrderCnt);
  AddUImm5("ADDI"  , 0x2000, 0, 1);  AddUImm5("ANDI"  , 0x2e00, 0, 0);
  AddUImm5("ASRI"  , 0x3a00, 1, 0);  AddUImm5("BCLRI" , 0x3000, 0, 0);
  AddUImm5("BSETI" , 0x3400, 0, 0);  AddUImm5("BTSTI" , 0x3600, 0, 0);
  AddUImm5("CMPLTI", 0x2200, 0, 1);  AddUImm5("CMPNEI", 0x2a00, 0, 0);
  AddUImm5("LSLI"  , 0x3c00, 1, 0);  AddUImm5("LSRI"  , 0x3e00, 1, 0);
  AddUImm5("ROTLI" , 0x3800, 1, 0);  AddUImm5("RSUBI" , 0x2800, 0, 0);
  AddUImm5("SUBI"  , 0x2400, 0, 1);

  InstrZ = 0; LJmpOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * LJmpOrderCnt);
  AddLJmp("BF"   , 0xe800, False);  AddLJmp("BR"   , 0xf000, False);
  AddLJmp("BSR"  , 0xf800, False);  AddLJmp("BT"   , 0xe000, False);

  InstrZ = 0; CRegs = (CReg *) malloc(sizeof(CReg) * CRegCnt);
  AddCReg("PSR" , 0); AddCReg("VBR" , 1);
  AddCReg("EPSR", 2); AddCReg("FPSR", 3);
  AddCReg("EPC" , 4); AddCReg("FPC",  5);
  AddCReg("SS0",  6); AddCReg("SS1",  7);
  AddCReg("SS2",  8); AddCReg("SS3",  9);
  AddCReg("SS4", 10); AddCReg("GCR", 11);
  AddCReg("GSR", 12);

  AddInstTable(InstTable, "BGENI" , 0, DecodeBGENI);
  AddInstTable(InstTable, "BMASKI", 0, DecodeBMASKI);
  AddInstTable(InstTable, "JMPI"  , 0, DecodeSJmp);
  AddInstTable(InstTable, "JSRI"  , 0, DecodeSJmp);
  AddInstTable(InstTable, "LD"    , 0x0ff, DecodeLdSt);
  AddInstTable(InstTable, "LDB"   , 0x000, DecodeLdSt);
  AddInstTable(InstTable, "LDH"   , 0x001, DecodeLdSt);
  AddInstTable(InstTable, "LDW"   , 0x002, DecodeLdSt);
  AddInstTable(InstTable, "ST"    , 0x1ff, DecodeLdSt);
  AddInstTable(InstTable, "STB"   , 0x100, DecodeLdSt);
  AddInstTable(InstTable, "STH"   , 0x101, DecodeLdSt);
  AddInstTable(InstTable, "STW"   , 0x102, DecodeLdSt);
  AddInstTable(InstTable, "LDM"   , 0, DecodeLdStm);
  AddInstTable(InstTable, "STM"   , 1, DecodeLdStm);
  AddInstTable(InstTable, "LDQ"   , 0, DecodeLdStq);
  AddInstTable(InstTable, "STQ"   , 1, DecodeLdStq);
  AddInstTable(InstTable, "LOOPT" , 0, DecodeLoopt);
  AddInstTable(InstTable, "LRM"   , 0, DecodeLrm);
  AddInstTable(InstTable, "MFCR"  , 0, DecodeMcr);
  AddInstTable(InstTable, "MTCR"  , 1, DecodeMcr);
  AddInstTable(InstTable, "MOVI"  , 0, DecodeMovi);
  AddInstTable(InstTable, "TRAP"  , 0, DecodeTrap);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
  free(FixedOrders);
  free(OneRegOrders);
  free(TwoRegOrders);
  free(UImm5Orders);
  free(LJmpOrders);
  free(CRegs);
}

/*--------------------------------------------------------------------------*/
/* Callbacks */

static void MakeCode_MCORE(void)
{
  CodeLen = 0;

  OpSize = 2;
  DontPrint = False;

  if (*AttrPart!='\0')
  {
    switch (mytoupper(*AttrPart))
    {
      case 'B': OpSize = 0; break;
      case 'H': OpSize = 1; break;
      case 'W': OpSize = 2; break;
      case 'Q': OpSize = 3; break;
      case 'S': OpSize = 4; break;
      case 'D': OpSize = 5; break;
      case 'X': OpSize = 6; break;
      case 'P': OpSize = 7; break;
      default: WrError(1107); return;
    }
  }

  /* Nullanweisung */

  if ((*OpPart == '\0') && (*AttrPart == '\0') && (ArgCnt == 0)) return;

  /* Pseudoanweisungen */

  if (DecodeMoto16Pseudo(OpSize,True)) return;

  /* Befehlszaehler ungerade ? */

  if (Odd(EProgCounter())) WrError(180);

  /* alles aus der Tabelle */

  if (!LookupInstTable(InstTable,OpPart))
    WrXError(1200,OpPart);
}

static Boolean IsDef_MCORE(void)
{
  return Memo("REG");
}

static void SwitchFrom_MCORE(void)
{
  DeinitFields();
}

static void SwitchTo_MCORE(void)
{
  TurnWords = True; ConstMode = ConstModeMoto; SetIsOccupied = False;

   PCSymbol = "*"; HeaderID = 0x03; NOPCode = 0x1200; /* ==MOV r0,r0 */
   DivideChars = ","; HasAttrs = True; AttrChars = ".";

   ValidSegs = (1 << SegCode);
   Grans[SegCode] = 1; ListGrans[SegCode] = 2; SegInits[SegCode] = 0;
#ifdef __STDC__
   SegLimits[SegCode] = 0xfffffffful;
#else
   SegLimits[SegCode] = 0xffffffffl;
#endif

   MakeCode = MakeCode_MCORE; IsDef = IsDef_MCORE;

   SwitchFrom = SwitchFrom_MCORE; InitFields();
   AddONOFF("SUPMODE" , &SupAllowed, SupAllowedName, False);
   AddMoto16PseudoONOFF();

   SetFlag(&DoPadding, DoPaddingName, True);
END

/*--------------------------------------------------------------------------*/
/* Initialisierung */

void codemcore_init(void)
{
  CPUMCORE = AddCPU("MCORE", SwitchTo_MCORE);
}
