#ifndef _NLS_H
#define _NLS_H
/* nls.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Abhandlung landesspezifischer Unterschiede                                */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

typedef enum {TimeFormatUSA,TimeFormatEurope,TimeFormatJapan} TimeFormat;
typedef enum {DateFormatMTY,DateFormatTMY,DateFormatYMT} DateFormat;
typedef enum {CurrFormatPreNoBlank,CurrFormatPostNoBlank,
              CurrFormatPreBlank  ,CurrFormatPostBlank  } CurrFormat;

typedef struct
         {
          Word Country;        /* = internationale Vorwahl */
          Word CodePage;       /* mom. gewaehlter Zeichensatz */
           DateFormat DateFmt;  /* Datumsreihenfolge */
           char *DateSep;       /* Trennzeichen zwischen Datumskomponenten */
          TimeFormat TimeFmt;  /* 12/24-Stundenanzeige */
           char *TimeSep;       /* Trennzeichen zwischen Zeitkomponenten */
           char *Currency;      /* Waehrungsname */
           CurrFormat CurrFmt;  /* Anzeigeformat Waehrung */
           Byte CurrDecimals;   /* Nachkommastellen Waehrungsbetraege */
           char *ThouSep;       /* Trennzeichen fuer Tausenderbloecke */
           char *DecSep;        /* Trennzeichen fuer Nachkommastellen */
          char *DataSep;       /* ??? */
         } NLS_CountryInfo;

typedef char CharTable[256];

extern CharTable UpCaseTable,LowCaseTable;


extern void NLS_Initialize(void);

extern void NLS_GetCountryInfo(NLS_CountryInfo *Info);

extern void NLS_DateString(Word Year, Word Month, Word Day, char *Dest);

extern void NLS_CurrDateString(char *Dest);

extern void NLS_TimeString(Word Hour, Word Minute, Word Second, Word Sec100, char *Dest);

extern void NLS_CurrTimeString(Boolean Use100, char *Dest);

extern void NLS_CurrencyString(double inp, char *erg);

extern char Upcase(char inp);

extern void NLS_UpString(char *s);

extern void NLS_LowString(char *s);

extern int NLS_StrCmp(const char *s1, const char *s2);


extern void nls_init(void);
#endif /* _NLS_H */
