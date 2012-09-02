/* codexcore.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator XCore                                                       */
/*****************************************************************************/
/* $Id: codexcore.c,v 1.5 2012-08-03 17:30:03 alfred Exp $                   */
/*****************************************************************************
 * $Log: codexcore.c,v $
 * Revision 1.5  2012-08-03 17:30:03  alfred
 * - completed machine instructions
 *
 * Revision 1.4  2012-08-02 20:15:03  alfred
 * - add lr2r instructions
 *
 * Revision 1.3  2012-07-29 09:44:43  alfred
 * - add more instructions
 *
 * Revision 1.2  2012-07-28 08:41:33  alfred
 * - add more instructions
 *
 * Revision 1.1  2012-07-22 11:51:45  alfred
 * - begun with XCore target
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
#include "intpseudo.h"
#include "asmitree.h"
#include "codevars.h"
#include "codexcore.h"

/*--------------------------------------------------------------------------*/
/* Types */

typedef struct
{
  Word Opcode1, Opcode2;
} lr2r_Order;

/*--------------------------------------------------------------------------*/
/* Variables */

static CPUVar CPUXS1;

static lr2r_Order lr2r_Orders[5];

/*--------------------------------------------------------------------------*/
/* Common Decoder Subroutines */

static unsigned UpVal(unsigned Val1, unsigned Val3, unsigned Val9, unsigned Val27)
{
  return (((Val1 >> 2) & 3) * 1)
       + (((Val3 >> 2) & 3) * 3)
       + (((Val9 >> 2) & 3) * 9)
       + (((Val27 >> 2) & 3) * 27);
}

static Boolean ParseReg(const char *pAsc, unsigned *pResult)
{
  char *pEnd;
  char *pAlias;

  if (FindRegDef(pAsc, &pAlias))
    pAsc = pAlias;

  if ((strlen(pAsc) < 2) || (mytoupper(*pAsc) != 'R'))
    return FALSE;

  *pResult = strtoul(pAsc + 1, &pEnd, 10);
  return ((!*pEnd) && (*pResult <= 11));
}

/*--------------------------------------------------------------------------*/
/* Instruction Decoders */

static void CodeREG(Word Index)
{
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else AddRegDef(LabPart, ArgStr[1]);
}

static void Code_3r(Word Index)
{
  unsigned Op1, Op2, Op3;

  if (ArgCnt != 3) WrError(1110);
  else if (!ParseReg(ArgStr[1], &Op1)) WrXError(1445, ArgStr[1]);
  else if (!ParseReg(ArgStr[2], &Op2)) WrXError(1445, ArgStr[2]);
  else if (!ParseReg(ArgStr[3], &Op3)) WrXError(1445, ArgStr[3]);
  {
    WAsmCode[0] = Index
                | ((Op3 & 3) << 0)
                | ((Op2 & 3) << 2)
                | ((Op1 & 3) << 4)
                | (UpVal(Op3, Op2, Op1, 0) << 6);
    CodeLen = 2;
  }
}

static void Code_2rus(Word Index)
{
  unsigned Op1, Op2, Op3;
  Boolean OK;

  if (ArgCnt != 3) WrError(1110);
  else if (!ParseReg(ArgStr[1], &Op1)) WrXError(1445, ArgStr[1]);
  else if (!ParseReg(ArgStr[2], &Op2)) WrXError(1445, ArgStr[2]);
  {
    FirstPassUnknown = FALSE;
    Op3 = EvalIntExpression(ArgStr[3], UInt4, &OK);
    if (FirstPassUnknown)
      Op3 &= 7;
    if (OK)
    {
      if (Op3 > 11) WrXError(1320, ArgStr[3]);
      else
      {
        WAsmCode[0] = Index
                    | ((Op3 & 3) << 0)
                    | ((Op2 & 3) << 2)
                    | ((Op1 & 3) << 4)
                    | (UpVal(Op3, Op2, Op1, 0) << 6);
        CodeLen = 2;
      }
    }
  }
}

static void Code_2r(Word Index)
{
  unsigned Op1, Op2;

  if (ArgCnt != 2) WrError(1110);
  else if (!ParseReg(ArgStr[1], &Op1)) WrXError(1445, ArgStr[1]);
  else if (!ParseReg(ArgStr[2], &Op2)) WrXError(1445, ArgStr[2]);
  {
    unsigned Up = UpVal(Op2, Op1, 0, 0) + 27;

    WAsmCode[0] = Index
                | ((Op2 & 3) << 0)
                | ((Op1 & 3) << 2)
                | ((Up & 0x1f) << 6)
                | (Up & 0x20);
    CodeLen = 2;
  }
}

static void Code_l3r(Word Index)
{
  unsigned Op1, Op2, Op3;

  if (ArgCnt != 3) WrError(1110);
  else if (!ParseReg(ArgStr[1], &Op1)) WrXError(1445, ArgStr[1]);
  else if (!ParseReg(ArgStr[2], &Op2)) WrXError(1445, ArgStr[2]);
  else if (!ParseReg(ArgStr[3], &Op3)) WrXError(1445, ArgStr[3]);
  {
    WAsmCode[0] = 0xf800
                | ((Op3 & 3) << 0)
                | ((Op2 & 3) << 2)
                | ((Op1 & 3) << 4)
                | (UpVal(Op3, Op2, Op1, 0) << 6);
    WAsmCode[1] = Index;
    CodeLen = 4;
  }
}

static void Code_l2rus(Word Index)
{
  unsigned Op1, Op2, Op3;
  Boolean OK;

  if (ArgCnt != 3) WrError(1110);
  else if (!ParseReg(ArgStr[1], &Op1)) WrXError(1445, ArgStr[1]);
  else if (!ParseReg(ArgStr[2], &Op2)) WrXError(1445, ArgStr[2]);
  {
    FirstPassUnknown = FALSE;
    Op3 = EvalIntExpression(ArgStr[3], UInt4, &OK);
    if (FirstPassUnknown)
      Op3 &= 7;
    if (OK)
    {
      if (Op3 > 11) WrXError(1320, ArgStr[3]);
      else
      {
        WAsmCode[0] = 0xf800
                    | ((Op3 & 3) << 0)
                    | ((Op2 & 3) << 2)
                    | ((Op1 & 3) << 4)
                    | (UpVal(Op3, Op2, Op1, 0) << 6);
        WAsmCode[1] = Index;
        CodeLen = 4;
      }
    }
  }
}

static void Code_1r(Word Index)
{
  unsigned Op1;

  if (ArgCnt != 1) WrError(1110);
  else if (!ParseReg(ArgStr[1], &Op1)) WrXError(1445, ArgStr[1]);
  else
  {
    WAsmCode[0] = Index | Op1;
    CodeLen = 2;
  }
}

static void Code_l2r(Word Index)
{
  unsigned Op1, Op2;

  if (ArgCnt != 2) WrError(1110);
  else if (!ParseReg(ArgStr[1], &Op1)) WrXError(1445, ArgStr[1]);
  else if (!ParseReg(ArgStr[2], &Op2)) WrXError(1445, ArgStr[2]);
  {
    unsigned Up = UpVal(Op2, Op1, 0, 0) + 27;

    WAsmCode[0] = 0xf800 | ((Index & 1) << 4)
                | ((Op2 & 3) << 0)
                | ((Op1 & 3) << 2)
                | ((Up & 0x1f) << 6)
                | (Up & 0x20);
    WAsmCode[1] = Index & (~1);
    CodeLen = 4;
  }
}

static void Code_u10(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    LongWord Op1;
    Boolean OK;

    Op1 = EvalIntExpression(ArgStr[1], UInt20, &OK);
    if (OK)
    {
      if (Op1 > 0x3ff)
      {
        WAsmCode[CodeLen >> 1] = 0xf000 | ((Op1 >> 10) & 0x3ff);
        CodeLen += 2;
      }
      WAsmCode[CodeLen >> 1] = Index | (Op1 & 0x3ff);
      CodeLen += 2;
    }
  }
}

static void Code_u6(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    LongWord Op1;
    Boolean OK;

    Op1 = EvalIntExpression(ArgStr[1], UInt16, &OK);
    if (OK)
    {
      if (Op1 > 0x3f)
      {
        WAsmCode[CodeLen >> 1] = 0xf000 | ((Op1 >> 6) & 0x3ff);
        CodeLen += 2;
      }
      WAsmCode[CodeLen >> 1] = Index | (Op1 & 0x3f);
      CodeLen += 2;
    }
  }
}

static void Code_ru6(Word Index)
{
  unsigned Op1;

  if (ArgCnt != 2) WrError(1110);
  else if (!ParseReg(ArgStr[1], &Op1)) WrXError(1445, ArgStr[1]);
  else
  {
    LongWord Op2;
    Boolean OK;

    Op2 = EvalIntExpression(ArgStr[2], UInt16, &OK);
    if (OK)
    {
      if (Op2 > 0x3f)
      {
        WAsmCode[CodeLen >> 1] = 0xf000 | ((Op2 >> 6) & 0x3ff);
        CodeLen += 2;
      }
      WAsmCode[CodeLen >> 1] = Index | (Op1 << 6) | (Op2 & 0x3f);
      CodeLen += 2;
    }
  }
}

static void Code_rus(Word Index)
{
  unsigned Op1;

  if (ArgCnt != 2) WrError(1110);
  else if (!ParseReg(ArgStr[1], &Op1)) WrXError(1445, ArgStr[1]);
  else
  {
    unsigned Op2;
    Boolean OK;

    FirstPassUnknown = FALSE;
    Op2 = EvalIntExpression(ArgStr[2], UInt4, &OK);
    if (FirstPassUnknown)
      Op2 &= 7;
    if (OK)
    {
      if (Op2 > 11) WrXError(1320, ArgStr[2]);
      else
      {
        unsigned Up = UpVal(Op2, Op1, 0, 0) + 27;

        WAsmCode[0] = Index
                    | ((Op2 & 3) << 0)
                    | ((Op1 & 3) << 2)
                    | ((Up & 0x1f) << 6)
                    | (Up & 0x20);
        CodeLen = 2;
      }
    }
  }
}

static void Code_0r(Word Index)
{
  if (ArgCnt != 0) WrError(1110);
  else
  {
    WAsmCode[0] = Index;
    CodeLen = 2;
  }
}

static void Code_l4r(Word Index)
{
  unsigned Op1, Op2, Op3, Op4;

  if (ArgCnt != 4) WrError(1110);
  else if (!ParseReg(ArgStr[1], &Op1)) WrXError(1445, ArgStr[1]);
  else if (!ParseReg(ArgStr[2], &Op2)) WrXError(1445, ArgStr[2]);
  else if (!ParseReg(ArgStr[3], &Op3)) WrXError(1445, ArgStr[3]);
  else if (!ParseReg(ArgStr[4], &Op4)) WrXError(1445, ArgStr[4]);
  {
    WAsmCode[0] = 0xf800
                | ((Op3 & 3) << 0)
                | ((Op2 & 3) << 2)
                | ((Op1 & 3) << 4)
                | (UpVal(Op3, Op2, Op1, 0) << 6);
    WAsmCode[1] = Index | Op4;
    CodeLen = 4;
  }
}

static void Code_l5r(Word Index)
{
  unsigned Op1, Op2, Op3, Op4, Op5;

  if (ArgCnt != 5) WrError(1110);
  else if (!ParseReg(ArgStr[1], &Op1)) WrXError(1445, ArgStr[1]);
  else if (!ParseReg(ArgStr[2], &Op4)) WrXError(1445, ArgStr[2]);
  else if (!ParseReg(ArgStr[3], &Op2)) WrXError(1445, ArgStr[3]);
  else if (!ParseReg(ArgStr[4], &Op3)) WrXError(1445, ArgStr[4]);
  else if (!ParseReg(ArgStr[5], &Op5)) WrXError(1445, ArgStr[5]);
  {
    unsigned Up = UpVal(Op5, Op4, 0, 0) + 27;
    WAsmCode[0] = 0xf800
                | ((Op3 & 3) << 0)
                | ((Op2 & 3) << 2)
                | ((Op1 & 3) << 4)
                | (UpVal(Op3, Op2, Op1, 0) << 6);
    WAsmCode[1] = Index
                | ((Op5 & 3) << 0)
                | ((Op4 & 3) << 2)
                | ((Up & 0x1f) << 6)
                | (Up & 0x20);
    CodeLen = 4;
  }
}

static void Code_l6r(Word Index)
{
  unsigned Op1, Op2, Op3, Op4, Op5, Op6;

  if (ArgCnt != 6) WrError(1110);
  else if (!ParseReg(ArgStr[1], &Op1)) WrXError(1445, ArgStr[1]);
  else if (!ParseReg(ArgStr[2], &Op4)) WrXError(1445, ArgStr[2]);
  else if (!ParseReg(ArgStr[3], &Op2)) WrXError(1445, ArgStr[3]);
  else if (!ParseReg(ArgStr[4], &Op3)) WrXError(1445, ArgStr[4]);
  else if (!ParseReg(ArgStr[5], &Op5)) WrXError(1445, ArgStr[5]);
  else if (!ParseReg(ArgStr[6], &Op6)) WrXError(1445, ArgStr[6]);
  {
    WAsmCode[0] = 0xf800
                | ((Op3 & 3) << 0)
                | ((Op2 & 3) << 2)
                | ((Op1 & 3) << 4)
                | (UpVal(Op3, Op2, Op1, 0) << 6);
    WAsmCode[1] = Index
                | ((Op6 & 3) << 0)
                | ((Op5 & 3) << 2)
                | ((Op4 & 3) << 4)
                | (UpVal(Op6, Op5, Op4, 0) << 6);
    CodeLen = 4;
  }
}

static void Code_branch_core(Word Code, int DistArgIndex)
{
  LongInt Delta;
  Boolean OK;

  /* assume short branch for distance computation */

  FirstPassUnknown = FALSE;
  Delta = EvalIntExpression(ArgStr[DistArgIndex], UInt32, &OK) - (EProgCounter() + 2);
  if (FirstPassUnknown)
    Delta &= 0x1fffe;

  /* distance must be even */

  if (Delta & 1) WrError(1325);

  /* short branch possible? */

  else if ((Delta >= -126) && (Delta <= 126))
  {
    Delta /= 2;
    WAsmCode[0] = Code;
    if (Delta < 0)
    {
      WAsmCode[0] |= 0x0400;
      Delta = -Delta;
    }
    WAsmCode[0] |= Delta & 0x3f;
    CodeLen = 2;
  }

  /* must use long: */

  else
  {
    /* correct delta for longer instruction */

    Delta -= 2;
    if ((Delta < -131070) || (Delta >= 131070)) WrError(1370);
    else
    {
      Delta /= 2;
      WAsmCode[1] = Code;
      if (Delta < 0)
      {
        WAsmCode[1] |= 0x0400;
        Delta = -Delta;
      }
      WAsmCode[1] |= Delta & 0x3f;
      WAsmCode[0] = 0xf00 | ((Delta >> 6) & 0x3ff);
      CodeLen = 4;
    }
  }
}

static void Code_BRU(Word Index)
{
  unsigned Op1;

  if (ArgCnt != 1) WrError(1110);
  else if (ParseReg(ArgStr[1], &Op1))
  {
    WAsmCode[0] = 0x2fe0 | Op1;
    CodeLen = 2;
  }
  else
    Code_branch_core(0x7300, 1);
}

static void Code_cbranch(Word Index)
{
  unsigned Op1;

  if (ArgCnt != 2) WrError(1110);
  else if (!ParseReg(ArgStr[1], &Op1)) WrXError(1445, ArgStr[1]);
    Code_branch_core(Index | (Op1 << 6), 2);
}

static void Code_r2r(Word Index)
{
  unsigned Op1, Op2;

  if (ArgCnt != 2) WrError(1110);
  else if (!ParseReg(ArgStr[1], &Op1)) WrXError(1445, ArgStr[1]);
  else if (!ParseReg(ArgStr[2], &Op2)) WrXError(1445, ArgStr[2]);
  {
    unsigned Up = UpVal(Op1, Op2, 0, 0) + 27;

    WAsmCode[0] = Index
                | ((Op1 & 3) << 0)
                | ((Op2 & 3) << 2)
                | ((Up & 0x1f) << 6)
                | (Up & 0x20);
    CodeLen = 2;
  }
}

static void Code_lr2r(Word Index)
{
  unsigned Op1, Op2;
  lr2r_Order *pOrder = lr2r_Orders + Index;

  if (ArgCnt != 2) WrError(1110);
  else if (!ParseReg(ArgStr[1], &Op1)) WrXError(1445, ArgStr[1]);
  else if (!ParseReg(ArgStr[2], &Op2)) WrXError(1445, ArgStr[2]);
  else
  {
    unsigned Up = UpVal(Op1, Op2, 0, 0) + 27;

    WAsmCode[0] = pOrder->Opcode1
                | ((Op1 & 3) << 0)
                | ((Op2 & 3) << 2)
                | ((Up & 0x1f) << 6)
                | (Up & 0x20);
    WAsmCode[1] = pOrder->Opcode2;
    CodeLen = 4;
  }
}

/*--------------------------------------------------------------------------*/
/* Dynamic Code Table Handling */  

static void Add_lr2r(char *NName, Word NCode1, Word NCode2)
{
  if (InstrZ >= (sizeof(lr2r_Orders) / sizeof(*lr2r_Orders)))
    exit(255);
  lr2r_Orders[InstrZ].Opcode1 = NCode1;
  lr2r_Orders[InstrZ].Opcode2 = NCode2;
  AddInstTable(InstTable, NName, InstrZ++, Code_lr2r);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(207);

  AddInstTable(InstTable, "REG", 0, CodeREG);

  AddInstTable(InstTable, "ADD"    , 0x1000, Code_3r);
  AddInstTable(InstTable, "AND"    , 0x3800, Code_3r);
  AddInstTable(InstTable, "EQ"     , 0x3000, Code_3r);
  AddInstTable(InstTable, "LD16S"  , 0x8000, Code_3r);
  AddInstTable(InstTable, "LD8U"   , 0x8800, Code_3r);
  AddInstTable(InstTable, "LDW"    , 0x4800, Code_3r);
  AddInstTable(InstTable, "LSS"    , 0xc000, Code_3r);
  AddInstTable(InstTable, "LSU"    , 0xc800, Code_3r);
  AddInstTable(InstTable, "LSU"    , 0xc800, Code_3r);
  AddInstTable(InstTable, "OR"     , 0x4000, Code_3r);
  AddInstTable(InstTable, "SHL"    , 0x2000, Code_3r);
  AddInstTable(InstTable, "SHR"    , 0x2800, Code_3r);
  AddInstTable(InstTable, "SUB"    , 0x1800, Code_3r);
  AddInstTable(InstTable, "TSETR"  , 0xb800, Code_3r);

  AddInstTable(InstTable, "ADDI"   , 0x9000, Code_2rus);
  AddInstTable(InstTable, "EQI"    , 0xb000, Code_2rus);
  AddInstTable(InstTable, "LDWI"   , 0x0800, Code_2rus);
  AddInstTable(InstTable, "SHLI"   , 0xa000, Code_2rus);
  AddInstTable(InstTable, "SHRI"   , 0xa800, Code_2rus);
  AddInstTable(InstTable, "STWI"   , 0x0000, Code_2rus);
  AddInstTable(InstTable, "SUBI"   , 0x9800, Code_2rus);

  AddInstTable(InstTable, "ANDNOT" , 0x2800, Code_2r);
  AddInstTable(InstTable, "CHKCT"  , 0xc800, Code_2r);
  AddInstTable(InstTable, "EEF"    , 0x2810, Code_2r);
  AddInstTable(InstTable, "EET"    , 0x2010, Code_2r);
  AddInstTable(InstTable, "ENDIN"  , 0x9010, Code_2r);
  AddInstTable(InstTable, "GETST"  , 0x0010, Code_2r);
  AddInstTable(InstTable, "GETTS"  , 0x3800, Code_2r);
  AddInstTable(InstTable, "IN"     , 0xb000, Code_2r);
  AddInstTable(InstTable, "INCT"   , 0x8010, Code_2r);
  AddInstTable(InstTable, "INSHR"  , 0xb010, Code_2r);
  AddInstTable(InstTable, "INT"    , 0x8810, Code_2r);
  AddInstTable(InstTable, "MKMSK"  , 0xa000, Code_2r);
  AddInstTable(InstTable, "NEG"    , 0x9000, Code_2r);
  AddInstTable(InstTable, "NOT"    , 0x8800, Code_2r);
  AddInstTable(InstTable, "OUTCT"  , 0x4800, Code_2r);
  AddInstTable(InstTable, "PEEK"   , 0xb800, Code_2r);
  AddInstTable(InstTable, "SEXT"   , 0x3000, Code_2r);
  AddInstTable(InstTable, "TESTCT" , 0xb810, Code_2r);
  AddInstTable(InstTable, "TESTWCT", 0xc010, Code_2r);
  AddInstTable(InstTable, "TINITCP", 0x1800, Code_2r);
  AddInstTable(InstTable, "TINITDP", 0x0800, Code_2r);
  AddInstTable(InstTable, "TINITPC", 0x0000, Code_2r);
  AddInstTable(InstTable, "TINITSP", 0x1000, Code_2r);
  AddInstTable(InstTable, "TSETMR" , 0x1810, Code_2r);
  AddInstTable(InstTable, "ZEXT"   , 0x4000, Code_2r);

  AddInstTable(InstTable, "ASHR"   , 0x17ec, Code_l3r);
  AddInstTable(InstTable, "LDA16F" , 0x2fec, Code_l3r);
  AddInstTable(InstTable, "REMU"   , 0xcfec, Code_l3r);
  AddInstTable(InstTable, "CRC"    , 0xafec, Code_l3r);
  AddInstTable(InstTable, "LDAWB"  , 0x27ec, Code_l3r);
  AddInstTable(InstTable, "ST16"   , 0x87ec, Code_l3r);
  AddInstTable(InstTable, "DIVS"   , 0x47ec, Code_l3r);
  AddInstTable(InstTable, "LDAWF"  , 0x1fec, Code_l3r);
  AddInstTable(InstTable, "ST8"    , 0x8fec, Code_l3r);
  AddInstTable(InstTable, "DIVU"   , 0x4fec, Code_l3r);
  AddInstTable(InstTable, "MUL"    , 0x3fec, Code_l3r);
  AddInstTable(InstTable, "STW"    , 0x07ec, Code_l3r);
  AddInstTable(InstTable, "LDA16B" , 0x37ec, Code_l3r);
  AddInstTable(InstTable, "REMS"   , 0xc7ec, Code_l3r);
  AddInstTable(InstTable, "XOR"    , 0x0fec, Code_l3r);

  AddInstTable(InstTable, "ASHRI"  , 0x97ec, Code_l2rus);
  AddInstTable(InstTable, "LDAWBI" , 0xa7ec, Code_l2rus);
  AddInstTable(InstTable, "OUTPW"  , 0x97ed, Code_l2rus);
  AddInstTable(InstTable, "INPW"   , 0x97ee, Code_l2rus);
  AddInstTable(InstTable, "LDAWFI" , 0x9fec, Code_l2rus);

  AddInstTable(InstTable, "BAU"    , 0x27f0, Code_1r);
  AddInstTable(InstTable, "EEU"    , 0x07f0, Code_1r);
  AddInstTable(InstTable, "SETSP"  , 0x2ff0, Code_1r);
  AddInstTable(InstTable, "BLA"    , 0x27e0, Code_1r);
  AddInstTable(InstTable, "FREER"  , 0x17e0, Code_1r);
  AddInstTable(InstTable, "SETV"   , 0x47f0, Code_1r);
  AddInstTable(InstTable, "KCALL"  , 0x27e0, Code_1r);
  AddInstTable(InstTable, "SYNCR"  , 0x87f0, Code_1r);
  AddInstTable(InstTable, "CLRPT"  , 0x87e0, Code_1r);
  AddInstTable(InstTable, "MJOIN"  , 0x17f0, Code_1r);
  AddInstTable(InstTable, "TSTART" , 0x1fe0, Code_1r);
  AddInstTable(InstTable, "DGETREG", 0x3fe0, Code_1r);
  AddInstTable(InstTable, "MSYNC"  , 0x1ff0, Code_1r);
  AddInstTable(InstTable, "WAITEF" , 0x0ff0, Code_1r);
  AddInstTable(InstTable, "ECALLF" , 0x4fe0, Code_1r);
  AddInstTable(InstTable, "SETCP"  , 0x37f0, Code_1r);
  AddInstTable(InstTable, "WAITET" , 0x0fe0, Code_1r);
  AddInstTable(InstTable, "ECALLT" , 0x4ff0, Code_1r);
  AddInstTable(InstTable, "SETDP"  , 0x37e0, Code_1r);
  AddInstTable(InstTable, "EDU"    , 0x07e0, Code_1r);
  AddInstTable(InstTable, "SETEV"  , 0x3ff0, Code_1r);

  AddInstTable(InstTable, "BITREV" , 0x07ec | 0, Code_l2r);
  AddInstTable(InstTable, "GETD"   , 0x1fec | 1, Code_l2r);
  AddInstTable(InstTable, "SETC"   , 0x2fec | 1, Code_l2r);
  AddInstTable(InstTable, "BYTEREV", 0x07ec | 1, Code_l2r);
  AddInstTable(InstTable, "GETN"   , 0x37ec | 1, Code_l2r);
  AddInstTable(InstTable, "TESTLCL", 0x27ec | 0, Code_l2r);
  AddInstTable(InstTable, "CLZ"    , 0x0fec | 0, Code_l2r);
  AddInstTable(InstTable, "GETPS"  , 0x17ec | 1, Code_l2r);
  AddInstTable(InstTable, "TINITLR", 0x17ec | 1, Code_l2r);

  AddInstTable(InstTable, "BLACP"  , 0xe000, Code_u10);
  AddInstTable(InstTable, "BLRF"   , 0xd000, Code_u10);
  AddInstTable(InstTable, "LDAPF"  , 0xd800, Code_u10);
  AddInstTable(InstTable, "BLRB"   , 0xd400, Code_u10);
  AddInstTable(InstTable, "LDAPB"  , 0xdc00, Code_u10);
  AddInstTable(InstTable, "LDWCPL" , 0xe400, Code_u10);

  AddInstTable(InstTable, "BLAT"   , 0x7340, Code_u6);
  AddInstTable(InstTable, "EXTDP"  , 0x7380, Code_u6);
  AddInstTable(InstTable, "KRESTSP", 0x7bc0, Code_u6);
  AddInstTable(InstTable, "BRBU"   , 0x7700, Code_u6);
  AddInstTable(InstTable, "EXTSP"  , 0x7780, Code_u6);
  AddInstTable(InstTable, "LDAWCP" , 0x7f40, Code_u6);
  AddInstTable(InstTable, "BRFU"   , 0x7300, Code_u6);
  AddInstTable(InstTable, "GETSR"  , 0x7f00, Code_u6);
  AddInstTable(InstTable, "RETSP"  , 0x77c0, Code_u6);
  AddInstTable(InstTable, "CLRSR"  , 0x7b00, Code_u6);
  AddInstTable(InstTable, "KCALLI" , 0x73c0, Code_u6);
  AddInstTable(InstTable, "SETSR"  , 0x7b40, Code_u6);
  AddInstTable(InstTable, "ENTSP"  , 0x7740, Code_u6);
  AddInstTable(InstTable, "KENTSP" , 0x7b80, Code_u6);

  AddInstTable(InstTable, "BRBF"   , 0x7c00, Code_ru6);
  AddInstTable(InstTable, "LDAWSP" , 0x6400, Code_ru6);
  AddInstTable(InstTable, "SETCI"  , 0xe800, Code_ru6);
  AddInstTable(InstTable, "BRBT"   , 0x7400, Code_ru6);
  AddInstTable(InstTable, "LDC"    , 0x6800, Code_ru6);
  AddInstTable(InstTable, "STWDP"  , 0x5000, Code_ru6);
  AddInstTable(InstTable, "BRFF"   , 0x7800, Code_ru6);
  AddInstTable(InstTable, "LDWCP"  , 0x6c00, Code_ru6);
  AddInstTable(InstTable, "STWSP"  , 0x5400, Code_ru6);
  AddInstTable(InstTable, "BRFT"   , 0x7000, Code_ru6);
  AddInstTable(InstTable, "LDWDP"  , 0x5800, Code_ru6);
  AddInstTable(InstTable, "LDAWDP" , 0x6000, Code_ru6);
  AddInstTable(InstTable, "LDWSP"  , 0x5c00, Code_ru6);

  AddInstTable(InstTable, "CHKCTI" , 0xc810, Code_rus);
  AddInstTable(InstTable, "MKMSKI" , 0xa010, Code_rus);
  AddInstTable(InstTable, "SEXTI"  , 0x3010, Code_rus);
  AddInstTable(InstTable, "GETR"   , 0x8000, Code_rus);
  AddInstTable(InstTable, "OUTCTI" , 0x4810, Code_rus);
  AddInstTable(InstTable, "ZEXTI"  , 0x4010, Code_rus);

  AddInstTable(InstTable, "CLRE"   , 0x07ed, Code_0r);
  AddInstTable(InstTable, "GETID"  , 0x17ee, Code_0r);
  AddInstTable(InstTable, "SETKEP" , 0x07ff, Code_0r);
  AddInstTable(InstTable, "DCALL"  , 0x07fc, Code_0r);
  AddInstTable(InstTable, "GETKEP" , 0x17ef, Code_0r);
  AddInstTable(InstTable, "SSYNC"  , 0x07ee, Code_0r);
  AddInstTable(InstTable, "DENTSP" , 0x17ec, Code_0r);
  AddInstTable(InstTable, "GETKSP" , 0x17fc, Code_0r);
  AddInstTable(InstTable, "STET"   , 0x0ffd, Code_0r);
  AddInstTable(InstTable, "DRESTSP", 0x17ed, Code_0r);
  AddInstTable(InstTable, "KRET"   , 0x07fd, Code_0r);
  AddInstTable(InstTable, "STSED"  , 0x0ffc, Code_0r);
  AddInstTable(InstTable, "DRET"   , 0x07fe, Code_0r);
  AddInstTable(InstTable, "LDET"   , 0x17fe, Code_0r);
  AddInstTable(InstTable, "STSPC"  , 0x0fed, Code_0r);
  AddInstTable(InstTable, "FREET"  , 0x07ef, Code_0r);
  AddInstTable(InstTable, "LDSED"  , 0x17fd, Code_0r);
  AddInstTable(InstTable, "STSSR"  , 0x0fef, Code_0r);
  AddInstTable(InstTable, "GETED"  , 0x0ffe, Code_0r);
  AddInstTable(InstTable, "LDSPC"  , 0x0fec, Code_0r);
  AddInstTable(InstTable, "WAITEU" , 0x07ec, Code_0r);
  AddInstTable(InstTable, "GETET"  , 0x0fff, Code_0r);
  AddInstTable(InstTable, "LDSSR"  , 0x0fee, Code_0r);

  AddInstTable(InstTable, "CRC8"   , 0x07e0, Code_l4r);
  AddInstTable(InstTable, "MACCS"  , 0x0fe0, Code_l4r);
  AddInstTable(InstTable, "MACCU"  , 0x07f0, Code_l4r);

  AddInstTable(InstTable, "LADD"   , 0x0010, Code_l5r);
  AddInstTable(InstTable, "LDIVU"  , 0x0000, Code_l5r);
  AddInstTable(InstTable, "LSUB"   , 0x0100, Code_l5r);

  AddInstTable(InstTable, "LMUL"   , 0x0000, Code_l6r);

  AddInstTable(InstTable, "OUT"    , 0xa800, Code_r2r);
  AddInstTable(InstTable, "OUTT"   , 0x0810, Code_r2r);
  AddInstTable(InstTable, "SETPSC" , 0xc000, Code_r2r);
  AddInstTable(InstTable, "OUTSHR" , 0xa810, Code_r2r);
  AddInstTable(InstTable, "SETD"   , 0x1010, Code_r2r);
  AddInstTable(InstTable, "SETPT"  , 0x3810, Code_r2r);


  InstrZ = 0;
  Add_lr2r("SETCLK"  , 0xf810, 0x0fec);
  Add_lr2r("SETPS"   , 0xf800, 0x1fec);
  Add_lr2r("SETTW"   , 0xf810, 0x27ec);
  Add_lr2r("SETN"    , 0xf800, 0x37ec);
  Add_lr2r("SETRDY"  , 0xf800, 0x2fec);

  AddInstTable(InstTable, "BRU"    , 0x7300, Code_BRU);
  AddInstTable(InstTable, "BRF"    , 0x7800, Code_cbranch);
  AddInstTable(InstTable, "BRT"    , 0x7000, Code_cbranch);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*--------------------------------------------------------------------------*/
/* Callbacks */

static void MakeCode_XCore(void)
{
  CodeLen = 0;

  DontPrint = False;

  /* Null Instruction */

  if ((*OpPart == '\0') && (ArgCnt == 0))
    return;

  /* Pseudo Instructions */

  if (DecodeIntelPseudo(True))
    return;

  /* Odd Program Counter ? */

  if (Odd(EProgCounter())) WrError(180);

  /* everything from hash table */

  if (!LookupInstTable(InstTable, OpPart))
    WrXError(1200, OpPart);
}

static Boolean IsDef_XCore(void)
{
  return Memo("REG");
}

static void SwitchFrom_XCore(void)
{
  DeinitFields();
}

static void SwitchTo_XCore(void)
{
  PFamilyDescr pDescr;

  TurnWords = False; ConstMode = ConstModeMoto; SetIsOccupied = False;

  pDescr = FindFamilyByName("XCore");
  PCSymbol = "$"; HeaderID = pDescr->Id; NOPCode = 0x0000;
  DivideChars = ","; HasAttrs = False;

  ValidSegs = (1 << SegCode);
  Grans[SegCode] = 1; ListGrans[SegCode] = 2; SegInits[SegCode] = 0;
  SegLimits[SegCode] = 0xffffffffl;

  MakeCode = MakeCode_XCore; IsDef = IsDef_XCore;

  SwitchFrom = SwitchFrom_XCore; InitFields();
}

/*--------------------------------------------------------------------------*/
/* Initialisation */  

void codexcore_init(void)
{
  CPUXS1 = AddCPU("XS1", SwitchTo_XCore);
}
