                save
		listing off   ; kein Listing über diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.41 - Datei REG251.INC                                             *
;*   								            *
;*   Sinn : enthält SFR- und Bitdefinitionen für MCS-251-Prozessoren        *
;* 									    *
;*   letzte Änderungen : 30.12.1995                                         *
;*                                                                          *
;****************************************************************************

                ifndef  reg251inc       ; verhindert Mehrfacheinbindung

reg251inc       equ     1

                if      MOMCPUNAME<>"80C251"
                 fatal  "Falscher Prozessortyp eingestellt: nur 80C251 erlaubt!"
		endif


                if      MOMPASS=1
                 message "MCS-251-SFR-Definitionen (C) 1995 Alfred Arnold"
		 message "binde \{MOMCPU}-SFRs ein"
		endif

;----------------------------------------------------------------------------
; CPU-Kern:
; ACC = A = R11
; B = R10
; SP/SPH = SPX = DR60
; DPL/DPH/DPXL = DPX = DR56
; CY wird von AS selbst benutzt

ACC             port    0e0h            ; Akkumulator

B               port    0f0h            ; B-Register

PSW             port    0d0h            ; 8051-kompatibles PSW
CARRY           bit     PSW.7           ; Carry
AC              bit     PSW.6           ; Auxiliary-Carry
F0              bit     PSW.5           ; Flag 0
RS1             bit     PSW.4           ; Register-Bank-Auswahl
RS0             bit     PSW.3
OV              bit     PSW.2           ; Overflow-Flag
UD              bit     PSW.1           ; User-Flag
P               bit     PSW.0           ; Parity-Flag

PSW1            port    0d1h            ; erweitertes PSW (Bits 2-4,6-7 wie in PSW)
N               bit     PSW1.5          ; Negatives Ergebnis?
Z               bit     PSW1.1          ; Ergebnis 0 ?

SP              port    81h             ; Stackpointer (Bit 0..7)
SPH             port    0bdh            ; Stackpointer (Bit 8..15)
DPL             port    082h            ; Datenzeiger (Bit 0..7)
DPH             port    083h            ; Datenzeiger (Bit 8..15)
DPXL            port    084h            ; Datenzeiger (Bit 15..23)

PCON            port    087h            ; Power-Down-Modi u.a.
SMOD1           bit     PCON.7          ; Verdopplun Baudrate in Modus 1..3
SMOD0           bit     PCON.6          ; Umschaltung SCON.7 als FE/SM0
POF             bit     PCON.4          ; war Spannung weg ?
GF1             bit     PCON.3          ; General Flag 1
GF0             bit     PCON.2          ; General Flag 0
PD              bit     PCON.1          ; in Powerdown-Modus schalten
IDL             bit     PCON.0          ; in Idle-Modus schalten

IE              port    0a8h            ; Interrupt-Freigaben (eigentlich IE0,
                                        ; aber das ist schon belegt
EA              bit     IE.7            ; generelle Interruptsperre
EC              bit     IE.6            ; PCA-Interrupts freigeben
ET2             bit     IE.5            ; Interrupts Timer 2 freigeben
ES              bit     IE.4            ; Interrupts ser. Schnittstelle freigeben
ET1             bit     IE.3            ; Interrupt Timer 1 freigeben
EX1             bit     IE.2            ; externen Interrupt 1 freigeben
ET0             bit     IE.1            ; Interrupt Timer 0 freigeben
EX0             bit     IE.0            ; externen Interrupt 0 freigeben

IPH0            port    0b7h            ; Interrupt-Prioritäten
IPL0            port    0b8h

;----------------------------------------------------------------------------
; Ports mit Extrafunktionen:

P0              port    080h            ; Port 0

P1              port    090h            ; Port 1
T2              bit     P1.0            ; Ein/Ausgabe Timer 2
T2EX            bit     P1.1            ; Trigger Timer 2
ECI             bit     P1.2            ; externer Takt PCA
CEX0            bit     P1.3            ; Ein/Ausgabe PCA-Modul 0
CEX1            bit     P1.4            ; Ein/Ausgabe PCA-Modul 1
CEX2            bit     P1.5            ; Ein/Ausgabe PCA-Modul 2
CEX3            bit     P1.6            ; Ein/Ausgabe PCA-Modul 3
CEX4            bit     P1.7            ; Ein/Ausgabe PCA-Modul 4

P2              port    0a0h            ; Port 2

P3              port    0b0h            ; Port 3
RXD             bit     P3.0            ; serielle Empfangsleitung
TXD             bit     P3.1            ; serielle Sendeleitung
INT0            bit     P3.2            ; externer Interrupt 0
INT1            bit     P3.3            ; externer Interrupt 1
T0              bit     P3.4            ; Takteingang Timer 0
T1              bit     P3.5            ; Takteingang Timer 1
WR              bit     P3.6            ; Leseleitung
RD              bit     P3.7            ; Schreibleitung

;----------------------------------------------------------------------------
; serieller Port:

SCON            port    098h            ; Konfiguration
FE              bit     SCON.7          ; fehlerhaftes Stopbit ?
SM0             bit     SCON.7          ; Modusauswahl
SM1             bit     SCON.6
SM2             bit     SCON.5
REN             bit     SCON.4          ; Empfänger freigeben
TB8             bit     SCON.3          ; Sendedatenbit 8
RB8             bit     SCON.2          ; Empfangsdatenbit 8
TI              bit     SCON.1          ; Zeichen vollständig versandt ?
RI              bit     SCON.0          ; Zeichen vollständig empfangen ?

SBUF            port    099h            ; Datenregister
SADEN           port    0b9h            ; Slave-Adreßmaske
SADDR           port    0a9h            ; Slave-Adresse

;----------------------------------------------------------------------------
; Zähler/Watchdog:

TL0             port    08ah            ; Zählwert Zähler 0
TH0             port    08ch
TL1             port    08bh            ; Zählwert Zähler 1
TH1             port    08dh

TCON            port    088h            ; Steuerung Zähler 0/1
TF1             bit     TCON.7          ; Überlauf Timer 1 ?
TR1             bit     TCON.6          ; Timer 1 starten/stoppen
TF0             bit     TCON.5          ; Überlauf Timer 0 ?
TR0             bit     TCON.4          ; Timer 0 starten/stoppen
IE1             bit     TCON.3          ; externer Interrupt 1 augetreten ?
IT1             bit     TCON.2          ; Flanken/Pegeltriggerung ext. Interrupt 1
IE0             bit     TCON.1          ; externer Interrupt 0 augetreten ?
IT0             bit     TCON.0          ; Flanken/Pegeltriggerung ext. Interrupt 0

TMOD            port    089h            ; Steuerung/Modus Zähler 0/1
M00             bit     TMOD.0          ; Modus Timer 0
M10             bit     TMOD.1
CT0             bit     TMOD.2          ; Zähler/Timerumschaltung Timer 0
GATE0           bit     TMOD.3          ; Freigabemodus Timer 0
M01             bit     TMOD.4          ; Modus Timer 1
M11             bit     TMOD.5
CT1             bit     TMOD.6          ; Zähler/Timerumschaltung Timer 1
GATE1           bit     TMOD.7          ; Freigabemodus Timer 1


TL2             port    0cch            ; Zählwert Zähler 2
TH2             port    0cdh

T2CON           port    0c8h            ; Steuerung Zähler 2
TF2             bit     T2CON.7         ; Überlauf Timer 2 ?
EXF2            bit     T2CON.6         ; Flanke an T2EX aufgetreten ?
RCLK            bit     T2CON.5         ; Timer 2 oder 1 für seriellen Empfangstakt nutzen ?
TCLK            bit     T2CON.4         ; Timer 2 oder 1 für seriellen Sendetakt nutzen ?
EXEN2           bit     T2CON.3         ; Fang durch T2EX freigeben
TR2             bit     T2CON.2         ; Timer 2 starten/stoppen
CT2             bit     T2CON.1         ; Zähler/Timerumschaltung Timer 2
CPRL2           bit     T2CON.0         ; Fang oder Neuladen durch T2EX

T2MOD           port    0c9h            ; Steuerung/Modus Zähler 2
T2OE            bit     T2MOD.1         ; Ausgabe Timer 2 an T2 freigeben
DCEN            bit     T2MOD.0         ; Zählrichtung Timer 2

RCAP2L          port    0cah            ; Reload/Capture-Wert Timer 2
RCAP2H          port    0cbh
WDTRST          port    0a6h            ; Watchdog zurücksetzen

;----------------------------------------------------------------------------
; Zählerfeld:

CCON            port    0d8h            ; Steuerregister
CF              bit     CCON.7          ; Überlauf PCA ?
CR              bit     CCON.6          ; PCA-Timer starten/stoppen
CCF4            bit     CCON.4          ; Hat PCA-Modul 4..0 angesprochen ?
CCF3            bit     CCON.3
CCF2            bit     CCON.2
CCF1            bit     CCON.1
CCF0            bit     CCON.0

CMOD            port    0d9h            ; Modusregister
CIDL            bit     CMOD.7          ; PCA während Idle-Mode abschalten
WDTE            bit     CMOD.6          ; Watchdog-Ausgabe an PCA-Modul 4 freigeben
CPS1            bit     CMOD.2          ; PCA-Taktauswahl
CPS0            bit     CMOD.1
ECF             bit     CMOD.0          ; Interruptfreigabe

CL              port    0e9h            ; Zählwert
CH              port    0f9h

__defpcamodule  macro   Offset,NUM
CCAPM{NUM}      port    0d0h+Offset     ; Modusregister
ECOM{NUM}       bit     CCAPM0.6        ; Komparator Modul x einschalten
CAPP{NUM}       bit     CCAPM0.5        ; Fangfunktion Modul x einschalten (pos. Flanke)
CAPN{NUM}       bit     CCAPM0.4        ; Fangfunktion Modul x einschalten (neg. Flanke)
MAT{NUM}        bit     CCAPM0.3        ; Interrupt bei Gleichheit   Modul x einschalten
TOG{NUM}        bit     CCAPM0.2        ; High-Speed-Output-Modus Modul x einschalten
PWM{NUM}        bit     CCAPM0.1        ; PWM-Modus Modul x einschalten
ECCF{NUM}       bit     CCAPM0.0        ; Interrupts durch CCFx einschalten
CCAP{NUM}L      port    0e0h+Offset     ; Vergleichs/Fangwert Modul x
CCAP{NUM}H      port    0f0h+Offset
                endm

                __defpcamodule 0ah,"0"
                __defpcamodule 0bh,"1"
                __defpcamodule 0ch,"2"
                __defpcamodule 0dh,"3"
                __defpcamodule 0eh,"4"

;----------------------------------------------------------------------------

                endif

                restore                 ; wieder erlauben


