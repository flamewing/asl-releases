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

extern void AddONOFF(char *InstName, Boolean *Flag, char *FlagName, Boolean Persist);

extern void ClearONOFF(void);

extern Boolean CodeGlobalPseudo(void);

extern void codeallg_init(void);
