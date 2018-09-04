/* tempresult.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* internal holder for int/float/string                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "strutil.h"
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
 * \fn     TempResultToPlainString(char *pDest, const TempResult *pResult, unsigned DestSize)
 * \brief  convert result to readable form
 * \param  pDest where to write ASCII representation
 * \param  pResult result to convert
 * \param  DestSize size of dest buffer
 * \return 0 or error code
 * ------------------------------------------------------------------------ */

int TempResultToPlainString(char *pDest, const TempResult *pResult, unsigned DestSize)
{
  switch (pResult->Typ)
  {
    case TempInt:
    {
      String s;

      LargeString(s, pResult->Contents.Int);
      sprintf(pDest, "%s", s);
      break;
    }
    case TempFloat:
      sprintf(pDest, "%0.16e", pResult->Contents.Float);
      KillBlanks(pDest);
      break;
    case TempString:
      *pDest = '"';
      snstrlenprint(pDest + 1, DestSize - 1, pResult->Contents.Ascii.Contents, pResult->Contents.Ascii.Length);
      strmaxcat(pDest, "\"", DestSize);
      break;
    default:
      return -1;
  }
  return 0;
}
