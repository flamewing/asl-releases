/* natpseudo.c */
/*****************************************************************************/
/* AS-Port                                                                   */
/*                                                                           */
/* Pseudo Instructions used for National Semiconductor CPUs                  */
/*                                                                           */
/*****************************************************************************/
/* $Id: natpseudo.c,v 1.1 2006/04/09 12:40:11 alfred Exp $                    */
/***************************************************************************** 
 * $Log: natpseudo.c,v $
 * Revision 1.1  2006/04/09 12:40:11  alfred
 * - unify COP pseudo instructions
 *
 *****************************************************************************/

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

#include "natpseudo.h"

/*****************************************************************************
 * Global Functions
 *****************************************************************************/

Boolean DecodeNatPseudo(Boolean *pBigFlag)
{
  Boolean ValOK;
  Word Size, Value;

  *pBigFlag = FALSE;

  if (Memo("SFR"))
  {
    CodeEquate(SegData, 0, 0xff);
    return True;
  }

  if (Memo("ADDR"))
  {
    strcpy(OpPart, "DB");
    *pBigFlag = True;
  }

  if (Memo("ADDRW"))
  {
    strcpy(OpPart, "DW");
    *pBigFlag = True;
  }

  if (Memo("BYTE"))
    strcpy(OpPart, "DB");

  if (Memo("WORD"))
    strcpy(OpPart,"DW");

  if ((Memo("DSB")) || (Memo("DSW")))
  {
    if (ArgCnt != 1) WrError(1110);
    else
    {
      FirstPassUnknown = False;
      Size = EvalIntExpression(ArgStr[1], UInt16, &ValOK);
      if (FirstPassUnknown) WrError(1820);
      else if (ValOK)
      {
        DontPrint = True;
        if (Memo("DSW"))
          Size += Size;
        if (!Size) WrError(290);
        CodeLen = Size;
        BookKeeping();
      }
    }
    return True;
  }

  if (Memo("FB"))
  {
    if (ArgCnt != 2) WrError(1110);
    else
    {
      FirstPassUnknown = False;
      Size = EvalIntExpression(ArgStr[1], UInt16, &ValOK);
      if (FirstPassUnknown) WrError(1820);
      else if (ValOK)
      {
        if (Size > MaxCodeLen) WrError(1920);
        else
        {
          BAsmCode[0] = EvalIntExpression(ArgStr[2], Int8, &ValOK);
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
    if (ArgCnt != 2) WrError(1110);
    else
    {
      FirstPassUnknown = False;
      Size = EvalIntExpression(ArgStr[1], UInt16, &ValOK);
      if (FirstPassUnknown) WrError(1820);
      else if (ValOK)
      {
        if ((Size << 1) > MaxCodeLen) WrError(1920);
        else
        {
          Value = EvalIntExpression(ArgStr[2], Int16, &ValOK);
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

