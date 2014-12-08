/* strutil.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* haeufig benoetigte String-Funktionen                                      */
/*                                                                           */
/* Historie:  5. 5.1996 Grundsteinlegung                                     */
/*           13. 8.1997 KillBlanks-Funktionen aus asmsub.c heruebergenommen  */
/*           29. 8.1998 sprintf-Emulation                                    */
/*           17. 4.1999 strcasecmp gegen Nullzeiger geschuetzt               */
/*           30. 5.1999 ConstLongInt akzeptiert auch 0x fuer Hex-Zahlen      */
/*                                                                           */
/*****************************************************************************/
/* $Id: strutil.c,v 1.14 2014/12/07 19:14:02 alfred Exp $                     */
/*****************************************************************************
 * $Log: strutil.c,v $
 * Revision 1.14  2014/12/07 19:14:02  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.13  2014/12/03 19:01:00  alfred
 * - remove static return value
 *
 * Revision 1.12  2014/12/03 18:54:53  alfred
 * - rework to current style
 *
 * Revision 1.11  2013/12/18 22:21:54  alfred
 * - regard \ escaping for quotes
 *
 * Revision 1.10  2013/08/07 19:44:38  alfred
 * - add test for overlong lines
 *
 * Revision 1.9  2013/08/05 20:10:10  alfred
 * - handle overlong lines
 *
 * Revision 1.8  2012-08-19 09:39:02  alfred
 * - handle cases where strcpy is a macro
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

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

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

#define BufferCnt 8

char *HexString(LargeWord i, Byte Stellen)
{
  static ShortString h[BufferCnt];
  static int z = 0, Cnt;
  LargeWord digit;
  char *ptr;

  if (Stellen > sizeof(ShortString) - 1)
    Stellen = sizeof(ShortString) - 1;

  ptr = h[z] + sizeof(ShortString) - 1;
  *ptr = '\0';
  Cnt = Stellen;
  do
  {
    digit = i & 15;
    if (digit < 10)
      *(--ptr) = digit + '0';
    else if (HexLowerCase)
      *(--ptr) = digit - 10 + 'a';
    else
      *(--ptr) = digit - 10 + 'A';
    i = i >> 4;
    Cnt--;
  }
  while ((Cnt > 0) || (i != 0));

  z = (z + 1) % BufferCnt;

  return ptr;
}

char *SysString(LargeWord i, LargeWord System, Byte Stellen)
{
  static ShortString h[BufferCnt];
  static int z = 0, Cnt;
  LargeWord digit;
  char *ptr;

  if (Stellen > sizeof(ShortString) - 1)
    Stellen = sizeof(ShortString) - 1;

  ptr = h[z] + sizeof(ShortString) - 1;
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
  }
  while ((Cnt > 0) || (i != 0));

  z = (z + 1) % BufferCnt;

  return ptr;
}

/*--------------------------------------------------------------------------*/
/* dito, nur vorne Leerzeichen */

char *HexBlankString(LargeWord i, Byte Stellen)
{
  static String temp;

  strmaxcpy(temp, HexString(i, 0), 255);
  if (strlen(temp) < Stellen)
    strmaxprep(temp, Blanks(Stellen - strlen(temp)), 255);
  return temp;
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
/* manche haben's nicht... */

#if defined(NEEDS_STRDUP) || defined(CKMALLOC)
#ifdef CKMALLOC
char *mystrdup(const char *s)
{
#else
char *strdup(const char *s)
{
#endif
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
#endif

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

int strncasecmp(const char *src1, const char *src2, int len)
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

  for (run = dest; *run != '\0'; run++);
  return run - dest;
}
#endif

/*---------------------------------------------------------------------------*/
/* das originale strncpy plaettet alle ueberstehenden Zeichen mit Nullen */

void strmaxcpy(char *dest, const char *src, int Max)
{
  int cnt = strlen(src);

  /* leave room for terminating NUL */

  if (cnt > (Max - 1))
    cnt = Max - 1;
  memcpy(dest, src, cnt);
  dest[cnt] = '\0';
}

/*---------------------------------------------------------------------------*/
/* einfuegen, mit Begrenzung */

void strmaxcat(char *Dest, const char *Src, int MaxLen)
{
  int TLen = strlen(Src), DLen = strlen(Dest);

  if (TLen > MaxLen - DLen)
    TLen = MaxLen - DLen;
  if (TLen > 0)
  {
    memcpy(Dest + DLen, Src, TLen);
    Dest[DLen + TLen] = '\0';
  }
}

void strprep(char *Dest, const char *Src)
{
  memmove(Dest + strlen(Src), Dest, strlen(Dest) + 1);
  memmove(Dest, Src, strlen(Src));
}

void strmaxprep(char *Dest, const char *Src, int MaxLen)
{
  int RLen;

  RLen = strlen(Src);
  if (RLen > MaxLen - ((int)strlen(Dest)))
    RLen = MaxLen - strlen(Dest);
  memmove(Dest + RLen, Dest, strlen(Dest) + 1);
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

LongInt ConstLongInt(const char *inp, Boolean *pErr, LongInt Base)
{
  static const char Prefixes[4] = { '$', '@', '%', '\0' }; /* die moeglichen Zahlensysteme */
  static const LongInt Bases[3] = { 16, 8, 2 };            /* die dazugehoerigen Basen */
  LongInt erg;
  LongInt z, val, vorz = 1;  /* Vermischtes */

  /* eventuelles Vorzeichen abspalten */

  if (*inp == '-')
  {
    vorz = -1;
    inp++;
  }

  /* Sonderbehandlung 0x --> $ */

  if ((strlen(inp) >= 2)
   && (*inp == '0')
   && (mytoupper(inp[1]) == 'X'))
  {
    inp += 2;
    Base = 16;
  }

  /* Jetzt das Zahlensystem feststellen.  Vorgabe ist dezimal, was
     sich aber durch den Initialwert von Base jederzeit aendern
     laesst.  Der break-Befehl verhindert, dass mehrere Basenzeichen
     hintereinander eingegeben werden koennen */

  else
    for (z = 0; z < 3; z++)
      if (*inp == Prefixes[z])
      {
        Base = Bases[z];
        inp++;
        break;
      }

  /* jetzt die Zahlenzeichen der Reihe nach durchverwursten */

  erg = 0;
  *pErr = False;
  for(; *inp != '\0'; inp++)
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

    erg = erg*Base + val;
  }

  /* bis zum Ende durchgelaufen ? */

  if (*inp == '\0')
  {
    /* Vorzeichen beruecksichtigen */

    erg*=vorz;
    *pErr = True;
  }

  return erg;
}

/*--------------------------------------------------------------------------*/
/* alle Leerzeichen aus einem String loeschen */

void KillBlanks(char *s)
{
  char *z, *dest;
  Boolean InHyp = False, InQuot = False;

  dest = s;
  for (z = s; *z != '\0'; z++)
  {
    switch (*z)
    {
      case '\'':
        if (!InQuot)
          InHyp = !InHyp;
        break;
      case '"':
        if (!InHyp)
          InQuot = !InQuot;
        break;
    }
    if ((!isspace((unsigned char)*z)) || (InHyp) || (InQuot))
      *(dest++) = (*z);
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

void KillPrefBlanks(char *s)
{
  char *z = s;

  while ((*z != '\0') && (isspace((unsigned char)*z)))
    z++;
  if (z != s)
    strmov(s, z);
}

/*--------------------------------------------------------------------------*/
/* anhaengende Leerzeichen loeschen */

void KillPostBlanks(char *s)
{
  char *z = s + strlen(s) - 1;

  while ((z >= s) && (isspace((unsigned char)*z)))
    *(z--) = '\0';
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
    fprintf(stderr, "overlapping strcpy() called from address 0x%p, resolve this address with addr2line and report to author\n",
            __builtin_return_address(0));

  return strmov(pDest, pSrc);
}

#endif

/*--------------------------------------------------------------------------*/

void strutil_init(void)
{
  HexLowerCase = False;
}
