/* nls.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Abhandlung landesspezifischer Unterschiede                                */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <time.h>
#include <ctype.h>
#ifndef BRAINDEAD_SYSTEM_WITHOUT_NLS
#include <locale.h>
#include <langinfo.h>
#endif

#include "stringutil.h"

#include "nls.h"

CharTable UpCaseTable;               /* Umsetzungstabellen */
CharTable LowCaseTable;

/**
{ NLS-Datenstrukturen neu initialisieren: }

        ***PROCEDURE NLS_Initialize;

{ kompletten Datensatz abfragen, da diese Unit nicht alles ausnutzt: }

        ***PROCEDURE NLS_GetCountryInfo(VAR Info:NLS_CountryInfo);

{ ein Datum in das korrekte Landesformat umsetzen: }

        ***FUNCTION  NLS_DateString(Year,Month,Day:Word):String;

{ dito fuer aktuelles Datum: }

        ***FUNCTION  NLS_CurrDateString:String;

{ eine Zeitangabe in das korrekte Landesformat umsetzen: }

        ***FUNCTION  NLS_TimeString(Hour,Minute,Second,Sec100:Word):String;

{ dito fuer aktuelle Zeit: }    { mit/ohne Hundertstel }
                                        { v }
        ***FUNCTION  NLS_CurrTimeString(Use100:Boolean):String;

{ einen Waehrungsbetrag wandeln: }

        FUNCTION  NLS_CurrencyString(inp:Extended):String;


{ ein Zeichen in Grossbuchstaben umsetzen; ersetzt Standard-UpCase }

        FUNCTION  UpCase(inp:Char):Char;

{ einen ganzen String in Grossbuchstaben umsetzen }

        PROCEDURE NLS_UpString(VAR s:String);

{ zwei Strings vergleichen: liefert -1, 0, 1, falls s1 <, =, > s2
  ACHTUNG! Falls 0 geliefert wird, muss dieses nicht notwendigerweise heissen,
  dass die Strings identisch sind! }

        FUNCTION NLS_StrCmp(s1,s2:String):ShortInt;

IMPLEMENTATION**/

static NLS_CountryInfo NLSInfo;
/**static CharTable CollateTable;**/

/**
{ Eine Zahl nicht mit Leerzeichen, sondern mit Nullen aufzufuellen, gehoert
  (neben dem vorzeichenlosen 32-Bit-Integer) zu den Dingen, die ich mir
  immer schon gewuenscht hatte.  Vielleicht erhoert Borland ja irgendwann
  einmal mein Flehen, anstelle die Welt mit OOP zu "begluecken" }

        FUNCTION StNull(inp:Word; Stellen:Byte):String;
VAR
   s:String;
BEGIN
   Str(inp,s); WHILE Length(s)<Stellen DO s:='0'+s;
   StNull:=s;
END;

{ einen String anhand einer Tabelle uebersetzen: }

        PROCEDURE TranslateString(VAR s:String; VAR Table:CharTable);

{$IFDEF SPEEDUP}

        Assembler;
ASM
        mov     dx,ds           { Datensegment retten }
        lds     bx,[Table]      { Zeiger auf Uebersetzungstabelle }
        les     si,[s]          { Zeiger aus String }
        seges   lodsb           { Laengenbyte holen }
        sub     cx,cx           { auf 16 Bit aufblasen }
        mov     cl,al
        jcxz    @Null           { 64K Durchlaeufe vermeiden }
        mov     di,si           { Ziel=Quellzeiger }
        cld
@schl:  seges   lodsb           { ein Zeichen laden... }
        xlat                    { ...uebersetzen... }
        stosb                   { ...ablegen }
        loop    @schl
@Null:  mov     ds,dx           { Datensegment wiederherstellen }
END;

{$ELSE}

VAR
   z:Integer;
BEGIN
   FOR z:=1 TO Length(s) DO s[z]:=Table[s[z]];
END;

{$ENDIF}*/

/*-------------------------------------------------------------------------------*/
/* Da es moeglich ist, die aktuelle Codeseite im Programmlauf zu wechseln,
   ist die Initialisierung in einer getrennten Routine untergebracht.  Nach
   einem Wechsel stellt ein erneuter Aufruf wieder korrekte Verhaeltnisse
   her.  Wen das stoert, der schreibe einfach einen Aufruf in den Initiali-
   sierungsteil der Unit hinein. */

        void NLS_Initialize(void)
BEGIN
#ifndef BRAINDEAD_SYSTEM_WITHOUT_NLS
   struct lconv *lc;
#endif
   char *tmpstr,*run,*cpy;
   Word FmtBuffer;
   Integer z;

   /* Zeit/Datums/Waehrungsformat holen */

#ifdef BRAINDEAD_SYSTEM_WITHOUT_NLS
   NLSInfo.DecSep=".";
   NLSInfo.ThouSep=",";
   NLSInfo.Currency="$";
   NLSInfo.CurrDecimals=2;
   NLSInfo.CurrFmt=CurrFormatPreNoBlank;
#else
   lc=localeconv();

   NLSInfo.DecSep=(lc->decimal_point!=Nil)?lc->decimal_point:".";

   NLSInfo.ThouSep=(lc->thousands_sep!=Nil)?lc->thousands_sep:",";

   NLSInfo.Currency=(lc->currency_symbol!=Nil)?lc->currency_symbol:"$";

   NLSInfo.CurrDecimals=lc->int_frac_digits;

   if (lc->p_cs_precedes)
    if (lc->p_sep_by_space) NLSInfo.CurrFmt=CurrFormatPreBlank;
    else NLSInfo.CurrFmt=CurrFormatPreNoBlank;
   else
    if (lc->p_sep_by_space) NLSInfo.CurrFmt=CurrFormatPostBlank;
    else NLSInfo.CurrFmt=CurrFormatPostNoBlank;
#endif

#ifdef BRAINDEAD_SYSTEM_WITHOUT_NLS
   tmpstr="%m/%d/%y";
#else
   tmpstr=nl_langinfo(D_FMT);
#endif
   NLSInfo.DateSep=Nil; FmtBuffer=0; run=tmpstr;
   while (*run!='\0')
    if (*run=='%')
     BEGIN
      FmtBuffer<<=4;
      switch (toupper(*(++run)))
       BEGIN
        case 'D': FmtBuffer+=1; break;
        case 'M': FmtBuffer+=2; break;
        case 'Y': FmtBuffer+=3; break;
       END
      if (NLSInfo.DateSep==Nil)
       BEGIN
        run++; cpy=NLSInfo.DateSep=strdup("                  ");
        while ((*run!=' ') AND (*run!='%')) *(cpy++)=*(run++);
        *cpy='\0';
       END
      else run++;
     END
    else run++;
   if (FmtBuffer==0x213) NLSInfo.DateFmt=DateFormatMTY;
   else if (FmtBuffer==0x123) NLSInfo.DateFmt=DateFormatTMY;
   else NLSInfo.DateFmt=DateFormatYMT;

#ifdef BRAINDEAD_SYSTEM_WITHOUT_NLS
   tmpstr="%H:%M:%S";
#else
   tmpstr=nl_langinfo(T_FMT);
#endif
   NLSInfo.TimeSep=Nil; FmtBuffer=0; run=tmpstr;
   while (*run!='\0')
    if (*run=='%')
     BEGIN
      FmtBuffer<<=4;
      switch (toupper(*(++run)))
       BEGIN
        case 'S': FmtBuffer+=1; break;
        case 'M': FmtBuffer+=2; break;
        case 'H': FmtBuffer+=3; break;
       END
      if (NLSInfo.TimeSep==Nil)
       BEGIN
        run++; cpy=NLSInfo.TimeSep=strdup("                  ");
        while ((*run!=' ') AND (*run!='%')) *(cpy++)=*(run++);
        *cpy='\0';
       END
      else run++;
     END
    else run++;
    NLSInfo.TimeFmt=TimeFormatEurope;

    /* Tabelle klein-->gross */

    for (z=0; z<256; z++) UpCaseTable[z]=toupper(z);

    /* umgekehrte Tabelle */

    for (z=0; z<256; z++) LowCaseTable[z]=tolower(z);

END

        void NLS_GetCountryInfo(NLS_CountryInfo *Info)
BEGIN
   *Info=NLSInfo;
END

        void NLS_DateString(Word Year, Word Month, Word Day, char *Dest)
BEGIN
   switch (NLSInfo.DateFmt)
    BEGIN
     case DateFormatMTY:
      sprintf(Dest,"%d%s%d%s%d",Month,NLSInfo.DateSep,Day,NLSInfo.DateSep,Year); break;
     case DateFormatTMY:
      sprintf(Dest,"%d%s%d%s%d",Day,NLSInfo.DateSep,Month,NLSInfo.DateSep,Year); break;
     case DateFormatYMT:
      sprintf(Dest,"%d%s%d%s%d",Year,NLSInfo.DateSep,Month,NLSInfo.DateSep,Day); break;
    END
END

	void NLS_CurrDateString(char *Dest)
BEGIN
   time_t timep;
   struct tm *trec;

   time(&timep); trec=localtime(&timep);
   NLS_DateString(trec->tm_year+1900,trec->tm_mon+1,trec->tm_mday,Dest);
END

        void NLS_TimeString(Word Hour, Word Minute, Word Second, Word Sec100, char *Dest)
BEGIN
   Word OriHour;
   String ext;

   OriHour=Hour;
   if (NLSInfo.TimeFmt==TimeFormatUSA)
    BEGIN
     Hour%=12; if (Hour==0) Hour=12;
    END
   sprintf(Dest,"%d%s%02d%s%02d",Hour,NLSInfo.TimeSep,Minute,NLSInfo.TimeSep,Second);
   if (Sec100<100)
    BEGIN
     sprintf(ext,"%s%02d",NLSInfo.DecSep,Sec100); strcat(Dest,ext);
    END
   if (NLSInfo.TimeFmt==TimeFormatUSA)
    strcat(Dest,(OriHour>12)?"p":"a");
END

	void NLS_CurrTimeString(Boolean Use100, char *Dest)
BEGIN
   time_t timep;
   struct tm *trec;

   time(&timep); trec=localtime(&timep);
   NLS_TimeString(trec->tm_hour,trec->tm_min,trec->tm_sec,100,Dest);
END
/**
        FUNCTION NLS_CurrencyString(inp:Extended):String;
VAR
   s:String;
   p:Byte;
   z:Integer;
BEGIN
   WITH NLSInfo DO
    BEGIN
     { Schritt 1: mit passender Nachkommastellenzahl wandeln }

     Str(inp:0:CurrDecimals,s);

     { Schritt 2: vorne den Punkt suchen }

     IF CurrDecimals=0 THEN p:=Length(s)+1 ELSE p:=Pos('.',s);

     { Schritt 3: Tausenderstellen einfuegen }

     z:=p;
     WHILE z>4 DO
      BEGIN
       Insert(ThouSep,s,z-3); Dec(z,3); Inc(p,Length(ThouSep));
      END;

     { Schritt 4: Komma anpassen }

     Delete(s,p,1); Insert(DecSep,s,p);

     { Schritt 5: Einheit anbauen }

     CASE CurrFmt OF
     CurrFormatPreNoBlank  : NLS_CurrencyString:=Currency+s;
     CurrFormatPreBlank    : NLS_CurrencyString:=Currency+' '+s;
     CurrFormatPostNoBlank : NLS_CurrencyString:=s+Currency;
     CurrFormatPostBlank   : NLS_CurrencyString:=s+' '+Currency;
     ELSE
      BEGIN
       Delete(s,p,Length(DecSep)); Insert(Currency,s,p);
       NLS_CurrencyString:=s;
      END;
     END;
    END;
END;

        FUNCTION Upcase(inp:Char):Char;

{$IFDEF SPEEDUP}

        Assembler;
ASM
        lea     bx,[UpCaseTable]{ Adresse Umsetzungstabelle }
        mov     al,inp          { Zeichen holen... }
        xlat                    { fertig }
END;

{$ELSE}

BEGIN
   UpCase:=UpCaseTable[inp];
END;

{$ENDIF}**/

        void NLS_UpString(char *s)
BEGIN
   char *z;

   for (z=s; *z!='\0'; z++) *z=UpCaseTable[(int)*z];
END

        void NLS_LowString(char *s)
BEGIN
   char *z;

   for (z=s; *z!='\0'; z++) *z=LowCaseTable[(int)*z];
END
/**
        FUNCTION NLS_StrCmp(s1,s2:String):ShortInt;
BEGIN
   TranslateString(s1,CollateTable);
   TranslateString(s2,CollateTable);
   IF s1>s2 THEN NLS_StrCmp:=1
   ELSE IF s1=s2 THEN NLS_StrCmp:=0
   ELSE NLS_StrCmp:=-1;
END;**/

	void nls_init(void)
BEGIN
#ifndef BRAINDEAD_SYSTEM_WITHOUT_NLS
   (void) setlocale(LC_ALL,"");
/*   printf("%s\n",setlocale(LC_ALL,NULL));*/
#endif
END
