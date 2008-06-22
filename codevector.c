/* codevector.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Atari Asteroids Vector Processor                            */
/*                                                                           */
/*****************************************************************************/
/* $Id: codevector.c,v 1.1 2008/04/13 20:23:46 alfred Exp $                  */
/*****************************************************************************
 * $Log: codevector.c,v $
 * Revision 1.1  2008/04/13 20:23:46  alfred
 * - add Atari Vecor Processor target
 *
 *****************************************************************************/

#include "stdinc.h"
#include <stdio.h>
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
#include "codevector.h"

static CPUVar CPUVector;

/*--------------------------------------------------------------------------
 * Operand Parsers
 *--------------------------------------------------------------------------*/

static Boolean Is4(const char *pAsc, Word *pResult)
{
  *pResult = 0;

  for (; *pAsc; pAsc++)
  {
    if (!isdigit(*pAsc))
      return False;
    *pResult = (*pResult * 10) + (*pAsc - '0');
  }
  return (*pResult <= 15);
}

static Boolean DecodeScale(char *pAsc, Word *pResult)
{
  Boolean OK;

  KillPrefBlanks(pAsc); KillPostBlanks(pAsc);

  if ((toupper(*pAsc) == 'S') && (Is4(pAsc + 1, pResult)))
    return True;

  *pResult = EvalIntExpression(pAsc, UInt4, &OK);
  return OK;
}

static Boolean DecodeBright(char *pAsc, Word *pResult)
{
  Boolean OK;

  KillPrefBlanks(pAsc); KillPostBlanks(pAsc);

  if ((toupper(*pAsc) == 'Z') && (Is4(pAsc + 1, pResult)))
    return True;

  *pResult = EvalIntExpression(pAsc, UInt4, &OK);
  return OK;
}

static Boolean DecodeSign(char *pAsc, Word *pResult, Boolean Signed)
{
  LongInt Val;
  Boolean OK;

  Val = EvalIntExpression(pAsc, SInt16, &OK);
  if (!OK)
    return False;

  if (FirstPassUnknown)
    Val = 0;

  if (!ChkRange(Val, Signed ? -1023 : 0, 1023))
    return False;

  if ((Signed) && (Val < 0))
    *pResult = (-Val) | (1 << 10);
  else
    *pResult = Val;
  return True;
}

static Boolean DecodeXY(char *pAsc, Word *pX, Word *pY, Boolean Signed)
{
  char *pEnd, *pPos;

  KillPrefBlanks(pAsc); KillPostBlanks(pAsc);

  if (*pAsc != '(')
  {
    WrError(1300);
    return False;
  }
  pAsc++;

  pEnd = pAsc + strlen(pAsc) - 1;
  if (*pEnd != ')')
  {
    WrError(1300);
    return False; 
  }
  *pEnd = '\0';

  pPos = strchr(pAsc, ',');
  if (!pPos)
  {
    WrError(1100);
    return False; 
  }
  *pPos = '\0';

  if (!DecodeSign(pAsc, pX, Signed))
    return False;
  if (!DecodeSign(pPos + 1, pY, Signed))
    return False;

  return True;
}

/*--------------------------------------------------------------------------
 * Code Handlers
 *--------------------------------------------------------------------------*/

static void DecodeFixed(Word Index)
{
  if (ArgCnt != 0) WrError(1110);
  else
  {
    WAsmCode[0] = Index;
    CodeLen = 1;
  }
}

static void DecodeJmp(Word Index)
{
  Boolean OK;

  if (ArgCnt != 1) WrError(1110);
  else
  {
    WAsmCode[0] = Index | EvalIntExpression(ArgStr[1], UInt12, &OK);
    if (OK)
      CodeLen = 1;
  }
}

static void DecodeLAbs(Word Index)
{
  Word X, Y, Scale;

  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else if (!DecodeXY(ArgStr[1], &X, &Y, False));
  else if (!DecodeScale(ArgStr[2], &Scale));
  else
  {
    WAsmCode[0] = 0xa000 | Y;
    WAsmCode[1] = (Scale << 12) | X;
    CodeLen = 2;
  }
}

static void DecodeVctr(Word Index)
{
  Word X, Y, Scale, Bright;

  UNUSED(Index);

  if (ArgCnt != 3) WrError(1110);
  else if (!DecodeXY(ArgStr[1], &X, &Y, True));
  else if (!DecodeScale(ArgStr[2], &Scale));
  else if (!DecodeBright(ArgStr[3], &Bright));
  else
  {
    WAsmCode[0] = (Scale << 12) | Y;
    WAsmCode[1] = (Bright << 12) | X;
    CodeLen = 2;
  }
}

static void DecodeSVec(Word Index)
{
  Word X, Y, Scale, Bright;

  UNUSED(Index);

  if (ArgCnt != 3) WrError(1110);
  else if (!DecodeXY(ArgStr[1], &X, &Y, True));
  else if ((X & 0xff) || (Y & 0xff)) WrError(1325);
  else if (!DecodeScale(ArgStr[2], &Scale));
  else if (Scale > 3) WrError(1320);
  else if (!DecodeBright(ArgStr[3], &Bright));
  else
  {
    WAsmCode[0] = 0xf000
                | (Bright << 4)
                | ((X >> 8) & 3)
                | (Y & 0x300)
                | ((Scale & 1) << 11)
                | ((Scale & 2) << 2);
    CodeLen = 1;
  }
}

/*--------------------------------------------------------------------------
 * Instruction Table Handling
 *--------------------------------------------------------------------------*/

static void InitFields(void)
{
  InstTable = CreateInstTable(17);

  AddInstTable(InstTable, "RTSL", 0xd000, DecodeFixed);
  AddInstTable(InstTable, "HALT", 0xb0b0, DecodeFixed);

  AddInstTable(InstTable, "JMPL", 0xe000, DecodeJmp);
  AddInstTable(InstTable, "JSRL", 0xc000, DecodeJmp);

  AddInstTable(InstTable, "LABS", 0x0000, DecodeLAbs);
  AddInstTable(InstTable, "VCTR", 0x0000, DecodeVctr);
  AddInstTable(InstTable, "SVEC", 0x0000, DecodeSVec);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*--------------------------------------------------------------------------
 * Semipublic Functions
 *--------------------------------------------------------------------------*/

static Boolean IsDef_Vector(void)
{
  return FALSE; 
}

static void SwitchFrom_Vector(void)
{
  DeinitFields();
}

static void MakeCode_Vector(void)
{
  CodeLen = 0; DontPrint = False;

  /* zu ignorierendes */

  if (Memo("")) return;

  if (!LookupInstTable(InstTable, OpPart))
    WrXError(1200, OpPart);
}

static void SwitchTo_Vector(void)
{
  PFamilyDescr FoundDescr;

  FoundDescr = FindFamilyByName("ATARI_VECTOR");

  TurnWords = False; ConstMode = ConstModeMoto; SetIsOccupied = False;

  PCSymbol = "$"; HeaderID = FoundDescr->Id;

  /* NOP = ??? */

  NOPCode = 0x00000;
  DivideChars = ","; HasAttrs = False;

  ValidSegs = (1 << SegCode);
  Grans[SegCode] = 2; ListGrans[SegCode] = 2; SegInits[SegCode] = 0;
  SegLimits[SegCode] = 0xfff;

  MakeCode = MakeCode_Vector; IsDef = IsDef_Vector;
  SwitchFrom = SwitchFrom_Vector; InitFields();
}

/*--------------------------------------------------------------------------
 * Initialization
 *--------------------------------------------------------------------------*/

void codevector_init(void)
{
  CPUVector = AddCPU("ATARI_VECTOR", SwitchTo_Vector);
}
