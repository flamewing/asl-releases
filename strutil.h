#ifndef _STRUTIL_H
#define _STRUTIL_H
/* strutil.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* haeufig benoetigte String-Funktionen                                      */
/*                                                                           */
/* Historie:  5. 5.1996 Grundsteinlegung                                     */
/*           13. 8.1997 KillBlanks-Funktionen aus asmsub.c heruebergenommen  */
/*           29. 8.1998 sprintf-Emulation                                    */
/*           29. 5.1999 SysString                                            */
/*                                                                           */
/*****************************************************************************/
/* $Id: strutil.h,v 1.10 2014/12/14 17:58:47 alfred Exp $                     */
/*****************************************************************************
 * $Log: strutil.h,v $
 * Revision 1.10  2014/12/14 17:58:47  alfred
 * - remove static variables in strutil.c
 *
 * Revision 1.9  2014/12/07 19:14:02  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.8  2014/12/03 19:01:01  alfred
 * - remove static return value
 *
 * Revision 1.7  2010/04/17 13:14:24  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.6  2008/11/23 10:39:17  alfred
 * - allow strings with NUL characters
 *
 * Revision 1.5  2007/11/24 22:48:08  alfred
 * - some NetBSD changes
 *
 * Revision 1.4  2005/10/02 10:00:46  alfred
 * - ConstLongInt gets default base, correct length check on KCPSM3 registers
 *
 * Revision 1.3  2004/05/30 20:54:26  alfred
 * - added CopyNoBlanks()
 *
 *****************************************************************************/

#include <stdio.h>

#include "datatypes.h"

extern Boolean HexLowerCase;

extern const char *Blanks(int cnt);

#define HexString(pDest, DestSize, i, Stellen) HexString2(pDest, DestSize, i, Stellen, HexLowerCase)

extern int HexString2(char *pDest, int DestSize, LargeWord i, Byte Stellen, Boolean LowerCase);

extern int SysString(char *pDest, int DestSize, LargeWord i, LargeWord System, int Stellen);

extern void HexBlankString(char *pDest, unsigned DestSize, LargeWord i, Byte Stellen);

extern char *LargeString(char *pDest, LargeInt i);

extern char *as_strdup(const char *s);

#ifdef NEEDS_CASECMP
extern int strcasecmp(const char *src1, const char *src2);
extern int strncasecmp(const char *src1, const char *src2, int maxlen);
#endif

#ifdef NEEDS_STRSTR
extern char *strstr(const char *haystack, const char *needle);
#endif

#ifdef BROKEN_SPRINTF
#define sprintf mysprintf
extern int mysprintf();
#endif

extern int strmaxcpy(char *dest, const char *src, int Max);
extern int strmaxcat(char *Dest, const char *Src, int MaxLen);
extern void strprep(char *Dest, const char *Src);
extern void strmaxprep(char *Dest, const char *Src, int MaxLen);
extern void strins(char *Dest, const char *Src, int Pos);
extern void strmaxins(char *Dest, const char *Src, int Pos, int MaxLen); 

extern int strlencmp(const char *pStr1, unsigned Str1Len,
                     const char *pStr2, unsigned Str2Len);

extern unsigned fstrlenprint(FILE *pFile, const char *pStr, unsigned StrLen);

extern unsigned snstrlenprint(char *pDest, unsigned DestLen,
                              const char *pStr, unsigned StrLen);

extern void ReadLn(FILE *Datei, char *Zeile);

extern int ReadLnCont(FILE *Datei, char *Zeile, int MaxLen);

extern LargeInt ConstLongInt(const char *inp, Boolean *pErr, LongInt Base);

extern char *ParenthPos(char *pHaystack, char Needle);

extern void KillBlanks(char *s);

extern int CopyNoBlanks(char *pDest, const char *pSrc, int MaxLen);

extern void KillPrefBlanks(char *s);

extern void KillPostBlanks(char *s);

extern int strqcmp(const char *s1, const char *s2);
  
extern char *strmov(char *pDest, const char *pSrc);
  
extern void strutil_init(void);

/* avoid nasty "subscript has type char..." messages on some platforms */

#define __chartouint(c) (((unsigned int)(c)) & 0xff)
#define mytoupper(c) (toupper(__chartouint(c)))
#define mytolower(c) (tolower(__chartouint(c)))
#define myisspace(c) (isspace(__chartouint(c)))
#define myisdigit(c) (isdigit(__chartouint(c)))
#define myisprint(c) (isprint(__chartouint(c)))
#define myisalpha(c) (isalpha(__chartouint(c)))

#endif /* _STRUTIL_H */
