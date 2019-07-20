/* operator.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* defintion of operators                                                    */
/*                                                                           */
/*****************************************************************************/

#include <math.h>

#include "stdinc.h"
#include "errmsg.h"
#include "asmerr.h"
#include "asmrelocs.h"
#include "operator.h"

static void DummyOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  UNUSED(pLVal);
  UNUSED(pRVal);
  UNUSED(pErg);
}

static void OneComplOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  UNUSED(pLVal);
  pErg->Typ = TempInt;
  pErg->Contents.Int = ~(pRVal->Contents.Int);
}

static void ShLeftOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  pErg->Contents.Int = pLVal->Contents.Int << pRVal->Contents.Int;
}

static void ShRightOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  pErg->Contents.Int = pLVal->Contents.Int >> pRVal->Contents.Int;
}

static void BitMirrorOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  int z;

  if ((pRVal->Contents.Int < 1) || (pRVal->Contents.Int > 32)) WrError(ErrNum_OverRange);
  else
  {
    pErg->Typ = TempInt;
    pErg->Contents.Int = (pLVal->Contents.Int >> pRVal->Contents.Int) << pRVal->Contents.Int;
    pRVal->Contents.Int--;
    for (z = 0; z <= pRVal->Contents.Int; z++)
    {
      if ((pLVal->Contents.Int & (1 << (pRVal->Contents.Int - z))) != 0)
        pErg->Contents.Int |= (1 << z);
    }
  }
}

static void BinAndOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  pErg->Contents.Int = pLVal->Contents.Int & pRVal->Contents.Int;
}

static void BinOrOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  pErg->Contents.Int = pLVal->Contents.Int | pRVal->Contents.Int;
}

static void BinXorOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  pErg->Contents.Int = pLVal->Contents.Int ^ pRVal->Contents.Int;
}

static void PotOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  LargeInt HVal;

  switch (pErg->Typ = pLVal->Typ)
  {
    case TempInt:
      if (pRVal->Contents.Int < 0) pErg->Contents.Int = 0;
      else
      {
        pErg->Contents.Int = 1;
        while (pRVal->Contents.Int > 0)
        {
          if (pRVal->Contents.Int & 1)
            pErg->Contents.Int *= pLVal->Contents.Int;
          pRVal->Contents.Int >>= 1;
          if (pRVal->Contents.Int != 0)
            pLVal->Contents.Int *= pLVal->Contents.Int;
        }
      }
      break;
    case TempFloat:
      if (pRVal->Contents.Float == 0.0)
        pErg->Contents.Float = 1.0;
      else if (pLVal->Contents.Float == 0.0)
        pErg->Contents.Float = 0.0;
      else if (pLVal->Contents.Float > 0)
        pErg->Contents.Float = pow(pLVal->Contents.Float, pRVal->Contents.Float);
      else if ((fabs(pRVal->Contents.Float) <= ((double)MaxLongInt)) && (floor(pRVal->Contents.Float) == pRVal->Contents.Float))
      {
        HVal = (LongInt) floor(pRVal->Contents.Float + 0.5);
        if (HVal < 0)
        {
          pLVal->Contents.Float = 1 / pLVal->Contents.Float;
          HVal = -HVal;
        }
        pErg->Contents.Float = 1.0;
        while (HVal > 0)
        {
          if ((HVal & 1) == 1)
            pErg->Contents.Float *= pLVal->Contents.Float;
          pLVal->Contents.Float *= pLVal->Contents.Float;
          HVal >>= 1;
        }
      }
      else
      {
        WrError(ErrNum_InvArgPair);
        pErg->Typ = TempNone;
      }
      break;
    default:
      break;
  }
}

static void MultOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  switch (pErg->Typ = pLVal->Typ)
  {
    case TempInt:
      pErg->Contents.Int = pLVal->Contents.Int * pRVal->Contents.Int;
      break;
    case TempFloat:
      pErg->Contents.Float = pLVal->Contents.Float * pRVal->Contents.Float;
      break;
    default:
      break;
  }
}

static void DivOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  switch (pLVal->Typ)
  {
    case TempInt:
      if (pRVal->Contents.Int == 0) WrError(ErrNum_DivByZero);
      else
      {
        pErg->Typ = TempInt;
        pErg->Contents.Int = pLVal->Contents.Int / pRVal->Contents.Int;
      }
      break;
    case TempFloat:
      if (pRVal->Contents.Float == 0.0) WrError(ErrNum_DivByZero);
      else
      {
        pErg->Typ = TempFloat;
        pErg->Contents.Float = pLVal->Contents.Float / pRVal->Contents.Float;
      }
    default:
      break;
  }
}

static void ModOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  if (pRVal->Contents.Int == 0) WrError(ErrNum_DivByZero);
  else
  {
    pErg->Typ = TempInt;
    pErg->Contents.Int = pLVal->Contents.Int % pRVal->Contents.Int;
  }
}

static void AddOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  switch (pErg->Typ = pLVal->Typ)
  {
    case TempInt:
      pErg->Contents.Int = pLVal->Contents.Int + pRVal->Contents.Int;
      pErg->Relocs = MergeRelocs(&(pLVal->Relocs), &(pRVal->Relocs), TRUE);
      break;
    case TempFloat:
      pErg->Contents.Float = pLVal->Contents.Float + pRVal->Contents.Float;
      break;
    case TempString:
      DynString2DynString(&pErg->Contents.Ascii, &pLVal->Contents.Ascii);
      DynStringAppendDynString(&pErg->Contents.Ascii, &pRVal->Contents.Ascii);
      break;
    default:
      break;
  }
}

static void SubOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  switch (pErg->Typ = pLVal->Typ)
  {
    case TempInt:
      pErg->Contents.Int = pLVal->Contents.Int - pRVal->Contents.Int;
      pErg->Relocs = MergeRelocs(&(pLVal->Relocs), &(pRVal->Relocs), FALSE);
      break;
    case TempFloat:
      pErg->Contents.Float = pLVal->Contents.Float - pRVal->Contents.Float;
      break;
    default:
      break;
  }
}

static void LogNotOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  UNUSED(pLVal);
  pErg->Typ = TempInt;
  pErg->Contents.Int = (pRVal->Contents.Int == 0) ? 1 : 0;
}

static void LogAndOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  pErg->Contents.Int = ((pLVal->Contents.Int != 0) && (pRVal->Contents.Int != 0)) ? 1 : 0;
}

static void LogOrOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  pErg->Contents.Int = ((pLVal->Contents.Int != 0) || (pRVal->Contents.Int != 0)) ? 1 : 0;
}

static void LogXorOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  pErg->Contents.Int = ((pLVal->Contents.Int != 0) != (pRVal->Contents.Int != 0)) ? 1 : 0;
}

static void EqOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  switch (pLVal->Typ)
  {
    case TempInt:
      pErg->Contents.Int = (pLVal->Contents.Int == pRVal->Contents.Int) ? 1 : 0;
      break;
    case TempFloat:
      pErg->Contents.Int = (pLVal->Contents.Float == pRVal->Contents.Float) ? 1 : 0;
      break;
    case TempString:
      pErg->Contents.Int = (DynStringCmp(&pLVal->Contents.Ascii, &pRVal->Contents.Ascii) == 0) ? 1 : 0;
      break;
    default:
      break;
  }
}

static void GtOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  switch (pLVal->Typ)
  {
    case TempInt:
      pErg->Contents.Int = (pLVal->Contents.Int > pRVal->Contents.Int) ? 1 : 0;
      break;
    case TempFloat:
      pErg->Contents.Int = (pLVal->Contents.Float > pRVal->Contents.Float) ? 1 : 0;
      break;
    case TempString:
      pErg->Contents.Int = (DynStringCmp(&pLVal->Contents.Ascii, &pRVal->Contents.Ascii) > 0) ? 1 : 0;
      break;
    default:
      break;
  }
}

static void LtOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  switch (pLVal->Typ)
  {
    case TempInt:
      pErg->Contents.Int = (pLVal->Contents.Int < pRVal->Contents.Int) ? 1 : 0;
      break;
    case TempFloat:
      pErg->Contents.Int = (pLVal->Contents.Float < pRVal->Contents.Float) ? 1 : 0;
      break;
    case TempString:
      pErg->Contents.Int = (DynStringCmp(&pLVal->Contents.Ascii, &pRVal->Contents.Ascii) < 0) ? 1 : 0;
      break;
    default:
      break;
  }
}

static void LeOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  switch (pLVal->Typ)
  {
    case TempInt:
      pErg->Contents.Int = (pLVal->Contents.Int <= pRVal->Contents.Int) ? 1 : 0;
      break;
    case TempFloat:
      pErg->Contents.Int = (pLVal->Contents.Float <= pRVal->Contents.Float) ? 1 : 0;
      break;
    case TempString:
      pErg->Contents.Int = (DynStringCmp(&pLVal->Contents.Ascii, &pRVal->Contents.Ascii) <= 0) ? 1 : 0;
      break;
    default:
      break;
  }
}

static void GeOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  switch (pLVal->Typ)
  {
    case TempInt:
      pErg->Contents.Int = (pLVal->Contents.Int >= pRVal->Contents.Int) ? 1 : 0;
      break;
    case TempFloat:
      pErg->Contents.Int = (pLVal->Contents.Float >= pRVal->Contents.Float) ? 1 : 0;
      break;
    case TempString:
      pErg->Contents.Int = (DynStringCmp(&pLVal->Contents.Ascii, &pRVal->Contents.Ascii) >= 0) ? 1 : 0;
      break;
    default:
      break;
  }
}

static void UneqOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  switch (pLVal->Typ)
  {
    case TempInt:
      pErg->Contents.Int = (pLVal->Contents.Int != pRVal->Contents.Int) ? 1 : 0;
      break;
    case TempFloat:
      pErg->Contents.Int = (pLVal->Contents.Float != pRVal->Contents.Float) ? 1 : 0;
      break;
    case TempString:
      pErg->Contents.Int = (DynStringCmp(&pLVal->Contents.Ascii, &pRVal->Contents.Ascii) != 0) ? 1 : 0;
      break;
    default:
      break;
  }
}

const Operator Operators[] =
{
  {" " , 1 , False,  0, False, False, False, False, DummyOp},
  {"~" , 1 , False,  1, True , False, False, False, OneComplOp},
  {"<<", 2 , True ,  3, True , False, False, False, ShLeftOp},
  {">>", 2 , True ,  3, True , False, False, False, ShRightOp},
  {"><", 2 , True ,  4, True , False, False, False, BitMirrorOp},
  {"&" , 1 , True ,  5, True , False, False, False, BinAndOp},
  {"|" , 1 , True ,  6, True , False, False, False, BinOrOp},
  {"!" , 1 , True ,  7, True , False, False, False, BinXorOp},
  {"^" , 1 , True ,  8, True , True , False, False, PotOp},
  {"*" , 1 , True , 11, True , True , False, False, MultOp},
  {"/" , 1 , True , 11, True , True , False, False, DivOp},
  {"#" , 1 , True , 11, True , False, False, False, ModOp},
  {"+" , 1 , True , 13, True , True , True , False, AddOp},
  {"-" , 1 , True , 13, True , True , False, False, SubOp},
  {"~~", 2 , False,  2, True , False, False, False, LogNotOp},
  {"&&", 2 , True , 15, True , False, False, False, LogAndOp},
  {"||", 2 , True , 16, True , False, False, False, LogOrOp},
  {"!!", 2 , True , 17, True , False, False, False, LogXorOp},
  {"=" , 1 , True , 23, True , True , True , False, EqOp},
  {"==", 2 , True , 23, True , True , True , False, EqOp},
  {">" , 1 , True , 23, True , True , True , False, GtOp},
  {"<" , 1 , True , 23, True , True , True , False, LtOp},
  {"<=", 2 , True , 23, True , True , True , False, LeOp},
  {">=", 2 , True , 23, True , True , True , False, GeOp},
  {"<>", 2 , True , 23, True , True , True , False, UneqOp},
  /* termination marker */
  {NULL, 0 , False,  0, False, False, False, False, NULL}
},
/* minus may have one or two operands */
MinusMonadicOperator =
{
  "-" ,1 , False, 13, True , True , False, False, SubOp
};
