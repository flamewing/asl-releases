#ifndef _chardefs_h
#define _chardefs_h

#ifdef CHARSET_ISO8859_1
#define CH_ae "\344"
#define CH_oe "\366"
#define CH_ue "\374"
#define CH_Ae "\304"
#define CH_Oe "\326"
#define CH_Ue "\334"
#define CH_sz "\337"
#endif

#ifdef CHARSET_IBM437
#define CH_ae "\204"
#define CH_oe "\224"
#define CH_ue "\201"
#define CH_Ae "\216"
#define CH_Oe "\231"
#define CH_Ue "\232"
#define CH_sz "\341"
#endif

#ifdef CHARSET_IBM850
#define CH_ae "\204"
#define CH_oe "\224"
#define CH_ue "\201"
#define CH_Ae "\216"
#define CH_Oe "\231"
#define CH_Ue "\232"
#define CH_sz "\341"
#endif

#ifdef CHARSET_ASCII7
#define CH_ae "ae"
#define CH_oe "oe"
#define CH_ue "ue"
#define CH_Ae "Ae"
#define CH_Oe "Oe"
#define CH_Ue "Ue"
#define CH_sz "ss"
#endif

#endif /* _chardefs_h */

