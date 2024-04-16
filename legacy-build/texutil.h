#ifndef TEXUTIL_H
#define TEXUTIL_H
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

#endif /* TEXUTIL_H */
