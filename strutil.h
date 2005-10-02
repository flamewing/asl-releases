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
/* $Id: strutil.h,v 1.4 2005/10/02 10:00:46 alfred Exp $                     */
/*****************************************************************************
 * $Log: strutil.h,v $
 * Revision 1.4  2005/10/02 10:00:46  alfred
 * - ConstLongInt gets default base, correct length check on KCPSM3 registers
 *
 * Revision 1.3  2004/05/30 20:54:26  alfred
 * - added CopyNoBlanks()
 *
 *****************************************************************************/

extern Boolean HexLowerCase;

extern char *Blanks(int cnt);

extern char *HexString(LargeWord i, Byte Stellen);

extern char *SysString(LargeWord i, LargeWord System, Byte Stellen);

extern char *HexBlankString(LargeWord i, Byte Stellen);

extern char *LargeString(LargeInt i);

#ifdef NEEDS_STRDUP
extern char *strdup(char *s);
#endif
#ifdef CKMALLOC
#define strdup(s) mystrdup(s)
extern char *mystrdup(char *s);
#endif

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

extern void strmaxcpy(char *dest, const char *src, int Max);
extern void strmaxcat(char *Dest, const char *Src, int MaxLen);
extern void strprep(char *Dest, const char *Src);
extern void strmaxprep(char *Dest, const char *Src, int MaxLen);
extern void strins(char *Dest, const char *Src, int Pos);
extern void strmaxins(char *Dest, const char *Src, int Pos, int MaxLen); 

extern void ReadLn(FILE *Datei, char *Zeile);

extern int ReadLnCont(FILE *Datei, char *Zeile, int MaxLen);

extern LongInt ConstLongInt(const char *inp, Boolean *pErr, LongInt Base);

extern void KillBlanks(char *s);

extern int CopyNoBlanks(char *pDest, const char *pSrc, int MaxLen);

extern void KillPrefBlanks(char *s);

extern void KillPostBlanks(char *s);

extern int strqcmp(const char *s1, const char *s2);
  
extern void strutil_init(void);
#endif /* _STRUTIL_H */
