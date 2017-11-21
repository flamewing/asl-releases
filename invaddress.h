#ifndef _INVADDRESS_H
#define _INVADDRESS_H
/* invaddress.h */
/*****************************************************************************/
/* Disassembler                                                              */
/*                                                                           */
/* inverse symbol storage                                                    */
/*                                                                           */
/*****************************************************************************/

extern void AddInvSymbol(const char *pSymbolName, LargeWord Value);

extern const char *LookupInvSymbol(LargeWord Value);

extern int GetMaxInvSymbolNameLen(void);

#endif /* _INVADDRESS_H */
