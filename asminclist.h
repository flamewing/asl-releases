/* asminclist.c */
/*****************************************************************************/
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
