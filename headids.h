#ifndef HEADIDS_H
#define HEADIDS_H
/* headids.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* Makroassembler AS                                                         */
/*                                                                           */
/* Hier sind alle Prozessor-IDs mit ihren Eigenschaften gesammelt            */
/*                                                                           */
/* Historie: 29. 8.1998 angelegt                                             */
/*                                                                           */
/*****************************************************************************/

/* Hex-Formate */

#include "datatypes.h"

typedef enum {
    eHexFormatDefault,
    eHexFormatMotoS,
    eHexFormatIntel,
    eHexFormatIntel16,
    eHexFormatIntel32,
    eHexFormatMOS,
    eHexFormatTek,
    eHexFormatTiDSK,
    eHexFormatAtmel,
    eHexFormatMico8,
    eHexFormatC
} tHexFormat;

typedef struct {
    char const* Name;
    Word        Id;
    tHexFormat  HexFormat;
} TFamilyDescr, *PFamilyDescr;

extern PFamilyDescr FindFamilyByName(char const* Name);

extern PFamilyDescr FindFamilyById(Word Id);

extern void headids_init(void);
#endif /* HEADIDS_H */
