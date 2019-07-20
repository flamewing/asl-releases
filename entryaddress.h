#ifndef _ENTRYADDRESS_H
#define _ENTRYADDRESS_H
/* entryaddress.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Maintain list of possible entry addresses                                 */
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>

#include "datatypes.h"

void entryaddress_init();

extern Boolean EntryAddressAvail(void);
extern void AddEntryAddress(LargeWord Address);
extern void PrintEntryAddress(FILE *pDestFile);
extern LargeWord GetEntryAddress(Boolean UsePreferredAddress, LargeWord PreferredAddress);

#endif /* _ENTRYADDRESS_H */
