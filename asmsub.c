/* asmsub.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Unterfunktionen, vermischtes                                              */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*           13. 8.1997 KillBlanks-Funktionen nach stringutil.c geschoben    */
/*           26. 6.1998 Fehlermeldung Codepage nicht gefunden                */
/*            7. 7.1998 Fix Zugriffe auf CharTransTable wg. signed chars     */
/*           17. 8.1998 Unterfunktion zur Buchhaltung Adressbereiche         */
/*            1. 9.1998 FloatString behandelte Sonderwerte nicht korrekt     */
/*           13. 9.1998 Prozessorliste macht Zeilenvorschub nach 6 Namen     */
/*           14.10.1998 Fehlerzeilen mit > > >                               */
/*           30. 1.1999 Formatstrings maschinenunabhaengig gemacht           */
/*           18. 4.1999 Ausgabeliste Sharefiles                              */
/*           13. 7.1999 Fehlermeldungen relokatible Symbole                  */
/*           13. 9.1999 I/O-Fehler 25 ignorieren                             */
/*            5.11.1999 ExtendErrors ist jetzt ShortInt                      */
/*           13. 2.2000 Ausgabeliste Listing                                 */
/*            6. 8.2000 added ValidSymChar array                             */
/*           21. 7.2001 added not repeatable message                         */
/*           2001-08-01 QuotPos also works for ) resp. ] characters          */
/*           2001-09-03 added warning message about X-indexed conversion     */
/*           2001-10-21 additions for GNU-style errors                       */
/*           2002-03-31 fixed operand order of memset                        */
/*                                                                           */
/*****************************************************************************/
/* $Id: asmsub.c,v 1.35 2017/04/02 11:10:37 alfred Exp $                      */
/*****************************************************************************
 * $Log: asmsub.c,v $
 * Revision 1.35  2017/04/02 11:10:37  alfred
 * - allow more fine-grained macro expansion in listing
 *
 * Revision 1.34  2016/11/25 18:12:13  alfred
 * - first version to support OLMS-50
 *
 * Revision 1.33  2016/09/12 19:49:16  alfred
 * - use gettime() to get DOS time (int86 leaks memory per call)
 *
 * Revision 1.32  2016/09/11 15:39:49  alfred
 * - determine DOS time without floatig point
 *
 * Revision 1.31  2016/08/30 09:53:46  alfred
 * - make string argument const
 *
 * Revision 1.30  2015/10/23 08:43:33  alfred
 * - beef up & fix structure handling
 *
 * Revision 1.29  2015/08/05 18:28:06  alfred
 * - correct initial construction of ALLARGS, compute ALLARGS/NUMARGS only if needed
 *
 * Revision 1.28  2014/12/14 17:58:46  alfred
 * - remove static variables in strutil.c
 *
 * Revision 1.27  2014/12/05 11:58:15  alfred
 * - collapse STDC queries into one file
 *
 * Revision 1.26  2014/12/03 19:01:00  alfred
 * - remove static return value
 *
 * Revision 1.25  2014/11/28 22:02:25  alfred
 * - rework to current style
 *
 * Revision 1.24  2014/11/23 17:06:32  alfred
 * - add error #2060 (unimplemented)
 *
 * Revision 1.23  2014/11/10 13:15:13  alfred
 * - make arg of QuotPos() const
 *
 * Revision 1.22  2014/11/06 11:22:01  alfred
 * - replace hook chain for ClearUp, document new mechanism
 *
 * Revision 1.21  2014/11/05 15:47:13  alfred
 * - replace InitPass callchain with registry
 *
 * Revision 1.20  2014/10/06 17:54:56  alfred
 * - display filename if include failed
 * - some valgrind workaraounds
 *
 * Revision 1.19  2014/09/14 13:22:33  alfred
 * - ass keyword arguments
 *
 * Revision 1.18  2014/05/29 10:59:05  alfred
 * - some const cleanups
 *
 * Revision 1.17  2014/03/08 10:52:07  alfred
 * - correctly handle escaped quotation marks
 *
 * Revision 1.16  2012-08-22 20:01:45  alfred
 * - regard UTF-8
 *
 * Revision 1.15  2012-05-26 13:49:19  alfred
 * - MSP additions, make implicit macro parameters always case-insensitive
 *
 * Revision 1.14  2011-10-20 14:00:40  alfred
 * - SRP handling more graceful on Z8
 *
 * Revision 1.13  2010/05/01 17:22:02  alfred
 * - use strmov()
 *
 * Revision 1.12  2010/04/17 13:14:19  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.11  2008/11/23 10:39:16  alfred
 * - allow strings with NUL characters
 *
 * Revision 1.10  2008/08/10 11:57:48  alfred
 * - handle truncated bit numbers for 68K
 *
 * Revision 1.9  2008/01/02 22:32:21  alfred
 * - better heap checking for DOS target
 *
 * Revision 1.8  2007/11/24 22:48:02  alfred
 * - some NetBSD changes
 *
 * Revision 1.7  2007/09/24 17:51:48  alfred
 * - better handle non-printable characters
 *
 * Revision 1.6  2007/04/30 18:37:52  alfred
 * - add weird integer coding
 *
 * Revision 1.5  2006/12/09 19:54:53  alfred
 * - remove unplausible part in time computation
 *
 * Revision 1.4  2005/10/02 10:00:44  alfred
 * - ConstLongInt gets default base, correct length check on KCPSM3 registers
 *
 * Revision 1.3  2004/10/03 11:44:58  alfred
 * - addition for MinGW
 *
 * Revision 1.2  2004/05/31 15:19:26  alfred
 * - add StrCaseCmp
 *
 * Revision 1.1  2003/11/06 02:49:19  alfred
 * - recreated
 *
 * Revision 1.12  2003/10/04 15:38:47  alfred
 * - differentiate constant/variable messages
 *
 * Revision 1.11  2003/10/04 14:00:39  alfred
 * - complain about empty arguments
 *
 * Revision 1.10  2003/09/21 21:15:54  alfred
 * - fix string length
 *
 * Revision 1.9  2003/05/20 17:45:03  alfred
 * - StrSym with length spec
 *
 * Revision 1.8  2003/05/02 21:23:09  alfred
 * - strlen() updates
 *
 * Revision 1.7  2002/11/16 20:52:18  alfred
 * - added ErrMsgStructNameMissing
 *
 * Revision 1.6  2002/11/04 19:04:26  alfred
 * - prevent modification of constants with SET
 *
 * Revision 1.5  2002/08/14 18:43:47  alfred
 * - warn null allocation, remove some warnings
 *
 * Revision 1.4  2002/05/13 18:17:13  alfred
 * - added error 2010/2020
 *
 * Revision 1.3  2002/05/12 20:56:28  alfred
 * - added 3206x error messages
 *
 *****************************************************************************/


#include "stdinc.h"
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "version.h"
#include "endian.h"
#include "stdhandl.h"
#include "nls.h"
#include "nlmessages.h"
#include "as.rsc"
#include "strutil.h"
#include "stringlists.h"
#include "chunks.h"
#include "ioerrs.h"
#include "errmsg.h"
#include "asmdef.h"
#include "asmpars.h"
#include "asmdebug.h"
#include "intconsts.h"
#include "as.h"

#include "asmsub.h"


#ifdef __TURBOC__
#ifdef __DPMI16__
#define STKSIZE 40960
#else
#define STKSIZE 49152
#endif
#endif

#define VALID_S1 1
#define VALID_SN 2
#define VALID_M1 4
#define VALID_MN 8

Word ErrorCount, WarnCount;
static StringList CopyrightList, OutList, ShareOutList, ListOutList;

static LongWord StartStack, MinStack, LowStack;

static Byte *ValidSymChar;

/****************************************************************************/
/* Modulinitialisierung */

void AsmSubInit(void)
{
  PageLength = 60;
  PageWidth = 0;
  ErrorCount = 0;
  WarnCount = 0;
}

/****************************************************************************/
/* Copyrightlistenverwaltung */

void AddCopyright(char *NewLine)
{
  AddStringListLast(&CopyrightList, NewLine);
}

void WriteCopyrights(TSwitchProc NxtProc)
{
  StringRecPtr Lauf;

  if (!StringListEmpty(CopyrightList))
  {
    printf("%s\n", GetStringListFirst(CopyrightList, &Lauf));
    NxtProc();
    while (Lauf)
    {
      printf("%s\n", GetStringListNext(&Lauf));
      NxtProc();
    }
  }
}

/*--------------------------------------------------------------------------*/
/* ermittelt das erste/letzte Auftauchen eines Zeichens ausserhalb */
/* "geschuetzten" Bereichen */

char *QuotMultPos(const char *s, const char *pSearch)
{
  register ShortInt Brack = 0, AngBrack = 0;
  register const char *i;
  Boolean InSglQuot = False, InDblQuot = False;

  for (i = s; *i; i++)
  {
    if (strchr(pSearch, *i))
    {
      if (!AngBrack && !Brack && !InSglQuot && !InDblQuot)
        return (char*)i;
    }
    switch (*i)
    {
      case '"':
        if (!InSglQuot)
          InDblQuot = !InDblQuot;
        break;
      case '\'':
        if (!InDblQuot)
          InSglQuot = !InSglQuot;
        break;
      case '(':
        if (!AngBrack && !InDblQuot && !InSglQuot)
          Brack++;
        break;
      case ')':
        if (!AngBrack && !InDblQuot && !InSglQuot)
          Brack--;
        break;
      case '[':
        if (!Brack && !InDblQuot && !InSglQuot)
          AngBrack++;
        break;
      case ']':
        if (!Brack && !InDblQuot && !InSglQuot)
          AngBrack--;
        break;
    }
  }

  return NULL;
}

char *QuotPos(const char *s, char Zeichen)
{
  char Tmp[2];

  Tmp[0] = Zeichen;
  Tmp[1] = '\0';
  return QuotMultPos(s, Tmp);
}

char *RQuotPos(char *s, char Zeichen)
{
  ShortInt Brack = 0, AngBrack = 0;
  char *i;
  Boolean Quot = False, Paren = False;

  for (i = s + strlen(s) - 1; i >= s; i--)
    if (*i == Zeichen)
    {
      if ((!AngBrack) && (!Brack) && (!Paren) && (!Quot))
        return i;
    }
    else switch (*i)
    {
      case '"':
        if ((!Brack) && (!AngBrack) && (!Quot))
          Paren = !Paren;
        break;
      case '\'':
        if ((!Brack) && (!AngBrack) && (!Paren))
          Quot = !Quot;
        break;
      case ')':
        if ((!AngBrack) && (!Paren) && (!Quot))
          Brack++;
        break;
      case '(':
        if ((!AngBrack) && (!Paren) && (!Quot))
          Brack--;
        break;
      case ']':
        if ((!Brack) && (!Paren) && (!Quot))
          AngBrack++;
        break;
      case '[':
        if ((!Brack) && (!Paren) && (!Quot))
          AngBrack--;
        break;
    }

  return NULL;
}

/*--------------------------------------------------------------------------*/
/* ermittelt das erste (nicht-) Leerzeichen in einem String */

char *FirstBlank(const char *s)
{
  const char *h, *Min = NULL;

  h = strchr(s, ' ');
  if (h)
    if ((!Min) || (h < Min))
      Min = h;
  h = strchr(s, Char_HT);
  if (h)
    if ((!Min) || (h < Min))
      Min = h;
  return (char*)Min;
}

/*--------------------------------------------------------------------------*/
/* einen String in zwei Teile zerlegen */

void SplitString(char *Source, char *Left, char *Right, char *Trenner)
{
  char Save;
  LongInt slen = strlen(Source);

  if ((!Trenner) || (Trenner >= Source + slen))
    Trenner = Source + slen;
  Save = (*Trenner);
  *Trenner = '\0';
  strmov(Left, Source);
  *Trenner = Save;
  if (Trenner >= Source + slen)
    *Right = '\0';
  else
    strmov(Right, Trenner + 1);
}

/*--------------------------------------------------------------------------*/
/* verbesserte Grossbuchstabenfunktion */

/* einen String in Grossbuchstaben umwandeln.  Dabei Stringkonstanten in Ruhe */
/* lassen */

void UpString(char *s)
{
  char *z;
  int hypquot = 0;
  Boolean LastBk = FALSE, ThisBk;

  for (z = s; *z != '\0'; z++)
  {
    ThisBk = FALSE;
    switch (*z)
    {
      case '\\':
        ThisBk = TRUE;
        break;
      case '\'':
        if ((!(hypquot & 2)) && (!LastBk))
          hypquot ^= 1;
        break;
      case '"':
        if ((!(hypquot & 1)) && (!LastBk))
          hypquot ^= 2;
        break;
      default:
        if (!hypquot)
          *z = UpCaseTable[(int)*z];
    }
    LastBk = ThisBk;
  }
}

/****************************************************************************/

void TranslateString(char *s, int Length)
{
  char *pRun, *pEnd;

  if (Length < 0)
    Length = strlen(s);
  for (pRun = s, pEnd = pRun + Length; pRun < pEnd; pRun++)
    *pRun = CharTransTable[((usint)(*pRun)) & 0xff];
}

ShortInt StrCaseCmp(const char *s1, const char *s2, LongInt Hand1, LongInt Hand2)
{
  int tmp;

  tmp = mytoupper(*s1) - mytoupper(*s2);
  if (!tmp)
    tmp = strcasecmp(s1, s2);
  if (!tmp)
    tmp = Hand1 - Hand2;
  if (tmp < 0)
    return -1;
  if (tmp > 0)
    return 1;
  return 0;
}

/****************************************************************************/
/* an einen Dateinamen eine Endung anhaengen */

void AddSuffix(char *s, char *Suff)
{
  char *p, *z, *Part;

  p = NULL;
  for (z = s; *z != '\0'; z++)
    if (*z == '\\')
      p = z;
  Part = p ? p : s;
  if (!strchr(Part, '.'))
    strmaxcat(s, Suff, 255);
}


/*--------------------------------------------------------------------------*/
/* von einem Dateinamen die Endung loeschen */

void KillSuffix(char *s)
{
  char *p, *z, *Part;

  p = NULL;
  for (z = s; *z != '\0'; z++)
    if (*z == '\\')
      p = z;
  Part = p ? p : s;
  Part = strchr(Part, '.');
  if (Part)
    *Part = '\0';
}

/*--------------------------------------------------------------------------*/
/* Pfadanteil (Laufwerk+Verzeichnis) von einem Dateinamen abspalten */

char *PathPart(char *Name)
{
  static String s;
  char *p;

  strmaxcpy(s, Name, 255);

  p = strrchr(Name, PATHSEP);
#ifdef DRSEP
  if (!p)
    p = strrchr(Name, DRSEP);
#endif

  if (!p)
    *s = '\0';
  else
    s[1] = '\0';

  return s;
}

/*--------------------------------------------------------------------------*/
/* Namensanteil von einem Dateinamen abspalten */

char *NamePart(char *Name)
{
  char *p = strrchr(Name, PATHSEP);

#ifdef DRSEP
  if (!p)
    p = strrchr(Name, DRSEP);
#endif

  return p ? p + 1 : Name;
}

/****************************************************************************/
/* eine Gleitkommazahl in einen String umwandeln */

char *FloatString(Double f)
{
#define MaxLen 18
  static String s;
  char *p, *d;
  sint n, ExpVal, nzeroes;
  Boolean WithE, OK;

  /* 1. mit Maximallaenge wandeln, fuehrendes Vorzeichen weg */

  sprintf(s, "%27.15e", f);
  for (p = s; (*p == ' ') || (*p == '+'); p++);
  if (p != s)
    strmov(s, p);

  /* 2. Exponenten soweit als moeglich kuerzen, evtl. ganz streichen */

  p = strchr(s, 'e');
  if (!p)
    return s;
  switch (*(++p))
  {
    case '+':
      strmov(p, p + 1);
      break;
    case '-':
      p++;
      break;
  }

  while (*p == '0')
    strmov(p, p + 1);
  WithE = (*p != '\0');
  if (!WithE)
    s[strlen(s) - 1] = '\0';

  /* 3. Nullen am Ende der Mantisse entfernen, Komma bleibt noch */

  p = WithE ? strchr(s, 'e') : s + strlen(s);
  p--;
  while (*p == '0')
  {
    strmov(p, p + 1);
    p--;
  }

  /* 4. auf die gewuenschte Maximalstellenzahl begrenzen */

  p = WithE ? strchr(s, 'e') : s + strlen(s);
  d = strchr(s, '.');
  n = p - d - 1;

  /* 5. Maximallaenge ueberschritten ? */

  if (strlen(s) > MaxLen)
    strmov(d + (n - (strlen(s) - MaxLen)), d + n);

  /* 6. Exponentenwert berechnen */

  if (WithE)
  {
    p = strchr(s, 'e');
    ExpVal = ConstLongInt(p + 1, &OK, 10);
  }
  else
  {
    p = s + strlen(s);
    ExpVal = 0;
  }

  /* 7. soviel Platz, dass wir den Exponenten weglassen und evtl. Nullen
       anhaengen koennen ? */

  if (ExpVal > 0)
  {
    nzeroes = ExpVal - (p - strchr(s, '.') - 1); /* = Zahl von Nullen, die anzuhaengen waere */

    /* 7a. nur Kommaverschiebung erforderlich. Exponenten loeschen und
          evtl. auch Komma */

    if (nzeroes <= 0)
    {
      *p = '\0';
      d = strchr(s, '.');
      strmov(d, d + 1);
      if (nzeroes != 0)
      {
        memmove(s + strlen(s) + nzeroes + 1, s + strlen(s) + nzeroes, -nzeroes);
        s[strlen(s) - 1 + nzeroes] = '.';
      }
    }

    /* 7b. Es muessen Nullen angehaengt werden. Schauen, ob nach Loeschen von
          Punkt und E-Teil genuegend Platz ist */

    else
    {
      n = strlen(p) + 1 + (MaxLen - strlen(s)); /* = Anzahl freizubekommender Zeichen+Gutschrift */
      if (n >= nzeroes)
      {
        *p = '\0';
        d = strchr(s, '.');
        strmov(d, d + 1);
        d = s + strlen(s);
        for (n = 0; n < nzeroes; n++)
          *(d++) = '0';
        *d = '\0';
      }
    }
  }

  /* 8. soviel Platz, dass Exponent wegkann und die Zahl mit vielen Nullen
       vorne geschrieben werden kann ? */

  else if (ExpVal < 0)
  {
    n = (-ExpVal) - (strlen(p)); /* = Verlaengerung nach Operation */
    if (strlen(s) + n <= MaxLen)
    {
      *p = '\0';
      d = strchr(s, '.'); 
      strmov(d, d + 1);
      d = (s[0] == '-') ? s + 1 : s;
      memmove(d - ExpVal + 1, d, strlen(s) + 1);
      *(d++) = '0';
      *(d++) = '.';
      for (n = 0; n < -ExpVal - 1; n++)
        *(d++) = '0';
    }
  }


  /* 9. Ueberfluessiges Komma entfernen */

  if (WithE)
  {
    p = strchr(s, 'e');
    if (p)
      *p = 'E';
  }
  else
    p = s + strlen(s);
  if ((p) && (*(p - 1) == '.'))
    strmov(p - 1, p);

  return s;
}

/****************************************************************************/
/* Symbol in String wandeln */

void StrSym(TempResult *t, Boolean WithSystem, char *Dest, int DestLen)
{
  switch (t->Typ)
  {
    case TempInt:
      HexString(Dest, DestLen - 3, t->Contents.Int, 1);
      if (WithSystem)
        switch (ConstMode)
        {
          case ConstModeIntel:
            strcat(Dest, "H");
            break;
          case ConstModeMoto:
            strprep(Dest, "$");
            break;
          case ConstModeC:
            strprep(Dest, "0x");
            break;
          case ConstModeWeird :
            strprep(Dest, "x'");
            strcat(Dest, "'");
            break;
        }
      break;
    case TempFloat:
      strmaxcpy(Dest, FloatString(t->Contents.Float), DestLen);
      break;
    case TempString:
      snstrlenprint(Dest, DestLen, t->Contents.Ascii.Contents, t->Contents.Ascii.Length);
      break;
    default:
      strmaxcpy(Dest, "???", DestLen);
  }
}

/****************************************************************************/
/* Listingzaehler zuruecksetzen */

void ResetPageCounter(void)
{
  int z;

  for (z = 0; z <= ChapMax; z++)
    PageCounter[z] = 0;
  LstCounter = 0;
  ChapDepth = 0;
}

/*--------------------------------------------------------------------------*/
/* eine neue Seite im Listing beginnen */

void NewPage(ShortInt Level, Boolean WithFF)
{
  ShortInt z;
  String Header, s;
  char Save;

  if (ListOn == 0)
    return;

  LstCounter = 0;

  if (ChapDepth < (Byte) Level)
  {
    memmove(PageCounter + (Level - ChapDepth), PageCounter, (ChapDepth + 1) * sizeof(Word));
    for (z = 0; z <= Level - ChapDepth; PageCounter[z++] = 1);
    ChapDepth = Level;
  }
  for (z = 0; z <= Level - 1; PageCounter[z++] = 1);
  PageCounter[Level]++;

  if ((WithFF) && (!ListToNull))
  {
    errno = 0;
    fprintf(LstFile, "%c", Char_FF);
    ChkIO(10002);
  }

  sprintf(Header, " AS V%s%s%s",
          Version,
          getmessage(Num_HeadingFileNameLab),
          NamePart(SourceFile));
  if ((strcmp(CurrFileName, "INTERNAL"))
   && (strcmp(NamePart(CurrFileName), NamePart(SourceFile))))
  {
    strmaxcat(Header, "(", 255);
    strmaxcat(Header, NamePart(CurrFileName), 255);
    strmaxcat(Header, ")", 255);
  }
  strmaxcat(Header, getmessage(Num_HeadingPageLab), 255);

  for (z = ChapDepth; z >= 0; z--)
  {
    sprintf(s, IntegerFormat, PageCounter[z]);
    strmaxcat(Header, s, 255);
    if (z != 0)
      strmaxcat(Header, ".", 255);
  }

  strmaxcat(Header, " - ", 255);
  NLS_CurrDateString(s);
  strmaxcat(Header, s, 255);
  strmaxcat(Header, " ", 255);
  NLS_CurrTimeString(False, s);
  strmaxcat(Header, s, 255);

  if (PageWidth != 0)
    while (strlen(Header) > PageWidth)
    {
      Save = Header[PageWidth];
      Header[PageWidth] = '\0';
      if (!ListToNull)
      {
        errno = 0;
        fprintf(LstFile, "%s\n", Header);
        ChkIO(10002);
      }
      Header[PageWidth] = Save;
      strmov(Header, Header + PageWidth);
    }

  if (!ListToNull)
  {
    errno = 0;
    fprintf(LstFile, "%s\n", Header);
    ChkIO(10002);

    if (PrtTitleString[0])
    {
      errno = 0;
      fprintf(LstFile, "%s\n", PrtTitleString);
      ChkIO(10002);
    }

    errno = 0;
    fprintf(LstFile, "\n\n");
    ChkIO(10002);
  }
}


/*--------------------------------------------------------------------------*/
/* eine Zeile ins Listing schieben */

void WrLstLine(char *Line)
{
  int LLength;
  char bbuf[2500];
  String LLine;
  int blen = 0, hlen, z, Start;

  if ((ListOn == 0) || (ListToNull))
    return;

  if (PageLength == 0)
  {
    errno = 0;
    fprintf(LstFile, "%s\n", Line);
    ChkIO(10002);
  }
  else
  {
    if ((PageWidth == 0) || ((strlen(Line) << 3) < PageWidth))
      LLength = 1;
    else
    {
      blen = 0;
      for (z = 0; z < (int)strlen(Line);  z++)
        if (Line[z] == Char_HT)
        {
          memset(bbuf + blen, ' ', 8 - (blen & 7));
          blen += 8 - (blen&7);
        }
        else
          bbuf[blen++] = Line[z];
      LLength = blen / PageWidth;
      if (blen % PageWidth)
        LLength++;
    }
    if (LLength == 1)
    {
      errno = 0;
      fprintf(LstFile, "%s\n", Line);
      ChkIO(10002);
      if ((++LstCounter) == PageLength)
        NewPage(0, True);
    }
    else
    {
      Start = 0;
      for (z = 1; z <= LLength; z++)
      {
        hlen = PageWidth;
        if (blen - Start < hlen)
          hlen = blen - Start;
        memcpy(LLine, bbuf + Start, hlen);
        LLine[hlen] = '\0';
        errno = 0;
        fprintf(LstFile, "%s\n", LLine);
        if ((++LstCounter) == PageLength)
          NewPage(0, True);
        Start += hlen;
      }
    }
  }
}

/*****************************************************************************/
/* Ausdruck in Spalte vor Listing */

void SetListLineVal(TempResult *t)
{
  StrSym(t, True, ListLine, STRINGSIZE);
  strmaxprep(ListLine, "=", STRINGSIZE - 1);
  if (strlen(ListLine) > 14)
  {
    ListLine[12] = '\0';
    strmaxcat(ListLine, "..", STRINGSIZE - 1);
  }
}

/*!------------------------------------------------------------------------
 * \fn     PrintOneLineMuted(FILE *pFile, const char *pLine,
                             const struct sLineComp *pMuteComponent,
                             const struct sLineComp *pMuteComponent2)
 * \brief  print a line, with a certain component muted out (i.e. replaced by spaces)
 * \param  pFile where to write
 * \param  pLine line to print
 * \param  pMuteComponent component to mute in printout
 * ------------------------------------------------------------------------ */

static Boolean CompMatch(int Col, const struct sLineComp *pComp)
{
  return (pComp
       && (pComp->StartCol >= 0)
       && (Col >= pComp->StartCol)
       && (Col < pComp->StartCol + (int)pComp->Len));
}

void PrintOneLineMuted(FILE *pFile, const char *pLine,
                       const struct sLineComp *pMuteComponent,
                       const struct sLineComp *pMuteComponent2)
{
  int z, Len = strlen(pLine);
  Boolean Match;

  errno = 0;
  for (z = 0; z < Len; z++)
  {
    Match = CompMatch(z, pMuteComponent) || CompMatch(z, pMuteComponent2);
    fputc(Match ? ' ' : pLine[z], pFile);
  }
  fputc('\n', pFile);
  ChkIO(10002);
}

/*!------------------------------------------------------------------------
 * \fn     PrLineMarker(FILE *pFile, const char *pLine, const char *pPrefix, const char *pTrailer,
                        char Marker, const struct sLineComp *pLineComp)
 * \brief  print a line, optionally with a marking of a component below
 * \param  pFile where to write
 * \param  pLine line to print/underline
 * \param  pPrefix what to print before (under)line
 * \param  pTrailer what to print after (under)line
 * \param  Marker character to use for marking
 * \param  pLineComp position and length of optional marker
 * ------------------------------------------------------------------------ */

/* replace TABs in line with spaces - column counting counts TAB as one character */

char TabCompressed(char in)
{
  return (in == '\t') ? ' ' : (myisprint(in) ? in : '*');
}

void PrLineMarker(FILE *pFile, const char *pLine, const char *pPrefix, const char *pTrailer,
                  char Marker, const struct sLineComp *pLineComp)
{
  const char *pRun;
  int z;

  fputs(pPrefix, pFile);
  for (pRun = pLine; *pRun; pRun++)
    fputc(TabCompressed(*pRun), pFile);
  fprintf(pFile, "%s\n", pTrailer);

  if (pLineComp && (pLineComp->StartCol >= 0) && (pLineComp->Len > 0))
  {
    fputs(pPrefix, pFile);
    if (pLineComp->StartCol > 0)
      fprintf(pFile, "%*s", pLineComp->StartCol, "");
    for (z = 0; z < (int)pLineComp->Len; z++)
      fputc(Marker, pFile);
    fprintf(pFile, "%s\n", pTrailer);
  }
}

/*!------------------------------------------------------------------------
 * \fn     GenLineForMarking(char *pDest, unsigned DestSize, const char *pSrc, const char *pPrefix)
 * \brief  generate a line, in compressed form for optional marking of a component below
 * \param  pDest where to write
 * \param  DestSize destination buffer size
 * \param  pSrc line to print/underline
 * \param  pPrefix what to print before (under)line
 * ------------------------------------------------------------------------ */

void GenLineForMarking(char *pDest, unsigned DestSize, const char *pSrc, const char *pPrefix)
{
  char *pRun;

  strmaxcpy(pDest, pPrefix, DestSize);
  pRun = pDest + strlen(pDest);

  /* replace TABs in line with spaces - column counting counts TAB as one character */

  for (; *pSrc && (pRun - pDest + 1 < (int)DestSize); pSrc++)
    *pRun++ = TabCompressed(*pSrc);
  *pRun = '\0';
}

/*!------------------------------------------------------------------------
 * \fn     GenLineMarker(char *pDest, unsigned DestSize, char Marker, const struct sLineComp *pLineComp, * const char *pPrefix)
 * \brief  print a line, optionally with a marking of a component below
 * \param  pDest where to write
 * \param  DestSize destination buffer size
 * \param  Marker character to use for marking
 * \param  pLineComp position and length of optional marker
 * \param  pPrefix what to print before (under)line
 * ------------------------------------------------------------------------ */

void GenLineMarker(char *pDest, unsigned DestSize, char Marker, const struct sLineComp *pLineComp, const char *pPrefix)
{
  char *pRun;
  int z;

  strmaxcpy(pDest, pPrefix, DestSize);
  pRun = pDest + strlen(pDest);

  for (z = 0; (z < pLineComp->StartCol) && (pRun - pDest + 1 < (int)DestSize); z++)
    *pRun++ = ' ';
  for (z = 0; (z < (int)pLineComp->Len) && (pRun - pDest + 1 < (int)DestSize); z++)
    *pRun++ = Marker;
  *pRun = '\0';
}

/****************************************************************************/
/* einen Symbolnamen auf Gueltigkeit ueberpruefen */

Boolean ChkSymbName(char *sym)
{
  char *z;

  if (!(ValidSymChar[((unsigned int) *sym) & 0xff] & VALID_S1))
    return False;

  for (z = sym + 1; *z != '\0'; z++)
    if (!(ValidSymChar[((unsigned int) *z) & 0xff] & VALID_SN))
      return False;

  return True;
}

Boolean ChkMacSymbName(char *sym)
{
  char *z;

  if (!(ValidSymChar[((unsigned int) *sym) & 0xff] & VALID_M1))
    return False;

  for (z = sym + 1; *z != '\0'; z++)
    if (!(ValidSymChar[((unsigned int) *z) & 0xff] & VALID_MN))
      return False;

  return True;
}

/****************************************************************************/
/* Fehlerkanal offen ? */

static void ForceErrorOpen(void)
{
  if (!IsErrorOpen)
  {
    RewriteStandard(&ErrorFile, ErrorName);
    IsErrorOpen = True;
    if (!ErrorFile)
      ChkIO(10001);
  }
}

/*--------------------------------------------------------------------------*/
/* eine Fehlermeldung mit Klartext ausgeben */

static void EmergencyStop(void)
{
  if ((IsErrorOpen) && (ErrorFile))
    fclose(ErrorFile);
  fclose(LstFile);
  if (ShareMode != 0)
  {
    fclose(ShareFile);
    unlink(ShareName);
  }
  if (MacProOutput)
  {
    fclose(MacProFile);
    unlink(MacProName);
  }
  if (MacroOutput)
  {
    fclose(MacroFile);
    unlink(MacroName);
  }
  if (MakeDebug)
    fclose(Debug);
  if (CodeOutput)
  {
    fclose(PrgFile);
    unlink(OutName);
  }
}

void WrErrorString(char *pMessage, char *pAdd, Boolean Warning, Boolean Fatal,
                   const char *pExtendError, const struct sLineComp *pLineComp)
{
  String ErrStr[4];
  unsigned ErrStrCount = 0, z;
  char *p;
  int l;
  const char *pLeadIn = GNUErrors ? "" : "> > > ";
  FILE *pErrFile = ErrorFile ? ErrorFile : stdout;

  if (TreatWarningsAsErrors && Warning && !Fatal)
   Warning = False;

  strcpy(ErrStr[ErrStrCount], pLeadIn);
  p = GetErrorPos();
  l = strlen(p) - 1;
  if ((l >= 0) && (p[l] == ' '))
    p[l] = '\0';
  strmaxcat(ErrStr[ErrStrCount], p, 255);
  free(p);
  if (pLineComp)
  {
    char Num[20];

    sprintf(Num, ":%d", pLineComp->StartCol + 1);
    strmaxcat(ErrStr[ErrStrCount], Num, 255);
  }
  if (Warning || !GNUErrors)
  {
    strmaxcat(ErrStr[ErrStrCount], ": ", 255);
    strmaxcat(ErrStr[ErrStrCount], getmessage(Warning ? Num_WarnName : Num_ErrName), 255);
  }
  strmaxcat(ErrStr[ErrStrCount], pAdd, 255);
  strmaxcat(ErrStr[ErrStrCount], ": ", 255);
  if (Warning)
    WarnCount++;
  else
    ErrorCount++;

  strmaxcat(ErrStr[ErrStrCount], pMessage, 255);
  if ((ExtendErrors > 0) && pExtendError)
  {
    if (GNUErrors)
      strmaxcat(ErrStr[ErrStrCount], " '", 255);
    else
      strcpy(ErrStr[++ErrStrCount], pLeadIn);
    strmaxcat(ErrStr[ErrStrCount], pExtendError, 255);
    if (GNUErrors)
      strmaxcat(ErrStr[ErrStrCount], "'", 255);
  }
  if ((ExtendErrors > 1) || ((ExtendErrors > 0) && pLineComp))
  {
    strcpy(ErrStr[++ErrStrCount], "");
    GenLineForMarking(ErrStr[ErrStrCount], 255, OneLine, pLeadIn);
    if (pLineComp)
    {
      strcpy(ErrStr[++ErrStrCount], "");
      GenLineMarker(ErrStr[ErrStrCount], 255, '~', pLineComp, pLeadIn);
    }
  }

  if ((strcmp(LstName, "/dev/null")) && (!Fatal))
  {
    for (z = 0; z <= ErrStrCount; z++)
      WrLstLine(ErrStr[z]);
  }

  ForceErrorOpen();
  if (strcmp(LstName, "!1") || Fatal)
  {
    for (z = 0; z <= ErrStrCount; z++)
      fprintf(pErrFile, "%s%s\n", ErrStr[z], ClrEol);
  }

  if (Fatal)
    fprintf(pErrFile, "%s\n", getmessage(Num_ErrMsgIsFatal));
  else if (MaxErrors && (ErrorCount >= MaxErrors))
  {
    fprintf(pErrFile, "%s\n", getmessage(Num_ErrMsgTooManyErrors));
    Fatal = True;
  }

  if (Fatal)
  {
    EmergencyStop();
    exit(3);
  }
}

/*--------------------------------------------------------------------------*/
/* eine Fehlermeldung ueber Code ausgeben */

void WrXErrorPos(Word Num, const char *pExtendError, const struct sLineComp *pLineComp)
{
  String h;
  char Add[11];
  int msgno;

  if ((!CodeOutput) && (Num == 1200))
    return;

  if ((SuppWarns) && (Num < 1000))
    return;

  switch (Num)
  {
    case ErrNum_UselessDisp:
      msgno = Num_ErrMsgUselessDisp; break;
    case ErrNum_ShortAddrPossible:
      msgno = Num_ErrMsgShortAddrPossible; break;
    case ErrNum_ShortJumpPossible:
      msgno = Num_ErrMsgShortJumpPossible; break;
    case ErrNum_NoShareFile:
      msgno = Num_ErrMsgNoShareFile; break;
    case ErrNum_BigDecFloat:
      msgno = Num_ErrMsgBigDecFloat; break;
    case ErrNum_PrivOrder:
      msgno = Num_ErrMsgPrivOrder; break;
    case ErrNum_DistNull:
      msgno = Num_ErrMsgDistNull; break;
    case ErrNum_WrongSegment:
      msgno = Num_ErrMsgWrongSegment; break;
    case ErrNum_InAccSegment:
      msgno = Num_ErrMsgInAccSegment; break;
    case ErrNum_PhaseErr:
      msgno = Num_ErrMsgPhaseErr; break;
    case ErrNum_Overlap:
      msgno = Num_ErrMsgOverlap; break;
    case ErrNum_NoCaseHit:
      msgno = Num_ErrMsgNoCaseHit; break;
    case ErrNum_InAccPage:
      msgno = Num_ErrMsgInAccPage; break;
    case ErrNum_RMustBeEven:
      msgno = Num_ErrMsgRMustBeEven; break;
    case ErrNum_Obsolete:
      msgno = Num_ErrMsgObsolete; break;
    case ErrNum_Unpredictable:
      msgno = Num_ErrMsgUnpredictable; break;
    case ErrNum_AlphaNoSense:
      msgno = Num_ErrMsgAlphaNoSense; break;
    case ErrNum_Senseless:
      msgno = Num_ErrMsgSenseless; break;
    case ErrNum_RepassUnknown:
      msgno = Num_ErrMsgRepassUnknown; break;
    case  ErrNum_AddrNotAligned:
      msgno = Num_ErrMsgAddrNotAligned; break;
    case ErrNum_IOAddrNotAllowed:
      msgno = Num_ErrMsgIOAddrNotAllowed; break;
    case ErrNum_Pipeline:
      msgno = Num_ErrMsgPipeline; break;
    case ErrNum_DoubleAdrRegUse:
      msgno = Num_ErrMsgDoubleAdrRegUse; break;
    case ErrNum_NotBitAddressable:
      msgno = Num_ErrMsgNotBitAddressable; break;
    case ErrNum_StackNotEmpty:
      msgno = Num_ErrMsgStackNotEmpty; break;
    case ErrNum_NULCharacter:
      msgno = Num_ErrMsgNULCharacter; break;
    case ErrNum_PageCrossing:
      msgno = Num_ErrMsgPageCrossing; break;
    case ErrNum_WOverRange:
      msgno = Num_ErrMsgWOverRange; break;
    case ErrNum_NegDUP:
      msgno = Num_ErrMsgNegDUP; break;
    case ErrNum_ConvIndX:
      msgno = Num_ErrMsgConvIndX; break;
    case ErrNum_NullResMem:
      msgno = Num_ErrMsgNullResMem; break;
    case ErrNum_BitNumberTruncated:
      msgno = Num_ErrMsgBitNumberTruncated; break;
    case ErrNum_InvRegisterPointer:
      msgno = Num_ErrMsgInvRegisterPointer; break;
    case ErrNum_MacArgRedef:
      msgno = Num_ErrMsgMacArgRedef; break;
    case ErrNum_Deprecated:
      msgno = Num_ErrMsgDeprecated; break;
    case ErrNum_SrcLEThanDest:
      msgno = Num_ErrMsgSrcLEThanDest; break;
    case ErrNum_TrapValidInstruction:
      msgno = Num_ErrMsgTrapValidInstruction; break;
    case ErrNum_DoubleDef:
      msgno = Num_ErrMsgDoubleDef; break;
    case ErrNum_SymbolUndef:
      msgno = Num_ErrMsgSymbolUndef; break;
    case ErrNum_InvSymName:
      msgno = Num_ErrMsgInvSymName; break;
    case ErrNum_InvFormat:
      msgno = Num_ErrMsgInvFormat; break;
    case ErrNum_UseLessAttr:
      msgno = Num_ErrMsgUseLessAttr; break;
    case ErrNum_TooLongAttr:
      msgno = Num_ErrMsgTooLongAttr; break;
    case ErrNum_UndefAttr:
      msgno = Num_ErrMsgUndefAttr; break;
    case ErrNum_WrongArgCnt:
      msgno = Num_ErrMsgWrongArgCnt; break;
    case ErrNum_CannotSplitArg:
      msgno = Num_ErrMsgCannotSplitArg; break;
    case ErrNum_WrongOptCnt:
      msgno = Num_ErrMsgWrongOptCnt; break;
    case ErrNum_OnlyImmAddr:
      msgno = Num_ErrMsgOnlyImmAddr; break;
    case ErrNum_InvOpsize:
      msgno = Num_ErrMsgInvOpsize; break;
    case ErrNum_ConfOpSizes:
      msgno = Num_ErrMsgConfOpSizes; break;
    case ErrNum_UndefOpSizes:
      msgno = Num_ErrMsgUndefOpSizes; break;
    case ErrNum_InvOpType:
      msgno = Num_ErrMsgInvOpType; break;
    case ErrNum_TooMuchArgs:
      msgno = Num_ErrMsgTooMuchArgs; break;
    case ErrNum_NoRelocs:
      msgno = Num_ErrMsgNoRelocs; break;
    case ErrNum_UnresRelocs:
      msgno = Num_ErrMsgUnresRelocs; break;
    case ErrNum_Unexportable:
      msgno = Num_ErrMsgUnexportable; break;
    case ErrNum_UnknownOpcode:
      msgno = Num_ErrMsgUnknownOpcode; break;
    case ErrNum_BrackErr:
      msgno = Num_ErrMsgBrackErr; break;
    case ErrNum_DivByZero:
      msgno = Num_ErrMsgDivByZero; break;
    case ErrNum_UnderRange:
      msgno = Num_ErrMsgUnderRange; break;
    case ErrNum_OverRange:
      msgno = Num_ErrMsgOverRange; break;
    case ErrNum_NotAligned:
      msgno = Num_ErrMsgNotAligned; break;
    case ErrNum_DistTooBig:
      msgno = Num_ErrMsgDistTooBig; break;
    case ErrNum_InAccReg:
      msgno = Num_ErrMsgInAccReg; break;
    case ErrNum_NoShortAddr:
      msgno = Num_ErrMsgNoShortAddr; break;
    case ErrNum_InvAddrMode:
      msgno = Num_ErrMsgInvAddrMode; break;
    case ErrNum_MustBeEven:
      msgno = Num_ErrMsgMustBeEven; break;
    case ErrNum_InvParAddrMode:
      msgno = Num_ErrMsgInvParAddrMode; break;
    case ErrNum_UndefCond:
      msgno = Num_ErrMsgUndefCond; break;
    case ErrNum_IncompCond:
      msgno = Num_ErrMsgIncompCond; break;
    case ErrNum_JmpDistTooBig:
      msgno = Num_ErrMsgJmpDistTooBig; break;
    case ErrNum_DistIsOdd:
      msgno = Num_ErrMsgDistIsOdd; break;
    case ErrNum_InvShiftArg:
      msgno = Num_ErrMsgInvShiftArg; break;
    case ErrNum_Range18:
      msgno = Num_ErrMsgRange18; break;
    case ErrNum_ShiftCntTooBig:
      msgno = Num_ErrMsgShiftCntTooBig; break;
    case ErrNum_InvRegList:
      msgno = Num_ErrMsgInvRegList; break;
    case ErrNum_InvCmpMode:
      msgno = Num_ErrMsgInvCmpMode; break;
    case ErrNum_InvCPUType:
      msgno = Num_ErrMsgInvCPUType; break;
    case ErrNum_InvCtrlReg:
      msgno = Num_ErrMsgInvCtrlReg; break;
    case ErrNum_InvReg:
      msgno = Num_ErrMsgInvReg; break;
    case ErrNum_DoubleReg:
      msgno = Num_ErrMsgDoubleReg; break;
    case ErrNum_NoSaveFrame:
      msgno = Num_ErrMsgNoSaveFrame; break;
    case ErrNum_NoRestoreFrame:
      msgno = Num_ErrMsgNoRestoreFrame; break;
    case ErrNum_UnknownMacArg:
      msgno = Num_ErrMsgUnknownMacArg; break;
    case ErrNum_MissEndif:
      msgno = Num_ErrMsgMissEndif; break;
    case ErrNum_InvIfConst:
      msgno = Num_ErrMsgInvIfConst; break;
    case ErrNum_DoubleSection:
      msgno = Num_ErrMsgDoubleSection; break;
    case ErrNum_InvSection:
      msgno = Num_ErrMsgInvSection; break;
    case ErrNum_MissingEndSect:
      msgno = Num_ErrMsgMissingEndSect; break;
    case ErrNum_WrongEndSect:
      msgno = Num_ErrMsgWrongEndSect; break;
    case ErrNum_NotInSection:
      msgno = Num_ErrMsgNotInSection; break;
    case ErrNum_UndefdForward:
      msgno = Num_ErrMsgUndefdForward; break;
    case ErrNum_ContForward:
      msgno = Num_ErrMsgContForward; break;
    case ErrNum_InvFuncArgCnt:
      msgno = Num_ErrMsgInvFuncArgCnt; break;
    case ErrNum_MsgMissingLTORG:
      msgno = Num_ErrMsgMissingLTORG; break;
    case ErrNum_InstructionNotSupported:
      msgno = -1;
      sprintf(h, getmessage(Num_ErrMsgInstructionNotOnThisCPUSupported), MomCPUIdent);
      break;
    case ErrNum_FPUNotEnabled: msgno = Num_ErrMsgFPUNotEnabled; break;
    case ErrNum_PMMUNotEnabled: msgno = Num_ErrMsgPMMUNotEnabled; break;
    case ErrNum_FullPMMUNotEnabled: msgno = Num_ErrMsgFullPMMUNotEnabled; break;
    case ErrNum_Z80SyntaxNotEnabled: msgno = Num_ErrMsgZ80SyntaxNotEnabled; break;
    case ErrNum_AddrModeNotSupported:
      msgno = -1;
      sprintf(h, getmessage(Num_ErrMsgAddrModeNotOnThisCPUSupported), MomCPUIdent);
      break;
    case ErrNum_InvBitPos:
      msgno = Num_ErrMsgInvBitPos; break;
    case ErrNum_OnlyOnOff:
      msgno = Num_ErrMsgOnlyOnOff; break;
    case ErrNum_StackEmpty:
      msgno = Num_ErrMsgStackEmpty; break;
    case ErrNum_NotOneBit:
      msgno = Num_ErrMsgNotOneBit; break;
    case ErrNum_MissingStruct:
      msgno = Num_ErrMsgMissingStruct; break;
    case ErrNum_OpenStruct:
      msgno = Num_ErrMsgOpenStruct; break;
    case ErrNum_WrongStruct:
      msgno = Num_ErrMsgWrongStruct; break;
    case ErrNum_PhaseDisallowed:
      msgno = Num_ErrMsgPhaseDisallowed; break;
    case ErrNum_InvStructDir:
      msgno = Num_ErrMsgInvStructDir; break;
    case ErrNum_DoubleStruct:
      msgno = Num_ErrMsgDoubleStruct; break;
    case ErrNum_UnresolvedStructRef:
      msgno = Num_ErrMsgUnresolvedStructRef; break;
    case ErrNum_NotRepeatable:
      msgno = Num_ErrMsgNotRepeatable; break;
    case ErrNum_ShortRead:
      msgno = Num_ErrMsgShortRead; break;
    case ErrNum_UnknownCodepage:
      msgno = Num_ErrMsgUnknownCodepage; break;
    case ErrNum_RomOffs063:
      msgno = Num_ErrMsgRomOffs063; break;
    case ErrNum_InvFCode:
      msgno = Num_ErrMsgInvFCode; break;
    case ErrNum_InvFMask:
      msgno = Num_ErrMsgInvFMask; break;
    case ErrNum_InvMMUReg:
      msgno = Num_ErrMsgInvMMUReg; break;
    case ErrNum_Level07:
      msgno = Num_ErrMsgLevel07; break;
    case ErrNum_InvBitMask:
      msgno = Num_ErrMsgInvBitMask; break;
    case ErrNum_InvRegPair:
      msgno = Num_ErrMsgInvRegPair; break;
    case ErrNum_OpenMacro:
      msgno = Num_ErrMsgOpenMacro; break;
    case ErrNum_OpenIRP:
      msgno = Num_ErrMsgOpenIRP; break;
    case ErrNum_OpenIRPC:
      msgno = Num_ErrMsgOpenIRPC; break;
    case ErrNum_OpenREPT:
      msgno = Num_ErrMsgOpenREPT; break;
    case ErrNum_OpenWHILE:
      msgno = Num_ErrMsgOpenWHILE; break;
    case ErrNum_EXITMOutsideMacro:
      msgno = Num_ErrMsgEXITMOutsideMacro; break;
    case ErrNum_TooManyMacParams:
      msgno = Num_ErrMsgTooManyMacParams; break;
    case ErrNum_UndefKeyArg:
      msgno = Num_ErrMsgUndefKeyArg; break;
    case ErrNum_NoPosArg:
      msgno = Num_ErrMsgNoPosArg; break;
    case ErrNum_DoubleMacro:
      msgno = Num_ErrMsgDoubleMacro; break;
    case ErrNum_FirstPassCalc:
      msgno = Num_ErrMsgFirstPassCalc; break;
    case ErrNum_TooManyNestedIfs:
      msgno = Num_ErrMsgTooManyNestedIfs; break;
    case ErrNum_MissingIf:
      msgno = Num_ErrMsgMissingIf; break;
    case ErrNum_RekMacro:
      msgno = Num_ErrMsgRekMacro; break;
    case ErrNum_UnknownFunc:
      msgno = Num_ErrMsgUnknownFunc; break;
    case ErrNum_InvFuncArg:
      msgno = Num_ErrMsgInvFuncArg; break;
    case ErrNum_FloatOverflow:
      msgno = Num_ErrMsgFloatOverflow; break;
    case ErrNum_InvArgPair:
      msgno = Num_ErrMsgInvArgPair; break;
    case ErrNum_NotOnThisAddress:
      msgno = Num_ErrMsgNotOnThisAddress; break;
    case ErrNum_NotFromThisAddress:
      msgno = Num_ErrMsgNotFromThisAddress; break;
    case ErrNum_TargOnDiffPage:
      msgno = Num_ErrMsgTargOnDiffPage; break;
    case ErrNum_CodeOverflow:
      msgno = Num_ErrMsgCodeOverflow; break;
    case ErrNum_AdrOverflow:
      msgno = Num_ErrMsgAdrOverflow; break;
    case ErrNum_MixDBDS:
      msgno = Num_ErrMsgMixDBDS; break;
    case ErrNum_NotInStruct:
      msgno = Num_ErrMsgNotInStruct; break;
    case ErrNum_ParNotPossible:
      msgno = Num_ErrMsgParNotPossible; break;
    case ErrNum_InvSegment:
      msgno = Num_ErrMsgInvSegment; break;
    case ErrNum_UnknownSegment:
      msgno = Num_ErrMsgUnknownSegment; break;
    case ErrNum_UnknownSegReg:
      msgno = Num_ErrMsgUnknownSegReg; break;
    case ErrNum_InvString:
      msgno = Num_ErrMsgInvString; break;
    case ErrNum_InvRegName:
      msgno = Num_ErrMsgInvRegName; break;
    case ErrNum_InvArg:
      msgno = Num_ErrMsgInvArg; break;
    case ErrNum_NoIndir:
      msgno = Num_ErrMsgNoIndir; break;
    case ErrNum_NotInThisSegment:
      msgno = Num_ErrMsgNotInThisSegment; break;
    case ErrNum_NotInMaxmode:
      msgno = Num_ErrMsgNotInMaxmode; break;
    case ErrNum_OnlyInMaxmode:
      msgno = Num_ErrMsgOnlyInMaxmode; break;
    case ErrNum_PackCrossBoundary:
      msgno = Num_ErrMsgPackCrossBoundary; break;
    case ErrNum_UnitMultipleUsed:
      msgno = Num_ErrMsgUnitMultipleUsed; break;
    case ErrNum_MultipleLongRead:
      msgno = Num_ErrMsgMultipleLongRead; break;
    case ErrNum_MultipleLongWrite:
      msgno = Num_ErrMsgMultipleLongWrite; break;
    case ErrNum_LongReadWithStore:
      msgno = Num_ErrMsgLongReadWithStore; break;
    case ErrNum_TooManyRegisterReads:
      msgno = Num_ErrMsgTooManyRegisterReads; break;
    case ErrNum_OverlapDests:
      msgno = Num_ErrMsgOverlapDests; break;
    case ErrNum_TooManyBranchesInExPacket:
      msgno = Num_ErrMsgTooManyBranchesInExPacket; break;
    case ErrNum_CannotUseUnit:
      msgno = Num_ErrMsgCannotUseUnit; break;
    case ErrNum_InvEscSequence:
      msgno = Num_ErrMsgInvEscSequence; break;
    case ErrNum_InvPrefixCombination:
      msgno = Num_ErrMsgInvPrefixCombination; break;
    case ErrNum_ConstantRedefinedAsVariable:
      msgno = Num_ErrMsgConstantRedefinedAsVariable; break;
    case ErrNum_VariableRedefinedAsConstant:
      msgno = Num_ErrMsgVariableRedefinedAsConstant; break;
    case ErrNum_StructNameMissing:
      msgno = Num_ErrMsgStructNameMissing; break;
    case ErrNum_EmptyArgument:
      msgno = Num_ErrMsgEmptyArgument; break;
    case ErrNum_Unimplemented:
      msgno = Num_ErrMsgUnimplemented; break;
    case ErrNum_FreestandingUnnamedStruct:
      msgno = Num_ErrMsgFreestandingUnnamedStruct; break;
    case ErrNum_STRUCTEndedByENDUNION:
      msgno = Num_ErrMsgSTRUCTEndedByENDUNION; break;
    case ErrNum_AddrOnDifferentPage:
      msgno = Num_ErrMsgAddrOnDifferentPage; break;
    case ErrNum_UnknownMacExpMod:
      msgno = Num_ErrMsgUnknownMacExpMod; break;
    case ErrNum_ConflictingMacExpMod:
      msgno = Num_ErrMsgConflictingMacExpMod; break;
    case ErrNum_InvalidPrepDir:
      msgno = Num_ErrMsgInvalidPrepDir; break;
    case ErrNum_InternalError:
      msgno = Num_ErrMsgInternalError; break;
    case ErrNum_OpeningFile:
      msgno = Num_ErrMsgOpeningFile; break;
    case ErrNum_ListWrError:
      msgno = Num_ErrMsgListWrError; break;
    case ErrNum_FileReadError:
      msgno = Num_ErrMsgFileReadError; break;
    case ErrNum_FileWriteError:
      msgno = Num_ErrMsgFileWriteError; break;
    case ErrNum_HeapOvfl:
      msgno = Num_ErrMsgHeapOvfl; break;
    case ErrNum_StackOvfl:
      msgno = Num_ErrMsgStackOvfl; break;
    default  : msgno = -1;
               sprintf(h, "%s %d", getmessage(Num_ErrMsgIntError), (int) Num);
  }
  if (msgno != -1)
    strmaxcpy(h, getmessage(msgno), 255);

  if (((Num == ErrNum_TargOnDiffPage) || (Num == ErrNum_JmpDistTooBig))
   && !Repass)
    JmpErrors++;

  if (NumericErrors)
    sprintf(Add, " #%d", (int)Num);
  else
    *Add = '\0';
  WrErrorString(h, Add, Num < 1000, Num >= 10000, pExtendError, pLineComp);
}

void WrStrErrorPos(Word Num, const struct sStrComp *pStrComp)
{
  WrXErrorPos(Num, pStrComp->Str, &pStrComp->Pos);
}

void WrError(Word Num)
{
  WrXErrorPos(Num, NULL, NULL);
}

void WrXError(Word Num, const char *pExtError)
{
  WrXErrorPos(Num, pExtError, NULL);
}

/*--------------------------------------------------------------------------*/
/* I/O-Fehler */

void ChkIO(Word ErrNo)
{
  int io;

  io = errno;
  if ((io == 0) || (io == 19) || (io == 25))
    return;

  WrXError(ErrNo, GetErrorMsg(io));
}

void ChkStrIO(Word ErrNo, const struct sStrComp *pComp)
{
  int io;
  String s;

  io = errno;
  if ((io == 0) || (io == 19) || (io == 25))
    return;

  strmaxcpy(s, pComp->Str, 255);
  strmaxcat(s, ": ", 255);
  strmaxcat(s, GetErrorMsg(io), 255);
  WrStrErrorPos(ErrNo, pComp);
}

/****************************************************************************/

LargeWord ProgCounter(void)
{
  return PCs[ActPC];
}

/*--------------------------------------------------------------------------*/
/* aktuellen Programmzaehler mit Phasenverschiebung holen */

LargeWord EProgCounter(void)
{
  return PCs[ActPC] + Phases[ActPC];
}

/*--------------------------------------------------------------------------*/
/* Granularitaet des aktuellen Segments holen */

Word Granularity(void)
{
  return Grans[ActPC];
}

/*--------------------------------------------------------------------------*/
/* Linstingbreite des aktuellen Segments holen */

Word ListGran(void)
{
  return ListGrans[ActPC];
}

/*--------------------------------------------------------------------------*/
/* pruefen, ob alle Symbole einer Formel im korrekten Adressraum lagen */

void ChkSpace(Byte Space)
{
  Byte Mask = 0xff - (1 << Space);

  if ((TypeFlag&Mask) != 0) WrError(ErrNum_WrongSegment);
}

/****************************************************************************/
/* eine Chunkliste im Listing ausgeben & Speicher loeschen */

void PrintChunk(ChunkList *NChunk, DissectBitProc Dissect, int ItemsPerLine)
{
  LargeWord NewMin, FMin;
  Boolean Found;
  Word p = 0, z;
  int BufferZ;
  String BufferS;
  int MaxItemLen = 79 / ItemsPerLine;

  NewMin = 0;
  BufferZ = 0;
  *BufferS = '\0';

  do
  {
    /* niedrigsten Start finden, der ueberhalb des letzten Endes liegt */

    Found = False;
    FMin = INTCONST_ffffffff;
    for (z = 0; z < NChunk->RealLen; z++)
      if (NChunk->Chunks[z].Start >= NewMin)
        if (FMin > NChunk->Chunks[z].Start)
        {
          Found = True;
          FMin = NChunk->Chunks[z].Start;
          p = z;
        }

    if (Found)
    {
      char Num[30];

      Dissect(Num, sizeof(Num), NChunk->Chunks[p].Start);
      strmaxcat(BufferS, Num, 255);
      if (NChunk->Chunks[p].Length != 1)
      {
        strmaxcat(BufferS, "-", 255);
        Dissect(Num, sizeof(Num), NChunk->Chunks[p].Start + NChunk->Chunks[p].Length - 1);
        strmaxcat(BufferS, Num, 255);
      }
      strmaxcat(BufferS, Blanks(MaxItemLen - strlen(BufferS) % MaxItemLen), 255);
      if (++BufferZ == ItemsPerLine)
      {
        WrLstLine(BufferS);
        *BufferS = '\0';
        BufferZ = 0;
      }
      NewMin = NChunk->Chunks[p].Start + NChunk->Chunks[p].Length;
    }
  }
  while (Found);

  if (BufferZ != 0)
    WrLstLine(BufferS);
}

/*--------------------------------------------------------------------------*/
/* Listen ausgeben */

void PrintUseList(void)
{
  int z, z2, l;
  String s;

  for (z = 1; z <= PCMax; z++)
    if (SegChunks[z].Chunks)
    {
      sprintf(s, "  %s%s%s", getmessage(Num_ListSegListHead1), SegNames[z],
                             getmessage(Num_ListSegListHead2));
      WrLstLine(s);
      strcpy(s, "  ");
      l = strlen(SegNames[z]) + strlen(getmessage(Num_ListSegListHead1)) + strlen(getmessage(Num_ListSegListHead2));
      for (z2 = 0; z2 < l; z2++)
        strmaxcat(s, "-", 255);
      WrLstLine(s);
      WrLstLine("");
      PrintChunk(SegChunks + z,
                 (z == SegBData) ? DissectBit : Default_DissectBit,
                 (z == SegBData) ? 3 : 4);
      WrLstLine("");
    }
}

void ClearUseList(void)
{
  int z;

  for (z = 1; z <= PCMax; z++)
    ClearChunk(SegChunks + z);
}

/****************************************************************************/
/* Include-Pfadlistenverarbeitung */

static char *GetPath(char *Acc)
{
  char *p;
  static String tmp;

  p = strchr(Acc, DIRSEP);
  if (!p)
  {
    strmaxcpy(tmp, Acc, 255);
    Acc[0] = '\0';
  }
  else
  {
    *p = '\0';
    strmaxcpy(tmp, Acc, 255);
    strmov(Acc, p + 1);
  }
  return tmp;
}

void AddIncludeList(char *NewPath)
{
  String Test;

  strmaxcpy(Test, IncludeList, 255);
  while (*Test != '\0')
    if (!strcmp(GetPath(Test), NewPath))
      return;
  if (*IncludeList != '\0')
    strmaxprep(IncludeList, SDIRSEP, 255);
  strmaxprep(IncludeList, NewPath, 255);
}


void RemoveIncludeList(char *RemPath)
{
  String Save;
  char *Part;

  strmaxcpy(IncludeList, Save, 255);
  IncludeList[0] = '\0';
  while (Save[0] != '\0')
  {
    Part = GetPath(Save);
    if (strcmp(Part, RemPath))
    {
      if (IncludeList[0] != '\0')
        strmaxcat(IncludeList, SDIRSEP, 255);
      strmaxcat(IncludeList, Part, 255);
    }
  }
}

/****************************************************************************/
/* Listen mit Ausgabedateien */

void ClearOutList(void)
{
  ClearStringList(&OutList);
}

void AddToOutList(const char *NewName)
{
  AddStringListLast(&OutList, NewName);
}

void RemoveFromOutList(const char *OldName)
{
  RemoveStringList(&OutList, OldName);
}

char *GetFromOutList(void)
{
  return GetAndCutStringList(&OutList);
}

void ClearShareOutList(void)
{
  ClearStringList(&ShareOutList);
}

void AddToShareOutList(const char *NewName)
{
  AddStringListLast(&ShareOutList, NewName);
}

void RemoveFromShareOutList(const char *OldName)
{
  RemoveStringList(&ShareOutList, OldName);
}

char *GetFromShareOutList(void)
{
  return GetAndCutStringList(&ShareOutList);
}

void ClearListOutList(void)
{
  ClearStringList(&ListOutList);
}

void AddToListOutList(const char *NewName)
{
  AddStringListLast(&ListOutList, NewName);
}

void RemoveFromListOutList(const char *OldName)
{
  RemoveStringList(&ListOutList, OldName);
}

char *GetFromListOutList(void)
{
  return GetAndCutStringList(&ListOutList);
}

/****************************************************************************/
/* Tokenverarbeitung */

static Boolean CompressLine_NErl(char ch)
{
  return (((ch >= 'A') && (ch <= 'Z'))
       || ((ch >= 'a') && (ch <= 'z'))
       || ((ch >= '0') && (ch <= '9')));
}

int CompressLine(char *TokNam, Byte Num, char *Line, Boolean ThisCaseSensitive)
{
  int z, e, tlen, llen;
  Boolean cmpres;
  int NumCompress = 0;

  z = 0;
  tlen = strlen(TokNam);
  llen = strlen(Line);
  while (z <= llen - tlen)
  {
    e = z + tlen;
    cmpres = ThisCaseSensitive ? strncmp(Line + z, TokNam, tlen) : strncasecmp(Line + z, TokNam, tlen);
    if ((!cmpres)
     && ((z == 0) || (!CompressLine_NErl(Line[z - 1])))
     && ((e >= (int)strlen(Line)) || (!CompressLine_NErl(Line[e]))))
    {
      strmov(Line + z + 1, Line + e);
      Line[z] = Num;
      llen = strlen(Line);
      NumCompress++;
    }
    z++;
  }
  return NumCompress;
}

void ExpandLine(char *TokNam, Byte Num, char *Line)
{
  char *z;

  do
  {
    z = strchr(Line, Num);
    if (z)
    {
      strmov(z, z + 1);
      strmaxins(Line, TokNam, z - Line, 255);
    }
  }
  while (z != 0);
}

void KillCtrl(char *Line)
{
  char *z;

  if (*(z = Line) == '\0')
    return;
  do
  {
    if (*z == '\0');
    else if (*z == Char_HT)
    {
      strmov(z, z + 1);
      strprep(z, Blanks(8 - ((z - Line) % 8)));
    }
    else if ((*z & 0xe0) == 0)
      *z = ' ';
    z++;
  }
  while (*z != '\0');
}

/****************************************************************************/
/* Buchhaltung */

void BookKeeping(void)
{
  if (MakeUseList)
    if (AddChunk(SegChunks + ActPC, ProgCounter(), CodeLen, ActPC == SegCode))
      WrError(ErrNum_Overlap);
  if (DebugMode != DebugNone)
  {
    AddSectionUsage(ProgCounter(), CodeLen);
    AddLineInfo(InMacroFlag, CurrLine, CurrFileName, ActPC, PCs[ActPC], CodeLen);
  }
}

/****************************************************************************/
/* Differenz zwischen zwei Zeiten mit Jahresueberlauf berechnen */

long DTime(long t1, long t2)
{
  LongInt d;

  d = t2 - t1;
  if (d < 0) d += (24*360000);
  return (d > 0) ? d : -d;
}

/*--------------------------------------------------------------------------*/
/* Init/Deinit passes */

typedef struct sProcStore
{
  struct sProcStore *pNext;
  SimpProc Proc;
} tProcStore;

static tProcStore *pInitPassProcStore = NULL,
                  *pClearUpProcStore = NULL;

void InitPass(void)
{
  tProcStore *pStore;

  for (pStore = pInitPassProcStore; pStore; pStore = pStore->pNext)
    pStore->Proc();
}

void ClearUp(void)
{
  tProcStore *pStore;

  for (pStore = pClearUpProcStore; pStore; pStore = pStore->pNext)
    pStore->Proc();
}

void AddInitPassProc(SimpProc NewProc)
{
  tProcStore *pNewStore = calloc(1, sizeof(*pNewStore));

  pNewStore->pNext = pInitPassProcStore;
  pNewStore->Proc = NewProc;
  pInitPassProcStore = pNewStore;
}

void AddClearUpProc(SimpProc NewProc)
{
  tProcStore *pNewStore = calloc(1, sizeof(*pNewStore));

  pNewStore->pNext = pClearUpProcStore;
  pNewStore->Proc = NewProc;
  pClearUpProcStore = pNewStore;
}

/*--------------------------------------------------------------------------*/
/* Zeit holen */

#ifdef __MSDOS__

#include <dos.h>

long GTime(void)
{
  struct time tbuf;
  long result;

  gettime(&tbuf);
  result = tbuf.ti_hour;
  result = (result * 60) + tbuf.ti_min;
  result = (result * 60) + tbuf.ti_sec;
  result = (result * 100) + tbuf.ti_hund;
  return result;
}

#elif __IBMC__

#include <time.h>
#define INCL_DOSDATETIME
#include <os2.h>

long GTime(void)
{
  DATETIME dt;
  struct tm ts;
  DosGetDateTime(&dt);
  memset(&ts, 0, sizeof(ts));
  ts.tm_year = dt.year - 1900;
  ts.tm_mon  = dt.month - 1;
  ts.tm_mday = dt.day;
  ts.tm_hour = dt.hours;
  ts.tm_min  = dt.minutes;
  ts.tm_sec  = dt.seconds;
  return (mktime(&ts) * 100) + (dt.hundredths);
}

#elif __MINGW32__

/* distribution by Gunnar Wallmann */

#include <sys/time.h>

/*time from 1 Jan 1601 to 1 Jan 1970 in 100ns units */
#define _W32_FT_OFFSET (116444736000000000LL)

typedef struct _FILETIME
{
  unsigned long dwLowDateTime;
  unsigned long dwHighDateTime;
} FILETIME;

void __stdcall GetSystemTimeAsFileTime(FILETIME*);

long GTime(void)
{
  union
  {
    long long ns100; /*time since 1 Jan 1601 in 100ns units */
    FILETIME ft;
  } _now;

  GetSystemTimeAsFileTime(&(_now.ft));
  return
      (_now.ns100 - _W32_FT_OFFSET) / 100000LL
#if 0
      + ((_now.ns100 / 10) % 1000000LL) / 10000
#endif
      ;
}

#else

#include <sys/time.h>

long GTime(void)
{
  struct timeval tv;

  gettimeofday(&tv, NULL);
  return (tv.tv_sec*100) + (tv.tv_usec/10000);
}

#endif

/*-------------------------------------------------------------------------*/
/* Stackfehler abfangen - bis auf DOS nur Dummies */

#ifdef __TURBOC__

#ifdef __DPMI16__
#else
unsigned _stklen = STKSIZE;
unsigned _ovrbuffer = 64*48;
#endif
#include <malloc.h>

void ChkStack(void)
{
  LongWord avail = stackavail();
  if (avail < MinStack)
    WrError(ErrNum_StackOvfl);
  if (avail < LowStack)
    LowStack = avail;
}

void ResetStack(void)
{
  LowStack = stackavail();
}

LongWord StackRes(void)
{
  return LowStack - MinStack;
}
#endif /* __TURBOC__ */

#ifdef CKMALLOC
#undef malloc
#undef realloc

void *ckmalloc(size_t s)
{
  void *tmp;

#ifdef __TURBOC__
  if (coreleft() < HEAPRESERVE + s)
    WrError(ErrNum_HeapOvfl);
#endif

  tmp = malloc(s);
  if (!tmp && (s > 0))
    WrError(ErrNum_HeapOvfl);
  return tmp;
}

void *ckrealloc(void *p, size_t s)
{
  void *tmp;

#ifdef __TURBOC__
  if (coreleft() < HEAPRESERVE + s)
    WrError(ErrNum_HeapOvfl);
#endif

  tmp = realloc(p, s);
  if (!tmp)
    WrError(ErrNum_HeapOvfl);
  return tmp;
}
#endif

void asmsub_init(void)
{
  Word z;

#ifdef __TURBOC__
#ifdef __MSDOS__
#ifdef __DPMI16__
  char *MemFlag, *p;
  String MemVal, TempName;
  unsigned long FileLen;
#else
  char *envval;
  int ovrerg;
#endif
#endif
#endif

  InitStringList(&CopyrightList);
  InitStringList(&OutList);
  InitStringList(&ShareOutList);
  InitStringList(&ListOutList);

#ifdef __TURBOC__
#ifdef __MSDOS__
#ifdef __DPMI16__
  /* Fuer DPMI evtl. Swapfile anlegen */

  MemFlag = getenv("ASXSWAP");
  if (MemFlag)
  {
    strmaxcpy(MemVal, MemFlag, 255);
    p = strchr(MemVal, ',');
    if (!p)
      strcpy(TempName, "ASX.TMP");
    else
    {
      *p = NULL;
      strcpy(TempName, MemVal);
      strmov(MemVal, p + 1);
    };
    KillBlanks(TempName);
    KillBlanks(MemVal);
    FileLen = strtol(MemFlag, &p, 0);
    if (*p != '\0')
    {
      fputs(getmessage(Num_ErrMsgInvSwapSize), stderr);
      exit(4);
    }
    if (MEMinitSwapFile(TempName, FileLen << 20) != RTM_OK)
    {
      fputs(getmessage(Num_ErrMsgSwapTooBig), stderr);
      exit(4);
    }
  }
#else
  /* Bei DOS Auslagerung Overlays in XMS/EMS versuchen */

  envval = getenv("USEXMS");
  if ((envval) && (mytoupper(*envval) == 'N'))
    ovrerg = -1;
  else
    ovrerg = _OvrInitExt(0, 0);
  if (ovrerg != 0)
  {
    envval = getenv("USEEMS");
    if ((!envval) || (mytoupper(*envval) != 'N'))
      _OvrInitEms(0, 0, 0);
  }
#endif
#endif
#endif

#ifdef __TURBOC__
  StartStack = stackavail();
  LowStack = stackavail();
  MinStack = StartStack - STKSIZE + 0x800;
#else
  StartStack = LowStack = MinStack = 0;
#endif

  /* initialize array of valid characters */

  ValidSymChar = (Byte*) malloc(sizeof(Byte) * 256);
  memset(ValidSymChar, 0, sizeof(Byte) * 256);
  for (z = 'a'; z <= 'z'; z++)
    ValidSymChar[z] = VALID_S1 | VALID_SN | VALID_M1 | VALID_MN;
  for (z = 'A'; z <= 'Z'; z++)
    ValidSymChar[z] = VALID_S1 | VALID_SN | VALID_M1 | VALID_MN;
  for (z = '0'; z <= '9'; z++)
    ValidSymChar[z] =            VALID_SN |            VALID_MN;
  ValidSymChar[(unsigned int) '.'] = VALID_S1 | VALID_SN;
  ValidSymChar[(unsigned int) '_'] = VALID_S1 | VALID_SN;
#if (defined CHARSET_IBM437) || (defined CHARSET_IBM850)
  for (z = 128; z <= 165; z++)
    ValidSymChar[z] = VALID_S1 | VALID_SN;
  ValidSymChar[225] = VALID_S1 | VALID_SN;
#elif defined CHARSET_ISO8859_1
  for (z = 192; z <= 255; z++)
    ValidSymChar[z] = VALID_S1 | VALID_SN;
#elif (defined CHARSET_ASCII7) || (defined CHARSET_UTF8)
#else
#error Oops, unkown charset - you will have to add some work here...
#endif

  version_init();
}
