; CPU_TIME.INC
;*************************************************************************
;*                                                                       *
;* Author...: Oli(ver Sellke)                 D-65199 Wiesbaden          *
;* auf AS umgestellt von Alfred Arnold, Oktober 1993                     *
;* die Low-Level-Ausgaberoutinen sind nach CONOUT.INC verlagert          *
;* Date.....: 14 Okt 1993                     Version..: 1.0             *
;* Target...: TLCS900-Family (TOSHIBA)        Compiler.: AS      V1.39p1 *
;* Project..: General Purpose / e.g. TMP96C141F / Watt Ihr Volt          *
;*                                                                       *
;* Function.: Mit dieser Routine kann man die AusfÅhrungszeit die        *
;*            ein Programm(teil) benîtigt ermitteln.                     *
;*            Die Zeit wird mit Timer4 und CAP1 davon gemessen, d.h.     *
;*            dieser darf innerhalb des gemessenen Programm(teil)s       *
;*            nicht (!) benutzt werden.                                  *
;*            !!! Alle Zeiten beziehen sich auf einen 14,7456MHz Quarz!! *
;*            Zur Ausgabe des Messwertes werden Monitor-Routinen benutzt,*
;*            deshalb kann es auch nur unter Anwendung von diesem zur    *
;*            AusfÅhrung gebracht werden.                                *
;*            Wenn ein Programm(teil) getestet wird, dessen AusfÅhrungs- *
;*            zeit unbekannt ist, sollte man die 8,681Ês Auflîsung wÑhlen*
;*            um einen Overrun des Counters zu verhindern. Wenn der Wert *
;*            entsprechend klein ist ( <0FFF ), kann man die 0,543Ês Auf-*
;*            lîsung wÑhlen um genauere Werte zu bekommen.               *
;*            Au·erdem ist die Ermittlung der 16 grî·ten und 16 kleinsten*
;*            Werte die bei mehreren DurchlÑufen erzielt wurden mîglich! *
;*            Man kann so einige 1000 DurchlÑufe fahren mit immer unter- *
;*            schiedlichen Randbedingungen auf die der zu testende Pro-  *
;*            teil entsprechend reagiert und auch Zeit benîtigt.         *
;*            So hat man sicher die minimale und maximale Laufzeit.      *
;*                                                                       *
;*            Bei allgemeiner Verwendung:                                *
;*            Max.Me·wert=0FFFFH =  35.585Ês bei 0,543Ês Auflîsung       *
;*            Max.Me·wert=0FFFFH = 568.909Ês bei 8,681Ês Auflîsung       *
;*                                                                       *
;* Hardware.: getested auf Micro-ICE TLCS900 mit 14,7456MHz Quarz !!!!   *
;*                                                                       *
;*  Routine      Funktion           Ausgabe             Stack            *
;*                                                                       *
;*  CPU_TI_INI   Initialisierung    ------              6 Byte           *
;*  CPU_TIME     Uhr starten        ------              0 Byte           *
;*  CPU_STOP     Uhr stoppen        Zeit+Statistik      8 Byte           *
;*                                                                       *
;*  - Der Prozessor mu· sich im Maximum-Modus befinden                   *
;*  - Symbol BigTime definieren fÅr grî·ere Auflîsung                    *
;*  - Da die Routinen lokale Labels verwenden, ist mindestens AS 1.39    *
;*    erforderlich                                                       *
;*  - MACROS.INC mu· vorher, CONOUT.INC irgendwo eingebunden werden      *
;*************************************************************************

;-------------------------------------------------------------------------
; gemeinsamer Anfang, Definitionen

                section Timer

TRUN            EQU   020H
CAP1L           EQU   034H
CAP1H           EQU   035H
T4MOD           EQU   038H

CR              EQU    0DH
LF              EQU    0AH

                ifdef  BigTime
Overhead         equ    1               ; Eigenverbrauch Start/Stop
                elseif
Overhead         equ    9
                endif

CPU_TI_FAA      db    40h dup (?)       ; FeldAnfangsAdresse, ab hier wer-
                                        ; den 40H (64) RAM-Zellen benîtigt
                                        ; gleich 2 * 16 Worte
CPU_TI_CR       dd    ?                 ; CPU_TIME-Control-Register,
                                        ; wird noch nicht benîtigt!!!

;*************************************************************************
;Dieser Aufruf gehîrt in den Initialisierungsbereich des zu testenden
;Programmabschnittes!!

                proc    CPU_TI_INI

;Hier werden die Feldwerte initialisiert..

                PUSH    XDE             ;Wird gleich benutzt
                PUSH    BC              ;Wird gleich benutzt
                LD      B,10H           ;16 Wîrter fÅr Max.Werte werden gebraucht
                LD      XDE,CPU_TI_FAA  ;FeldAnfangsAdresse laden
CPU_TI_INI1:    LDW     (XDE),00000H    ;Feldzelle fÅr Max.Werte initalisieren
                INC     2,XDE           ;nÑchste Max.Wert-Feldzelle
                                        ;adressieren
                DJNZ    CPU_TI_INI1     ;Alle Max.Werte initialisiert?
                LD      B,10H           ;16 Wîrter fÅr Min.Werte werden gebraucht
CPU_TI_INI2:    LDW     (XDE),0FFFFH    ;Feldzelle fÅr Min.Werte initalisieren
                INC     2,XDE           ;nÑchste Max.Wert-Feldzelle
                                        ;adressieren
                DJNZ    CPU_TI_INI2     ;Alle Min.Werte initialisiert?
                POP     BC              ;Und wieder restaurieren
                POP     XDE             ;hier auch...
                RET                     ;ZurÅck zum Aufruf!
                endp

;*************************************************************************
; Uhr starten

                proc    CPU_TIME

                                          ;Timer4 CPU-Time-Messung vorbereiten
                RES     4,(TRUN)          ;Timer4 stop and clear !
                ifdef   BigTime
                 LD      (T4MOD),00100011B ;Bit 0+1:Source-Clock: 8,681Ês
                elseif
                 LD      (T4MOD),00100001B ;Bit 0+1:Source-Clock: 0,543Ês
                endif
                                          ;Bit 2  :clear from TREG5 disabled
                                          ;Bit 3+4:INT-Capture disabled
                                          ;Bit 5  :No Sw. capture now
                SET     4,(TRUN)          ;Timer4 start and count !
                RET
                endp

;*************************************************************************

                proc    CPU_STOP

                RES     5,(T4MOD)         ;Capture1 grabs count
                CALL    CPU_TI_SOUT       ;Einzelausgabe des gemessenen Wertes
                CALL    CPU_TI_SOR        ;gemessenen Wert ins Feld sortieren
                RET                       ;ZurÅck zum Aufruf!
                endp

;*************************************************************************
;Hier wird der gerade gemessene Wert ausgegeben. Diese Ausgabe ist 
;ausreichend um Laufzeitwerte von statischen Programmabschnitten
;zu ermitteln (keine Verzweigungen im Programmabschnitt).

CPU_TI_SOUT:    PUSH    A                 ; needed little later
                PUSH    F                 ; needed little later
                push    bc                ; needed little later
                ld      wa,(cap1l)        ; gemesser Wert
                call    WriteTime
                pop     bc                ; back to the roots ...
                POP     F 
                POP     A
                RET                       ; ZurÅck zum Aufruf!

;*************************************************************************
;Hier werden die ermittelten Werte sortiert abgelegt!
;Jeweils am Feldanfang steht der grî·te und der kleinste Wert.
;Falls ein Wert einem anderen im Feld entspricht (gleicher Messwert)
;wird dieser nicht nochmal eingetragen!!!! 
;!!Achtung diese Routine benîtigt max. 145Ês (14,7456MHz Quarz) 
;  im worst case!! Aber nur wenn Daten und Code 16Bit breit sind 
;  und keine Waitstates zugeschaltet sind (Micro-ICE TLCS900 default RAM)!

CPU_TI_SOR:     PUSH    HL              ;Wird gleich benutzt
                PUSH    BC              ;Wird gleich benutzt
                PUSH    XDE             ;Wird gleich benutzt
                                        ;Max.Werte sortiert ablegen!!!
                LD      B,10H           ;16 Wîrter enthÑlt Max.Wert-Feld
                LD      HL,(CAP1L)      ;gemessenen Wert aus Capture-Reg. holen
                LD      XDE,CPU_TI_FAA  ;erste Max.Wert-Feldzelle adressieren
CPU_TI_SOR1:    CP      HL,(XDE)        ;Wert mit Feldinhalt vergleichen
                JR      ULT,CPU_TI_SOR2 ;Ist Wert kleiner als Feldinhalt?
                JR      Z,CPU_TI_SOR3   ;Ist Wert gleich Feldinhalt? Abbrechen!
                EX      (XDE),HL        ;Nein-->Wert mit Feldinhalt tauschen!
CPU_TI_SOR2:    INC     2,XDE           ;nÑchste Feldzelle adressieren
                DJNZ    B,CPU_TI_SOR1   ;Alle 16 Max.Werte kontrolliert?
                                        ;Min.Werte sortiert ablegen!!!
CPU_TI_SOR3:    LD      B,10H           ;16 Wîrter enthÑlt Min.Wert-Feld
                LD      HL,(CAP1L)      ;gemessenen Wert aus Capture-Reg. holen
                LD      XDE,CPU_TI_FAA+20H ;erste Min.Wert-Feldzelle adressieren
CPU_TI_SOR4:    CP      HL,(XDE)        ;Wert mit Feldinhalt vergleichen
                JR      UGT,CPU_TI_SOR5 ;Ist Wert grî·er als Feldinhalt?
                JR      Z,CPU_TI_SOR6   ;Ist Wert gleich Feldinhalt? Abbrechen!
                EX      (XDE),HL        ;Nein-->Wert mit Feldinhalt tauschen!
CPU_TI_SOR5:    INC     2,XDE           ;nÑchste Feldzelle adressieren
                DJNZ    B,CPU_TI_SOR4   ;Alle 16 Min.Werte kontrolliert?
CPU_TI_SOR6:    POP     XDE             ;Und wieder restaurieren
                POP     BC              ;wieder restaurieren
                POP     HL              ;hier auch...
                RET                     ;ZurÅck zum Aufruf!

;*************************************************************************
;Hier werden die im Feld abgelegten Werte ausgeben. 

CPU_TI_MOUT:                            ;Mu· noch geschrieben werden!
                RET                     ;ZurÅck zum Aufruf!

;*************************************************************************
; eine Zeitdifferenz in WA umrechnen und ausgeben
; wegen der Auflîsung der Timer ist die letzte Stelle hinter dem Komma
; bei hoher Auflîsung mit Vorsicht zu genie·en

WriteTime:      push    xwa             ; Register retten
                push    bc
                sub     wa,Overhead     ; Zeit korrigieren
                ifdef   BigTime         ; Fall 1: niedrige Auflîsung
                 mul    xwa,8681        ; -->Nanos in XWA
                 add    xwa,5000        ; !!Rundung!!
                 div    xwa,10000       ; Nanos , einzelne Mikros wegschmei·en
                 extz   xwa
                 div    xwa,100         ; Millisekunden in WA
                 ld     bc,2003h        ; ausgeben
                 call   WriteDec
                 ld     a,'.'
                 call   CONOUT
                 ld     wa,qwa          ; auf 10 us genau ausgeben
                 ld     bc,3002h
                 call   WriteDec
                 call   PSTR
                 db     " Milli",StrTerm
                elseif                  ; Fall 2: hohe Auflîsung
                 mul    xwa,543         ; -->Nanosekunden in XWA
                 div    xwa,1000        ; -->Nanos in QWA, Mikros in WA
                 ld     bc,2005h        ; Mikros 5-stellig mit Blanks
                 call   WriteDec
                 ld     a,'.'
                 call   CONOUT
                 ld     wa,qwa          ; Nanos einstellig
                 add    wa,50           ; Rundung
                 extz   xwa
                 div    xwa,100         ; Ergebnis 0..9
                 cp     wa,10
                 jr     ne,NoErr
                 ld     wa,9
NoErr:           ld     bc,3001h        ; einstellig ausgeben
                 call   WriteDec
                 call   PSTR
                 db     " Mikro",StrTerm
                endif
                call    PSTR
                db      "sekunden",StrTerm ; gemeinsamer Rest
                pop     bc              ; Register zurÅck
                pop     xwa
                ret

;*************************************************************************
; gemeinsames Ende

                endsection
