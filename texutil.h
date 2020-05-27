#ifndef _TEXUTIL_H
#define _TEXUTIL_H
/* texutil.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* TeX-->ASCII/HTML Converter: Common Utils/Variables                        */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

/*---------------------------------------------------------------------------*/

extern Boolean DoRepass;
extern char *pInFileName;
extern int CurrLine, CurrColumn;

/*---------------------------------------------------------------------------*/

extern void Warning(const char *pMsg);

#endif /* _TEXUTIL_H */
