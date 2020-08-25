/* natpseudo.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Port                                                                   */
/*                                                                           */
/* Pseudo Instructions used for National Semiconductor CPUs                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
 * Includes
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include "bpemu.h"

#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "errmsg.h"

#include "natpseudo.h"

/*****************************************************************************
 * Global Functions
 *****************************************************************************/

Boolean DecodeNatPseudo(Boolean *pBigFlag)
{
  Boolean ValOK;
  tSymbolFlags Flags;
  Word Size, Value;

  *pBigFlag = FALSE;

  if (Memo("SFR"))
  {
    CodeEquate(SegData, 0, 0xff);
    return True;
  }

  if (Memo("ADDR"))
  {
    strcpy(OpPart.Str, "DB");
    *pBigFlag = True;
  }

  if (Memo("ADDRW"))
  {
    strcpy(OpPart.Str, "DW");
    *pBigFlag = True;
  }

  if (Memo("BYTE"))
    strcpy(OpPart.Str, "DB");

  if (Memo("WORD"))
    strcpy(OpPart.Str, "DW");

  if ((Memo("DSB")) || (Memo("DSW")))
  {
    if (ChkArgCnt(1, 1))
    {
      Size = EvalStrIntExpressionWithFlags(&ArgStr[1], UInt16, &ValOK, &Flags);
      if (mFirstPassUnknown(Flags)) WrError(ErrNum_FirstPassCalc);
      else if (ValOK)
      {
        DontPrint = True;
        if (Memo("DSW"))
          Size += Size;
        if (!Size) WrError(ErrNum_NullResMem);
        CodeLen = Size;
        BookKeeping();
      }
    }
    return True;
  }

  if (Memo("FB"))
  {
    if (ChkArgCnt(2, 2))
    {
      Size = EvalStrIntExpressionWithFlags(&ArgStr[1], UInt16, &ValOK, &Flags);
      if (mFirstPassUnknown(Flags)) WrError(ErrNum_FirstPassCalc);
      else if (ValOK)
      {
        if (SetMaxCodeLen(Size)) WrError(ErrNum_CodeOverflow);
        else
        {
          BAsmCode[0] = EvalStrIntExpression(&ArgStr[2], Int8, &ValOK);
          if (ValOK)
          {
            CodeLen = Size;
            memset(BAsmCode + 1, BAsmCode[0], Size - 1);
          }
        }
      }
    }
    return True;
  }

  if (Memo("FW"))
  {
    if (ChkArgCnt(2, 2))
    {
      Size = EvalStrIntExpressionWithFlags(&ArgStr[1], UInt16, &ValOK, &Flags);
      if (mFirstPassUnknown(Flags)) WrError(ErrNum_FirstPassCalc);
      else if (ValOK)
      {
        if (SetMaxCodeLen(Size << 1)) WrError(ErrNum_CodeOverflow);
        else
        {
          Value = EvalStrIntExpression(&ArgStr[2], Int16, &ValOK);
          if (ValOK)
          {
            Word z;

            for (z = 0; z < Size; z++)
            {
              BAsmCode[CodeLen++] = Lo(Value);
              BAsmCode[CodeLen++] = Hi(Value);
            }
          }
        }
      }
    }
    return True;
  }

  return False;
}

