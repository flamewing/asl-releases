                save
                listing off   ; kein Listing über diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.41 - Datei REGMSP.INC                                             *
;*   								            *
;*   Sinn : enthält Makro- und Registerdefinitionen für den MSP430          *
;* 									    *
;*   letzte Änderungen : 11. 9.1996                                         *
;*                                                                          *
;****************************************************************************

                ifndef  regmspinc      ; verhindert Mehrfacheinbindung

regmspinc       equ     1

                if      (MOMCPUNAME<>"MSP430")
                 fatal  "Falscher Prozessortyp eingestellt: nur MSP430 erlaubt!"
		endif

                if      MOMPASS=1
                 message "MSP430-Register+Befehlsdefinitionen (C) 1996 Alfred Arnold"
		endif

;----------------------------------------------------------------------------
; Arithmetik

adc             macro   op
                addc.attribute #0,op
                endm

dadc            macro   op
                dadd.attribute #0,op
                endm

dec             macro   op
                sub.attribute #1,op
                endm

decd            macro   op
                sub.attribute #2,op
                endm

inc             macro   op
                add.attribute #1,op
                endm

incd            macro   op
                add.attribute #2,op
                endm

sbc             macro   op
                subc.attribute #0,op
                endm

;----------------------------------------------------------------------------
; Logik

inv             macro   op
                xor.attribute #-1,op
                endm

rla             macro   op
                add.attribute op,op
                endm

rlc             macro   op
                addc.attribute op,op
                endm

;----------------------------------------------------------------------------
; Daten bewegen ;-)

clr             macro   op
                mov.attribute #0,op
                endm

clrc            macro
                bic     #1,sr
                endm

clrn            macro
                bic     #4,sr
                endm

clrz            macro
                bic     #2,sr
                endm

pop             macro   op
                mov     @sp+,op
                endm

setc            macro
                bis     #1,sr
                endm

setn            macro
                bis     #4,sr
                endm

setz            macro
                bis     #2,sr
                endm

tst             macro   op
                cmp.attribute #0,op
                endm

;----------------------------------------------------------------------------
; Sprungbefehle

br              macro   op
                mov     op,pc
                endm

dint            macro
                bic     #8,sr
                endm

eint            macro
                bis     #8,sr
                endm

nop             macro
                .word   04303h         ; den symbolischen Befehl würde AS zurückweisen
                endm

ret             macro
                mov     @sp+,pc
                endm

;----------------------------------------------------------------------------
; Taktgenerator

SCFQCTL         equ     052h            ; Multiplikator Quarzfrequenz
SCFI0           equ     050h            ; Integrator
SCFI1           equ     051h
CBCTL           equ     053h            ; Puffersteuerung
EPCTL           equ     054h            ; EPROM-Steuerung

;----------------------------------------------------------------------------
; Module

IE1             equ     000h            ; Interrupt-Freigaben
IE2             equ     001h
IFG1            equ     002h            ; Interrupt-Anzeigen
IFG2            equ     003h
ME1             equ     004h            ; Modul-Freigaben
ME2             equ     005h

;----------------------------------------------------------------------------
; Timer

BTCTL           equ     040h            ; Basis-Steuerregister Timer 1

BTCNT1          equ     046h            ; Zählregister
BTCNT2          equ     047h

TCCTL           equ     042h

TCPLD           equ     043h            ; Vorladewert

TCDAT           equ     044h            ; Zählwert

TPCTL           equ     04bh            ; Timer/Port Steuerregister

TPCNT1          equ     04ch            ; Zählregister
TPCNT2          equ     04dh

TPD             equ     04eh            ; Datenregister

TPE             equ     04fh            ; Freigaberegister

;----------------------------------------------------------------------------
; Watchdog

WDTCTL          equ     0120h

;----------------------------------------------------------------------------
; PWM

PWMCTL          equ     058h            ; Zählwert
PWMDTB          equ     059h            ; Pulsweite (Puffer)
PWMDTR          equ     05ah            ; Pulsweite
PWMCNT          equ     05bh            ; Steuerung

;----------------------------------------------------------------------------
; Ports

P0IN            equ     010h            ; Leseregister (Pinzustand)
P0OUT           equ     011h            ; Schreibregister (Latches)
P0DIR           equ     012h            ; Richtungsregister
P0IFG           equ     013h            ; Interrupt-Flags
P0IES           equ     014h            ; Interrupf-Flankenwahl
P0IE            equ     015h            ; Interrupt-Freigaben

;----------------------------------------------------------------------------
; LCD-Interface

LCDCTL          equ     030h            ; Steuerung
LCD_Start       equ     031h            ; Startadresse
LCD_Stop        equ     03fh            ; Endadresse
__TMP           set     1               ; Einzeldefinitionen
                rept    9
LCD{"\{__TMP}"} equ     030h+__TMP
__TMP           set     __TMP+1
                endm
                rept    6
LCD1{"\{__TMP-10}"} equ 030h+__TMP
__TMP           set     __TMP+1
                endm

;----------------------------------------------------------------------------
; A/D-Wandler

AIN             equ     0110h           ; Eingaberegister
AEN             equ     0112h           ; Eingabefreigaben
ACTL            equ     0114h           ; Steuerung
ADAT            equ     0118h           ; Daten

;----------------------------------------------------------------------------

                endif                   ; von IFDEF...
		restore                 ; wieder erlauben

