#ifndef CHARDEFS_H
#define CHARDEFS_H

/* chardefs.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* system-dependant definition of national-specific characters               */
/*                                                                           */
/* History:  2001-10-13 /AArnold - created this header                       */
/*                                                                           */
/*****************************************************************************/

typedef enum {
    eCH_ae,
    eCH_ee,
    eCH_ie,
    eCH_oe,
    eCH_ue,
    eCH_Ae,
    eCH_Ee,
    eCH_Ie,
    eCH_Oe,
    eCH_Ue,
    eCH_sz,
    eCH_e2,
    eCH_mu,
    eCH_agrave,
    eCH_Agrave,
    eCH_egrave,
    eCH_Egrave,
    eCH_igrave,
    eCH_Igrave,
    eCH_ograve,
    eCH_Ograve,
    eCH_ugrave,
    eCH_Ugrave,
    eCH_aacute,
    eCH_Aacute,
    eCH_eacute,
    eCH_Eacute,
    eCH_iacute,
    eCH_Iacute,
    eCH_oacute,
    eCH_Oacute,
    eCH_uacute,
    eCH_Uacute,
    eCH_acirc,
    eCH_Acirc,
    eCH_ecirc,
    eCH_Ecirc,
    eCH_icirc,
    eCH_Icirc,
    eCH_ocirc,
    eCH_Ocirc,
    eCH_ucirc,
    eCH_Ucirc,
    eCH_ccedil,
    eCH_Ccedil,
    eCH_ntilde,
    eCH_Ntilde,
    eCH_aring,
    eCH_Aring,
    eCH_aelig,
    eCH_Aelig,
    eCH_oslash,
    eCH_Oslash,
    eCH_iquest,
    eCH_iexcl,
    eCH_cnt
} tNLSCharacter;

#ifdef __cplusplus
#    include "cppops.h"
DefCPPOps_Enum(tNLSCharacter)
#endif

        typedef char tNLSCharacterTab[eCH_cnt][2];

typedef enum {
    eCodepageASCII,
    eCodepageISO8859_1,
    eCodepageISO8859_15,
    eCodepageKOI8_R,
    eCodepage437,
    eCodepage850,
    eCodepage866,
    eCodepage1251,
    eCodepage1252,
    eCodepageUTF8,
    eCodepageCnt
} tCodepage;

#ifdef __cplusplus
#    include "cppops.h"
DefCPPOps_Enum(tCodepage)
#endif

#include "datatypes.h"

        extern tNLSCharacterTab const* GetCharacterTab(tCodepage Codepage);

extern char const NLS_HtmlCharacterTab[eCH_cnt][9];

extern int CharTab_GetLength(tNLSCharacterTab const* pTab, tNLSCharacter Character);

extern char const* CharTab_GetNULTermString(
        tNLSCharacterTab const* pTab, tNLSCharacter Character, char* pBuffer);

extern LongWord UTF8ToUnicode(char const** ppChr);

extern void UnicodeToUTF8(char** ppChr, LongWord Unicode);

#endif /* CHARDEFS_H */
