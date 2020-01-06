#ifndef _ASMALLG_H
#define _ASMALLG_H
/* codeallg.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* von allen Codegeneratoren benutzte Pseudobefehle                          */
/*                                                                           */
/* Historie:  10. 5.1996 Grundsteinlegung                                    */
/*                                                                           */
/*****************************************************************************/

extern void SetCPU(CPUVar NewCPU, Boolean NotPrev);

extern Boolean SetNCPU(const char *pName, Boolean NotPrev);

extern void AddONOFF(const char *InstName, Boolean *Flag, const char *FlagName, Boolean Persist);

extern void ClearONOFF(void);

extern Boolean CodeGlobalPseudo(void);

struct sStrComp;

extern void INCLUDE_SearchCore(struct sStrComp *pDest, const struct sStrComp *pArg, Boolean SearchPath);

extern void codeallg_init(void);
#endif /* _ASMALLG_H */
