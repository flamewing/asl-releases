{$r-,n+,v-,s-}

        UNIT NLS;

{***************************************************************************}
{*                                                                         *}
{*   NLS - Eine Unit zur Unterstuetzung landesspezifischer Unterschiede    *}
{*         fuer Borland Pascal.                                            *}
{*                                                                         *}
{*   Autor: Alfred Arnold               E-Mail: a.arnold@kfa-juelich.de    *}
{*          Praelat-Lewen-Str.3                                            *}
{*          53819 Neunkirchen-1                                            *}
{*                                                                         *}
{*   Diese Unit ist sowohl zur Benutzung unter DOS, DPMI oder auch OS/2    *}
{*   konzipiert.  Sie ist frei kopier- und benutzbar!                      *}
{*                                                                         *}
{*   Vielen Dank an Ingo T. Storm fuer die Hilfe bei der OS/2-Implementa-  *}
{*   tion!                                                                 *}
{*                                                                         *}
{*   Literatur: - Arne Schaepers:                                          *}
{*                DOS 5 fuer Programmierer - Die endgueltige Referenz      *}
{*                Addison-Weseley 1991                                     *}
{*              - IBM Operating System/2 Programming Tools and Information:*}
{*                Version 1.2 Control Program Programming Reference        *}
{*                IBM Corporation 1989                                     *}
{*                                                                         *}
{***************************************************************************}

{$IFNDEF VIRTUALPASCAL}
{$DEFINE SPEEDUP}   { <--entfernen, falls reines Pascal gewuenscht }
{$ENDIF}

INTERFACE
        USES
        {$IFDEF VIRTUALPASCAL}
        OS2Def,OS2Base,
        {$ENDIF}
        {$IFDEF WINDOWS}
        WinDOS;
        {$ELSE}
        DOS;
        {$ENDIF}

{ Da DOS und OS/2 leicht unterschiedliche Datenstrukturen verwenden und die
  Stringformate fuer Pascal nicht optimal sind (s.u.), habe ich eine eigene
  Datenstruktur definiert, in der zusaetzlich die Daten etwas besser sortiert
  sind: }

TYPE
   TimeFormat=(TimeFormatUSA,TimeFormatEurope,TimeFormatJapan);
   DateFormat=(DateFormatMTY,DateFormatTMY,DateFormatYMT);
   CurrFormat=(CurrFormatPreNoBlank,CurrFormatPostNoBlank,
               CurrFormatPreBlank  ,CurrFormatPostBlank  );
   NLS_CountryInfo=RECORD
                    Country:Word;           { = internationale Vorwahl }
                    CodePage:Word;          { mom. gewaehlter Zeichensatz }
                    DateFmt:DateFormat;     { Datumsreihenfolge }
                    DateSep:String[2];      { Trennzeichen zwischen Datumskomponenten }
                    TimeFmt:TimeFormat;     { 12/24-Stundenanzeige }
                    TimeSep:String[2];      { Trennzeichen zwischen Zeitkomponenten }
                    Currency:String[5];     { Waehrungsname }
                    CurrFmt:CurrFormat;     { Anzeigeformat Waehrung }
                    CurrDecimals:Byte;      { Nachkommastellen Waehrungsbetraege }
                    ThouSep:String[2];      { Trennzeichen fuer Tausenderbloecke }
                    DecSep:String[2];       { Trennzeichen fuer Nachkommastellen }
                    DataSep:String[2];      { ??? }
                   END;
   CharTable=ARRAY[Char] OF Char;

{$IFDEF VIRTUALPASCAL}
   DateTimeType=LongInt;
{$ELSE}
   DateTimeType=Word;
{$ENDIF}

VAR
{$IFDEF OS2}
   NLSResult:Word;                      { Ergebnis des letzten OS/2-Aufrufes }
{$ENDIF}
   UpCaseTable:CharTable;               { Umsetzungstabellen }
   LowCaseTable:CharTable;

{ NLS-Datenstrukturen neu initialisieren: }

        PROCEDURE NLS_Initialize;

{ kompletten Datensatz abfragen, da diese Unit nicht alles ausnutzt: }

        PROCEDURE NLS_GetCountryInfo(VAR Info:NLS_CountryInfo);

{ ein Datum in das korrekte Landesformat umsetzen: }

        FUNCTION  NLS_DateString(Year,Month,Day:Word):String;

{ dito fuer aktuelles Datum: }

        FUNCTION  NLS_CurrDateString:String;

{ eine Zeitangabe in das korrekte Landesformat umsetzen: }

        FUNCTION  NLS_TimeString(Hour,Minute,Second,Sec100:Word):String;

{ dito fuer aktuelle Zeit: }    { mit/ohne Hundertstel }
                                        { v }
        FUNCTION  NLS_CurrTimeString(Use100:Boolean):String;

{ einen Waehrungsbetrag wandeln: }

        FUNCTION  NLS_CurrencyString(inp:Extended):String;


{ ein Zeichen in Groábuchstaben umsetzen; ersetzt Standard-UpCase }

        FUNCTION  UpCase(inp:Char):Char;

{ einen ganzen String in Grossbuchstaben umsetzen }

        PROCEDURE NLS_UpString(VAR s:String);

{ ein Zeichen in Kleinbuchstaben umsetzen; ersetzt Standard-UpCase }

        FUNCTION  LowCase(inp:Char):Char;

{ einen ganzen String in Kleinbuchstaben umsetzen }

        PROCEDURE NLS_LowString(VAR s:String);

{ ohne Beruecksichtigung der Gross/Kleinschreibung vergleichen }

        FUNCTION NLS_StrCaseCmp(s1,s2:String):Integer;
        FUNCTION NLS_LStrCaseCmp(s1,s2:String):Integer;

{ zwei Strings vergleichen: liefer -1, 0, 1, falls s1 <, =, > s2
  ACHTUNG! Falls 0 geliefert wird, muss dieses nicht notwendigerweise heissen,
  dass die Strings identisch sind! }

        FUNCTION NLS_StrCmp(s1,s2:String):ShortInt;

IMPLEMENTATION

{$IFDEF WINDOWS}
TYPE
   Registers=TRegisters;
{$ENDIF}

TYPE
   PChar=^Char;

VAR
   NLSInfo:NLS_CountryInfo;
   CollateTable:CharTable;

{ Umsetzung eines Zeichenfeldes in einen String.  StrPas geht hier nicht,
  weil es moeglich ist, daá die Zeichenfelder bis auf die letzte Stelle mit
  Zeichen aufgefuellt sind, das abschliessende NUL-Zeichen also fehlt. }

        FUNCTION Char2Str(Start:PChar; MaxLength:Byte):String;
VAR
   s:String;
BEGIN
   s:='';
   WHILE (Start^<>#0) AND (Length(s)<MaxLength) DO
    BEGIN
     Inc(Byte(s[0]));
     s[Length(s)]:=Start^;
     Inc(LongInt(Start));
    END;
   Char2Str:=s;
END;

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

{$ENDIF}

{ Die Datenstrukturen der jeweiligen Betriebssysteme.  DOS 2 faellt etwas aus
  dem Rahmen: }

{$IFDEF OS2}
 {$IFNDEF VIRTUALPASCAL}

TYPE
   OS2CountryCode=RECORD
                   Country,CodePage:Word;
                  END;

   OS2CountryInfo=RECORD
                   Country:Word;
                   CodePage:Word;
                   DateFmt:Word;
                   Currency:ARRAY[0..4] OF Char;
                   ThouSep:ARRAY[0..1] OF Char;
                   DecSep:ARRAY[0..1] OF Char;
                   DateSep:ARRAY[0..1] OF Char;
                   TimeSep:ARRAY[0..1] OF Char;
                   CurrFmt:Byte;
                   CurrDecimals:Byte;
                   TimeFmt:Byte;
                   Res1:ARRAY[0..1] OF Word;
                   DataSep:ARRAY[0..1] OF Char;
                   Res2:ARRAY[0..4] OF Byte;
                  END;

 {$ENDIF}
{$ELSE}

TYPE
   Dos2CountryInfo=RECORD
                    TimeFmt:Byte;
                    DateFmt:Byte;
                    Currency:ARRAY[0..1] OF Char;
                    ThouSep:ARRAY[0..1] OF Char;
                    DecSep:ARRAY[0..1] OF Char;
                    Res:ARRAY[0..23] OF Byte
                   END;
   Dos3CountryInfo=RECORD
                    DateFmt:Word;
                    Currency:ARRAY[0..4] OF Char;
                    ThouSep:ARRAY[0..1] OF Char;
                    DecSep:ARRAY[0..1] OF Char;
                    DateSep:ARRAY[0..1] OF Char;
                    TimeSep:ARRAY[0..1] OF Char;
                    CurrFmt:Byte;
                    CurrDecimals:Byte;
                    TimeFmt:Byte;
                    UpcasePtr:Pointer;
                    DataSep:ARRAY[0..1] OF Char;
                    Dummy:ARRAY[24..33] OF Byte;
                   END;
   DOSTableRec=RECORD
                SubFuncNo:Byte;
                Result:PChar;
               END;

{$ENDIF}

{ fuer BP-OS/2 die benoetigten DLL-Funktionen definieren }

{$IFDEF OS2}
{$IFNDEF VIRTUALPASCAL}
        FUNCTION DosGetCtryInfo(Length:Word;
                                VAR Country:OS2CountryCode;
                                VAR Info:OS2CountryInfo;
                                VAR DataLength:Word):Word;
        Far;
        EXTERNAL 'NLS';

        FUNCTION DosCaseMap(Length:Word;
                            VAR Country:OS2CountryCode;
                            Str:PChar):Word;
        Far;
        EXTERNAL 'NLS';

        FUNCTION DosGetCollate(Length:Word;
                               VAR Country:OS2CountryCode;
                               VAR Dest:CharTable;
                               VAR ErgLen:Word):Word;
        Far;
        EXTERNAL 'NLS';
{$ENDIF}
{$ENDIF}

{ fuer DPMI/Windows eine Funktion, die eine Segmentadresse  in einen Deskriptor
  umsetzt : }

{$IFDEF DPMISERVER}
        FUNCTION GetSelector(Inp:Pointer):Pointer;
BEGIN
   ASM
        mov     ax,word ptr [Inp]           { Offset kopieren }
        mov     word ptr @result,ax
        mov     word ptr @result+2,0        { Segment initialisieren }
        mov     bx,word ptr [Inp+2]         { umsetzen }
        mov     ax,0002h
        int     31h
        jc      @@1
        mov     word ptr @result+2,ax
@@1:
   END;
END;
{$ENDIF}

{ Da es moeglich ist, die aktuelle Codeseite im Programmlauf zu wechseln,
  ist die Initialisierung in einer getrennten Routine untergebracht.  Nach
  einem Wechsel stellt ein erneuter Aufruf wieder korrekte Verhaeltnisse
  her.  Wen das stoert, der schreibe einfach einen Aufruf in den Initiali-
  sierungsteil der Unit hinein. }

        PROCEDURE NLS_Initialize;
VAR
{$IFDEF OS2}
 {$IFDEF VIRTUALPASCAL}
   OS2Code      : CountryCode;
   OS2Info      : CountryInfo;
   ErgLen       : ULong;
 {$ELSE}
   OS2Code      : OS2CountryCode;
   OS2Info      : OS2CountryInfo;
   ErgLen       : Word;
 {$ENDIF}
{$ELSE}
   Regs         : Registers;
   DOS2Info     : DOS2CountryInfo;
   DOS3Info     : DOS3CountryInfo;
   DOSTablePtr  : DOSTableRec;
{$ENDIF}
   Ch           : Char;
   z            : Integer;

        PROCEDURE StandardUpCases;
BEGIN
   {$IFDEF WINDOWS}
   UpCaseTable['ä']:='Ä';
   UpCaseTable['ö']:='Ö';
   UpCaseTable['ü']:='Ü';
   {$ELSE}
   UpCaseTable['„']:='Ž';
   UpCaseTable['”']:='™';
   UpCaseTable['']:='š';
   UpCaseTable['‡']:='€';
   UpCaseTable['†']:='';
   UpCaseTable['‚']:='';
   UpCaseTable['‘']:='’';
   UpCaseTable['¤']:='¥';
   {$ENDIF}
END;

BEGIN
   { Zeit/Datums/Waehrungsformat holen }

{$IFDEF OS2}
 {$IFDEF VIRTUALPASCAL}

   OS2Code.Country:=0; OS2Code.CodePage:=0;
   NLSResult:=DosQueryCtryInfo(SizeOf(OS2Info),OS2Code,OS2Info,ErgLen);
   NLSInfo.Country:=OS2Info.Country;
   NLSInfo.CodePage:=OS2Info.CodePage;
   NLSInfo.TimeFmt:=TimeFormat(OS2Info.fsTimeFmt);
   NLSInfo.TimeSep:=Char2Str(@OS2Info.szTimeSeparator,2);
   NLSInfo.DateFmt:=DateFormat(OS2Info.fsDateFmt);
   NLSInfo.DateSep:=Char2Str(@OS2Info.szDateSeparator,2);
   NLSInfo.Currency:=Char2Str(@OS2Info.szCurrency,5);
   NLSInfo.CurrFmt:=CurrFormat(OS2Info.fsCurrencyFmt);
   NLSInfo.CurrDecimals:=OS2Info.cDecimalPlace;
   NLSInfo.ThouSep:=Char2Str(@OS2Info.szThousandsSeparator,2);
   NLSInfo.DecSep:=Char2Str(@OS2Info.szDecimal,2);
   NLSInfo.DataSep:=Char2Str(@OS2Info.szDataSeparator,2);

 {$ELSE}

   { bei BP-OS/2 die 16-Bit-DLL benutzen }

   OS2Code.Country:=0; OS2Code.CodePage:=0;
   NLSResult:=DosGetCtryInfo(SizeOf(OS2Info),OS2Code,OS2Info,ErgLen);
   NLSInfo.Country:=OS2Info.Country;
   NLSInfo.CodePage:=OS2Info.CodePage;
   NLSInfo.TimeFmt:=TimeFormat(OS2Info.TimeFmt);
   NLSInfo.TimeSep:=Char2Str(@OS2Info.TimeSep,2);
   NLSInfo.DateFmt:=DateFormat(OS2Info.DateFmt);
   NLSInfo.DateSep:=Char2Str(@OS2Info.DateSep,2);
   NLSInfo.Currency:=Char2Str(@OS2Info.Currency,5);
   NLSInfo.CurrFmt:=CurrFormat(OS2Info.CurrFmt);
   NLSInfo.CurrDecimals:=OS2Info.CurrDecimals;
   NLSInfo.ThouSep:=Char2Str(@OS2Info.ThouSep,2);
   NLSInfo.DecSep:=Char2Str(@OS2Info.DecSep,2);
   NLSInfo.DataSep:=Char2Str(@OS2Info.DataSep,2);

 {$ENDIF}
{$ELSE}

   { Codepages erst ab DOS 3.3, davor die Default-Codepage setzen }

   IF Swap(DOSVersion)<$330 THEN NLSInfo.CodePage:=437
   ELSE
    ASM
        mov     ax,6601h
        int     21h
        mov     [NLSInfo.CodePage],bx
    END;

   { VORSICHT: Die Struktur unterscheidet sich zwischen DOS 2 und DOS 3 ! }

   ASM
        call    DOSVersion
        cmp     al,3
        jb      @is2
        lea     dx,[DOS3Info]
        jmp     @is3
@is2:   lea     dx,[DOS2Info]
@is3:   push    ds
        mov     ax,ss
        mov     ds,ax
        mov     ax,3800h
        int     21h
        pop     ds
        mov     [NLSInfo.Country],bx
   END;
   IF Lo(DOSVersion)>=3 THEN
    BEGIN
     NLSInfo.TimeFmt:=TimeFormat(DOS3Info.TimeFmt);
     NLSInfo.TimeSep:=Char2Str(@DOS3Info.TimeSep,2);
     NLSInfo.DateFmt:=DateFormat(DOS3Info.DateFmt);
     NLSInfo.DateSep:=Char2Str(@DOS3Info.DateSep,2);
     NLSInfo.Currency:=Char2Str(@DOS3Info.Currency,5);
     NLSInfo.CurrFmt:=CurrFormat(DOS3Info.CurrFmt);
     NLSInfo.CurrDecimals:=DOS3Info.CurrDecimals;
     NLSInfo.ThouSep:=Char2Str(@DOS3Info.ThouSep,2);
     NLSInfo.DecSep:=Char2Str(@DOS3Info.DecSep,2);
     NLSInfo.DataSep:=Char2Str(@DOS3Info.DataSep,2);
    END

   { DOS 2 kennt noch nicht soviel, daher muessen wir selber etwas beisteuern }

   ELSE
    BEGIN
     NLSInfo.TimeFmt:=TimeFormat(DOS2Info.TimeFmt);
     CASE NLSInfo.Country OF
     41,46,47,358:NLSInfo.TimeSep:='.';
     ELSE NLSInfo.TimeSep:=':';
     END;
     NLSInfo.DateFmt:=DateFormat(DOS2Info.DateFmt);
     CASE NLSInfo.Country OF
     3,47,351,32,33,39,34:NLSInfo.DateSep:='/';
     49,358,41:NLSInfo.DateSep:='.';
     972:NLSInfo.DateSep:=' ';
     ELSE NLSInfo.DateSep:='-';
     END;
     NLSInfo.Currency:=Char2Str(@DOS2Info.Currency,2);
     CASE NLSInfo.Country OF
     1,39:NLSInfo.CurrFmt:=CurrFormatPreNoBlank;
     3,33,34,358:NLSInfo.CurrFmt:=CurrFormatPostBlank;
     ELSE NLSInfo.CurrFmt:=CurrFormatPreBlank;
     END;
     IF NLSInfo.Country=39 THEN NLSInfo.CurrDecimals:=0
     ELSE NLSInfo.CurrDecimals:=2;
     CASE NLSInfo.Country OF
     972:NLSInfo.DataSep:=' ';
     1,41:NLSInfo.DataSep:=',';
     ELSE NLSInfo.DataSep:=';';
     END;
    END;
{$ENDIF}

   { Klein->Grossbuchstabenumsetzung: Erstmal eine 1:1-Umsetzung vorbesetzen }
   { Dann die einfachen Buchstaben beruecksichtigen }
   { unter DOS-DPMI wird irgendein Zeiger noch nicht korrekt vom Extender umgesetzt }
   { DOS vor 3.3 kann es einfach noch nicht }

   FOR z:=0 TO 255 DO UpCaseTable[Chr(z)]:=Chr(z);
   FOR Ch:='a' TO 'z' DO UpCaseTable[Ch]:=Chr(Ord(Ch)-(Ord('a')-Ord('A')));

   {$IFDEF OS2}
    {$IFDEF VIRTUALPASCAL}

   NLSResult:=DosMapCase(Sizeof(UpCaseTable),OS2Code,@UpCaseTable);

    {$ELSE}

   NLSResult:=DosCaseMap(Sizeof(UpCaseTable),OS2Code,@UpCaseTable);

    {$ENDIF}
   {$ENDIF}
   {$IFDEF MSDOS}

   IF Swap(DosVersion)>=$330 THEN
    BEGIN
     WITH Regs DO
      BEGIN
       AX:=$6502; BX:=NLSInfo.CodePage; DX:=NLSInfo.Country;
       CX:=Sizeof(DOSTablePtr); ES:=Seg(DOSTablePtr); DI:=Ofs(DOSTablePtr);
      END;
     MsDOS(Regs);
     IF Regs.CX=Sizeof(DOSTablePtr) THEN
      BEGIN
       Inc(LongInt(DOSTablePtr.Result),Sizeof(Word));
       {$IFDEF DPMISERVER}
       DOSTablePtr.Result:=GetSelector(DOSTablePtr.Result);
       {$ENDIF}
       IF Lo(DOSVersion)<$20 THEN StandardUpCases
       ELSE Move(DOSTablePtr.Result^,UpCaseTable[#128],128);
      END
     ELSE StandardUpCases;
    END
   ELSE StandardUpCases;

   {$ENDIF}

   { daraus die umgekehrte Tabelle zur Berechnung groá-->klein berechnen }
   { Ausgangspunkt: gleiche Zeichen }

   FOR z:=0 TO 255 DO LowCaseTable[Chr(z)]:=Chr(z);
   FOR z:=0 TO 255 DO
    IF UpCaseTable[Chr(z)]<>Chr(z) THEN
     LowCaseTable[UpCaseTable[Chr(z)]]:=Chr(z);

   { Sortierreihenfolgetabelle erstellen.  Das geht nur unter OS/2 und DOS }
   { >=3.3, unter DPMI klappt es wieder nicht. Fuer solche Faelle einen Default }
   { annehmen. }

   FOR z:=0 TO 255 DO CollateTable[Chr(z)]:=Chr(z);
   FOR Ch:='a' TO 'z' DO CollateTable[Ch]:=Chr(Ord(Ch)-(Ord('a')-Ord('A')));

   {$IFDEF OS2}
    {$IFDEF VIRTUALPASCAL}

   NLSResult:=DosQueryCollate(Sizeof(CharTable),OS2Code,@CollateTable,ErgLen);

    {$ELSE}

   NLSResult:=DosGetCollate(Sizeof(CharTable),OS2Code,CollateTable,ErgLen);

    {$ENDIF}
   {$ENDIF}
   {$IFDEF MSDOS}

   IF Swap(DOSVersion)>=$330 THEN
    BEGIN
     WITH Regs DO
      BEGIN
       AX:=$6506; BX:=NLSInfo.CodePage; DX:=NLSInfo.Country;
       CX:=Sizeof(DOSTablePtr); ES:=Seg(DOSTablePtr); DI:=Ofs(DOSTablePtr);
      END;
     MsDOS(Regs);
     IF Regs.CX=Sizeof(DOSTablePtr) THEN
      BEGIN
       Inc(LongInt(DOSTablePtr.Result),Sizeof(Word));
       {$IFDEF DPMISERVER}
       DOSTablePtr.Result:=GetSelector(DOSTablePtr.Result);
       {$ENDIF}
       IF Lo(DOSVersion)>=$20 THEN
       Move(DOSTablePtr.Result^,CollateTable,256);
      END
    END;

   {$ENDIF}
END;

        PROCEDURE NLS_GetCountryInfo(VAR Info:NLS_CountryInfo);
BEGIN
   Info:=NLSInfo;
END;

        FUNCTION NLS_DateString(Year,Month,Day:Word):String;
BEGIN
   WITH NLSInfo DO
    CASE DateFmt OF
    DateFormatMTY:
     NLS_DateString:=StNull(Month,1)+DateSep+StNull(Day,1)+DateSep+StNull(Year,1);
    DateFormatTMY:
     NLS_DateString:=StNull(Day,1)+DateSep+StNull(Month,1)+DateSep+StNull(Year,1);
    DateFormatYMT:
     NLS_DateString:=StNull(Year,1)+DateSep+StNull(Month,1)+DateSep+StNull(Day,1);
    END;
END;

        FUNCTION NLS_CurrDateString:String;
VAR
   Year,Month,Day,DayOfWeek:DateTimeType;
BEGIN
   GetDate(Year,Month,Day,DayOfWeek);
   NLS_CurrDateString:=NLS_DateString(Year,Month,Day);
END;

        FUNCTION NLS_TimeString(Hour,Minute,Second,Sec100:Word):String;
VAR
   s:String;
   OriHour:Word;
BEGIN
   WITH NLSInfo DO
    BEGIN
     OriHour:=Hour;
     IF TimeFmt=TimeFormatUSA THEN
      BEGIN
       Hour:=Hour MOD 12; IF Hour=0 THEN Hour:=12;
      END;
     s:=StNull(Hour,1)+TimeSep+StNull(Minute,2)+TimeSep+StNull(Second,2);
     IF Sec100<100 THEN s:=s+DecSep+StNull(Sec100,2);
     IF TimeFmt=TimeFormatUSA THEN
      IF OriHour>12 THEN s:=s+'p' ELSE s:=s+'a';
    END;
   NLS_TimeString:=s;
END;

        FUNCTION NLS_CurrTimeString(Use100:Boolean):String;
VAR
   Hour,Minute,Second,Sec100:DateTimeType;
BEGIN
   GetTime(Hour,Minute,Second,Sec100);
   IF NOT Use100 THEN Sec100:=100;
   NLS_CurrTimeString:=NLS_TimeString(Hour,Minute,Second,Sec100);
END;

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

{$ENDIF}

        PROCEDURE NLS_UpString(VAR s:String);
BEGIN
   TranslateString(s,UpCaseTable);
END;

        FUNCTION Lowcase(inp:Char):Char;

{$IFDEF SPEEDUP}

        Assembler;
ASM
        lea     bx,[LowCaseTable]{ Adresse Umsetzungstabelle }
        mov     al,inp          { Zeichen holen... }
        xlat                    { fertig }
END;

{$ELSE}

BEGIN
   LowCase:=LowCaseTable[inp];
END;

{$ENDIF}

        PROCEDURE NLS_LowString(VAR s:String);
BEGIN
   TranslateString(s,LowCaseTable);
END;

        FUNCTION NLS_StrCaseCmp(s1,s2:String):Integer;
BEGIN
   TranslateString(s1,UpCaseTable);
   TranslateString(s2,UpCaseTable);
   IF s1<s2 THEN NLS_StrCaseCmp:=-1
   ELSE IF s1>s2 THEN NLS_StrCaseCmp:=1
   ELSE NLS_StrCaseCmp:=0;
END;

        FUNCTION NLS_LStrCaseCmp(s1,s2:String):Integer;
BEGIN
   TranslateString(s1,UpCaseTable);
   IF s1<s2 THEN NLS_LStrCaseCmp:=-1
   ELSE IF s1>s2 THEN NLS_LStrCaseCmp:=1
   ELSE NLS_LStrCaseCmp:=0;
END;

        FUNCTION NLS_StrCmp(s1,s2:String):ShortInt;
BEGIN
   TranslateString(s1,CollateTable);
   TranslateString(s2,CollateTable);
   IF s1>s2 THEN NLS_StrCmp:=1
   ELSE IF s1=s2 THEN NLS_StrCmp:=0
   ELSE NLS_StrCmp:=-1;
END;

END.
