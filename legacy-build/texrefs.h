#ifndef TEXREFS_H
#define TEXREFS_H
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
extern void AddLabel(char const* pName, char const* pValue);
extern void GetLabel(char const* pName, char* pDest);
extern void PrintLabels(char const* pFileName);
extern void FreeLabels(void);

extern void InitCites(void);
extern void AddCite(char const* pName, char const* pValue);
extern void GetCite(char* pName, char* pDest);
extern void PrintCites(char const* pFileName);
extern void FreeCites(void);

#endif /* TEXREFS_H */
