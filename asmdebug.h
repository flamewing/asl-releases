/* asmdebug.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung der Debug-Informationen zur Assemblierzeit                     */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

extern void AddLineInfo(Boolean InAsm, LongInt LineNum, char *FileName, 
                        ShortInt Space, LongInt Address);

extern void InitLineInfo(void);

extern void ClearLineInfo(void);

extern void DumpDebugInfo(void);

extern void asmdebug_init(void);
