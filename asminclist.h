#ifndef _ASMINCLIST_H
#define _ASMINCLIST_H
/* asminclist.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung der Include-Verschachtelungsliste                              */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

extern void PushInclude(char *S);

extern void PopInclude(void);

extern void PrintIncludeList(void);

extern void ClearIncludeList(void);

extern void asminclist_init(void);
#endif /* _ASMINCLIST_H */
