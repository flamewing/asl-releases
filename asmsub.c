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
/* $Id: asmsub.c,v 1.34 2016/11/25 18:12:13 alfred Exp $                      */
/*****************************************************************************
 * $Log: asmsub.c,v $
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
/* neuen Prozessor definieren */

CPUVar AddCPU(char *NewName, TSwitchProc Switcher)
{
  PCPUDef Lauf, Neu;
  char *p;

  Neu = (PCPUDef) malloc(sizeof(TCPUDef));
  Neu->Name = strdup(NewName);
  /* kein UpString, weil noch nicht initialisiert ! */
  for (p = Neu->Name; *p != '\0'; p++)
    *p = mytoupper(*p);
  Neu->SwitchProc = Switcher;
  Neu->Next = NULL;
  Neu->Number = Neu->Orig = CPUCnt;

  Lauf = FirstCPUDef;
  if (!Lauf)
    FirstCPUDef = Neu;
  else
  {
    while (Lauf->Next)
      Lauf = Lauf->Next;
    Lauf->Next = Neu;
  }

  return CPUCnt++;
}

Boolean AddCPUAlias(char *OrigName, char *AliasName)
{
  PCPUDef Lauf = FirstCPUDef, Neu;

  while ((Lauf) && (strcmp(Lauf->Name, OrigName)))
    Lauf = Lauf->Next;

  if (!Lauf)
    return False;
  else
  {
    Neu = (PCPUDef) malloc(sizeof(TCPUDef));
    Neu->Next = NULL;
    Neu->Name = strdup(AliasName);
    Neu->Number = CPUCnt++;
    Neu->Orig = Lauf->Orig;
    Neu->SwitchProc = Lauf->SwitchProc;
    while (Lauf->Next)
      Lauf = Lauf->Next;
    Lauf->Next = Neu;
    return True;
  }
}

void PrintCPUList(TSwitchProc NxtProc)
{
  PCPUDef Lauf;
  TSwitchProc Proc;
  int cnt;

  Lauf = FirstCPUDef;
  Proc = NullProc;
  cnt = 0;
  while (Lauf)
  {
    if (Lauf->Number == Lauf->Orig)
    {
      if ((Lauf->SwitchProc != Proc) || (cnt == 7))
      {
        Proc = Lauf->SwitchProc;
        printf("\n");
        NxtProc();
        cnt = 0;
      }
      printf("%-10s", Lauf->Name);
      cnt++;
    }
    Lauf = Lauf->Next;
  }
  printf("\n");
  NxtProc();
}

void ClearCPUList(void)
{
  PCPUDef Save;

  while (FirstCPUDef)
  {
    Save = FirstCPUDef;
    FirstCPUDef = Save->Next;
    free(Save->Name);
    free(Save);
  }
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

#if 0
char *QuotPos(const char *s, char Zeichen)
{
  register int Cnt = 0;
  register char *i;
  register char ch, Cmp2, Cmp3;

  for (i = s; (ch = *i) != '\0'; i++)
   if (Cnt == 0)
   {
     if (ch == Zeichen)
       return i;
     else
       switch (ch)
       {
         case '"':
         case '\'':
           Cmp2 = '\0';
           Cmp3 = ch;
           Cnt = 1;
           break;
         case '(':
           Cmp2 = '(';
           Cmp3 = ')';
           Cnt = 1;
           break;
         case '[':
           Cmp2 = '[';
           Cmp3 = ']';
           Cnt = 1;
           break;
       }
   }
   else
   {
     if (ch == Cmp2)
       Cnt++;
     else if (ch == Cmp3)
       Cnt--;
   }

  return NULL;
}
#else
char *QuotPos(const char *s, char Zeichen)
{
  register ShortInt Brack = 0, AngBrack = 0;
  register const char *i;
  register LongWord Flag = 0;
  static Boolean First = True, Imp[256];
  Boolean Save;
  char *pResult = NULL;

  if (First)
  {
    memset(Imp, False, 256);
    Imp['"'] = Imp['\''] = Imp['('] = Imp[')'] = Imp['['] = Imp[']'] = True;
    First = False;
  }

  Save = Imp[(unsigned char)Zeichen];
  Imp[(unsigned char)Zeichen] = True;
  for (i = s; *i != '\0'; i++)
   if (Imp[(unsigned char)*i])
   {
     if (*i == Zeichen)
     {
       if ((AngBrack | Brack | Flag) == 0)
       {
         pResult = (char*)i;
         goto func_exit;
       }
     }
     switch(*i)
     {
       case '"':
         if ((!(Brack | AngBrack)) && (!(Flag & 2)))
           Flag ^= 1;
         break;
       case '\'':
         if ((!(Brack | AngBrack)) && (!(Flag & 1)))
           Flag ^= 2;
         break;
       case '(':
         if (!(AngBrack | Flag))
           Brack++;
         break;
       case ')':
         if (!(AngBrack | Flag))
           Brack--;
         break;
       case '[':
         if (!(Brack | Flag))
           AngBrack++;
         break;
       case ']':
         if (!(Brack | Flag))
           AngBrack--;
         break;
     }
   }

func_exit:
  Imp[(unsigned char)Zeichen] = Save;
  return pResult;
}
#endif
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
/* ermittelt das erste Leerzeichen in einem String */

char *FirstBlank(char *s)
{
  char *h, *Min = NULL;

  h = strchr(s, ' ');
  if (h)
    if ((!Min) || (h < Min))
      Min = h;
  h = strchr(s, Char_HT);
  if (h)
    if ((!Min) || (h < Min))
      Min = h;
  return Min;
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

ShortInt StrCmp(const char *s1, const char *s2, LongInt Hand1, LongInt Hand2)
{
  int tmp;

  tmp = (*s1) - (*s2);
  if (!tmp)
    tmp = strcmp(s1, s2);
  if (!tmp)
    tmp = Hand1 - Hand2;
  if (tmp < 0)
    return -1;
  if (tmp > 0)
    return 1;
  return 0;
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

static void PrPrintable(FILE *pFile, const char *pStr)
{
  const char *pRun;

  for (pRun = pStr; *pRun; pRun++)
    if (myisprint(*pRun))
      fputc(*pRun, pFile);
    else
      fprintf(pFile, "<0x%02x>", (unsigned char)*pRun);
}

void WrErrorString(char *Message, char *Add, Boolean Warning, Boolean Fatal)
{
  String h, h2;
  char *p;
  FILE *errfile;
  int l;

  *h = '\0';
  if (!GNUErrors)
    strcpy(h, "> > >");
  p = GetErrorPos();
  if (p[l = strlen(p) - 1] == ' ')
    p[l] = '\0';
  strmaxcat(h, p, 255);
  free(p);
  if (Warning)
  {
    strmaxcat(h, ": ", 255);
    strmaxcat(h, getmessage(Num_WarnName), 255);
    strmaxcat(h, Add, 255);
    strmaxcat(h, ": ", 255);
    WarnCount++;
  }
  else
  {
    if (!GNUErrors)
    {
      strmaxcat(h, ": ", 255);
      strmaxcat(h, getmessage(Num_ErrName), 255);
    }
    strmaxcat(h, Add, 255);
    strmaxcat(h, ": ", 255);
    ErrorCount++;
  }

  if ((strcmp(LstName, "/dev/null")) && (!Fatal))
  {
    strmaxcpy(h2, h, 255);
    strmaxcat(h2, Message, 255);
    WrLstLine(h2);
    if ((ExtendErrors > 0) && (*ExtendError != '\0'))
    {
      sprintf(h2, "> > > %s", ExtendError);
      WrLstLine(h2);
    }
    if (ExtendErrors > 1)
    {
      sprintf(h2, "> > > %s", OneLine);
      WrLstLine(h2);
    }
  }

  ForceErrorOpen();
  if ((strcmp(LstName, "!1")) || (Fatal))
  {
    errfile = (!ErrorFile) ? stdout : ErrorFile;
    fprintf(errfile, "%s%s%s\n", h, Message, ClrEol);
    if ((ExtendErrors > 0) && (*ExtendError != '\0'))
    {
      fprintf(errfile, "> > > ");
      PrPrintable(errfile, ExtendError);
      fprintf(errfile, "%s\n", ClrEol);
    }
    if (ExtendErrors > 1)
      fprintf(errfile, "> > > %s%s\n", OneLine, ClrEol);
  }
  *ExtendError = '\0';

  if (Fatal)
  {
    fprintf(ErrorFile ? ErrorFile : stdout, "%s\n", getmessage(Num_ErrMsgIsFatal));
    EmergencyStop();
    exit(3);
  }
}

/*--------------------------------------------------------------------------*/
/* eine Fehlermeldung ueber Code ausgeben */

static void WrErrorNum(Word Num)
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
    case    0: msgno = Num_ErrMsgUselessDisp; break;
    case   10: msgno = Num_ErrMsgShortAddrPossible; break;
    case   20: msgno = Num_ErrMsgShortJumpPossible; break;
    case   30: msgno = Num_ErrMsgNoShareFile; break;
    case   40: msgno = Num_ErrMsgBigDecFloat; break;
    case   50: msgno = Num_ErrMsgPrivOrder; break;
    case   60: msgno = Num_ErrMsgDistNull; break;
    case   70: msgno = Num_ErrMsgWrongSegment; break;
    case   75: msgno = Num_ErrMsgInAccSegment; break;
    case   80: msgno = Num_ErrMsgPhaseErr; break;
    case   90: msgno = Num_ErrMsgOverlap; break;
    case  100: msgno = Num_ErrMsgNoCaseHit; break;
    case  110: msgno = Num_ErrMsgInAccPage; break;
    case  120: msgno = Num_ErrMsgRMustBeEven; break;
    case  130: msgno = Num_ErrMsgObsolete; break;
    case  140: msgno = Num_ErrMsgUnpredictable; break;
    case  150: msgno = Num_ErrMsgAlphaNoSense; break;
    case  160: msgno = Num_ErrMsgSenseless; break;
    case  170: msgno = Num_ErrMsgRepassUnknown; break;
    case  180: msgno = Num_ErrMsgAddrNotAligned; break;
    case  190: msgno = Num_ErrMsgIOAddrNotAllowed; break;
    case  200: msgno = Num_ErrMsgPipeline; break;
    case  210: msgno = Num_ErrMsgDoubleAdrRegUse; break;
    case  220: msgno = Num_ErrMsgNotBitAddressable; break;
    case  230: msgno = Num_ErrMsgStackNotEmpty; break;
    case  240: msgno = Num_ErrMsgNULCharacter; break;
    case  250: msgno = Num_ErrMsgPageCrossing; break;
    case  260: msgno = Num_ErrMsgWOverRange; break;
    case  270: msgno = Num_ErrMsgNegDUP; break;
    case  280: msgno = Num_ErrMsgConvIndX; break;
    case  290: msgno = Num_ErrMsgNullResMem; break;
    case  300: msgno = Num_ErrMsgBitNumberTruncated; break;
    case  310: msgno = Num_ErrMsgInvRegisterPointer; break;
    case  320: msgno = Num_ErrMsgMacArgRedef; break;
    case 1000: msgno = Num_ErrMsgDoubleDef; break;
    case 1010: msgno = Num_ErrMsgSymbolUndef; break;
    case 1020: msgno = Num_ErrMsgInvSymName; break;
    case 1090: msgno = Num_ErrMsgInvFormat; break;
    case 1100: msgno = Num_ErrMsgUseLessAttr; break;
    case 1105: msgno = Num_ErrMsgTooLongAttr; break;
    case 1107: msgno = Num_ErrMsgUndefAttr; break;
    case 1110: msgno = Num_ErrMsgWrongArgCnt; break;
    case 1115: msgno = Num_ErrMsgWrongOptCnt; break;
    case 1120: msgno = Num_ErrMsgOnlyImmAddr; break;
    case 1130: msgno = Num_ErrMsgInvOpsize; break;
    case 1131: msgno = Num_ErrMsgConfOpSizes; break;
    case 1132: msgno = Num_ErrMsgUndefOpSizes; break;
    case 1135: msgno = Num_ErrMsgInvOpType; break;
    case 1140: msgno = Num_ErrMsgTooMuchArgs; break;
    case 1150: msgno = Num_ErrMsgNoRelocs; break;
    case 1155: msgno = Num_ErrMsgUnresRelocs; break;
    case 1156: msgno = Num_ErrMsgUnexportable; break;
    case 1200: msgno = Num_ErrMsgUnknownOpcode; break;
    case 1300: msgno = Num_ErrMsgBrackErr; break;
    case 1310: msgno = Num_ErrMsgDivByZero; break;
    case 1315: msgno = Num_ErrMsgUnderRange; break;
    case 1320: msgno = Num_ErrMsgOverRange; break;
    case 1325: msgno = Num_ErrMsgNotAligned; break;
    case 1330: msgno = Num_ErrMsgDistTooBig; break;
    case 1335: msgno = Num_ErrMsgInAccReg; break;
    case 1340: msgno = Num_ErrMsgNoShortAddr; break;
    case 1350: msgno = Num_ErrMsgInvAddrMode; break;
    case 1351: msgno = Num_ErrMsgMustBeEven; break;
    case 1355: msgno = Num_ErrMsgInvParAddrMode; break;
    case 1360: msgno = Num_ErrMsgUndefCond; break;
    case 1365: msgno = Num_ErrMsgIncompCond; break;
    case 1370: msgno = Num_ErrMsgJmpDistTooBig; break;
    case 1375: msgno = Num_ErrMsgDistIsOdd; break;
    case 1380: msgno = Num_ErrMsgInvShiftArg; break;
    case 1390: msgno = Num_ErrMsgRange18; break;
    case 1400: msgno = Num_ErrMsgShiftCntTooBig; break;
    case 1410: msgno = Num_ErrMsgInvRegList; break;
    case 1420: msgno = Num_ErrMsgInvCmpMode; break;
    case 1430: msgno = Num_ErrMsgInvCPUType; break;
    case 1440: msgno = Num_ErrMsgInvCtrlReg; break;
    case 1445: msgno = Num_ErrMsgInvReg; break;
    case 1450: msgno = Num_ErrMsgNoSaveFrame; break;
    case 1460: msgno = Num_ErrMsgNoRestoreFrame; break;
    case 1465: msgno = Num_ErrMsgUnknownMacArg; break;
    case 1470: msgno = Num_ErrMsgMissEndif; break;
    case 1480: msgno = Num_ErrMsgInvIfConst; break;
    case 1483: msgno = Num_ErrMsgDoubleSection; break;
    case 1484: msgno = Num_ErrMsgInvSection; break;
    case 1485: msgno = Num_ErrMsgMissingEndSect; break;
    case 1486: msgno = Num_ErrMsgWrongEndSect; break;
    case 1487: msgno = Num_ErrMsgNotInSection; break;
    case 1488: msgno = Num_ErrMsgUndefdForward; break;
    case 1489: msgno = Num_ErrMsgContForward; break;
    case 1490: msgno = Num_ErrMsgInvFuncArgCnt; break;
    case 1495: msgno = Num_ErrMsgMissingLTORG; break;
    case 1500: msgno = -1;
               sprintf(h, "%s%s%s", getmessage(Num_ErrMsgNotOnThisCPU1),
                       MomCPUIdent, getmessage(Num_ErrMsgNotOnThisCPU2));
               break;
    case 1505: msgno = -1;
               sprintf(h, "%s%s%s", getmessage(Num_ErrMsgNotOnThisCPU3),
                       MomCPUIdent, getmessage(Num_ErrMsgNotOnThisCPU2));
               break;
    case 1510: msgno = Num_ErrMsgInvBitPos; break;
    case 1520: msgno = Num_ErrMsgOnlyOnOff; break;
    case 1530: msgno = Num_ErrMsgStackEmpty; break;
    case 1540: msgno = Num_ErrMsgNotOneBit; break;
    case 1550: msgno = Num_ErrMsgMissingStruct; break;
    case 1551: msgno = Num_ErrMsgOpenStruct; break;
    case 1552: msgno = Num_ErrMsgWrongStruct; break;
    case 1553: msgno = Num_ErrMsgPhaseDisallowed; break;
    case 1554: msgno = Num_ErrMsgInvStructDir; break;
    case 1560: msgno = Num_ErrMsgNotRepeatable; break;
    case 1600: msgno = Num_ErrMsgShortRead; break;
    case 1610: msgno = Num_ErrMsgUnknownCodepage; break;
    case 1700: msgno = Num_ErrMsgRomOffs063; break;
    case 1710: msgno = Num_ErrMsgInvFCode; break;
    case 1720: msgno = Num_ErrMsgInvFMask; break;
    case 1730: msgno = Num_ErrMsgInvMMUReg; break;
    case 1740: msgno = Num_ErrMsgLevel07; break;
    case 1750: msgno = Num_ErrMsgInvBitMask; break;
    case 1760: msgno = Num_ErrMsgInvRegPair; break;
    case 1800: msgno = Num_ErrMsgOpenMacro; break;
    case 1805: msgno = Num_ErrMsgEXITMOutsideMacro; break;
    case 1810: msgno = Num_ErrMsgTooManyMacParams; break;
    case 1811: msgno = Num_ErrMsgUndefKeyArg; break;
    case 1812: msgno = Num_ErrMsgNoPosArg; break;
    case 1815: msgno = Num_ErrMsgDoubleMacro; break;
    case 1820: msgno = Num_ErrMsgFirstPassCalc; break;
    case 1830: msgno = Num_ErrMsgTooManyNestedIfs; break;
    case 1840: msgno = Num_ErrMsgMissingIf; break;
    case 1850: msgno = Num_ErrMsgRekMacro; break;
    case 1860: msgno = Num_ErrMsgUnknownFunc; break;
    case 1870: msgno = Num_ErrMsgInvFuncArg; break;
    case 1880: msgno = Num_ErrMsgFloatOverflow; break;
    case 1890: msgno = Num_ErrMsgInvArgPair; break;
    case 1900: msgno = Num_ErrMsgNotOnThisAddress; break;
    case 1905: msgno = Num_ErrMsgNotFromThisAddress; break;
    case 1910: msgno = Num_ErrMsgTargOnDiffPage; break;
    case 1920: msgno = Num_ErrMsgCodeOverflow; break;
    case 1925: msgno = Num_ErrMsgAdrOverflow; break;
    case 1930: msgno = Num_ErrMsgMixDBDS; break;
    case 1940: msgno = Num_ErrMsgNotInStruct; break;
    case 1950: msgno = Num_ErrMsgParNotPossible; break;
    case 1960: msgno = Num_ErrMsgInvSegment; break;
    case 1961: msgno = Num_ErrMsgUnknownSegment; break;
    case 1962: msgno = Num_ErrMsgUnknownSegReg; break;
    case 1970: msgno = Num_ErrMsgInvString; break;
    case 1980: msgno = Num_ErrMsgInvRegName; break;
    case 1985: msgno = Num_ErrMsgInvArg; break;
    case 1990: msgno = Num_ErrMsgNoIndir; break;
    case 1995: msgno = Num_ErrMsgNotInThisSegment; break;
    case 1996: msgno = Num_ErrMsgNotInMaxmode; break;
    case 1997: msgno = Num_ErrMsgOnlyInMaxmode; break;
    case 2000: msgno = Num_ErrMsgPackCrossBoundary; break;
    case 2001: msgno = Num_ErrMsgUnitMultipleUsed; break;
    case 2002: msgno = Num_ErrMsgMultipleLongRead; break;
    case 2003: msgno = Num_ErrMsgMultipleLongWrite; break;
    case 2004: msgno = Num_ErrMsgLongReadWithStore; break;
    case 2005: msgno = Num_ErrMsgTooManyRegisterReads; break;
    case 2006: msgno = Num_ErrMsgOverlapDests; break;
    case 2008: msgno = Num_ErrMsgTooManyBranchesInExPacket; break;
    case 2009: msgno = Num_ErrMsgCannotUseUnit; break;
    case 2010: msgno = Num_ErrMsgInvEscSequence; break;
    case 2020: msgno = Num_ErrMsgInvPrefixCombination; break;
    case 2030: msgno = Num_ErrConstantRedefinedAsVariable; break;
    case 2035: msgno = Num_ErrVariableRedefinedAsConstant; break;
    case 2040: msgno = Num_ErrMsgStructNameMissing; break;
    case 2050: msgno = Num_ErrMsgEmptyArgument; break;
    case 2060: msgno = Num_ErrMsgUnimplemented; break;
    case 2070: msgno = Num_ErrMsgFreestandingUnnamedStruct; break;
    case 2080: msgno = Num_ErrMsgSTRUCTEndedByENDUNION; break;
    case 2090: msgno = Num_ErrMsgAddrOnDifferentPage; break;
    case 10001: msgno = Num_ErrMsgOpeningFile; break;
    case 10002: msgno = Num_ErrMsgListWrError; break;
    case 10003: msgno = Num_ErrMsgFileReadError; break;
    case 10004: msgno = Num_ErrMsgFileWriteError; break;
    case 10006: msgno = Num_ErrMsgHeapOvfl; break;
    case 10007: msgno = Num_ErrMsgStackOvfl; break;
    default  : msgno = -1;
               sprintf(h, "%s %d", getmessage(Num_ErrMsgIntError), (int) Num);
  }
  if (msgno != -1)
    strmaxcpy(h, getmessage(msgno), 255);

  if (((Num == 1910) || (Num == 1370)) && (!Repass))
    JmpErrors++;

  if (NumericErrors)
    sprintf(Add, " #%d", (int)Num);
  else
    *Add = '\0';
  WrErrorString(h, Add, Num < 1000, Num >= 10000);
}

void WrError(Word Num)
{
  *ExtendError = '\0';
  WrErrorNum(Num);
}

void WrXError(Word Num, const char *Message)
{
  strmaxcpy(ExtendError, Message, 255);
  WrErrorNum(Num);
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

void ChkXIO(Word ErrNo, const char *pPath)
{
  int io;
  String s;

  io = errno;
  if ((io == 0) || (io == 19) || (io == 25))
    return;

  strmaxcpy(s, pPath, 255);
  strmaxcat(s, ": ", 255);
  strmaxcat(s, GetErrorMsg(io), 255);
  WrXError(ErrNo, s);
}

/*--------------------------------------------------------------------------*/
/* Bereichsfehler */

Boolean ChkRange(LargeInt Value, LargeInt Min, LargeInt Max)
{
  char s1[100], s2[100];

  if (Value < Min)
  {
    LargeString(s1, Value);
    LargeString(s2, Min);
    strmaxcat(s1, "<", 99);
    strmaxcat(s1, s2, 99);
    WrXError(1315, s1);
    return False;
  }
  else if (Value > Max)
  {
    LargeString(s1, Value);
    LargeString(s2, Max);
    strmaxcat(s1, ">", 99);
    strmaxcat(s1, s2, 99);
    WrXError(1320, s1);
    return False;
  }
  else
    return True;
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

  if ((TypeFlag&Mask) != 0) WrError(70);
}

/****************************************************************************/
/* eine Chunkliste im Listing ausgeben & Speicher loeschen */

void PrintChunk(ChunkList *NChunk)
{
  LargeWord NewMin, FMin;
  Boolean Found;
  Word p = 0, z;
  int BufferZ;
  String BufferS;

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

      HexString(Num, sizeof(Num), NChunk->Chunks[p].Start, 0);
      strmaxcat(BufferS, Num, 255);
      if (NChunk->Chunks[p].Length != 1)
      {
        strmaxcat(BufferS, "-", 255);
        HexString(Num, sizeof(Num), NChunk->Chunks[p].Start + NChunk->Chunks[p].Length - 1, 0);
        strmaxcat(BufferS, Num, 255);
      }
      strmaxcat(BufferS, Blanks(19 - strlen(BufferS) % 19), 255);
      if (++BufferZ == 4)
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
      PrintChunk(SegChunks + z);
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
      WrError(90);
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
    WrError(10007);
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
    WrError(10006);
#endif

  tmp = malloc(s);
  if (!tmp)
    WrError(10006);
  return tmp;
}

void *ckrealloc(void *p, size_t s)
{
  void *tmp;

#ifdef __TURBOC__
  if (coreleft() < HEAPRESERVE + s)
    WrError(10006);
#endif

  tmp = realloc(p, s);
  if (!tmp)
    WrError(10006);
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
