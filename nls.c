/* nls.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Abhandlung landesspezifischer Unterschiede                                */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*           28. 7.1999 %T ist Abkuerzung fuer %H:%M:%S                      */
/*                                                                           */
/*****************************************************************************/
/* $Id: nls.c,v 1.5 2014/12/07 19:14:02 alfred Exp $                         */
/*****************************************************************************
 * $Log: nls.c,v $
 * Revision 1.5  2014/12/07 19:14:02  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.4  2014/12/03 20:42:45  alfred
 * - adapt to current style
 *
 * Revision 1.3  2013/12/17 18:17:19  alfred
 * - add missing break statement
 *
 * Revision 1.2  2010/08/27 14:52:43  alfred
 * - some more overlapping strcpy() cleanups
 *
 * Revision 1.1  2003/11/06 02:49:24  alfred
 * - recreated
 *
 * Revision 1.2  2002/05/11 20:24:35  alfred
 * - flush stdout before printing errors
 *
 *****************************************************************************/

#undef DEBUG_NLS

#include "stdinc.h"
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef LOCALE_NLS
#include <locale.h>
#include <langinfo.h>
#endif

#ifdef OS2_NLS
#define INCL_DOSNLS
#include <os2.h>
#endif

#ifdef DOS_NLS
#include <dos.h>
#endif

#include "strutil.h"

#include "nls.h"

CharTable UpCaseTable;               /* Umsetzungstabellen */
CharTable LowCaseTable;

static NLS_CountryInfo NLSInfo;
static CharTable CollateTable;

/*-------------------------------------------------------------------------------*/

#if 0
/* einen String anhand einer Tabelle uebersetzen: */

static void TranslateString(char *s, CharTable Table)
{
  for (; *s != '\0'; s++) *s = Table[((unsigned int) *s)&0xff];
}
#endif

/*-------------------------------------------------------------------------------*/
/* Da es moeglich ist, die aktuelle Codeseite im Programmlauf zu wechseln,
   ist die Initialisierung in einer getrennten Routine untergebracht.  Nach
   einem Wechsel stellt ein erneuter Aufruf wieder korrekte Verhaeltnisse
   her.  Wen das stoert, der schreibe einfach einen Aufruf in den Initiali-
   sierungsteil der Unit hinein. */

#ifdef DOS_NLS
typedef struct
{
  Byte TimeFmt;
  Byte DateFmt;
  char Currency[2];
  char ThouSep[2];
  char DecSep[2];
  Byte Reserved[24];
} Dos2CountryInfo;
typedef struct
{
  Word DateFmt;
  char Currency[5];
  char ThouSep[2];
  char DecSep[2];
  char DateSep[2];
  char TimeSep[2];
  Byte CurrFmt;
  Byte CurrDecimals;
  Byte TimeFmt;
  char *UpCasePtr;
  char DataSep[2];
  Byte Dummy[8];
} Dos3CountryInfo;
typedef struct
{
  Byte SubFuncNo;
  char *Result;
} DosTableRec;

char *DosCopy(char *Src, int Len)
{
  char *res = malloc(sizeof(char)*(Len + 1));
  memcpy(res, Src, Len);
  res[Len] = '\0';
  return res;
}

void StandardUpCases(void)
{
  char *s1, *s2;

  s1 = CH_ae; s2 = CH_Ae; UpCaseTable[((unsigned int) *s1) & 0xff] = *s2;
  s1 = CH_oe; s2 = CH_Oe; UpCaseTable[((unsigned int) *s1) & 0xff] = *s2;
  s1 = CH_ue; s2 = CH_Ue; UpCaseTable[((unsigned int) *s1) & 0xff] = *s2;
}
#endif

void NLS_Initialize(void)
{
  char *tmpstr, *run, *cpy;
  Word FmtBuffer;
  int z;
#ifdef DEBUG_NLS
  int z2;
#endif
  Boolean DidDate;

#ifdef LOCALE_NLS
  struct lconv *lc;
#endif

#ifdef OS2_NLS
  COUNTRYCODE ccode;
  COUNTRYINFO cinfo;
  ULONG erglen;
#endif

#ifdef DOS_NLS
  union REGS Regs;
  struct SREGS SRegs;
  void *info;
  Dos2CountryInfo DOS2Info;
  Dos3CountryInfo DOS3Info;
  DosTableRec DOSTablePtr;
#endif

   /* get currency format, separators */

#ifdef NO_NLS
  NLSInfo.DecSep = ".";
  NLSInfo.DataSep = ",";
  NLSInfo.ThouSep = ",";
  NLSInfo.Currency = "$";
  NLSInfo.CurrDecimals = 2;
  NLSInfo.CurrFmt = CurrFormatPreNoBlank;
#endif

#ifdef LOCALE_NLS
  lc = localeconv();

  NLSInfo.DecSep = lc->mon_decimal_point ? lc->decimal_point : ".";

  NLSInfo.ThouSep = lc->mon_thousands_sep ? lc->mon_thousands_sep : ",";

  NLSInfo.DataSep = ",";

  NLSInfo.Currency = lc->currency_symbol ? lc->currency_symbol : "$";

  NLSInfo.CurrDecimals = lc->int_frac_digits;
  if (NLSInfo.CurrDecimals > 4)
    NLSInfo.CurrDecimals = 2;

  if (lc->p_cs_precedes)
    NLSInfo.CurrFmt = lc->p_sep_by_space ? CurrFormatPreBlank : CurrFormatPreNoBlank;
  else
    NLSInfo.CurrFmt = lc->p_sep_by_space ? CurrFormatPostBlank : CurrFormatPostNoBlank;
#endif

#ifdef OS2_NLS
  ccode.country = 0;
  ccode.codepage = 0;
  DosQueryCtryInfo(sizeof(cinfo), &ccode, &cinfo, &erglen);

  NLSInfo.Country = cinfo.country;
  NLSInfo.CodePage = cinfo.codepage;
  NLSInfo.DecSep = strdup(cinfo.szDecimal);
  NLSInfo.DataSep = strdup(cinfo.szDataSeparator);
  NLSInfo.ThouSep = strdup(cinfo.szThousandsSeparator);
  NLSInfo.Currency = strdup(cinfo.szCurrency);
  NLSInfo.CurrDecimals = cinfo.cDecimalPlace;
  NLSInfo.CurrFmt = (CurrFormat) cinfo.fsCurrencyFmt;
#endif

#ifdef DOS_NLS
  if (_osmajor < 3)
    NLSInfo.CodePage = 437;
  else if (_osminor < 30)
    NLSInfo.CodePage = 437;
  else
  {
    Regs.x.ax = 0x6601;
    int86(0x21, &Regs, &Regs);
    NLSInfo.CodePage = Regs.x.bx;
  }

  Regs.x.ax = 0x3800;
  info = (_osmajor < 3) ? (void*)&DOS2Info : (void*)&DOS3Info;
  SRegs.ds = FP_SEG(info);
  Regs.x.dx = FP_OFF(info);
  int86x(0x21, &Regs, &Regs, &SRegs);
  NLSInfo.Country = Regs.x.bx;
  if (_osmajor >= 3)
  {
    NLSInfo.DecSep = DosCopy(DOS3Info.DecSep, 2);
    NLSInfo.DataSep = DosCopy(DOS3Info.DataSep, 2);
    NLSInfo.ThouSep = DosCopy(DOS3Info.ThouSep, 2);
    NLSInfo.Currency = DosCopy(DOS3Info.Currency, 5);
    NLSInfo.CurrDecimals = DOS3Info.CurrDecimals;
    NLSInfo.CurrFmt = (CurrFormat) DOS3Info.CurrFmt;
  }
  /* DOS 2 kennt noch nicht soviel, daher muessen wir selber etwas beisteuern */
  else
  {
    NLSInfo.DecSep = DosCopy(DOS2Info.DecSep, 2);
    NLSInfo.DataSep = ",";
    NLSInfo.ThouSep = DosCopy(DOS2Info.ThouSep, 2);
    NLSInfo.Currency = DosCopy(DOS2Info.Currency, 2);
    NLSInfo.CurrDecimals = (NLSInfo.Country == 39) ? 0 : 2;
    switch (NLSInfo.Country)
    {
      case 1:
      case 39:
        NLSInfo.CurrFmt = CurrFormatPreNoBlank;
        break;
      case 3:
      case 33:
      case 34:
      case 358:
        NLSInfo.CurrFmt = CurrFormatPostBlank;
        break;
      default:
        NLSInfo.CurrFmt = CurrFormatPreBlank;
    }
  }
#endif

  /* get date format */

#ifdef NO_NLS
  tmpstr = "%m/%d/%y";
  DidDate = False;
#endif

#ifdef LOCALE_NLS
  tmpstr = nl_langinfo(D_FMT);
  if ((!tmpstr) || (*tmpstr == '\0'))
    tmpstr = "%m/%d/%y";
  DidDate = False;
#endif

#ifdef OS2_NLS
  NLSInfo.DateFmt = (DateFormat) cinfo.fsDateFmt;
  NLSInfo.DateSep = strdup(cinfo.szDateSeparator);
  DidDate = True;
#endif

#ifdef DOS_NLS
  if (_osmajor >= 3)
  {
    NLSInfo.DateFmt = (DateFormat) DOS3Info.DateFmt;
    NLSInfo.DateSep = DosCopy(DOS3Info.DateSep, 2);
  }
  else
  {
    NLSInfo.DateFmt = (DateFormat) DOS2Info.DateFmt;
    switch (NLSInfo.Country)
    {
      case 3:
      case 47:
      case 351:
      case 32:
      case 33:
      case 39:
      case 34:
        NLSInfo.DateSep = "/";
        break;
      case 49:
      case 358:
      case 41:
        NLSInfo.DateSep = ".";
        break;
      case 972:
        NLSInfo.DateSep = " ";
        break;
      default:
        NLSInfo.DateSep = "-";
    }
  }
  DidDate = True;
#endif

  if (!DidDate)
  {
    NLSInfo.DateSep = NULL;
    FmtBuffer = 0;
    run = tmpstr;
    while (*run != '\0')
      if (*run == '%')
      {
        FmtBuffer <<= 4;
        switch (toupper(*(++run)))
        {
          case 'D':
            FmtBuffer += 1;
            break;
          case 'M':
            FmtBuffer += 2;
            break;
          case 'Y':
            FmtBuffer += 3;
            break;
        }
        if (!NLSInfo.DateSep)
        {
          run++;
          cpy = NLSInfo.DateSep = strdup("                  ");
          while ((*run != ' ') && (*run != '%'))
            *(cpy++) = *(run++);
          *cpy = '\0';
        }
        else
          run++;
      }
      else
        run++;
    if (FmtBuffer == 0x213)
      NLSInfo.DateFmt = DateFormatMTY;
    else if (FmtBuffer == 0x123)
      NLSInfo.DateFmt = DateFormatTMY;
    else
      NLSInfo.DateFmt = DateFormatYMT;
  }

  /* get time format */

#ifdef NO_NLS
  tmpstr = "%H:%M:%S";
  DidDate = False;
#endif

#ifdef LOCALE_NLS
  tmpstr = nl_langinfo(T_FMT);
  if ((!tmpstr) || (*tmpstr == '\0'))
    tmpstr = "%H:%M:%S";
  DidDate = False;
#endif

#ifdef OS2_NLS
  NLSInfo.TimeFmt = (TimeFormat) cinfo.fsTimeFmt;
  NLSInfo.TimeSep = strdup(cinfo.szTimeSeparator);
  DidDate = True;
#endif

#ifdef DOS_NLS
  if (_osmajor >= 3)
  {
    NLSInfo.TimeFmt = (TimeFormat) DOS3Info.TimeFmt;
    NLSInfo.TimeSep = DosCopy(DOS3Info.TimeSep, 2);
  }
  else
  {
    NLSInfo.TimeFmt = (TimeFormat) DOS2Info.TimeFmt;
    switch (NLSInfo.Country)
    {
      case 41:
      case 46:
      case 47:
      case 358:
        NLSInfo.TimeSep = ".";
        break;
      default:
        NLSInfo.TimeSep = ":";
    }
  }
  DidDate = True;
#endif

  if (!DidDate)
  {
    NLSInfo.TimeSep = NULL;
    FmtBuffer = 0;
    run = tmpstr;
    while (*run != '\0')
      if (*run == '%')
      {
        FmtBuffer <<= 4;
        switch (toupper(*(++run)))
        {
          case 'S':
            FmtBuffer += 1;
            break;
          case 'M':
            FmtBuffer += 2;
            break;
          case 'H':
            FmtBuffer += 3;
            break;
          case 'T':
            fflush(stdout);
            fprintf(stderr, "\nwarning, detected non-ANSI time format specifier '%%T'\n");
            run = "H:%M:%S";
            break;
          case 'R':
            fflush(stdout);
            fprintf(stderr, "\nwarning, detected non-ANSI time format specifier '%%R'\n");
            run = "H:%M";
            break;
        }
        if (!NLSInfo.TimeSep)
        {
          run++; cpy = NLSInfo.TimeSep = strdup("                  ");
          while ((*run != '\0') && (*run != ' ') && (*run != '%'))
           *(cpy++) = (*(run++));
          *cpy = '\0';
        }
        else
          run++;
      }
      else
        run++;
    NLSInfo.TimeFmt = TimeFormatEurope;
  }

  /* get lower->upper case table */

#if defined(NO_NLS) || defined(LOCALE_NLS)
  for (z = 0; z < 256; z++)
    UpCaseTable[z] = toupper(z);
#endif

#ifdef OS2_NLS
  for (z = 0; z < 256; z++)
    UpCaseTable[z] = (char) z;
  for (z = 'a'; z <= 'z'; z++)
    UpCaseTable[z] -= 'a' - 'A';
  DosMapCase(sizeof(UpCaseTable), &ccode, UpCaseTable);
#endif

#ifdef DOS_NLS
  for (z = 0; z < 256; z++)
    UpCaseTable[z] = (char) z;
  for (z = 'a'; z <= 'z'; z++)
    UpCaseTable[z] -= 'a' - 'A';
#ifdef __DPMI16__
  StandardUpCases();
#else
  if ((((Word)_osmajor) * 100) + _osminor >= 330)
  {
    Regs.x.ax = 0x6502;
    Regs.x.bx = NLSInfo.CodePage;
    Regs.x.dx = NLSInfo.Country;
    Regs.x.cx = sizeof(DOSTablePtr);
    info = &DOSTablePtr;
    SRegs.es = FP_SEG(info);
    Regs.x.di = FP_OFF(info);
    int86x(0x21, &Regs, &Regs, &SRegs);
    if (Regs.x.cx == sizeof(DOSTablePtr))
    {
      DOSTablePtr.Result += sizeof(Word);
      memcpy(UpCaseTable + 128, DOSTablePtr.Result, 128);
    }
    else
      StandardUpCases();
  }
  else
    StandardUpCases();
#endif
#endif

  /* get upper->lower case table */

#if defined(NO_NLS) || defined(LOCALE_NLS)
  for (z = 0; z < 256; z++)
    LowCaseTable[z] = tolower(z);
#endif

#if defined(OS2_NLS) || defined(DOS_NLS)
  for (z = 0; z < 256; z++)
    LowCaseTable[z] = (char) z;
  for (z = 255; z >= 0; z--)
    if (UpCaseTable[z] != (char) z)
      LowCaseTable[((unsigned int) UpCaseTable[z])&0xff] = (char) z;
#endif

  /* get collation table */

#if defined(NO_NLS) || defined(LOCALE_NLS)
  for (z = 0; z < 256; z++)
    CollateTable[z] = z;
  for (z = 'a'; z <= 'z'; z++)
    CollateTable[z] = toupper(z);
#endif

#ifdef OS2_NLS
  for (z = 0; z < 256; z++)
    CollateTable[z] = (char) z;
  DosQueryCollate(sizeof(CollateTable), &ccode, CollateTable, &erglen);
#endif

#ifdef DOS_NLS
  for (z = 0; z < 256; z++)
    CollateTable[z] = (char) z;
  for (z = 'a'; z <= 'z'; z++)
    CollateTable[z] = (char) (z - ('a' - 'A'));
#ifndef __DPMI16__
  if ((((Word)_osmajor)*100) + _osminor >= 330)
  {
    Regs.x.ax = 0x6506;
    Regs.x.bx = NLSInfo.CodePage;
    Regs.x.dx = NLSInfo.Country;
    Regs.x.cx = sizeof(DOSTablePtr);
    info = &DOSTablePtr;
    SRegs.es = FP_SEG(info);
    Regs.x.di = FP_OFF(info);
    int86x(0x21, &Regs, &Regs, &SRegs);
    if (Regs.x.cx == sizeof(DOSTablePtr))
    {
      DOSTablePtr.Result += sizeof(Word);
      memcpy(CollateTable, DOSTablePtr.Result, 256);
    }
  }
#endif
#endif

#ifdef DEBUG_NLS
  printf("Country      = %d\n", NLSInfo.Country);
  printf("CodePage     = %d\n", NLSInfo.CodePage);
  printf("DateFmt      = ");
  switch(NLSInfo.DateFmt)
  {
    case DateFormatMTY:
      printf("MTY\n");
      break;
    case DateFormatTMY:
      printf("TMY\n");
      break;
    case DateFormatYMT:
      printf("YMT\n");
      break;
    default:
      printf("???\n");
  }
  printf("DateSep      = %s\n", NLSInfo.DateSep);
  printf("TimeFmt      = ");
  switch(NLSInfo.TimeFmt)
  {
    case TimeFormatUSA:
      printf("USA\n");
      break;
    case TimeFormatEurope:
      printf("Europe\n");
      break;
    case TimeFormatJapan:
      printf("Japan\n");
      break;
    default:
      printf("???\n");
  }
  printf("TimeSep      = %s\n", NLSInfo.TimeSep);
  printf("Currency     = %s\n", NLSInfo.Currency);
  printf("CurrFmt      = ");
  switch (NLSInfo.CurrFmt)
  {
    case CurrFormatPreNoBlank:
      printf("PreNoBlank\n");
      break;
    case CurrFormatPostNoBlank:
      printf("PostNoBlank\n");
      break;
    case CurrFormatPreBlank:
      printf("PreBlank\n");
      break;
    case CurrFormatPostBlank:
      printf("PostBlank\n");
      break;
    default:
      printf("???\n");
  }
  printf("CurrDecimals = %d\n", NLSInfo.CurrDecimals);
  printf("ThouSep      = %s\n", NLSInfo.ThouSep);
  printf("DecSep       = %s\n", NLSInfo.DecSep);
  printf("DataSep      = %s\n", NLSInfo.DataSep);

  printf("\nUpcaseTable:\n");
  for (z = 0; z < 4; z++)
  {
    for (z2 = 0; z2 < 63; z2++)
      if (z * 64 + z2 > 32)
        putchar(z * 64 + z2);
    putchar('\n');
    for (z2 = 0; z2 < 63; z2++)
      if (z * 64 + z2 > 32)
        putchar(UpCaseTable[z * 64 + z2]);
    putchar('\n');
    putchar('\n');
  }

  printf("\nLowcaseTable:\n");
  for (z = 0; z < 4; z++)
  {
    for (z2 = 0; z2 < 63; z2++)
      if (z * 64 + z2 > 32)
        putchar(z * 64 + z2);
    putchar('\n');
    for (z2 = 0; z2 < 63; z2++)
      if (z * 64 + z2 > 32)
        putchar(LowCaseTable[z * 64 + z2]);
    putchar('\n');
    putchar('\n');
  }

  printf("\nCollateTable:\n");
  for (z = 0; z < 4; z++)
  {
    for (z2 = 0; z2 < 63; z2++)
      if (z * 64 + z2 > 32)
        putchar(z * 64 + z2);
    putchar ('\n');
    for (z2 = 0; z2 < 63; z2++)
      if (z * 64 + z2 > 32)
        putchar(CollateTable[z * 64 + z2]);
    putchar ('\n');
    putchar('\n');
  }

  exit(0);
#endif
}

void NLS_GetCountryInfo(NLS_CountryInfo *Info)
{
  *Info = NLSInfo;
}

void NLS_DateString(Word Year, Word Month, Word Day, char *Dest)
{
  switch (NLSInfo.DateFmt)
  {
    case DateFormatMTY:
      sprintf(Dest, "%d%s%d%s%d", Month, NLSInfo.DateSep, Day, NLSInfo.DateSep, Year);
      break;
    case DateFormatTMY:
      sprintf(Dest, "%d%s%d%s%d", Day, NLSInfo.DateSep, Month, NLSInfo.DateSep, Year);
      break;
    case DateFormatYMT:
      sprintf(Dest, "%d%s%d%s%d", Year, NLSInfo.DateSep, Month, NLSInfo.DateSep, Day);
      break;
  }
}

void NLS_CurrDateString(char *Dest)
{
  time_t timep;
  struct tm *trec;

  time(&timep);
  trec = localtime(&timep);
  NLS_DateString(trec->tm_year + 1900, trec->tm_mon + 1, trec->tm_mday, Dest);
}

void NLS_TimeString(Word Hour, Word Minute, Word Second, Word Sec100, char *Dest)
{
  Word OriHour;
  String ext;

  OriHour = Hour;
  if (NLSInfo.TimeFmt == TimeFormatUSA)
  {
    Hour %= 12;
    if (Hour == 0)
      Hour = 12;
  }
  sprintf(Dest, "%d%s%02d%s%02d", Hour, NLSInfo.TimeSep, Minute, NLSInfo.TimeSep, Second);
  if (Sec100 < 100)
  {
    sprintf(ext, "%s%02d", NLSInfo.DecSep, Sec100);
    strcat(Dest, ext);
  }
  if (NLSInfo.TimeFmt == TimeFormatUSA)
    strcat(Dest, (OriHour > 12) ? "p" : "a");
}

void NLS_CurrTimeString(Boolean Use100, char *Dest)
{
  time_t timep;
  struct tm *trec;

  time(&timep); trec = localtime(&timep);
  NLS_TimeString(trec->tm_hour, trec->tm_min, trec->tm_sec, Use100 ? 0 : 100, Dest);
}

void NLS_CurrencyString(double inp, char *erg)
{
  char s[1024], form[1024];
  char *p, *z;

  /* Schritt 1: mit passender Nachkommastellenzahl wandeln */

  sprintf(form, "%%0.%df", NLSInfo.CurrDecimals);
  sprintf(s, form, inp);

  /* Schritt 2: vorne den Punkt suchen */

  p = (NLSInfo.CurrDecimals == 0) ? s + strlen(s) : strchr(s, '.');

  /* Schritt 3: Tausenderstellen einfuegen */

  z = p;
  while (z - s > 3)
  {
    strins(s, NLSInfo.ThouSep, z - s - 3);
    z -= 3;
    p += strlen(NLSInfo.ThouSep);
  };

  /* Schritt 4: Komma anpassen */

  strmov(p, p + 1);
  strins(s, NLSInfo.DecSep, p - s);

  /* Schritt 5: Einheit anbauen */

  switch (NLSInfo.CurrFmt)
  {
    case CurrFormatPreNoBlank:
      sprintf(erg, "%s%s", NLSInfo.Currency, s);
      break;
    case CurrFormatPreBlank:
      sprintf(erg, "%s %s", NLSInfo.Currency, s);
      break;
    case CurrFormatPostNoBlank:
      sprintf(erg, "%s%s", s, NLSInfo.Currency);
      break;
    case CurrFormatPostBlank:
      sprintf(erg, "%s%s", s, NLSInfo.Currency);
      break;
    default:
      strmov(p, p + strlen(NLSInfo.DecSep));
      strins(NLSInfo.Currency, s, p - s);
  }
}

char Upcase(char inp)
{
  return UpCaseTable[((unsigned int) inp) & 0xff];
}

void NLS_UpString(char *s)
{
  char *z;

  for (z = s; *z != '\0'; z++)
    *z = UpCaseTable[((unsigned int)*z) & 0xff];
}

void NLS_LowString(char *s)
{
  char *z;

  for (z = s; *z != '\0'; z++)
    *z = LowCaseTable[((unsigned int)*z) & 0xff];
}

int NLS_StrCmp(const char *s1, const char *s2)
{
  while (CollateTable[((unsigned int)*s1) & 0xff] == CollateTable[((unsigned int)*s2) & 0xff])
  {
    if ((!*s1) && (!*s2))
      return 0;
    s1++;
    s2++;
  }
  return ((int) CollateTable[((unsigned int)*s1) & 0xff] - CollateTable[((unsigned int)*s2) & 0xff]);
}

void nls_init(void)
{
#ifdef LOCALE_NLS
  (void) setlocale(LC_TIME, "");
  (void) setlocale(LC_MONETARY, "");
#endif
}
