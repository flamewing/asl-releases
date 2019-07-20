/* strutil.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* haeufig benoetigte String-Funktionen                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#include "strutil.h"
#undef strlen   /* VORSICHT, Rekursion!!! */
#ifdef BROKEN_SPRINTF
#undef sprintf
#endif

Boolean HexLowerCase;	    /* Hex-Konstanten mit Kleinbuchstaben */

/*--------------------------------------------------------------------------*/
/* eine bestimmte Anzahl Leerzeichen liefern */

const char *Blanks(int cnt)
{
  static const char *BlkStr = "                                                                                                           ";
  static int BlkStrLen = 0;

  if (!BlkStrLen)
    BlkStrLen = strlen(BlkStr);

  if (cnt < 0)
    cnt = 0;
  if (cnt > BlkStrLen)
    cnt = BlkStrLen;

  return BlkStr + (BlkStrLen - cnt);
}

/****************************************************************************/
/* eine Integerzahl in eine Hexstring umsetzen. Weitere vordere Stellen als */
/* Nullen */

int HexString2(char *pDest, int DestSize, LargeWord i, Byte Stellen, Boolean LowerCase)
{
  int Cnt, Len = 0;
  LargeWord digit;
  char *ptr;

  if (DestSize < 1)
    return 0;

  if (Stellen > DestSize - 1)
    Stellen = DestSize - 1;

  ptr = pDest + DestSize - 1;
  *ptr = '\0';
  Cnt = Stellen;
  do
  {
    digit = i & 15;
    if (digit < 10)
      *(--ptr) = digit + '0';
    else if (LowerCase)
      *(--ptr) = digit - 10 + 'a';
    else
      *(--ptr) = digit - 10 + 'A';
    i = i >> 4;
    Cnt--;
    Len++;
  }
  while ((Cnt > 0) || (i != 0));
  if (ptr != pDest)
    strmov(pDest, ptr);

  return Len;
}

int SysString(char *pDest, int DestSize, LargeWord i, LargeWord System, int Stellen)
{
  int Len = 0, Cnt;
  LargeWord digit;
  char *ptr;

  if (DestSize < 1)
    return 0;

  if (Stellen > DestSize - 1)
    Stellen = DestSize - 1;

  ptr = pDest + DestSize - 1;
  *ptr = '\0';
  Cnt = Stellen;
  do
  {
    digit = i % System;
    if (digit < 10)
      *(--ptr) = digit + '0';
    else if (HexLowerCase)
      *(--ptr) = digit - 10 + 'a';
    else
      *(--ptr) = digit - 10 + 'A';
    i /= System;
    Cnt--;
    Len++;
  }
  while ((Cnt > 0) || (i != 0));

  if (ptr != pDest)
    strmov(pDest, ptr);
  return Len;
}

/*--------------------------------------------------------------------------*/
/* dito, nur vorne Leerzeichen */

void HexBlankString(char *pDest, unsigned DestSize, LargeWord i, Byte Stellen)
{
  unsigned DestLen;

  DestLen = HexString(pDest, DestSize, i, 0);
  if (DestLen < Stellen)
    strmaxprep(pDest, Blanks(Stellen - DestLen), DestSize);
}

/*---------------------------------------------------------------------------*/
/* Da manche Systeme (SunOS) Probleme mit der Ausgabe extra langer Ints
   haben, machen wir das jetzt zu Fuss :-( */

char *LargeString(char *pDest, LargeInt i)
{
  Boolean SignFlag = False;
  String tmp;
  char *p, *p2;

  if (i < 0)
  {
    i = -i;
    SignFlag = True;
  }

  p = tmp;
  do
  {
    *(p++) = '0' + (i % 10);
    i /= 10;
  }
  while (i > 0);

  p2 = pDest;
  if (SignFlag)
    *(p2++) = '-';
  while (p > tmp)
    *(p2++) = (*(--p));
  *p2 = '\0';
  return pDest;
}


/*---------------------------------------------------------------------------*/
/* strdup() is not part of ANSI C89 */

char *as_strdup(const char *s)
{
  char *ptr = (char *) malloc(strlen(s) + 1);
#ifdef CKMALLOC
  if (!ptr)
  {
    fprintf(stderr, "strdup: out of memory?\n");
    exit(255);
  }
#endif
  if (ptr != 0)
    strcpy(ptr, s);
  return ptr;
}
/*---------------------------------------------------------------------------*/
/* ...so is snprintf... */

static int AppendPad(char **ppDest, int *pDestSize, char Src, int Cnt)
{
  int AddCnt = Cnt;

  if (AddCnt + 1 > *pDestSize)
    AddCnt = *pDestSize - 1;
  memset(*ppDest, Src, AddCnt);
  *ppDest += AddCnt;
  *pDestSize -= AddCnt;
  return Cnt;
}

static int Append(char **ppDest, int *pDestSize, const char *pSrc, int Cnt, int MinPrintLen)
{
  int AddCnt = Cnt, PadLen, Result = 0;

  PadLen = MinPrintLen - Cnt;
  if (PadLen < 0)
    PadLen = 0;

  if (PadLen > 0)
    Result += AppendPad(ppDest, pDestSize, ' ', PadLen);

  if (AddCnt + 1 > *pDestSize)
    AddCnt = *pDestSize - 1;
  memcpy(*ppDest, pSrc, AddCnt);
  *ppDest += AddCnt;
  *pDestSize -= AddCnt;
  return Result + Cnt;
}

int as_snprintf(char *pDest, int DestSize, const char *pFormat, ...)
{
  va_list ap;
  int Result = 0;
  Boolean InFormat = False;
  int MinPrintLen = 0;

  va_start(ap, pFormat);
  for (; *pFormat; pFormat++)
    if (InFormat)
      switch (*pFormat)
      {
        case '0': case '1': case '2': case 3: case '4':
        case '5': case '6': case '7': case 8: case '9':
        {
          /* if (!MinPrintLen && (*pFormat == '0'))... */
          MinPrintLen = (MinPrintLen * 10) + (*pFormat - '0');
          break;
        }
        case 'c':
        {
          char ch = va_arg(ap, int);

          Result += Append(&pDest, &DestSize, &ch, 1, MinPrintLen);
          InFormat = False;
          break;
        }
        case 's':
        {
          const char *pStr = va_arg(ap, char*);

          Result += Append(&pDest, &DestSize, pStr, strlen(pStr), MinPrintLen);
          InFormat = False;
          break;
        }
        default:
          fprintf(stderr, "invalid format: '%c'\n", *pFormat);
          exit(255);
      }
    else if (*pFormat == '%')
    {
      InFormat = True;
      MinPrintLen = 0;
    }
    else
      Result += Append(&pDest, &DestSize, pFormat, 1, 0);
  va_end(ap);
  *pDest++ = '\0';
  return Result;
}

#ifdef NEEDS_CASECMP
int strcasecmp(const char *src1, const char *src2)
{
  if (!src1)
    src1 = "";
  if (!src2)
    src2 = "";
  while (toupper(*src1) == toupper(*src2))
  {
    if ((!*src1) && (!*src2))
      return 0;
    src1++;
    src2++;
  }
  return ((int) toupper(*src1)) - ((int) toupper(*src2));
}	

int strncasecmp(const char *src1, const char *src2, size_t len)
{
  if (!src1)
    src1 = "";
  if (!src2)
    src2 = "";
  while (toupper(*src1) == toupper(*src2))
  {
    if (--len == 0)
      return 0;
    if ((!*src1) && (!*src2))
      return 0;
    src1++;
    src2++;
  }
  return ((int) toupper(*src1)) - ((int) toupper(*src2));
}	
#endif

#ifdef NEEDS_STRSTR
char *strstr(const char *haystack, const char *needle)
{
  int lh = strlen(haystack), ln = strlen(needle);
  int z;
  char *p;

  for (z = 0; z <= lh - ln; z++)
    if (strncmp(p = haystack + z, needle, ln) == 0)
      return p;
  return NULL;
}
#endif

/*!------------------------------------------------------------------------
 * \fn     strrmultchr(const char *haystack, const char *needles)
 * \brief  find the last occurence of either character in string
 * \param  haystack string to search in
 * \param  needles characters to search for
 * \return last occurence or NULL
 * ------------------------------------------------------------------------ */

char *strrmultchr(const char *haystack, const char *needles)
{
  const char *pPos;

  for (pPos = haystack + strlen(haystack) - 1; pPos >= haystack; pPos--)
    if (strchr(needles, *pPos))
      return (char*)pPos;
  return NULL;
}

#ifdef BROKEN_SPRINTF
#include <varargs.h>

int mysprintf(va_alist) va_dcl
{
  va_list pvar;
  char *form, *dest, *run;

  va_start(pvar);
  dest = va_arg(pvar, char*);
  form = va_arg(pvar, char*);
  vsprintf(dest, form, pvar);
  va_end(pvar);

  for (run = dest; *run != '\0'; run++);
  return run - dest;
}
#endif

#if 0
#include <stdarg.h>

/* Some s(n)printf implementations do not NUL-terminate the string.
   Furthermore, snprintf() returns the number of characters it
   *would* have written if it had had enough space... */

int my_snprintf(char *pDest, size_t DestSize, const char *pFmt, ...)
{
  va_list ap;

  va_start(ap, pFmt);
  vsnprintf(pDest, DestSize, pFmt, ap);
  va_end(ap);
  if (DestSize > 0)
  {
    pDest[DestSize - 1] = '\0';
    return strlen(pDest);
  }
  else
    return 0;
}
#endif

/*---------------------------------------------------------------------------*/
/* das originale strncpy plaettet alle ueberstehenden Zeichen mit Nullen */

int strmaxcpy(char *dest, const char *src, int Max)
{
  int cnt = strlen(src);

  /* leave room for terminating NUL */

  if (cnt > (Max - 1))
    cnt = Max - 1;
  memcpy(dest, src, cnt);
  dest[cnt] = '\0';
  return cnt;
}

/*---------------------------------------------------------------------------*/
/* einfuegen, mit Begrenzung */

int strmaxcat(char *Dest, const char *Src, int MaxLen)
{
  int TLen = strlen(Src), DLen = strlen(Dest);

  if (TLen > MaxLen - 1 - DLen)
    TLen = MaxLen - DLen - 1;
  if (TLen > 0)
  {
    memcpy(Dest + DLen, Src, TLen);
    Dest[DLen + TLen] = '\0';
    return DLen + TLen;
  }
  else
    return DLen;
}

void strprep(char *Dest, const char *Src)
{
  memmove(Dest + strlen(Src), Dest, strlen(Dest) + 1);
  memmove(Dest, Src, strlen(Src));
}

void strmaxprep(char *Dest, const char *Src, int MaxLen)
{
  int RLen, DestLen;

  RLen = strlen(Src);
  DestLen = strlen(Dest);
  if (RLen > MaxLen - DestLen - 1)
    RLen = MaxLen - DestLen - 1;
  memmove(Dest + RLen, Dest, DestLen + 1);
  memmove(Dest, Src, RLen);
}

void strins(char *Dest, const char *Src, int Pos)
{
  memmove(Dest + Pos + strlen(Src), Dest + Pos, strlen(Dest) + 1 - Pos);
  memmove(Dest + Pos, Src, strlen(Src));
}

void strmaxins(char *Dest, const char *Src, int Pos, int MaxLen)
{
  int RLen;

  RLen = strlen(Src);
  if (RLen > MaxLen - ((int)strlen(Dest)))
    RLen = MaxLen - strlen(Dest);
  memmove(Dest + Pos + RLen, Dest + Pos, strlen(Dest) + 1 - Pos);
  memmove(Dest + Pos, Src, RLen);
}

int strlencmp(const char *pStr1, unsigned Str1Len,
              const char *pStr2, unsigned Str2Len)
{
  const char *p1, *p2, *p1End, *p2End;
  int Diff;

  for (p1 = pStr1, p1End = p1 + Str1Len,
       p2 = pStr2, p2End = p2 + Str2Len;
       p1 < p1End && p2 < p2End; p1++, p2++)
  {
    Diff = ((int)*p1) - ((int)*p2);
    if (Diff)
      return Diff;
  }
  return ((int)Str1Len) - ((int)Str2Len);
}

unsigned fstrlenprint(FILE *pFile, const char *pStr, unsigned StrLen)
{
  unsigned Result = 0;
  const char *pRun, *pEnd;

  for (pRun = pStr, pEnd = pStr + StrLen; pRun < pEnd; pRun++)
    if ((*pRun == '\\') || (*pRun == '"') || (!isprint(*pRun)))
    {
      fprintf(pFile, "\\%03d", *pRun);
      Result += 4;
    }
    else
    {
      fputc(*pRun, pFile);
      Result++;
    }

  return Result;
}

unsigned snstrlenprint(char *pDest, unsigned DestLen,
                       const char *pStr, unsigned StrLen)
{
  unsigned Result = 0;
  const char *pRun, *pEnd;

  for (pRun = pStr, pEnd = pStr + StrLen; pRun < pEnd; pRun++)
    if ((*pRun == '\\') || (*pRun == '"') || (!isprint(*pRun)))
    {
      if (DestLen < 5)
        break;
      sprintf(pDest, "\\%03d", *pRun);
      pDest += 4;
      DestLen -= 4;
      Result += 4;
    }
    else
    {
      if (DestLen < 2)
        break;
      *pDest++ = *pRun;
      DestLen--;
      Result++;
    }
  *pDest = '\0';

  return Result;
}

/*---------------------------------------------------------------------------*/
/* Bis Zeilenende lesen */

void ReadLn(FILE *Datei, char *Zeile)
{
  char *ptr;
  int l;

  *Zeile = '\0';
  ptr = fgets(Zeile, 256, Datei);
  if ((!ptr) && (ferror(Datei) != 0))
    *Zeile = '\0';
  l = strlen(Zeile);
  if ((l > 0) && (Zeile[l - 1] == '\n'))
    Zeile[--l] = '\0';
  if ((l > 0) && (Zeile[l - 1] == '\r'))
    Zeile[--l] = '\0';
  if ((l > 0) && (Zeile[l - 1] == 26))
    Zeile[--l] = '\0';
}

#if 0

static void dump(const char *pLine, unsigned Cnt)
{
  unsigned z;

  fputc('\n', stderr);
  for (z = 0; z < Cnt; z++)
  {
    fprintf(stderr, " %02x", pLine[z]);
    if ((z & 15) == 15)
      fputc('\n', stderr);
  }
  fputc('\n', stderr);
}

#endif

int ReadLnCont(FILE *Datei, char *Zeile, int MaxLen)
{
  char *ptr, *pDest;
  int l, RemLen, Count;
  Boolean cont, Terminated;

  /* read from input until either string has reached maximum length,
     or no continuation is present */

  RemLen = MaxLen;
  pDest = Zeile;
  Count = 0;
  do
  {
    /* get a line from file */

    Terminated = False;
    *pDest = '\0';
    ptr = fgets(pDest, RemLen, Datei);
    if ((!ptr) && (ferror(Datei) != 0))
      *pDest = '\0';

    /* strip off trailing CR/LF */

    l = strlen(pDest);
    cont = False;
    if ((l > 0) && (pDest[l - 1] == '\n'))
    {
      pDest[--l] = '\0';
      Terminated = True;
    }
    if ((l > 0) && (pDest[l - 1] == '\r'))
      pDest[--l] = '\0';

    /* yes - this is necessary, when we get an old DOS textfile with
       Ctrl-Z as EOF */

    if ((l > 0) && (pDest[l - 1] == 26))
      pDest[--l] = '\0';

    /* optional line continuation */

    if ((l > 0) && (pDest[l - 1] == '\\'))
    {
      pDest[--l] = '\0';
      cont = True;
    }

    /* prepare for next chunk */

    RemLen -= l;
    pDest += l;
    Count++;
  }
  while ((RemLen > 2) && (cont));

  if (!Terminated)
  {
    char Tmp[100];

    while (TRUE)
    {
      Terminated = False;
      ptr = fgets(Tmp, sizeof(Tmp), Datei);
      if (!ptr)
        break;
      l = strlen(Tmp);
      if (!l)
        break;
      if ((l > 0) && (Tmp[l - 1] == '\n'))
        break;
    }
  }

  return Count;
}

/*--------------------------------------------------------------------*/
/* Zahlenkonstante umsetzen: $ hex, % binaer, @ oktal */
/* inp: Eingabezeichenkette */
/* erg: Zeiger auf Ergebnis-Longint */
/* liefert TRUE, falls fehlerfrei, sonst FALSE */

LargeInt ConstLongInt(const char *inp, Boolean *pErr, LongInt Base)
{
  static const char Prefixes[4] = { '$', '@', '%', '\0' }; /* die moeglichen Zahlensysteme */
  static const char Postfixes[4] = { 'H', 'O', '\0', '\0' };
  static const LongInt Bases[3] = { 16, 8, 2 };            /* die dazugehoerigen Basen */
  LargeInt erg, val;
  int z, vorz = 1;  /* Vermischtes */
  int InpLen = strlen(inp);

  /* eventuelles Vorzeichen abspalten */

  if (*inp == '-')
  {
    vorz = -1;
    inp++;
    InpLen--;
  }

  /* Sonderbehandlung 0x --> $ */

  if ((InpLen >= 2)
   && (*inp == '0')
   && (mytoupper(inp[1]) == 'X'))
  {
    inp += 2;
    InpLen -= 2;
    Base = 16;
  }

  /* Jetzt das Zahlensystem feststellen.  Vorgabe ist dezimal, was
     sich aber durch den Initialwert von Base jederzeit aendern
     laesst.  Der break-Befehl verhindert, dass mehrere Basenzeichen
     hintereinander eingegeben werden koennen */

  else if (InpLen > 0)
  {
    for (z = 0; z < 3; z++)
      if (*inp == Prefixes[z])
      {
        Base = Bases[z];
        inp++;
        InpLen--;
        break;
      }
      else if (mytoupper(inp[InpLen - 1]) == Postfixes[z])
      {
        Base = Bases[z];
        InpLen--;
        break;
      }
  }

  /* jetzt die Zahlenzeichen der Reihe nach durchverwursten */

  erg = 0;
  *pErr = False;
  for(; InpLen > 0; inp++, InpLen--)
  {
    /* Ziffern 0..9 ergeben selbiges */

    if ((*inp >= '0') && (*inp <= '9'))
      val = (*inp) - '0';

    /* Grossbuchstaben fuer Hexziffern */

    else if ((*inp >= 'A') && (*inp <= 'F'))
      val = (*inp) - 'A' + 10;

    /* Kleinbuchstaben nicht vergessen...! */

    else if ((*inp >= 'a') && (*inp <= 'f'))
      val = (*inp) - 'a' + 10;

    /* alles andere ist Schrott */

    else
      break;

    /* entsprechend der Basis zulaessige Ziffer ? */

    if (val >= Base)
      break;

    /* Zahl linksschieben, zusammenfassen, naechster bitte */

    erg = erg * Base + val;
  }

  /* bis zum Ende durchgelaufen ? */

  if (!InpLen)
  {
    /* Vorzeichen beruecksichtigen */

    erg *= vorz;
    *pErr = True;
  }

  return erg;
}

/*--------------------------------------------------------------------------*/
/* alle Leerzeichen aus einem String loeschen */

void KillBlanks(char *s)
{
  char *z, *dest;
  Boolean InSgl = False, InDbl = False, ThisEscaped = False, NextEscaped = False;

  dest = s;
  for (z = s; *z != '\0'; z++, ThisEscaped = NextEscaped)
  {
    NextEscaped = False;
    switch (*z)
    {
      case '\'':
        if (!InDbl && !ThisEscaped)
          InSgl = !InSgl;
        break;
      case '"':
        if (!InSgl && !ThisEscaped)
          InDbl = !InDbl;
        break;
      case '\\':
        if ((InSgl || InDbl) && !ThisEscaped)
          NextEscaped = True;
        break;
    }
    if (!isspace((unsigned char)*z) || InSgl || InDbl)
      *dest++ = *z;
  }
  *dest = '\0';
}

int CopyNoBlanks(char *pDest, const char *pSrc, int MaxLen)
{
  const char *pSrcRun;
  char *pDestRun = pDest;
  int Cnt = 0;
  Byte Flags = 0;
  char ch;
  Boolean ThisEscaped, PrevEscaped;

  /* leave space for NUL */

  MaxLen--;

  PrevEscaped = False;
  for (pSrcRun = pSrc; *pSrcRun; pSrcRun++)
  {
    ch = *pSrcRun;
    ThisEscaped = False;
    switch (ch)
    {
      case '\'':
        if (!(Flags & 2) && !PrevEscaped)
          Flags ^= 1;
        break;
      case '"':
        if (!(Flags & 1) && !PrevEscaped)
          Flags ^= 2;
        break;
      case '\\':
        if (!PrevEscaped)
          ThisEscaped = True;
        break;
    }
    if ((!isspace((unsigned char)ch)) || (Flags))
      *(pDestRun++) = ch;
    if (++Cnt >= MaxLen)
      break;
    PrevEscaped = ThisEscaped;
  }
  *pDestRun = '\0';

  return Cnt;
}

/*--------------------------------------------------------------------------*/
/* fuehrende Leerzeichen loeschen */

int KillPrefBlanks(char *s)
{
  char *z = s;

  while ((*z != '\0') && (isspace((unsigned char)*z)))
    z++;
  if (z != s)
    strmov(s, z);
  return z - s;
}

/*--------------------------------------------------------------------------*/
/* anhaengende Leerzeichen loeschen */

int KillPostBlanks(char *s)
{
  char *z = s + strlen(s) - 1;
  int count = 0;

  while ((z >= s) && (isspace((unsigned char)*z)))
  {
    *(z--) = '\0';
    count++;
  }
  return count;
}

/*--------------------------------------------------------------------------*/

int strqcmp(const char *s1, const char *s2)
{
  int erg = (*s1) - (*s2);

  return (erg != 0) ? erg : strcmp(s1, s2);
}

/*--------------------------------------------------------------------------*/

/* we need a strcpy() with a defined behaviour in case of overlapping source
   and destination: */

char *strmov(char *pDest, const char *pSrc)
{
  memmove(pDest, pSrc, strlen(pSrc) + 1);
  return pDest;
}

#ifdef __GNUC__

#ifdef strcpy
# undef strcpy
#endif
char *strcpy(char *pDest, const char *pSrc)
{
  int l = strlen(pSrc) + 1;
  int Overlap = 0;

  if (pSrc < pDest)
  {
    if (pSrc + l > pDest)
      Overlap = 1;
  }
  else if (pSrc > pDest)
  {
    if (pDest + l > pSrc)
      Overlap = 1;
  }
  else if (l > 0)
  {
    Overlap = 1;
  }

  if (Overlap)
    fprintf(stderr, "overlapping strcpy() called from address %p, resolve this address with addr2line and report to author\n",
            __builtin_return_address(0));

  return strmov(pDest, pSrc);
}

#endif

/*!------------------------------------------------------------------------
 * \fn     strmemcpy(char *pDest, int DestSize, const char *pSrc, int SrcLen)
 * \brief  copy string with length limitation
 * \param  pDest where to write
 * \param  DestSize destination capacity
 * \param  pSrc copy source
 * \param  SrcLen # of characters to copy at most
 * \return actual, possibly limited length
 * ------------------------------------------------------------------------ */

int strmemcpy(char *pDest, int DestSize, const char *pSrc, int SrcLen)
{
  if (DestSize < SrcLen + 1)
    SrcLen = DestSize - 1;
  if (SrcLen < 0)
    SrcLen = 0;
  memmove(pDest, pSrc, SrcLen);
  pDest[SrcLen] = '\0';
  return SrcLen;
}

/*--------------------------------------------------------------------------*/

char *ParenthPos(char *pHaystack, char Needle)
{
  char *pRun;
  int Level = 0;

  for (pRun = pHaystack; *pRun; pRun++)
  {
    switch (*pRun)
    {
      case '(':
        Level++;
        break;
      case ')':
        if (Level < 1)
          return NULL;
        Level--;
        break;
      default:
        if (*pRun == Needle && !Level)
          return pRun;
    }
  }
  return NULL;
}

/*--------------------------------------------------------------------------*/

void strutil_init(void)
{
  HexLowerCase = False;
}
