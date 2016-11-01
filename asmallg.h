#ifndef _ASMALLG_H
#define _ASMALLG_H
/* codeallg.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* von allen Codegeneratoren benutzte Pseudobefehle                          */
/*                                                                           */
/* Historie:  10. 5.1996 Grundsteinlegung                                    */
/*                                                                           */
/*****************************************************************************/

extern void SetCPU(CPUVar NewCPU, Boolean NotPrev);

extern Boolean SetNCPU(char *Name, Boolean NotPrev);

extern void AddONOFF(const char *InstName, Boolean *Flag, const char *FlagName, Boolean Persist);

extern void ClearONOFF(void);

extern Boolean CodeGlobalPseudo(void);

extern void codeallg_init(void);
#endif /* _ASMALLG_H */
