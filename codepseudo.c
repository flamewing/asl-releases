/* codepseudo.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Haeufiger benutzte Pseudo-Befehle                                         */
/*                                                                           */
/* Historie: 23. 5.1996 Grundsteinlegung                                     */
/*            7. 7.1998 Fix Zugriffe auf CharTransTable wg. signed chars     */
/*           18. 8.1998 BookKeeping-Aufrufe bei SPeicherreservierungen       */
/*            8. 3.2000 'ambigious else'-Warnungen beseitigt                 */
/*           14. 1.2001 silenced warnings about unused parameters            */
/*           2001-11-11 added DecodeTIPseudo                                 */
/*           2002-01-13 Borland Pascal 3.1 doesn't like empty default clause */
/*                                                                           */
/*****************************************************************************/
/* $Id: codepseudo.c,v 1.8 2004/05/29 12:28:13 alfred Exp $                  */
/***************************************************************************** 
 * $Log: codepseudo.c,v $
 * Revision 1.8  2004/05/29 12:28:13  alfred
 * - remove unneccessary dummy fcn
 *
 * Revision 1.7  2004/05/29 12:18:06  alfred
 * - relocated DecodeTIPseudo() to separate module
 *
 * Revision 1.6  2004/05/29 12:04:48  alfred
 * - relocated DecodeMot(16)Pseudo into separate module
 *
 * Revision 1.5  2004/05/29 11:33:03  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 * Revision 1.4  2004/05/29 10:56:11  alfred
 * - cleanup, first step
 *
 * Revision 1.3  2004/05/29 09:35:26  alfred
 * - removed unneeded function
 *
 * Revision 1.2  2004/05/28 16:12:43  alfred
 * - fixed search for DUP, added some const definitions
 *
 * Revision 1.1  2003/11/06 02:49:23  alfred
 * - recreated
 *
 * Revision 1.6  2003/10/04 14:00:39  alfred
 * - complain about empty arguments
 *
 * Revision 1.5  2003/05/02 21:23:12  alfred
 * - strlen() updates
 *
 * Revision 1.4  2002/10/26 09:57:47  alfred
 * - allow DC with ? as argument
 *
 * Revision 1.3  2002/08/14 18:43:49  alfred
 * - warn null allocation, remove some warnings
 *
 *
 *****************************************************************************/

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

#include "codepseudo.h"
#include "motpseudo.h"

#include "code68.h"

/*****************************************************************************
 * Global Functions
 *****************************************************************************/

/*****************************************************************************
 * Function:    IsIndirect
 * Purpose:     check whether argument is syntactically 'indirect', i.e.
 *              enclosed in 'extra' parentheses.
 * Result:      TRUE if indirect
 *****************************************************************************/
   
Boolean IsIndirect(char *Asc)
{
  int z,Level,l;

  if (((l = strlen(Asc)) <= 2)
   || (Asc[0] != '(')
   || (Asc[l - 1] != ')'))
    return False;

  Level = 0;
  for (z = 1; z <= l - 2; z++)
  {
    if (Asc[z] == '(') Level++;
    if (Asc[z] == ')') Level--;
    if (Level < 0) return False;
  }

  return True;
}

/*****************************************************************************
 * Function:    CodeEquate
 * Purpose:     EQU for different segment
 * Result:      -
 *****************************************************************************/
   
void CodeEquate(ShortInt DestSeg, LargeInt Min, LargeInt Max)
{
  Boolean OK;
  TempResult t;
  LargeInt Erg;

  FirstPassUnknown = False;
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Erg = EvalIntExpression(ArgStr[1], Int32, &OK);
    if ((OK) && (!FirstPassUnknown))
    {
      if (Min > Erg) WrError(1315);
      else if (Erg > Max) WrError(1320);
      else
      {
        PushLocHandle(-1);
        EnterIntSymbol(LabPart, Erg, DestSeg, False);
        PopLocHandle();
        if (MakeUseList)
         if (AddChunk(SegChunks + DestSeg, Erg, 1, False)) WrError(90);
        t.Typ = TempInt; t.Contents.Int = Erg; SetListLineVal(&t);
      }
    }
  }
}

/*****************************************************************************
 * Function:    CodeASSUME
 * Purpose:     handle ASSUME statement
 * Result:      -
 *****************************************************************************/
   
void CodeASSUME(ASSUMERec *Def, Integer Cnt)
{
  int z1, z2;
  Boolean OK;
  LongInt HVal;
  String RegPart, ValPart;

  if (ArgCnt == 0) WrError(1110);
  else
  {
    z1 = 1; OK = True;
    while ((z1 <= ArgCnt) && (OK))
    {
      SplitString(ArgStr[z1], RegPart, ValPart, QuotPos(ArgStr[z1], ':'));
      z2 = 0; NLS_UpString(RegPart);
      while ((z2 < Cnt) && (strcmp(Def[z2].Name,RegPart)))
        z2++;
      OK = (z2 < Cnt);
      if (!OK) WrXError(1980, RegPart);
      else
      {
        if (strcmp(ValPart, "NOTHING") == 0)
        {
          if (Def[z2].NothingVal == -1) WrError(1350);
          else
            *(Def[z2].Dest) = Def[z2].NothingVal;
        }
        else
        {
          FirstPassUnknown = False;
          HVal = EvalIntExpression(ValPart, Int32, &OK);
          if (OK)
          {
            if (FirstPassUnknown)
            {
              WrError(1820); OK = False;
            }
            else if (ChkRange(HVal,Def[z2].Min,Def[z2].Max))
              *(Def[z2].Dest) = HVal;
          }
        }
      }
      z1++;
    }
  }
}
