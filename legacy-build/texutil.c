/* texutil.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* TeX-->ASCII/HTML Converter: Common Utils/Variables                        */
/*                                                                           */
/*****************************************************************************/

#include "texutil.h"

#include <stdio.h>

/*--------------------------------------------------------------------------*/

Boolean DoRepass;
char*   pInFileName;
int     CurrLine, CurrColumn;

/*--------------------------------------------------------------------------*/

/*!------------------------------------------------------------------------
 * \fn     Warning(const char *pMsg)
 * \brief  print warning
 * \param  pMsg warning message
 * ------------------------------------------------------------------------ */

void Warning(char const* pMsg) {
    fprintf(stderr, "%s:%d.%d: %s\n", pInFileName, CurrLine, CurrColumn, pMsg);
}
