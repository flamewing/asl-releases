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

#define HexString(pDest, DestSize, i, Digits)                                \
    SysString(                                                               \
            pDest, DestSize, i, 16, False, Digits, False, HexStartCharacter, \
            SplitByteCharacter)
#define DecString(pDest, DestSize, i, Digits) \
    SysString(pDest, DestSize, i, 10, False, Digits, False, HexStartCharacter, '\0')

extern int SysString(
        char* pDest, size_t DestSize, LargeWord i, int System, Boolean is_signed,
        int Digits, Boolean ForceLeadZero, char StartCharacter, char SplitCharacter);

extern char* as_strdup(char const* s);

typedef enum {
    e_not_set,
    e_set,
    e_finished
} as_format_arg_state_t;

typedef struct {
    as_format_arg_state_t arg_state[3];
    Boolean in_format, lead_zero, is_signed, left_align, add_plus, force_lead_zero,
            force_upper;
    int arg[3], curr_arg, int_size;
} as_format_ctx_t;

extern void    as_format_context_reset(as_format_ctx_t* p_context);
extern Boolean as_format_context_consume(as_format_ctx_t* p_context, char ch);

/* Bounded formatters return characters actually stored (excluding NUL).
   With zero capacity they return zero without accessing destination/format.
   With positive capacity, replace initializes the destination; append requires
   an existing NUL within that capacity. Both retain space for the final NUL.
   Passing sizeof(char*) as the capacity remains a fatal usage error. */
extern int as_vsnprcatf(char* pDest, size_t DestSize, char const* pFormat, va_list ap);
extern int as_vsnprintf(char* pDest, size_t DestSize, char const* pFormat, va_list ap);
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

extern char* strmultchr(char const* p_haystack, char const* p_needles);
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

extern char* ParenthPos(char* pHaystack, char Needle);

extern int KillPrefBlanks(char* s);

extern int KillPostBlanks(char* s);

extern char TabCompressed(char in);

extern int strqcmp(char const* s1, char const* s2);

extern char* strmov(char* pDest, char const* pSrc);

extern int strmemcpy(char* pDest, size_t DestSize, char const* pSrc, size_t SrcLen);

extern unsigned as_bit_count(LongWord i);

extern void strutil_init(void);

/* avoid nasty "subscript has type char..." messages on some platforms */

#include <ctype.h>    // IWYU pragma: export

#define chartouint(c) (((unsigned int)(c)) & 0xff)
#define as_toupper(c) (toupper(chartouint(c)))
#define as_tolower(c) (tolower(chartouint(c)))

/* Initialized for the C locale; strutil_init refreshes a changed LC_CTYPE. */
extern unsigned char as_space_class[256];

static inline int as_isspace(int c) {
    return as_space_class[chartouint(c)];
}

#define as_isdigit(c)  (!!isdigit(chartouint(c)))
#define as_isxdigit(c) (!!isxdigit(chartouint(c)))
#define as_isprint(c)  (!!isprint(chartouint(c)))
#define as_isalpha(c)  (!!isalpha(chartouint(c)))
#define as_isupper(c)  (!!isupper(chartouint(c)))
#define as_islower(c)  (!!islower(chartouint(c)))
#define as_isalnum(c)  (!!isalnum(chartouint(c)))

#define as_isalnum_ubar(c) (as_isalnum(c) || ((c) == '_'))

#define as_isspace_or_nul(c) (as_isspace(c) || ((c) == '\0'))

#endif /* STRUTIL_H */
