#ifndef _TEXREFS_H
#define _TEXREFS_H
/* texrefs.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* TeX Convertes: Label/Citation Bookkeeping                                 */
/*                                                                           */
/*****************************************************************************/

extern void InitLabels(void);
extern void AddLabel(const char *pName, const char *pValue);
extern void GetLabel(const char *pName, char *pDest);
extern void PrintLabels(const char *pFileName);
extern void FreeLabels(void);

extern void InitCites(void);
extern void AddCite(const char *pName, const char *pValue);
extern void GetCite(char *pName, char *pDest);
extern void PrintCites(const char *pFileName);
extern void FreeCites(void);

#endif /* _TEXREFS_H */
