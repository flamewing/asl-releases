#ifndef _CHARDEFS_H
#define _CHARDEFS_H
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

#ifdef CHARSET_ISO8859_1
#define CH_ae "\344"
#define CH_ee "\353"
#define CH_ie "\357"
#define CH_oe "\366"
#define CH_ue "\374"
#define CH_Ae "\304"
#define CH_Ee "\313"
#define CH_Ie "\317"
#define CH_Oe "\326"
#define CH_Ue "\334"
#define CH_sz "\337"
#define CH_e2 "\262"
#define CH_mu "\265"
#define CH_agrave "\340"
#define CH_Agrave "\300"
#define CH_egrave "\350"
#define CH_Egrave "\310"
#define CH_igrave "\354"
#define CH_Igrave "\314"
#define CH_ograve "\362"
#define CH_Ograve "\322"
#define CH_ugrave "\371"
#define CH_Ugrave "\331"
#define CH_aacute "\341"
#define CH_Aacute "\301"
#define CH_eacute "\351"
#define CH_Eacute "\311"
#define CH_iacute "\355"
#define CH_Iacute "\315"
#define CH_oacute "\363"
#define CH_Oacute "\323"
#define CH_uacute "\372"
#define CH_Uacute "\332"
#define CH_acirc  "\342"
#define CH_Acirc  "\302"
#define CH_ecirc  "\352"
#define CH_Ecirc  "\312"
#define CH_icirc  "\356"
#define CH_Icirc  "\316"
#define CH_ocirc  "\364"
#define CH_Ocirc  "\324"
#define CH_ucirc  "\373"
#define CH_Ucirc  "\333"
#define CH_ccedil "\347"
#define CH_Ccedil "\307"
#define CH_ntilde "\361"
#define CH_Ntilde "\321"
#define CH_aring  "\345"
#define CH_Aring  "\305"
#define CH_aelig  "\346"
#define CH_Aelig  "\306"
#define CH_oslash "\370"
#define CH_Oslash "\330"
#define CH_iquest "\277"
#define CH_iexcl  "\241"
#endif

#ifdef CHARSET_IBM437
#define CH_ae "\204"
#define CH_ee "\211"
#define CH_ie "\213"
#define CH_oe "\224"
#define CH_ue "\201"
#define CH_Ae "\216"
#define CH_Ee "Ee"
#define CH_Ie "Ie"
#define CH_Oe "\231"
#define CH_Ue "\232"
#define CH_sz "\341"
#define CH_e2 "\375"
#define CH_mu "\346"
#define CH_agrave "\205"
#define CH_Agrave "`A"
#define CH_egrave "\212"
#define CH_Egrave "`E"
#define CH_igrave "\215"
#define CH_Igrave "`I"
#define CH_ograve "\225"
#define CH_Ograve "`O"
#define CH_ugrave "\227"
#define CH_Ugrave "\`U"
#define CH_aacute "\240"
#define CH_Aacute "'A"
#define CH_eacute "\202"
#define CH_Eacute "\220"
#define CH_iacute "\241"
#define CH_Iacute "'I"
#define CH_oacute "\242"
#define CH_Oacute "'O"
#define CH_uacute "\243"
#define CH_Uacute "'U"
#define CH_acirc  "\203"
#define CH_Acirc  "^A"
#define CH_ecirc  "\210"
#define CH_Ecirc  "^E"
#define CH_icirc  "\214"
#define CH_Icirc  "^I"
#define CH_ocirc  "\223"
#define CH_Ocirc  "^O"
#define CH_ucirc  "\226"
#define CH_Ucirc  "\^U"
#define CH_ccedil "\207"
#define CH_Ccedil "\200"
#define CH_ntilde "\244"
#define CH_Ntilde "\245"
#define CH_aring  "\206"
#define CH_Aring  "\217"
#define CH_aelig  "\221"
#define CH_Aelig  "\222"
#define CH_oslash "o"
#define CH_Oslash "O"
#define CH_iquest "\250"
#define CH_iexcl  "\255"
#endif

#ifdef CHARSET_IBM850
#define CH_ae "\204"
#define CH_ee "\211"
#define CH_ie "\213"
#define CH_oe "\224"
#define CH_ue "\201"
#define CH_Ae "\216"
#define CH_Ee "\323"
#define CH_Ie "\330"
#define CH_Oe "\231"
#define CH_Ue "\232"
#define CH_sz "\341"
#define CH_e2 "\375"
#define CH_mu "\346"
#define CH_agrave "\205"
#define CH_Agrave "\267"
#define CH_egrave "\212"
#define CH_Egrave "\324"
#define CH_igrave "\215"
#define CH_Igrave "\336"
#define CH_ograve "\225"
#define CH_Ograve "\343"
#define CH_ugrave "\227"
#define CH_Ugrave "\353"
#define CH_aacute "\240"
#define CH_Aacute "\265"
#define CH_eacute "\202"
#define CH_Eacute "\220"
#define CH_iacute "\241"
#define CH_Iacute "\326"
#define CH_oacute "\242"
#define CH_Oacute "\340"
#define CH_uacute "\243"
#define CH_Uacute "\351"
#define CH_acirc  "\203"
#define CH_Acirc  "\266"
#define CH_ecirc  "\210"
#define CH_Ecirc  "\322"
#define CH_icirc  "\214"
#define CH_Icirc  "\327"
#define CH_ocirc  "\223"
#define CH_Ocirc  "\342"
#define CH_ucirc  "\226"
#define CH_Ucirc  "\352"
#define CH_ccedil "\207"
#define CH_Ccedil "\200"
#define CH_ntilde "\244"
#define CH_Ntilde "\245"
#define CH_aring  "\206"
#define CH_Aring  "\217"
#define CH_aelig  "\221"
#define CH_Aelig  "\222"
#define CH_oslash "\233"
#define CH_Oslash "\235"
#define CH_iquest "\250"
#define CH_iexcl  "\255"
#endif

#ifdef CHARSET_UTF8
#define CH_ae "\303\244" /* 0xe4 */
#define CH_ee "\303\253" /* 0xeb */
#define CH_ie "\303\257" /* 0xef */
#define CH_oe "\303\266" /* 0xf6 */
#define CH_ue "\303\274" /* 0xfc */
#define CH_Ae "\303\204" /* 0xc4 */
#define CH_Ee "\303\213" /* 0xcb */
#define CH_Ie "\303\217" /* 0xcf */
#define CH_Oe "\303\226" /* 0xd6 */
#define CH_Ue "\303\234" /* 0xdc */
#define CH_sz "\303\237" /* 0xdf */
#define CH_e2 "\302\262" /* 0xb2 */
#define CH_mu "\302\265" /* 0xb5 */
#define CH_agrave "\303\240" /* 0xe0 */
#define CH_Agrave "\303\200" /* 0xc0 */
#define CH_egrave "\303\250" /* 0xe8 */
#define CH_Egrave "\303\210" /* 0xc8 */
#define CH_igrave "\303\254" /* 0xec */
#define CH_Igrave "\303\214" /* 0xcc */
#define CH_ograve "\303\262" /* 0xf2 */
#define CH_Ograve "\303\222" /* 0xd2 */
#define CH_ugrave "\303\271" /* 0xf9 */
#define CH_Ugrave "\303\231" /* 0xd9 */
#define CH_aacute "\303\241" /* 0xe1 */
#define CH_Aacute "\303\201" /* 0xc1 */
#define CH_eacute "\303\251" /* 0xe9 */
#define CH_Eacute "\303\211" /* 0xc9 */
#define CH_iacute "\303\255" /* 0xed */
#define CH_Iacute "\303\215" /* 0xcd */
#define CH_oacute "\303\263" /* 0xf3 */
#define CH_Oacute "\303\223" /* 0xd3 */
#define CH_uacute "\303\272" /* 0xfa */
#define CH_Uacute "\303\232" /* 0xda */
#define CH_acirc  "\303\242" /* 0xe2 */
#define CH_Acirc  "\303\202" /* 0xc2 */
#define CH_ecirc  "\303\252" /* 0xea */
#define CH_Ecirc  "\303\212" /* 0xca */
#define CH_icirc  "\303\256" /* 0xee */
#define CH_Icirc  "\303\216" /* 0xce */
#define CH_ocirc  "\303\264" /* 0xf4 */
#define CH_Ocirc  "\303\224" /* 0xd4 */
#define CH_ucirc  "\303\273" /* 0xfb */
#define CH_Ucirc  "\303\233" /* 0xdb */
#define CH_ccedil "\303\247" /* 0xe7 */
#define CH_Ccedil "\303\207" /* 0xc7 */
#define CH_ntilde "\303\261" /* 0xf1 */
#define CH_Ntilde "\303\221" /* 0xd1 */
#define CH_aring  "\303\245" /* 0xe5 */
#define CH_Aring  "\303\205" /* 0xc5 */
#define CH_aelig  "\303\246" /* 0xe6 */
#define CH_Aelig  "\303\206" /* 0xc6 */
#define CH_oslash "\303\270" /* 0xf8 */
#define CH_Oslash "\303\230" /* 0xd8 */
#define CH_iquest "\302\277" /* 0xbf */
#define CH_iexcl  "\302\241" /* 0xa1 */
#endif

#ifdef CHARSET_ASCII7
#define CH_ae "ae"
#define CH_ee "ee"
#define CH_ie "ie"
#define CH_oe "oe"
#define CH_ue "ue"
#define CH_Ae "Ae"
#define CH_Ee "Ee"
#define CH_Ie "Ie"
#define CH_Oe "Oe"
#define CH_Ue "Ue"
#define CH_sz "ss"
#define CH_e2 "^2"
#define CH_mu "u"
#define CH_agrave "`a"
#define CH_Agrave "`A"
#define CH_egrave "`e"
#define CH_Egrave "`E"
#define CH_igrave "`i"
#define CH_Igrave "`I"
#define CH_ograve "`o"
#define CH_Ograve "`O"
#define CH_ugrave "`u"
#define CH_Ugrave "`U"
#define CH_aacute "'a"
#define CH_Aacute "'A"
#define CH_eacute "'e"
#define CH_Eacute "'E"
#define CH_iacute "'i"
#define CH_Iacute "'I"
#define CH_oacute "'o"
#define CH_Oacute "'O"
#define CH_uacute "'u"
#define CH_Uacute "'U"
#define CH_acirc  "^a"
#define CH_Acirc  "^A"
#define CH_ecirc  "^e"
#define CH_Ecirc  "^E"
#define CH_icirc  "^i"
#define CH_Icirc  "^I"
#define CH_ocirc  "^o"
#define CH_Ocirc  "^O"
#define CH_ucirc  "^u"
#define CH_Ucirc  "^U"
#define CH_ccedil "c"
#define CH_Ccedil "C"
#define CH_ntilde "~n"
#define CH_Ntilde "~N"
#define CH_aring  "a"
#define CH_Aring  "A"
#define CH_aelig  "ae"
#define CH_Aelig  "AE"
#define CH_oslash "o"
#define CH_Oslash "O"
#define CH_iquest "?"
#define CH_iexcl  "!"
#endif

#endif /* _CHARDEFS_H */
