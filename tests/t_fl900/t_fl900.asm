; FTEST.ASM
;******************************************************************************
;* Testet Gleitkommabibliothek fr TLCS900                                    *
;*                                                                            *
;* Hardware: Micro-ICE TLCS900                                                *
;* Software: AS 1.39p1 oder h”her                                             *
;*           Includes MACROS.INC, FLOAT.INC, CONOUT.INC, CPU_TIME.INC         *
;*                                                                            *
;* šbersetzen mit AS ftest oder beiliegendem Makefile                         *
;*                                                                            *
;******************************************************************************

                cpu     96c141

                org     1000h           ; Startadresse User-RAM

;------------------------------------------------------------------------------

CR              equ     13
LF              equ     10
Format_Tab      equ     0000100000000110b ; fftoa-Format fr tab. Ausgabe
Format_Min      equ     0010001100000101b ; fftoa-Format fr minimale L„nge
;                         ³<Â>³³<ÄÄÂÄÄÄ>
;                         ³ ³ ³³   ³
;                         ³ ³ ³³   ÀÄÄÄÄÄÄ Maximalzahl Nachkommastellen
;                         ³ ³ ³ÀÄÄÄÄÄÄÄÄÄÄ Mantissenpluszeichen unterdrcken
;                         ³ ³ ÀÄÄÄÄÄÄÄÄÄÄÄ Exponentenpluszeichen unterdrcken
;                         ³ ÀÄÄÄÄÄÄÄÄÄÄÄÄÄ Minimalstellenzahl Exponent
;                         ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ anh„ngende Nullen Mantisse l”schen
Format          equ     Format_Tab      ; gew„hltes fftoa-Format

                supmode on              ; Vorgaben
                maxmode on
                macexp  off
                page    0               ; keine FFs
                include macros.inc

;------------------------------------------------------------------------------
; Hauptroutine, Test

; kleine Schreiberleichterung:

bench           macro   op,arg1,arg2,arg3,msg
                call    PSTR            ; Kennmeldung ausgeben
                db      msg,StrTerm
                call    CPU_TIME        ; Uhr starten
                ld      xwa,arg1        ; Operanden holen
                if      "ARG2"<>""      ; 2. Operanden evtl. weglassen
                 ld     xhl,arg2
                endif
                if      "ARG3"<>""      ; dito 3. Operanden
                 ld     bc,arg3
                endif
                call    op              ; Probanden laufen lassen
                ld      (xiz),xwa       ; Ergebnis weglegen...
                call    CPU_STOP        ; Uhr anhalten, Zeit ausgeben
                if ("OP"<>"FNOP")&&("OP"<>"FFTOI")
                 call    PSTR            ; etwas Platz
                 db      ", Ergebnis: ",StrTerm
                 ld      xwa,(xiz+)      ; Wert ausgeben
                 lda     xhl,(Buffer)
                 ld      bc,Format
                 call    fftoa
                 call    TXTAUS
                endif
                call    PSTR
                db      CR,LF,StrTerm
                endm

                proc    Main

                max                     ; ohne das macht das keinen Spaá !
                lda     xsp,(Stack)     ; etwas brauchen wir schon...
                lda     xiz,(Ergs)      ; Zeiger auf Ergebnisfeld
                call    CPU_TI_INI      ; Timer initialisieren

                ; Overhead messen
                bench   fnop,(FConst1),(FConst1),,"Overhead            : "

                ; Addition zweier fast gleicher Zahlen
                bench   fadd,(FConst1),(FConst2),,"Laufzeit 1+2        : "

                ; Addition zweier unterschiedl. groáer Zahlen
                bench   fadd,(FConst1),(FConst100000),,"Laufzeit 1+100000   : "

                ; Subtraktion zweier fast gleicher Zahlen
                bench   fsub,(FConst1),(FConst2),,"Laufzeit 1-2        : "

                ; Subtraktion zweier unterschiedl. groáer Zahlen
                bench   fsub,(FConst1),(FConst100000),,"Laufzeit 1-100000   : "

                ; Multiplikation
                bench   fmul,(FConst2),(FConstPi),,"Laufzeit 2*Pi       : "

                ; Division
                bench   fdiv,(FConst2),(FConstPi),,"Laufzeit 2/Pi       : "

                ; Multiplikation mit 2er-Potenz
                bench   fmul2,(FConstPi),,10,"Laufzeit Pi*2^(10)  : "

                ; Division durch 2er-Potenz
                bench   fmul2,(FConstPi),,-10,"Laufzeit Pi*2^(-10) : "

                ; kleine Zahl nach Float wandeln
                bench   fitof,1,,,"Laufzeit 1-->Float  : "

                ; groáe Zahl nach Float wandeln
                bench   fitof,100000,,,"Laufzeit 1E5-->Float: "

                ; kleine Zahl nach Int wandeln
                bench   fftoi,(FConst1),,,"Laufzeit 1-->Int    : "

                ; groáe Zahl nach Int wandeln
                bench   fftoi,(FConst100000),,,"Laufzeit 1E5-->Int  : "

                ; Wurzel
                bench   fsqrt,(FConst2),,,"Laufzeit SQRT(2)    : "

                call    PSTR
                db      "Eingabe: ",StrTerm
                lda     xhl,(InpBuffer)
                call    TXTAUS
                call    fatof
                call    PSTR
                db      ", Ergebnis: ",StrTerm
                lda     xhl,(Buffer)
                ld      bc,Format
                call    fftoa
                call    TXTAUS
                call    PSTR
                db      13,10,StrTerm

                swi     7               ; zum Monitor zurck

                endp

fnop:           ld      xwa,0           ; Dummy
                ret

                include "float.inc"
                include "conout.inc"
                include "cpu_time.inc"

;------------------------------------------------------------------------------
; Gleitkommakonstanten

                align   4               ; fr schnelleren Zugriff

FConst1         dd      1.0
FConst2         dd      2.0
FConst100000    dd      100000.0
FConstM1        dd      -1.0
FConstM2        dd      -2.0
FConstPi        dd      40490fdbh       ; um Vergleichsfehler durch Rundung zu 
					; vermeiden
Ergs            dd      30 dup (?)      ; Platz fr Ergebnisse

Buffer:         db      20 dup (?)
InpBuffer:      db      "12.3456E-12",0

;------------------------------------------------------------------------------
; Stack

                db      200 dup (?)
Stack:

