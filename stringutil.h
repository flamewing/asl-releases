/* stringutil.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* haeufig benoetigte String-Funktionen                                      */
/*                                                                           */
/* Historie:  5. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

extern Boolean HexLowerCase;

extern char *Blanks(int cnt);

extern char *HexString(LargeWord i, Byte Stellen);

extern char *HexBlankString(LargeWord i, Byte Stellen);

extern char *LargeString(LargeInt i);

#ifdef __ultrix
extern char *strdup(char *s);
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

extern void stringutil_init(void);
