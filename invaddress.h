#ifndef _INVADDRESS_H
#define _INVADDRESS_H
/* invaddress.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* Disassembler                                                              */
/*                                                                           */
/* inverse symbol storage                                                    */
/*                                                                           */
/*****************************************************************************/

extern void AddInvSymbol(const char *pSymbolName, LargeWord Value);

extern const char *LookupInvSymbol(LargeWord Value);

extern int GetMaxInvSymbolNameLen(void);

#endif /* _INVADDRESS_H */
