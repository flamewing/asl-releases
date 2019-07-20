#ifndef _ASMERR_H
#define _ASMERR_H
/* asmerr.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Error Handling Functions                                                  */
/*                                                                           */
/*****************************************************************************/
/* $Id: asmerr.h,v 1.2 2016/08/30 09:53:46 alfred Exp $                      */
/*****************************************************************************
 * $Log: asmerr.h,v $
 * Revision 1.2  2016/08/30 09:53:46  alfred
 * - make string argument const
 *
 * Revision 1.1  2008/01/02 22:32:21  alfred
 * - better heap checking for DOS target
 *
 *****************************************************************************/

#include "datatypes.h"

struct sLineComp;
struct sStrComp;
extern void WrErrorString(char *Message, char *Add, Boolean Warning, Boolean Fatal,
                          const char *pExtendError, const struct sLineComp *pLineComp);

extern void WrError(Word Num);

extern void WrXError(Word Num, const char *pExtError);

extern void WrXErrorPos(Word Num, const char *pExtError, const struct sLineComp *pLineComp);

extern void WrStrErrorPos(Word Num, const struct sStrComp *pStrComp);

#endif /* _ASMERR_H */

