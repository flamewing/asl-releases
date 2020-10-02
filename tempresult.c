/* tempresult.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* internal holder for int/float/string                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "strutil.h"
#include "asmdef.h"
#include "tempresult.h"

/*!------------------------------------------------------------------------
 * \fn     TempResultToFloat(TempResult *pResult)
 * \brief  convert TempResult to float
 * \param  pResult tempresult to convert
 * \return 0 or error code
 * ------------------------------------------------------------------------ */

int TempResultToFloat(TempResult *pResult)
{
  switch (pResult->Typ)
  {
    case TempInt:
      pResult->Contents.Float = pResult->Contents.Int;
      pResult->Typ = TempFloat;
      break;
    case TempFloat:
      break;
    default:
      pResult->Typ = TempNone;
      return -1;
  }
  return 0;
}

/*!------------------------------------------------------------------------
 * \fn     TempResultToPlainString(char *pDest, const TempResult *pResult, size_t DestSize)
 * \brief  convert result to readable form
 * \param  pDest where to write ASCII representation
 * \param  pResult result to convert
 * \param  DestSize size of dest buffer
 * \return 0 or error code
 * ------------------------------------------------------------------------ */

int TempResultToPlainString(char *pDest, const TempResult *pResult, size_t DestSize)
{
  switch (pResult->Typ)
  {
    case TempInt:
      as_snprintf(pDest, DestSize, "%llld", pResult->Contents.Int);
      break;
    case TempFloat:
      as_snprintf(pDest, DestSize, "%0.16e", pResult->Contents.Float);
      KillBlanks(pDest);
      break;
    case TempString:
    {
      char TmpStr[2];

      TmpStr[0] = *pDest = (pResult->Flags & eSymbolFlag_StringSingleQuoted) ? '\'' : '"';
      TmpStr[1] = '\0';
      snstrlenprint(pDest + 1, DestSize - 1,
                    pResult->Contents.Ascii.Contents, pResult->Contents.Ascii.Length,
                    TmpStr[0]);
      strmaxcat(pDest, TmpStr, DestSize);
      break;
    }
    default:
      return -1;
  }
  return 0;
}
