                save
                listing off   ; kein Listing über diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.40 - Datei REG166.INC                                             *
;*   								            *
;*   Sinn : enthält SFR-, Makro- und Adreßdefinitionen für 80C166/167       *
;*          OK, der Name fällt aus dem Rahmen, aber STDDEF16.INC war schon  *
;*          belegt...                                                       *
;* 									    *
;*   letzte Änderungen : 23. 6.1994                                         *
;*                       27. 6.1994 Makros                                  *
;*                       28. 6.1994 Bitadressen                             *
;*			  9. 7.1994 Interrupt-Register-Korrektur            *
;*                       10. 7.1994 80C167-Register                         *
;*                       10.12.1994 Korrekturen/Erweiterungen 80C166        *
;*                                  (Hinweis von A. Kipper/A.Bergmann)      *
;*                                                                          *
;****************************************************************************

		ifndef  reg166inc       ; verhindert Mehrfacheinbindung

reg166inc       equ     1

                if      (MOMCPU<>8438118)&&(MOMCPU<>8438119)
		 fatal  "Falscher Prozessortyp eingestellt: nur 80C166 oder 80C167 erlaubt!"
		endif


                if      MOMPASS=1
		 message "80C166/167-SFR-Definitionen (C) 1994 Alfred Arnold"
		 message "binde \{MOMCPU}-SFRs ein"
		endif

; damit's etwas kürzer wird:

DefIntBits      macro   BASE,adr,{NoExpand} ; Interruptregister+Bits darin def.
tmpstr166	set	"BASE"
{tmpstr166}IC   equ     adr
{tmpstr166}_GLVL bit    {tmpstr166}IC.0 ; Gruppenpriorität
{tmpstr166}_ILVL bit    {tmpstr166}IC.2 ; Interruptpriorität
{tmpstr166}IE   bit     {tmpstr166}IC.6 ; Interruptfreigabe
{tmpstr166}IR   bit     {tmpstr166}IC.7 ; Interruptanzeige
		endm

;----------------------------------------------------------------------------
; Spezialregister:
; Bitadressen nur für bitadressierbare Register !
; bei Bitfeldern nur Startbit !

                if      MOMCPU=80C167H
SYSCON           equ     0ff12h         ; Systemkonfiguration
WRCFG            bit     SYSCON.7       ; /WRL und /WRH statt /WR und /BHE
ROMS1            bit     SYSCON.12      ; Lage internes ROM
                elseif
SYSCON           equ     0ff0ch
MCTC             bit     SYSCON.0       ; Speicherzykluslänge
RWDC             bit     SYSCON.4       ; Schreib/Lese-Verzögerung
MTTC             bit     SYSCON.5       ; Tri-State-Zeit
BTYP             bit     SYSCON.6       ; Status der EBC-Pins
RDYEN            bit     SYSCON.12      ; /READY freigeben
                endif
CLKEN           bit     SYSCON.8        ; CLKOUT freigeben
BYTDIS          bit     SYSCON.9        ; /BHE freigeben
ROMEN           bit     SYSCON.10       ; internes ROM freigeben
SGTDIS          bit     SYSCON.11       ; Adreßraum auf 64K beschränken
STKSZ           bit     SYSCON.13       ; Maximalgröße Stack

                if      MOMCPU=80C167H
RP0H             equ     0f108h         ; Systemanlaufregister (nur Lesen)
CSSEL            bit     RP0H.1         ; Anzahl CS-Leitungen
SALSEL           bit     RP0H.3         ; Anzahl Adreßleitungen ab A16
                endif

PSW             equ     0ff10h          ; Flagregister
N               bit     PSW.0           ; negatives Ergebnis ?
C               bit     PSW.1           ; Carry ?
V               bit     PSW.2           ; Overflow ?
Z               bit     PSW.3           ; Ergebnis Null ?
E               bit     PSW.4           ; Tabellenende ?
MULIP           bit     PSW.5           ; Multiplikation/Division unterbrochen ?
USR0            bit     PSW.6           ; benutzerdefiniert
IEN             bit     PSW.11          ; Interrupts freigeben
ILVL            bit     PSW.12          ; mom. bediente Interruptebene

TFR             equ     0ffach          ; Trap Flagregister
ILLBUS          bit     TFR.0           ; externer Bus benutzt, aber nicht konfiguriert
ILLINA          bit     TFR.1           ; Codezugriff auf ungerade Adresse
ILLOPA          bit     TFR.2           ; Wortzugriff auf ungerade Adresse
PRTFLT          bit     TFR.3           ; Schutzverletzung
UNDOPC          bit     TFR.7           ; undefinierter Maschinenbefehl
STKUF           bit     TFR.13          ; Stack-Unterlauf
STKOF           bit     TFR.14          ; Stack-Überlauf
NMI             bit     TFR.15          ; nicht maskierbarer Interrupt
                if      MOMCPU=80C167H
EXICON           equ     0f1c0h         ; Steuerung externer Interrupt-Controller
EXI0ES           bit     EXICON.0       ; Flankensteuerung Interrupt 0
EXI1ES           bit     EXICON.2       ; Flankensteuerung Interrupt 1
EXI2ES           bit     EXICON.4       ; Flankensteuerung Interrupt 2
EXI3ES           bit     EXICON.6       ; Flankensteuerung Interrupt 3
EXI4ES           bit     EXICON.8       ; Flankensteuerung Interrupt 4
EXI5ES           bit     EXICON.10      ; Flankensteuerung Interrupt 5
EXI6ES           bit     EXICON.12      ; Flankensteuerung Interrupt 6
EXI7ES           bit     EXICON.14      ; Flankensteuerung Interrupt 7
                endif

DPP0            equ     0fe00h          ; Pagingregister
DPP1            equ     0fe02h
DPP2            equ     0fe04h
DPP3            equ     0fe06h

CSP             equ     0fe08h          ; Bits 16.. Programmzähler

MDH             equ     0fe0ch          ; Multiplizier/Dividierregister
MDL             equ     0fe0eh
MDC             equ     0ff0eh          ; Steuerung Multiplizierer/Dividierer
MDRIU           bit     MDC.4           ; Multiplizierer in Benutzung

CP              equ     0fe10h          ; Registerkontextzeiger

SP              equ     0fe12h          ; Stackpointer
STKOV           equ     0fe14h          ; obere Grenze Stackpointer
STKUN           equ     0fe16h          ; untere Grenze Stackpointer

ZEROS           equ     0ff1ch          ; konstant Nullen
ONES            equ     0ff1eh          ; konstant Einsen

;----------------------------------------------------------------------------
; Speicher-Interface

                if      MOMCPU=80C167H
ADDRSEL1         equ     0fe18h         ; Adreßdekoder Register 1..4
ADDRSEL2         equ     0fe1ah
ADDRSEL3         equ     0fe1ch
ADDRSEL4         equ     0fe1eh

BUSCON0          equ     0ff0ch         ; Buskonfiguration Register 0..4
MCTC0            bit     BUSCON0.0      ; Speicherzykluslänge
RWDC0            bit     BUSCON0.4      ; Schreib/Leseverzögerung
MTTC0            bit     BUSCON0.5      ; Tri-State-Zeit
BTYP0            bit     BUSCON0.6      ; Status EBC-Pins
ALECTL0          bit     BUSCON0.9      ; ALE Verlängerung
BUSACT0          bit     BUSCON0.10     ; Bus aktiv-Steuerbit
RDYEN0           bit     BUSCON0.12     ; /READY benutzen
BUSCON1          equ     0ff14h
MCTC1            bit     BUSCON1.0      ; Speicherzykluslänge
RWDC1            bit     BUSCON1.4      ; Schreib/Leseverzögerung
MTTC1            bit     BUSCON1.5      ; Tri-State-Zeit
BTYP1            bit     BUSCON1.6      ; Status EBC-Pins
ALECTL1          bit     BUSCON1.9      ; ALE Verlängerung
BUSACT1          bit     BUSCON1.10     ; Bus aktiv-Steuerbit
RDYEN1           bit     BUSCON1.12     ; /READY benutzen
CSREN1           bit     BUSCON1.14     ; Arbeitsweise CS-Pins
CSWEN1           bit     BUSCON1.15
BUSCON2          equ     0ff16h
MCTC2            bit     BUSCON2.0      ; s.o.
RWDC2            bit     BUSCON2.4
MTTC2            bit     BUSCON2.5
BTYP2            bit     BUSCON2.6
ALECTL2          bit     BUSCON2.9
BUSACT2          bit     BUSCON2.10
RDYEN2           bit     BUSCON2.12
CSREN2           bit     BUSCON2.14
CSWEN2           bit     BUSCON2.15
BUSCON3          equ     0ff18h
MCTC3            bit     BUSCON3.0
RWDC3            bit     BUSCON3.4
MTTC3            bit     BUSCON3.5
BTYP3            bit     BUSCON3.6
ALECTL3          bit     BUSCON3.9
BUSACT3          bit     BUSCON3.10
RDYEN3           bit     BUSCON3.12
CSREN3           bit     BUSCON3.14
CSWEN3           bit     BUSCON3.15
BUSCON4          equ     0ff1ah
MCTC4            bit     BUSCON4.0
RWDC4            bit     BUSCON4.4
MTTC4            bit     BUSCON4.5
BTYP4            bit     BUSCON4.6
ALECTL4          bit     BUSCON4.9
BUSACT4          bit     BUSCON4.10
RDYEN4           bit     BUSCON4.12
CSREN4           bit     BUSCON4.14
CSWEN4           bit     BUSCON4.15

                 DefIntBits XP0,0f186h  ; Interruptsteuerung Peripheriegerät 0
                 DefIntBits XP1,0f18eh  ; Interruptsteuerung Peripheriegerät 1
                 DefIntBits XP2,0f196h  ; Interruptsteuerung Peripheriegerät 2
                 DefIntBits XP3,0f19eh  ; Interruptsteuerung Peripheriegerät 3
                endif

;----------------------------------------------------------------------------
; Ports:

                if      MOMCPU=80C167H
P0L              equ     0ff00h         ; Port 0 Datenregister
P0H              equ     0ff02h
DP0L             equ     0f100h         ; Port 0 Datenrichtungsregister
DP0H             equ     0f102h
                elseif
P0               equ     0ff00h
DP0              equ     0ff02h
                endif

                if      MOMCPU=80C167H
P1L              equ     0ff04h         ; Port 1 Datenregister
P1H              equ     0ff06h
DP1L             equ     0f104h         ; Port 1 Datenrichtungsregister
DP1H             equ     0f106h
                elseif
P1               equ     0ff04h 
DP1              equ     0ff06h
                endif

P2              equ     0ffc0h          ; Port 2 Datenregister
DP2             equ     0ffc2h          ; Port 2 Datenrichtungsregister
                if      MOMCPU=80C167H
ODP2             equ     0f1c2h         ; Port 2 Open-Drain-Einstellung
                endif

P3              equ     0ffc4h          ; Port 3 Datenregister
DP3             equ     0ffc6h          ; Port 3 Datenrichtungsregister
                if      MOMCPU=80C167H
ODP3             equ     0f1c6h         ; Port 3 Open-Drain-Einstellung
                endif

                if      MOMCPU=80C167H
P4               equ     0ffc8h         ; Port 4 Datenregister
DP4              equ     0ffcah         ; Port 4 Datenrichtungsregister
                elseif
P4               equ     0ff08h
DP4              equ     0ff0ah
                endif

P5              equ     0ffa2h          ; Port 5 Datenregister (nur Eingang)

                if      MOMCPU=80C167H
P6               equ     0ffcch         ; Port 6 Datenregister
DP6              equ     0ffceh         ; Port 6 Datenrichtungsregister
ODP6             equ     0f1ceh         ; Port 6 Open-Drain-Einstellung

P7               equ     0ffd0h         ; Port 7 Datenregister
DP7              equ     0ffd2h         ; Port 7 Datenrichtungsregister
ODP7             equ     0f1d2h         ; Port 7 Open-Drain-Einstellung

P8               equ     0ffd4h         ; Port 8 Datenregister
DP8              equ     0ffd6h         ; Port 8 Datenrichtungsregister
ODP8             equ     0f1d6h         ; Port 8 Open-Drain-Einstellung
                endif

;----------------------------------------------------------------------------
; Timer:

T0              equ     0fe50h          ; CAPCOM Timer 0
T0REL           equ     0fe54h          ; Timer 0 Reload Register
T01CON          equ     0ff50h          ; Steuerregister Timer 0+1
T0I             bit     T01CON.0        ; Timer 0 Eingangssignalwahl
T0M             bit     T01CON.3        ; Timer 0 Modus
T0R             bit     T01CON.6        ; Timer 0 starten/stoppen
T1I             bit     T01CON.8        ; Timer 1 Eingangssignalwahl
T1M             bit     T01CON.11       ; Timer 1 Modus
T1R             bit     T01CON.14       ; Timer 1 starten/stoppen
                DefIntBits T0,0ff9ch    ; Timer 0 Interruptsteuerung

T1              equ     0fe52h          ; CAPCOM Timer 1
T1REL           equ     0fe56h          ; Timer 1 Reload Register
                DefIntBits T1,0ff9eh    ; Timer 1 Interruptsteuerung

T2              equ     0fe40h          ; Timer 2
T2CON           equ     0ff40h          ; Steuerregister Timer 2
T2I             bit     T2CON.0         ; Eingangssignal Timer 2
T2M             bit     T2CON.3         ; Modus Timer 2
T2R             bit     T2CON.6         ; Timer 2 starten/stoppen
T2UD            bit     T2CON.7         ; Timer 2 Zählrichtung
                if      MOMCPU=80C167H
T2UDE            bit    T2CON.8         ; Timer 2 externe Zählrichtungswahl
                endif
		DefIntBits T2,0ff60h    ; Timer 2 Interruptsteuerung

T3              equ     0fe42h          ; Timer 3
T3CON           equ     0ff42h          ; Steuerregister Timer 3
T3I             bit     T3CON.0         ; Eingangssignal Timer 3
T3M             bit     T3CON.3         ; Modus Timer 3
T3R             bit     T3CON.6         ; Timer 3 starten/stoppen
T3UD            bit     T3CON.7         ; Timer 3 Zählrichtung
T3UDE           bit     T3CON.8         ; Timer 3 externe Zählrichtungswahl
T3OE            bit     T3CON.9         ; Timer 3 Ausgang freischalten
T3OTL           bit     T3CON.10        ; Timer 3 übergelaufen ?
		DefIntBits T3,0ff62h    ; Timer 3 Interruptsteuerung

T4              equ     0fe44h          ; Timer 4
T4CON           equ     0ff44h          ; Steuerregister Timer 4
T4I             bit     T4CON.0         ; Eingangssignal Timer 4
T4M             bit     T4CON.3         ; Modus Timer 4
T4R             bit     T4CON.6         ; Timer 4 starten/stoppen
T4UD            bit     T4CON.7         ; Timer 4 Zählrichtung
                if      MOMCPU=80C167H
T4UDE            bit    T4CON.8         ; Timer 4 externe Zählrichtungswahl
                endif
                DefIntBits T4,0ff64h    ; Timer 4 Interruptsteuerung

T5              equ     0fe46h          ; Timer 5
T5CON           equ     0ff46h          ; Steuerregister Timer 5
T5I             bit     T5CON.0         ; Eingangssignal Timer 5
T5M             bit     T5CON.3         ; Modus Timer 5
T5R             bit     T5CON.6         ; Timer 5 starten/stoppen
T5UD            bit     T5CON.7         ; Timer 5 Zählrichtung
                if      MOMCPU=80C167H
T5UDE            bit    T5CON.8         ; Timer 5 externe Zählrichtungswahl
                endif
CI              bit     T5CON.12        ; CAPREL Eingangswahl
T5CLR           bit     T5CON.14        ; Timer 5 bei Fang löschen ?
T5SC            bit     T5CON.15        ; Timer 5 Fangen freigeben
		DefIntBits T5,0ff66h    ; Timer 5 Interruptsteuerung

T6              equ     0fe48h          ; Timer 6
T6CON           equ     0ff48h          ; Steuerregister Timer 6
T6I             bit     T6CON.0         ; Eingangssignal Timer 6
                if      MOMCPU=80C167H
T6M              bit    T6CON.3         ; Modus Timer 6
                endif
T6R             bit     T6CON.6         ; Timer 6 starten/stoppen
T6UD            bit     T6CON.7         ; Timer 6 Zählrichtung
                if      MOMCPU=80C167H
T6UDE            bit    T6CON.8         ; Timer 6 externe Zählrichtungswahl
                endif
T6OE            bit     T6CON.9         ; Timer 6 Ausgang freischalten
T6OTL           bit     T6CON.10        ; Timer 6 übergelaufen ?
T6SR            bit     T6CON.15        ; Timer 6 Reload
		DefIntBits T6,0ff68h    ; Timer 6 Interruptsteuerung

                if      MOMCPU=80C167H
T7               equ     0f050h         ; Timer 7
T7REL            equ     0f054h         ; Timer 7 Reload Register
T78CON           equ     0ff20h         ; Steuerregister Timer 7+8
                 DefIntBits T7,0f17ah   ; Timer 0 Interruptsteuerung

T8               equ     0f052h         ; CAPCOM Timer 8
T8REL            equ     0f056h         ; Timer 8 Reload Register
                 DefIntBits T8,0f17ch   ; Timer 8 Interruptsteuerung
                endif

;-----------------------------------------------------------------------
; Capture/Compare-Einheit:

CAPREL          equ     0fe4ah		; Capture/Reload-Register
CC0             equ     0fe80h          ; CAPCOM Register
CC1             equ     0fe82h
CC2             equ     0fe84h
CC3             equ     0fe86h
CC4             equ     0fe88h
CC5             equ     0fe8ah
CC6             equ     0fe8ch
CC7             equ     0fe8eh
CC8             equ     0fe90h
CC9             equ     0fe92h
CC10            equ     0fe94h
CC11            equ     0fe96h
CC12            equ     0fe98h
CC13            equ     0fe9ah
CC14            equ     0fe9ch
CC15            equ     0fe9eh
                if      MOMCPU=80C167H
CC16             equ     0fe60h         ; zus. CAPCOM Register
CC17             equ     0fe62h
CC18             equ     0fe64h
CC19             equ     0fe66h
CC20             equ     0fe68h
CC21             equ     0fe6ah
CC22             equ     0fe6ch
CC23             equ     0fe6eh
CC24             equ     0fe70h
CC25             equ     0fe72h
CC26             equ     0fe74h
CC27             equ     0fe76h
CC28             equ     0fe78h
CC29             equ     0fe7ah
CC30             equ     0fe7ch
CC31             equ     0fe7eh
                endif

CCM0            equ     0ff52h          ; CAPCOM Modusregister
CCMOD0          bit     CCM0.0          ; CC0 Modusauswahl
ACC0            bit     CCM0.3          ; CC0 Timer 0/1 zuordnen
CCMOD1          bit     CCM0.4          ; CC1 Modusauswahl
ACC1            bit     CCM0.7          ; CC1 Timer 0/1 zuordnen
CCMOD2          bit     CCM0.8          ; CC2 Modusauswahl
ACC2            bit     CCM0.11         ; CC2 Timer 0/1 zuordnen
CCMOD3          bit     CCM0.12         ; CC3 Modusauswahl
ACC3            bit     CCM0.15         ; CC3 Timer 0/1 zuordnen

CCM1            equ     0ff54h
CCMOD4          bit     CCM1.0          ; CC4 Modusauswahl
ACC4            bit     CCM1.3          ; CC4 Timer 0/1 zuordnen
CCMOD5          bit     CCM1.4          ; CC5 Modusauswahl
ACC5            bit     CCM1.7          ; CC5 Timer 0/1 zuordnen
CCMOD6          bit     CCM1.8          ; CC6 Modusauswahl
ACC6            bit     CCM1.11         ; CC6 Timer 0/1 zuordnen
CCMOD7          bit     CCM1.12         ; CC7 Modusauswahl
ACC7            bit     CCM1.15         ; CC7 Timer 0/1 zuordnen

CCM2            equ     0ff56h
CCMOD8          bit     CCM2.0          ; CC8 Modusauswahl
ACC8            bit     CCM2.3          ; CC8 Timer 0/1 zuordnen
CCMOD9          bit     CCM2.4          ; CC9 Modusauswahl
ACC9            bit     CCM2.7          ; CC9 Timer 0/1 zuordnen
CCMOD10         bit     CCM2.8          ; CC10 Modusauswahl
ACC10           bit     CCM2.11         ; CC10 Timer 0/1 zuordnen
CCMOD11         bit     CCM2.12         ; CC11 Modusauswahl
ACC11           bit     CCM2.15         ; CC11 Timer 0/1 zuordnen

CCM3            equ     0ff58h
CCMOD12         bit     CCM3.0          ; CC12 Modusauswahl
ACC12           bit     CCM3.3          ; CC12 Timer 0/1 zuordnen
CCMOD13         bit     CCM3.4          ; CC13 Modusauswahl
ACC13           bit     CCM3.7          ; CC13 Timer 0/1 zuordnen
CCMOD14         bit     CCM3.8          ; CC14 Modusauswahl
ACC14           bit     CCM3.11         ; CC14 Timer 0/1 zuordnen
CCMOD15         bit     CCM3.12         ; CC15 Modusauswahl
ACC15           bit     CCM3.15         ; CC15 Timer 0/1 zuordnen

CCM4            equ     0ff22h
CCMOD16         bit     CCM4.0          ; CC16 Modusauswahl
ACC16           bit     CCM4.3          ; CC16 Timer 0/1 zuordnen
CCMOD17         bit     CCM4.4          ; CC17 Modusauswahl
ACC17           bit     CCM4.7          ; CC17 Timer 0/1 zuordnen
CCMOD18         bit     CCM4.8          ; CC18 Modusauswahl
ACC18           bit     CCM4.11         ; CC18 Timer 0/1 zuordnen
CCMOD19         bit     CCM4.12         ; CC19 Modusauswahl
ACC19           bit     CCM4.15         ; CC19 Timer 0/1 zuordnen

CCM5            equ     0ff24h
CCMOD20         bit     CCM5.0          ; CC20 Modusauswahl
ACC20           bit     CCM5.3          ; CC20 Timer 0/1 zuordnen
CCMOD21         bit     CCM5.4          ; CC21 Modusauswahl
ACC21           bit     CCM5.7          ; CC21 Timer 0/1 zuordnen
CCMOD22         bit     CCM5.8          ; CC22 Modusauswahl
ACC22           bit     CCM5.11         ; CC22 Timer 0/1 zuordnen
CCMOD23         bit     CCM5.12         ; CC23 Modusauswahl
ACC23           bit     CCM5.15         ; CC23 Timer 0/1 zuordnen

CCM6            equ     0ff26h
CCMOD24         bit     CCM6.0          ; CC24 Modusauswahl
ACC24           bit     CCM6.3          ; CC24 Timer 0/1 zuordnen
CCMOD25         bit     CCM6.4          ; CC25 Modusauswahl
ACC25           bit     CCM6.7          ; CC25 Timer 0/1 zuordnen
CCMOD26         bit     CCM6.8          ; CC26 Modusauswahl
ACC26           bit     CCM6.11         ; CC26 Timer 0/1 zuordnen
CCMOD27         bit     CCM6.12         ; CC27 Modusauswahl
ACC27           bit     CCM6.15         ; CC27 Timer 0/1 zuordnen

CCM7            equ     0ff28h
CCMOD28         bit     CCM7.0          ; CC28 Modusauswahl
ACC28           bit     CCM7.3          ; CC28 Timer 0/1 zuordnen
CCMOD29         bit     CCM7.4          ; CC29 Modusauswahl
ACC29           bit     CCM7.7          ; CC29 Timer 0/1 zuordnen
CCMOD30         bit     CCM7.8          ; CC30 Modusauswahl
ACC30           bit     CCM7.11         ; CC30 Timer 0/1 zuordnen
CCMOD31         bit     CCM7.12         ; CC31 Modusauswahl
ACC31           bit     CCM7.15         ; CC31 Timer 0/1 zuordnen

		DefIntBits CR,0ff6ah    ; CAPREL Interruptsteuerregister

		DefIntBits CC0,0ff78h   ; Interruptsteuerung CAPCOM-Kanäle
		DefIntBits CC1,0ff7ah
                DefIntBits CC2,0ff7ch
                DefIntBits CC3,0ff7eh
                DefIntBits CC4,0ff80h
                DefIntBits CC5,0ff82h
                DefIntBits CC6,0ff84h
                DefIntBits CC7,0ff86h
                DefIntBits CC8,0ff88h
                DefIntBits CC9,0ff8ah
		DefIntBits CC10,0ff8ch
		DefIntBits CC11,0ff8eh
		DefIntBits CC12,0ff90h
		DefIntBits CC13,0ff92h
		DefIntBits CC14,0ff94h
		DefIntBits CC15,0ff96h
                if      MOMCPU=80C167H
                 DefIntBits CC16,0f160h ; Interruptsteuerung zus. CAPCOM-Kanäle
                 DefIntBits CC17,0f162h
                 DefIntBits CC18,0f164h
                 DefIntBits CC19,0f166h
                 DefIntBits CC20,0f168h
                 DefIntBits CC21,0f16ah
                 DefIntBits CC22,0f16ch
                 DefIntBits CC23,0f16eh
                 DefIntBits CC24,0f170h
                 DefIntBits CC25,0f172h
                 DefIntBits CC26,0f174h
                 DefIntBits CC27,0f176h
                 DefIntBits CC28,0f178h
                 DefIntBits CC29,0f184h
                 DefIntBits CC30,0f18ch
                 DefIntBits CC31,0f194h
                endif

PECC0           equ     0fec0h          ; PEC Kanalsteuerregister
PECC1           equ     0fec2h
PECC2           equ     0fec4h
PECC3           equ     0fec6h
PECC4           equ     0fec8h
PECC5           equ     0fecah
PECC6           equ     0fecch
PECC7           equ     0feceh

SRCP0           equ    0fde0h           ; hoffentlich gilt das auch so für
SRCP1           equ    0fde4h           ; den 167er, ich habe momentan näm-
SRCP2           equ    0fde8h           ; lich nicht das Datenbuch zur Hand...
SRCP3           equ    0fdech
SRCP4           equ    0fdf0h
SRCP5           equ    0fdf4h
SRCP6           equ    0fdf8h
SRCP7           equ    0fdfch
DSTP0           equ    0fde2h
DSTP1           equ    0fde6h
DSTP2           equ    0fdeah
DSTP3           equ    0fdeeh
DSTP4           equ    0fdf2h
DSTP5           equ    0fdf6h
DSTP6           equ    0fdfah
DSTP7           equ    0fdfeh

;----------------------------------------------------------------------------
; Pulsweitenmodulator:

                if      MOMCPU=80C167H
PP0              equ     0f038h         ; PWM Periode 0
PT0              equ     0f030h         ; PWM Zähler 0
PW0              equ     0fe30h         ; PWM Pulsweite 0
PP1              equ     0f03ah         ; PWM Periode 1
PT1              equ     0f032h         ; PWM Zähler 1
PW1              equ     0fe30h         ; PWM Pulsweite 1
PP2              equ     0f03ch         ; PWM Periode 2
PT2              equ     0f034h         ; PWM Zähler 2
PW2              equ     0fe30h         ; PWM Pulsweite 2
PP3              equ     0f03eh         ; PWM Periode 3
PT3              equ     0f036h         ; PWM Zähler 3
PW3              equ     0fe30h         ; PWM Pulsweite 3
PWMCON0          equ     0ff30h         ; PWM Seuerregister 0
PTR0             bit     PWMCON0.0      ; PT0 laufen lassen
PTR1             bit     PWMCON0.1      ; PT1 laufen lassen
PTR2             bit     PWMCON0.2      ; PT2 laufen lassen
PTR3             bit     PWMCON0.3      ; PT3 laufen lassen
PTI0             bit     PWMCON0.4      ; PT0 Eingangswahl
PTI1             bit     PWMCON0.5      ; PT1 Eingangswahl
PTI2             bit     PWMCON0.6      ; PT2 Eingangswahl
PTI3             bit     PWMCON0.7      ; PT3 Eingangswahl
PIE0             bit     PWMCON0.8      ; PT0 Interupts freigeben
PIE1             bit     PWMCON0.9      ; PT1 Interupts freigeben
PIE2             bit     PWMCON0.10     ; PT2 Interupts freigeben
PIE3             bit     PWMCON0.11     ; PT3 Interupts freigeben
PIR0             bit     PWMCON0.12     ; PT0 Interupt aufgetreten
PIR1             bit     PWMCON0.13     ; PT1 Interupt aufgetreten
PIR2             bit     PWMCON0.14     ; PT2 Interupt aufgetreten
PIR3             bit     PWMCON0.15     ; PT3 Interupt aufgetreten
PWMCON1          equ     0ff32h         ; PWM Seuerregister 1
PEN0             bit     PWMCON1.0      ; PT0 Ausgang freigeben
PEN1             bit     PWMCON1.1      ; PT1 Ausgang freigeben
PEN2             bit     PWMCON1.2      ; PT2 Ausgang freigeben
PEN3             bit     PWMCON1.3      ; PT3 Ausgang freigeben
PM0              bit     PWMCON1.4      ; PT0 Modus
PM1              bit     PWMCON1.5      ; PT1 Modus
PM2              bit     PWMCON1.6      ; PT2 Modus
PM3              bit     PWMCON1.7      ; PT3 Modus
PB01             bit     PWMCON1.12     ; PT0&1 Burst-Modus
PS2              bit     PWMCON1.14     ; PT2 Einzelimpulsmodus
PS3              bit     PWMCON1.15     ; PT3 Einzelimpulsmodus
                 DefIntBits PWM,0f17eh  ; PWM Interruptsteuerung
                endif

;----------------------------------------------------------------------------
; A/D-Wandler:

ADDAT           equ     0fea0h          ; A/D-Wandler Ergebnis
                if      MOMCPU=80C167H
ADDAT2           equ    0f0a0h          ; A/D-Wandler Ergebnis 2
                endif
		DefIntBits ADC,0ff98h   ; A/D-Wandler Interruptsteuerung für Wandlungsende
		DefIntBits ADE,0ff9ah   ; A/D-Wandler Interruptsteuerung für Fehler
ADCON           equ     0ffa0h          ; A/D-Wandler Steuerregister
ADCH            bit     ADCON.0         ; A/D-Wandler Kanalwahl
ADM             bit     ADCON.4         ; A/D-Wandler Modus
ADST            bit     ADCON.7         ; A/D-Wandlung starten
ADBSY           bit     ADCON.8         ; A/D-Wandler beschäftigt
                if      MOMCPU=80C167H
ADWR             bit     ADCON.9        ; A/D-Wandler erst wieder starten, wenn gelesen
ADCIN            bit     ADCON.10       ; A/D-Wandler Kanalinjektion (??...)
ADCRQ            bit     ADCON.11       ; A/D-Wandler Kanalinjektion angefordert
                endif

;----------------------------------------------------------------------------
; Watchdog:

WDT             equ     0feaeh          ; Watchdog-Zähler
WDTCON          equ     0ffaeh          ; Watchdog Steuerregister
WDTIN           bit     WDTCON.0        ; Watchdog Eingangsfrequenz
WDTR            bit     WDTCON.1        ; Reset durch Watchdog ?
WDTREL          bit     WDTCON.8        ; Reload-Wert Watchdog

;----------------------------------------------------------------------------
; Serielles:

S0TBUF          equ     0feb0h          ; SIO0 Senderegister
S0RBUF          equ     0feb2h          ; SIO0 Empfangsregister
S0BG            equ     0feb4h          ; SIO0 Baudrateneinstellung
		DefIntBits S0T,0ff6ch   ; Interruptsteuerung Sendeteil SIO0
		DefIntBits S0R,0ff6eh   ; Interruptsteuerung Empfangsteil SIO0
		DefIntBits S0E,0ff70h   ; Interruptsteuerung Fehlererkennung SIO0
S0CON           equ     0ffb0h          ; Steuerregister SIO0
S0M             bit     S0CON.0         ; SIO0 Modus
S0STP           bit     S0CON.3         ; SIO0 Anzahl Stopbits
S0REN           bit     S0CON.4         ; SIO0 Empfänger freigeben
S0PEN           bit     S0CON.5         ; SIO0 mit Parität
S0FEN           bit     S0CON.6         ; SIO0 mit Rahmenprüfung
S0OEN           bit     S0CON.7         ; SIO0 mit Überlaufprüfung
S0PE            bit     S0CON.8         ; SIO0 Paritätsfehler
S0FE            bit     S0CON.9         ; SIO0 Rahmenfehler
S0OE            bit     S0CON.10        ; SIO0 Überlauffehler
                if      MOMCPU=80C167H
S0ODD            bit     S0CON.12       ; SIO0 Odd Parity
S0BRS            bit     S0CON.13       ; SIO0 Baudratenfaktor
                endif
S0LB            bit     S0CON.14        ; SIO0 Rückkopplung
S0R             bit     S0CON.15        ; SIO0 Baudratengenerator freigeben

                if      MOMCPU=80C167H
SSCTB            equ     0f0b0h         ; SSC Senderegister
SSCRB            equ     0f0b2h         ; SSC Empfangsregister
SSCBR            equ     0f0b4h         ; SSC Baudrateneinstellung
                 DefIntBits SSCT,0ff72h ; Interruptsteuerung Sendeteil SSC
                 DefIntBits SSCR,0ff74h ; Interruptsteuerung Empfangsteil SSC
                 DefIntBits SSCE,0ff76h ; Interruptsteuerung Fehlererkennung SSC
SSCCON           equ     0ffb2h         ; Steuerregister SSC
SSCBM            bit     SSCCON.0       ; Wortbreite
SSCBC            bit     SSCCON.0       ; Bitzahl
SSCHB            bit     SSCCON.4       ; Startbitsteuerung
SSCPH            bit     SSCCON.5       ; Taktphase
SSCP0            bit     SSCCON.6       ; Taktpolarität
SSCTEN           bit     SSCCON.8       ; Sendefehler freigeben
SSCTE            bit     SSCCON.8       ; Sendefehler aufgetreten ?
SSCREN           bit     SSCCON.9       ; Empfangsfehler freigeben
SSCRE            bit     SSCCON.9       ; Empfangsfehler aufgetreten ?
SSCPEN           bit     SSCCON.10      ; Phasenfehler freigeben
SSCPE            bit     SSCCON.10      ; Phasenfehler aufgetreten ?
SSCBEN           bit     SSCCON.11      ; Baudratenfehler freigeben
SSCBE            bit     SSCCON.11      ; Baudratenfehler freigegeben ?
SSCBSY           bit     SSCCON.12      ; SSC beschäftigt ?
SSCMS            bit     SSCCON.14      ; SSC als Master betreiben
SSCEN            bit     SSCCON.15      ; SSC freigeben
                elseif
S1TBUF           equ     0feb8h         ; SIO1 Senderegister
S1RBUF           equ     0febah         ; SIO1 Empfangsregister
S1BG             equ     0febch         ; SIO1 Baudrateneinstellung
                 DefIntBits S1T,0ff72h  ; Interruptsteuerung Sendeteil SIO1
                 DefIntBits S1R,0ff74h  ; Interruptsteuerung Empfangsteil SIO1
                 DefIntBits S1E,0ff76h  ; Interruptsteuerung Fehlererkennung SIO1
S1CON            equ     0ffb8h         ; Steuerregister SIO1
S1M              bit     S1CON.0        ; SIO1 Modus
S1STP            bit     S1CON.3        ; SIO1 Anzahl Stopbits
S1REN            bit     S1CON.4        ; SIO1 Empfänger freigeben
S1PEN            bit     S1CON.5        ; SIO1 mit Parität
S1FEN            bit     S1CON.6        ; SIO1 mit Rahmenprüfung
S1OEN            bit     S1CON.7        ; SIO1 mit Überlaufprüfung
S1PE             bit     S1CON.8        ; SIO1 Paritätsfehler
S1FE             bit     S1CON.9        ; SIO1 Rahmenfehler
S1OE             bit     S1CON.10       ; SIO1 Überlauffehler
S1LB             bit     S1CON.14       ; SIO1 Rückkopplung
S1R              bit     S1CON.15       ; SIO1 Baudratengenerator freigeben
                endif

;----------------------------------------------------------------------------
; Vektoren/Sonderadressen

RESET           equ     000h            ; Reseteinsprung
NMITRAP         equ     008h            ; NMI-Einsprung
STOTRAP         equ     010h            ; Einsprung Stacküberlauf
STUTRAP         equ     018h            ; Einsprung Stackunterlauf
BTRAP           equ     028h            ; undef. Opcode, Protection Fault,
                                        ; unerlaubter Wortzugriff, ungültige Instruktionsadresse,
                                        ; ungültiger Buszugriff
CC0INT          equ     040h            ; Interrupteinsprünge CAPCOM
CC1INT          equ     044h
CC2INT          equ     048h
CC3INT          equ     04ch
CC4INT          equ     050h
CC5INT          equ     054h
CC6INT          equ     058h
CC7INT          equ     05ch
CC8INT          equ     060h
CC9INT          equ     064h
CC10INT         equ     068h
CC11INT         equ     06ch
CC12INT         equ     070h
CC13INT         equ     074h
CC14INT         equ     078h
CC15INT         equ     07ch
T0INT           equ     080h            ; Interrupteinsprung Timer 0
T1INT           equ     084h            ; Interrupteinsprung Timer 1
T2INT           equ     088h            ; Interrupteinsprung Timer 2
T3INT           equ     08ch            ; Interrupteinsprung Timer 3
T4INT           equ     090h            ; Interrupteinsprung Timer 4
T5INT           equ     094h            ; Interrupteinsprung Timer 5
T6INT           equ     098h            ; Interrupteinsprung Timer 6
CRINT           equ     09ch            ; Interrupteinsprung CAPREL
ADCINT          equ     0a0h            ; Interrupteinsprung A/D-Wandlung fertig
ADEINT          equ     0a4h            ; Interrupteinsprung A/D-Wandler-Überlauf
S0TINT          equ     0a8h            ; Interrupteinsprung SIO0 wieder sendebereit
S0RINT          equ     0ach            ; Interrupteinsprung SIO0 hat Zeichen empfangen
S0EINT          equ     0b0h            ; Interrupteinsprung SIO0 hat Fehler erkannt
S1TINT          equ     0b4h            ; Interrupteinsprung SIO1 wieder sendebereit
S1RINT          equ     0b8h            ; Interrupteinsprung SIO1 hat Zeichen empfangen
S1EINT          equ     0bch            ; Interrupteinsprung SIO1 hat Fehler erkannt


;----------------------------------------------------------------------------
; Speicherbereiche

                if      MOMCPU=80C167H
IRAM             equ     0f600h         ; Beginn internes RAM
                elseif
IRAM             equ     0fa00h
                endif
IRAMEND         equ     0fdffh          ; Ende      "      "

BITRAM          equ     0fd00h          ; Beginn bitadressierbarer RAM-Bereich
BITRAMEND       equ     0fdffh          ; Ende           "          "     "

                if      MOMCPU=80C167H
PECPTR           equ     0fce0h         ; Begin PEC-Vektoren (optional)
PECPTREND        equ     0fcffh         ; Ende   "     "
                elseif
PECPTR           equ     0fde0h
PECPTREND        equ     0fdffh
                endif

SFRSTART        equ     0fe00h          ; Beginn SFR-Bereich
SFREND          equ     0ffdfh          ; Ende SFR-Bereich
                if      MOMCPU=80C167H
ESFRSTART        equ     0f000h         ; Beginn erweiterter SFR-Bereich
ESFREND          equ     0f1dfh         ; Ende erweiterter SFR-Bereich
                endif

IROM            equ     0               ; Begin internes ROM
                if      MOMCPU=80C167H
IROMEND          equ     07fffh         ; Ende internes ROM (n. voll ausgebaut)
                elseif
IROMEND          equ     01fffh
                endif

;----------------------------------------------------------------------------
; Bequemlichkeitsmakros

clr             macro   op              ; Operand auf 0 setzen
                and     op,#0
                endm

bchg            macro   op              ; Bit invertieren
                bmovn   op,op
                endm

dec             macro   op              ; um eins runter
                sub     op,#1
                endm

inc             macro   op              ; um eins rauf
                add     op,#1
                endm

swapb           macro   op              ; Bytes tauschen
                ror     op,#8
                endm

;----------------------------------------------------------------------------

		endif

		restore                 ; wieder erlauben
