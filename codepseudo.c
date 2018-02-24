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
/* $Id: codepseudo.c,v 1.10 2014/03/08 21:06:36 alfred Exp $                  */
/***************************************************************************** 
 * $Log: codepseudo.c,v $
 * Revision 1.10  2014/03/08 21:06:36  alfred
 * - rework ASSUME framework
 *
 * Revision 1.9  2011-08-01 19:58:03  alfred
 * - parse noting value case-insensitive
 *
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
  if (ChkArgCnt(1,  1))
  {
    Erg = EvalIntExpression(ArgStr[1], Int32, &OK);
    if ((OK) && (!FirstPassUnknown))
    {
      if (Min > Erg) WrError(1315);
      else if (Erg > Max) WrError(1320);
      else
      {
        PushLocHandle(-1);
        EnterIntSymbol(LabPart.Str, Erg, DestSeg, False);
        PopLocHandle();
        if (MakeUseList)
         if (AddChunk(SegChunks + DestSeg, Erg, 1, False)) WrError(90);
        t.Typ = TempInt; t.Contents.Int = Erg; SetListLineVal(&t);
      }
    }
  }
}

