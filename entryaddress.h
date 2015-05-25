#ifndef _ENTRYADDRESS_H
#define _ENTRYADDRESS_H

/* entryaddress.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Maintain list of possible entry addresses                                 */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

void entryaddress_init();

extern Boolean EntryAddressAvail(void);
extern void AddEntryAddress(LargeWord Address);
extern LargeWord GetEntryAddress(void);

#endif /* _ENTRYADDRESS_H */
