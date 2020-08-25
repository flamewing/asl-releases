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
#include "endian.h"
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

/*****************************************************************************
 * Function:    CodeEquate
 * Purpose:     EQU for different segment
 * Result:      -
 *****************************************************************************/
   
void CodeEquate(ShortInt DestSeg, LargeInt Min, LargeInt Max)
{
  Boolean OK;
  tSymbolFlags Flags;
  TempResult t;
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
        PushLocHandle(-1);
        EnterIntSymbol(&LabPart, Erg, DestSeg, False);
        PopLocHandle();
        if (MakeUseList)
          if (AddChunk(SegChunks + DestSeg, Erg, 1, False)) WrError(ErrNum_Overlap);
        t.Typ = TempInt; t.Contents.Int = Erg; SetListLineVal(&t);
      }
    }
  }
}

