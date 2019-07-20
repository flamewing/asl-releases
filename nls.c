/* nls.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Abhandlung landesspezifischer Unterschiede                                */
/*                                                                           */
/*****************************************************************************/

#undef DEBUG_NLS

#include "stdinc.h"
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "strutil.h"

#include "nls.h"

/*-------------------------------------------------------------------------------*/

typedef struct
{
  Word Country;        /* = internationale Vorwahl */
  Word CodePage;       /* mom. gewaehlter Zeichensatz */
  void (*DateString)(Word Year, Word Month, Word Day, char *Dest);
  void (*TimeString)(Word Hour, Word Minute, Word Second, Word Sec100, char *Dest);
#if (defined OS2_NLS) || (defined DOS_NLS)
  DateFormat DateFmt;  /* Datumsreihenfolge */
  char *DateSep;       /* Trennzeichen zwischen Datumskomponenten */
  TimeFormat TimeFmt;  /* 12/24-Stundenanzeige */
  char *TimeSep;       /* Trennzeichen zwischen Zeitkomponenten */
#elif defined LOCALE_NLS
  const char *DateFmtStr;
  const char *TimeFmtStr;
#endif
  char *Currency;      /* Waehrungsname */
  CurrFormat CurrFmt;  /* Anzeigeformat Waehrung */
  Byte CurrDecimals;   /* Nachkommastellen Waehrungsbetraege */
  char *ThouSep;       /* Trennzeichen fuer Tausenderbloecke */
  char *DecSep;        /* Trennzeichen fuer Nachkommastellen */
  char *DataSep;       /* ??? */
  Boolean Initialized;
} NLS_CountryInfo;

/*-------------------------------------------------------------------------------*/

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

static void DumpNLSInfo(void)
{
#ifdef DEBUG_NLS
  int z, z2;

  printf("Country      = %d\n", NLSInfo.Country);
  printf("CodePage     = %d\n", NLSInfo.CodePage);
#if (defined OS2_NLS) || (defined DOS_NLS)
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
#elif defined LOCALE_NLS
  printf("DateFmtStr   = %s\n", NLSInfo.DateFmtStr);
  printf("TimeFmtStr   = %s\n", NLSInfo.TimeFmtStr);
#endif
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
#endif /* DEBUG_NLS */
}

/*-------------------------------------------------------------------------------*/

#if (defined OS2_NLS) || (defined DOS_NLS)

static void DOS_OS2_DateString(Word Year, Word Month, Word Day, char *Dest)
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

static void DOS_OS2_TimeString(Word Hour, Word Minute, Word Second, Word Sec100, char *Dest)
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
#endif /* OS2_NLS || DOS_NLS */

/*-------------------------------------------------------------------------------*/

#if defined OS2_NLS

#define INCL_DOSNLS
#include <os2.h>

static void QueryInfo(void)
{
  COUNTRYCODE ccode;
  COUNTRYINFO cinfo;
  ULONG erglen;
  int z;

  ccode.country = 0;
  ccode.codepage = 0;
  DosQueryCtryInfo(sizeof(cinfo), &ccode, &cinfo, &erglen);

  NLSInfo.Country = cinfo.country;
  NLSInfo.CodePage = cinfo.codepage;
  NLSInfo.DecSep = as_strdup(cinfo.szDecimal);
  NLSInfo.DataSep = as_strdup(cinfo.szDataSeparator);
  NLSInfo.ThouSep = as_strdup(cinfo.szThousandsSeparator);
  NLSInfo.Currency = as_strdup(cinfo.szCurrency);
  NLSInfo.CurrDecimals = cinfo.cDecimalPlace;
  NLSInfo.CurrFmt = (CurrFormat) cinfo.fsCurrencyFmt;
  NLSInfo.DateFmt = (DateFormat) cinfo.fsDateFmt;
  NLSInfo.DateSep = as_strdup(cinfo.szDateSeparator);
  NLSInfo.DateString = DOS_OS2_DateString;
  NLSInfo.TimeFmt = (TimeFormat) cinfo.fsTimeFmt;
  NLSInfo.TimeSep = as_strdup(cinfo.szTimeSeparator);
  NLSInfo.TimeString = DOS_OS2_TimeString;
  for (z = 0; z < 256; z++)
    UpCaseTable[z] = (char) z;
  for (z = 'a'; z <= 'z'; z++)
    UpCaseTable[z] -= 'a' - 'A';
  DosMapCase(sizeof(UpCaseTable), &ccode, UpCaseTable);
  for (z = 0; z < 256; z++)
    LowCaseTable[z] = (char) z;
  for (z = 255; z >= 0; z--)
    if (UpCaseTable[z] != (char) z)
      LowCaseTable[((unsigned int) UpCaseTable[z])&0xff] = (char) z;
  for (z = 0; z < 256; z++)
    CollateTable[z] = (char) z;
  DosQueryCollate(sizeof(CollateTable), &ccode, CollateTable, &erglen);
}

#elif defined DOS_NLS

#include <dos.h>

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

static void QueryInfo(void)
{
  union REGS Regs;
  struct SREGS SRegs;
  struct COUNTRY country_info;
  DosTableRec DOSTablePtr;
  int z;

  if (_osmajor < 3)
  {
    fprintf(stderr, "requires DOS 3.x or above\n");
    exit(255);
  }

  if (_osminor < 30)
    NLSInfo.CodePage = 437;
  else
  {
    Regs.x.ax = 0x6601;
    int86(0x21, &Regs, &Regs);
    NLSInfo.CodePage = Regs.x.bx;
  }

  country(0x0000, &country_info);

  NLSInfo.DecSep = DosCopy(country_info.co_desep, 2);
  NLSInfo.DataSep = DosCopy(country_info.co_dtsep, 2);
  NLSInfo.ThouSep = DosCopy(country_info.co_thsep, 2);
  NLSInfo.Currency = DosCopy(country_info.co_curr, 5);
  NLSInfo.CurrDecimals = country_info.co_digits;
  NLSInfo.CurrFmt = (CurrFormat) country_info.co_currstyle;

  NLSInfo.Country = strcmp(NLSInfo.Currency, "DM") ? 1 : 49;

  NLSInfo.DateFmt = (DateFormat) country_info.co_date;
  NLSInfo.DateSep = DosCopy(country_info.co_dtsep, 2);
  NLSInfo.DateString = DOS_OS2_DateString;

  NLSInfo.TimeFmt = (TimeFormat) country_info.co_time;
  NLSInfo.TimeSep = DosCopy(country_info.co_tmsep, 2);
  NLSInfo.TimeString = DOS_OS2_TimeString;

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
#endif /* __DPMI16__ */

  for (z = 0; z < 256; z++)
    LowCaseTable[z] = (char) z;
  for (z = 255; z >= 0; z--)
    if (UpCaseTable[z] != (char) z)
      LowCaseTable[((unsigned int) UpCaseTable[z])&0xff] = (char) z;

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
#endif /* __DPMI16__ */
}

#elif defined LOCALE_NLS

#include <locale.h>
#include <langinfo.h>

static void Locale_DateString(Word Year, Word Month, Word Day, char *Dest)
{
  struct tm tm;

  tm.tm_year = Year - 1900;
  tm.tm_mon = Month - 1;
  tm.tm_mday = Day;
  tm.tm_hour = 0;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  strftime(Dest, STRINGSIZE, NLSInfo.DateFmtStr, &tm);
}

static void Locale_TimeString(Word Hour, Word Minute, Word Second, Word Sec100, char *Dest)
{
  struct tm tm;

  (void)Sec100;
  tm.tm_year = 0;
  tm.tm_mon = 0;
  tm.tm_mday = 1;
  tm.tm_hour = Hour;
  tm.tm_min = Minute;
  tm.tm_sec = Second;
  strftime(Dest, STRINGSIZE, NLSInfo.TimeFmtStr, &tm);
}

static void QueryInfo(void)
{
  struct lconv *lc;
  int z;

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

  NLSInfo.DateFmtStr = nl_langinfo(D_FMT);
  if (!NLSInfo.DateFmtStr || !*NLSInfo.DateFmtStr)
    NLSInfo.DateFmtStr = "%m/%d/%y";
  NLSInfo.DateString = Locale_DateString;

  NLSInfo.TimeFmtStr = nl_langinfo(T_FMT);
  if (!NLSInfo.TimeFmtStr || !*NLSInfo.TimeFmtStr)
    NLSInfo.TimeFmtStr = "%H:%M:%S";
  NLSInfo.TimeString = Locale_TimeString;

  for (z = 0; z < 256; z++)
    UpCaseTable[z] = toupper(z);

  for (z = 0; z < 256; z++)
    LowCaseTable[z] = tolower(z);

  for (z = 0; z < 256; z++)
    CollateTable[z] = z;
  for (z = 'a'; z <= 'z'; z++)
    CollateTable[z] = toupper(z);
}

#else /* NO_NLS */

static void Default_DateString(Word Year, Word Month, Word Day, char *Dest)
{
  sprintf(Dest, "%u/%u/%u", Month, Day, Year);
}

static void Default_TimeString(Word Hour, Word Minute, Word Second, Word Sec100, char *Dest)
{
  (void)Sec100;
  sprintf(Dest, "%u:%u:%u", Hour, Minute, Second);
}

static void QueryInfo(void)
{
  int z;

  NLSInfo.DecSep = ".";
  NLSInfo.DataSep = ",";
  NLSInfo.ThouSep = ",";
  NLSInfo.Currency = "$";
  NLSInfo.CurrDecimals = 2;
  NLSInfo.CurrFmt = CurrFormatPreNoBlank;
  NLSInfo.DateString = Default_DateString;
  NLSInfo.TimeString = Default_TimeString;

  for (z = 0; z < 256; z++)
    UpCaseTable[z] = toupper(z);

  for (z = 0; z < 256; z++)
    LowCaseTable[z] = tolower(z);

  for (z = 0; z < 256; z++)
    CollateTable[z] = z;
  for (z = 'a'; z <= 'z'; z++)
    CollateTable[z] = toupper(z);
}

#endif /* xxx_NLS */

/*-------------------------------------------------------------------------------*/
/* Da es moeglich ist, die aktuelle Codeseite im Programmlauf zu wechseln,
   ist die Initialisierung in einer getrennten Routine untergebracht.  Nach
   einem Wechsel stellt ein erneuter Aufruf wieder korrekte Verhaeltnisse
   her.  Wen das stoert, der schreibe einfach einen Aufruf in den Initiali-
   sierungsteil der Unit hinein. */

void NLS_Initialize(void)
{
  QueryInfo();

  DumpNLSInfo();

  NLSInfo.Initialized = True;
}

void NLS_DateString(Word Year, Word Month, Word Day, char *Dest)
{
  if (!NLSInfo.DateString)
  {
    fprintf(stderr, "NLS not yet initialized\n");
    exit(255);
  }
  NLSInfo.DateString(Year, Month, Day, Dest);
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
  if (!NLSInfo.TimeString)
  {
    fprintf(stderr, "NLS not yet initialized\n");
    exit(255);
  }
  NLSInfo.TimeString(Hour, Minute, Second, Sec100, Dest);
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

Word NLS_GetCountryCode(void)
{
  if (!NLSInfo.Initialized)
  {
    fprintf(stderr, "NLS_GetCountryCode() called before initialization\n");
    exit(255);
  }
  return NLSInfo.Country;
}

void nls_init(void)
{
#ifdef LOCALE_NLS
  (void) setlocale(LC_TIME, "");
  (void) setlocale(LC_MONETARY, "");
#endif
}
