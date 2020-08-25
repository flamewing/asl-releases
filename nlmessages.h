#ifndef _NLMESSAGES_H
#define _NLMESSAGES_H
/* nlmessages.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Einlesen und Verwalten von Meldungs-Strings                               */
/*                                                                           */
/* Historie: 13. 8.1997 Grundsteinlegung                                     */
/*           17. 8.1997 Verallgemeinerung auf mehrere Kataloge               */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

typedef struct
{
  char *MsgBlock;
  LongInt *StrPosis, MsgCount;
} TMsgCat, *PMsgCat;

extern char *catgetmessage(PMsgCat Catalog, int Num);

extern void opencatalog(PMsgCat Catalog, const char *File, const char *Path, LongInt File_MsgId1, LongInt File_MsgId2);

extern char *getmessage(int Num);

extern void nlmessages_init(const char *File, char *Path, LongInt File_MsgId1, LongInt File_MsgId2);
#endif /* _NLMESSAGES_H */
