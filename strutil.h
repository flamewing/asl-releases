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

extern Boolean HexLowerCase;

extern char *Blanks(int cnt);

extern char *HexString(LargeWord i, int Stellen);

extern char *SysString(LargeWord i, LargeWord System, int Stellen);

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
extern char *strstr(char *haystack, char *needle);
#endif

#ifdef BROKEN_SPRINTF
#define sprintf mysprintf
extern int mysprintf();
#endif

#undef strlen
#define strlen(s) strslen(s)
extern signed int strslen(const char *s);
extern void strmaxcpy(char *dest, const char *src, int Max);
extern void strmaxcat(char *Dest, const char *Src, int MaxLen);
extern void strprep(char *Dest, const char *Src);
extern void strmaxprep(char *Dest, const char *Src, int MaxLen);
extern void strins(char *Dest, const char *Src, int Pos);
extern void strmaxins(char *Dest, const char *Src, int Pos, int MaxLen); 

extern void ReadLn(FILE *Datei, char *Zeile);

extern LongInt ConstLongInt(const char *inp, Boolean *err);

extern void KillBlanks(char *s);

extern void KillPrefBlanks(char *s);

extern void KillPostBlanks(char *s);

extern int strqcmp(const char *s1, const char *s2);
  
extern void strutil_init(void);
