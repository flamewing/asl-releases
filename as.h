#ifndef _AS_H
#define _AS_H
/* as.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Hauptmodul                                                                */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

extern char *GetErrorPos(void);

extern void HandleLabel(const char *Name, LargeWord Value);

#endif /* _AS_H */
