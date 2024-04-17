/* codepseudo.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Haeufiger benutzte Pseudo-Befehle                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
 * Includes
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "nls.h"
#include "bpemu.h"
#include "as_endian.h"
#include "strutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "asmitree.h"
#include "errmsg.h"

#include "codepseudo.h"
#include "motpseudo.h"

/*****************************************************************************
 * Global Functions
 *****************************************************************************/

/*****************************************************************************
 * Function:    IsIndirect
 * Purpose:     check whether argument is syntactically 'indirect', i.e.
 *              enclosed in 'extra' parentheses.
 * Result:      TRUE if indirect
 *****************************************************************************/

Boolean IsIndirectGen(const char *Asc, const char *pBeginEnd)
{
  int z,Level,l;

  if (((l = strlen(Asc)) <= 2)
   || (Asc[0] != pBeginEnd[0])
   || (Asc[l - 1] != pBeginEnd[1]))
    return False;

  Level = 0;
  for (z = 1; z <= l - 2; z++)
  {
    if (Asc[z] == pBeginEnd[0]) Level++;
    if (Asc[z] == pBeginEnd[1]) Level--;
    if (Level < 0) return False;
  }

  return True;
}

Boolean IsIndirect(const char *Asc)
{
  return IsIndirectGen(Asc, "()");
}

/*!------------------------------------------------------------------------
 * \fn     FindDispBaseSplitWithQualifier(const char *pArg, int *pArgLen, tDispBaseSplitQualifier Qualifier)
 * \brief  check for argument of type xxx(yyyy)
 * \param  pArg argument to check
 * \param  pArgLen returns argument length
 * \param  Qualifier possible qualifier to allow more positive decisions
 * \return index to opening parenthese or -1 if not like pattern
 * ------------------------------------------------------------------------ */

int FindDispBaseSplitWithQualifier(const char *pArg, int *pArgLen, tDispBaseSplitQualifier Qualifier)
{
  int Nest = 0, Start, SplitPos = -1;
  Boolean InSgl = False, InDbl = False;

  *pArgLen = strlen(pArg);

  if (!*pArgLen || (pArg[*pArgLen - 1] != ')'))
    return -1;

  /* We are looking for expressions of the form xxx(yyy),
     but we want to avoid false positives on things like
     xxx+(yyy*zzz).  So we look at the (non-blank) character
     right before opening parenthese in question.  If it is
     something that might be the last letter of an identifier,
     or another parenthized expression, and not an operator,
     it might be OK... */

  for (Start = *pArgLen - 1; Start >= 0; Start--)
  {
    switch (pArg[Start])
    {
      case '\'':
        if (!InDbl) InSgl = !InSgl;
        break;
      case '"':
        if (!InSgl) InDbl = !InDbl;
        break;
      case ')':
        if (!InSgl && !InDbl) Nest++;
        break;
      case '(':
        if (!InSgl && !InDbl) Nest--;
        break;
      default:
        break;
    }
    if (!Nest && (SplitPos < 0))
      SplitPos = Start;
    else if (SplitPos >= 0)
    {
      if (as_isspace(pArg[Start])); /* delay decision to to next non-blank */
      else if (as_isalnum(pArg[Start]) || (pArg[Start] == ')') || (pArg[Start] == '\'') || (pArg[Start] == '"'))
        return SplitPos;
      else
        return Qualifier ? Qualifier(pArg, Start, SplitPos) : -1;
    }
  }

  /* if SplitPos >= 0, and we end up here, xxx is empty string or only consists of spaces: */

  return SplitPos;
}

/*****************************************************************************
 * Function:    CodeEquate
 * Purpose:     EQU for different segment
 * Result:      -
 *****************************************************************************/

void CodeEquate(as_addrspace_t DestSeg, LargeInt Min, LargeInt Max)
{
  Boolean OK;
  tSymbolFlags Flags;
  LargeInt Erg;

  if (ChkArgCnt(1, 1))
  {
    Erg = EvalStrIntExpressionWithFlags(&ArgStr[1], Int32, &OK, &Flags);
    if (OK && !mFirstPassUnknown(Flags))
    {
      if (Min > Erg) WrError(ErrNum_UnderRange);
      else if (Erg > Max) WrError(ErrNum_OverRange);
      else
      {
        TempResult t;

        PushLocHandle(-1);
        EnterIntSymbol(&LabPart, Erg, DestSeg, False);
        PopLocHandle();
        if (MakeUseList)
          if (AddChunk(SegChunks + DestSeg, Erg, 1, False)) WrError(ErrNum_Overlap);

        as_tempres_ini(&t);
        as_tempres_set_int(&t, Erg);
        SetListLineVal(&t);
        as_tempres_free(&t);
      }
    }
  }
}

/*!------------------------------------------------------------------------
 * \fn     QualifyQuote_SingleQuoteConstant(const char *pStart, const char *pQuotePos)
 * \brief  check whether ' in source is lead-in to character string or int constant
 * \param  pStart complete string
 * \param  pQuotePos single quote position
 * \return True if this is NO lead-in of int constant
 * ------------------------------------------------------------------------ */

Boolean QualifyQuote_SingleQuoteConstant(const char *pStart, const char *pQuotePos)
{
  const char *pRun;
  Boolean OK;
  int Base;

  /* previous character must be H X B O */

  if (pQuotePos == pStart)
    return True;
  switch (as_toupper(*(pQuotePos - 1)))
  {
    case 'B':
      Base = 2; break;
    case 'O':
      Base = 8; break;
    case 'X':
    case 'H':
      Base = 16; break;
    default:
      return True;
  }

  /* Scan for following valid (binary/octal/hex) character(s) */

  for (pRun = pQuotePos + 1; *pRun; pRun++)
  {
    switch (Base)
    {
      case 16: OK = as_isxdigit(*pRun); break;
      case 8: OK = as_isdigit(*pRun) && (*pRun < '8'); break;
      case 2: OK = as_isdigit(*pRun) && (*pRun < '2'); break;
      default: OK = False;
    }
    if (!OK)
      break;
  }

  /* none? -> bad */

  if (pRun <= pQuotePos + 1)
    return True;

  /* If we've hit another ' after them, it is the "harmless" x'...' form,
     and no special treatment is needed */

  if ('\'' == *pRun)
    return True;

  /* Other token or string continues -> cannot be such a constant, otherwise we
     have a match and the ' does NOT lead in a character string: */

  return as_isalnum(*pRun);
}

