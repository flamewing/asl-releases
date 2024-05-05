#ifndef INVADDRESS_H
#define INVADDRESS_H
/* invaddress.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* Disassembler                                                              */
/*                                                                           */
/* inverse symbol storage                                                    */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

extern void AddInvSymbol(char const* pSymbolName, LargeWord Value);

extern char const* LookupInvSymbol(LargeWord Value);

extern int GetMaxInvSymbolNameLen(void);

#endif /* INVADDRESS_H */
