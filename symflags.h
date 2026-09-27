#ifndef SYMFLAGS_H
#define SYMFLAGS_H

/* symflags.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* Symbol Flags Used in Symbol Table and TempResult                          */
/*                                                                           */
/*****************************************************************************/

typedef enum eSymbolFlags {
    eSymbolFlag_None               = 0,
    eSymbolFlag_NextLabelAfterBSR  = 1 << 0,
    eSymbolFlag_StringSingleQuoted = 1 << 1,

    /* Hint flag: symbol possibly unknown in first pass,
       expression was not evaluated: */

    eSymbolFlag_FirstPassUnknown = 1 << 2,

    /* Hint flag:  symbol value may no longer be correct due
       to phase errors: */

    eSymbolFlag_Questionable = 1 << 3,

    /* Hint flag: uses forward references */

    eSymbolFlag_UsesForwards = 1 << 4,

    eSymbolFlag_Label = 1 << 5,

    eSymbolFlag_UserShort  = 1 << 6,
    eSymbolFlag_UserMedium = 1 << 7,
    eSymbolFlag_UserLong   = 1 << 8,

    /* Integer value is to be treated as negative value.  Should only
       be set if MSB is 1: */

    eSymbolFlag_Negative = 1 << 9,

    eSymbolFlags_Promotable = eSymbolFlag_FirstPassUnknown | eSymbolFlag_Questionable
                              | eSymbolFlag_UsesForwards | eSymbolFlag_UserShort
                              | eSymbolFlag_UserMedium | eSymbolFlag_UserLong
} tSymbolFlags;

#ifdef __cplusplus
#    include "cppops.h"
DefCPPOps_Mask(tSymbolFlags)
#endif

#define mFirstPassUnknown(Flags) (!!((Flags) & eSymbolFlag_FirstPassUnknown))

#define mSymbolQuestionable(Flags) (!!((Flags) & eSymbolFlag_Questionable))

#define mFirstPassUnknownOrQuestionable(Flags) \
    (!!((Flags) & (eSymbolFlag_FirstPassUnknown | eSymbolFlag_Questionable)))

#define mUsesForwards(Flags) (!!((Flags) & eSymbolFlag_UsesForwards))

#endif /* SYMFLAGS_H */
