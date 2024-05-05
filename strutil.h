#ifndef STRUTIL_H
#define STRUTIL_H
/* strutil.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* haeufig benoetigte String-Funktionen                                      */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

struct as_dynstr;

extern char HexStartCharacter;
extern char SplitByteCharacter;

extern char const* Blanks(int cnt);

#define HexString(pDest, DestSize, i, Stellen)                         \
    SysString(                                                         \
            pDest, DestSize, i, 16, Stellen, False, HexStartCharacter, \
            SplitByteCharacter)
#define DecString(pDest, DestSize, i, Digits) \
    SysString(pDest, DestSize, i, 10, Digits, False, HexStartCharacter, '\0')

extern int SysString(
        char* pDest, size_t DestSize, LargeWord i, int System, int Stellen,
        Boolean ForceLeadZero, char StartCharacter, char SplitCharacter);

extern char* as_strdup(char const* s);

extern int as_snprcatf(
        char* pDest, size_t DestSize, PRINTF_FORMAT const char* pFormat, ...)
        PRINTF_FORMAT_ATTR(3, 4);
extern int as_snprintf(
        char* pDest, size_t DestSize, PRINTF_FORMAT const char* pFormat, ...)
        PRINTF_FORMAT_ATTR(3, 4);

extern int as_sdprcatf(struct as_dynstr* p_dest, PRINTF_FORMAT const char* pFormat, ...)
        PRINTF_FORMAT_ATTR(2, 3);
extern int as_sdprintf(struct as_dynstr* p_dest, PRINTF_FORMAT const char* pFormat, ...)
        PRINTF_FORMAT_ATTR(2, 3);

extern int as_strcasecmp(char const* src1, char const* src2);
extern int as_strncasecmp(char const* src1, char const* src2, size_t maxlen);

#ifdef NEEDS_STRSTR
extern char* strstr(char const* haystack, char const* needle);
#endif

extern char* strrmultchr(char const* haystack, char const* needles);

extern size_t strmaxcpy(char* dest, char const* src, size_t Max);
extern size_t strmaxcat(char* Dest, char const* Src, size_t MaxLen);
extern void   strprep(char* Dest, char const* Src);
extern void   strmaxprep(char* p_dest, char const* p_src, size_t max_len);
extern void   strmaxprep2(char* p_dest, char const* p_src, size_t max_len);
extern void   strins(char* Dest, char const* Src, int Pos);
extern void   strmaxins(char* Dest, char const* Src, int Pos, size_t MaxLen);

extern size_t as_strnlen(char const* pStr, size_t MaxLen);

extern int strreplace(
        char* pHaystack, char const* pFrom, char const* pTo, size_t ToMaxLen,
        size_t HaystackSize);

extern int strlencmp(
        char const* pStr1, unsigned Str1Len, char const* pStr2, unsigned Str2Len);

extern unsigned fstrlenprint(FILE* pFile, char const* pStr, unsigned StrLen);

extern void ReadLn(FILE* Datei, char* Zeile);

extern size_t ReadLnCont(FILE* Datei, struct as_dynstr* p_line);

extern int DigitVal(char ch, int Base);

extern LargeInt ConstLongInt(char const* inp, Boolean* pErr, LongInt Base);

extern char* ParenthPos(char* pHaystack, char Needle);

extern void KillBlanks(char* s);

extern int CopyNoBlanks(char* pDest, char const* pSrc, size_t MaxLen);

extern int KillPrefBlanks(char* s);

extern int KillPostBlanks(char* s);

extern char TabCompressed(char in);

extern int strqcmp(char const* s1, char const* s2);

extern char* strmov(char* pDest, char const* pSrc);

extern int strmemcpy(char* pDest, size_t DestSize, char const* pSrc, size_t SrcLen);

extern void strutil_init(void);

/* avoid nasty "subscript has type char..." messages on some platforms */

#include <ctype.h>    // IWYU pragma: export

#define chartouint(c)  (((unsigned int)(c)) & 0xff)
#define as_toupper(c)  (toupper(chartouint(c)))
#define as_tolower(c)  (tolower(chartouint(c)))
#define as_isspace(c)  (!!isspace(chartouint(c)))
#define as_isdigit(c)  (!!isdigit(chartouint(c)))
#define as_isxdigit(c) (!!isxdigit(chartouint(c)))
#define as_isprint(c)  (!!isprint(chartouint(c)))
#define as_isalpha(c)  (!!isalpha(chartouint(c)))
#define as_isupper(c)  (!!isupper(chartouint(c)))
#define as_islower(c)  (!!islower(chartouint(c)))
#define as_isalnum(c)  (!!isalnum(chartouint(c)))

#endif /* STRUTIL_H */
