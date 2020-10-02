/* console.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS Port                                                                   */
/*                                                                           */
/* Console Output Handling                                                   */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "console.h"

/*!------------------------------------------------------------------------
 * \fn     WrConsoleLine(const char *pLine, Boolean NewLine)
 * \brief  print a line to console, possibly erasing previous output
 * \param  pLine line to print
 * \param  NewLine start new line or jump back to current line?
 * ------------------------------------------------------------------------ */

void WrConsoleLine(const char *pLine, Boolean NewLine)
{
  static size_t LastLength;
  size_t ThisLength = strlen(pLine);

  printf("%s", pLine);
  if (LastLength && (LastLength > ThisLength))
    printf("%*s", (int)(LastLength - ThisLength), "");
  if (NewLine)
  {
    printf("\n");
    LastLength = 0;
  }
  else
  {
    printf("\r");
    LastLength = ThisLength;
  }
}

