#ifndef _AS_H
#define _AS_H
/* as.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Hauptmodul                                                                */
/*                                                                           */
/*****************************************************************************/

struct sStrComp;

extern char *GetErrorPos(void);

extern void HandleLabel(const struct sStrComp *pName, LargeWord Value);

#endif /* _AS_H */
