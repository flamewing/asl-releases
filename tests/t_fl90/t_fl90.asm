; FTEST.ASM
;******************************************************************************
;* Testet Gleitkommabibliothek f¸r TLCS90                                     *
;*                                                                            *
;* Hardware: TDB-TMP90                                                        *
;* Software: AS 1.39p5 oder hˆher                                             *
;*           Includes MACROS.INC, FLOAT.INC, CPU_TIME.INC                     *
;*                                                                            *
;* ‹bersetzen mit AS ftest oder beiliegendem Makefile                         *
;*                                                                            *
;******************************************************************************

                cpu     90c141

                org     8500h           ; Startadresse User-RAM

;------------------------------------------------------------------------------

CR              equ     13
LF              equ     10
Format_Tab      equ     0000100000000110b ; fftoa-Format f¸r tab. Ausgabe
Format_Min      equ     0010001100000101b ; fftoa-Format f¸r minimale L‰nge
;                         ^<+>^^<--+--->
;                         | | ||   |
;                         | | ||   +------ Maximalzahl Nachkommastellen
;                         | | |+---------- Mantissenpluszeichen unterdr¸cken
;                         | | +----------- Exponentenpluszeichen unterdr¸cken
;                         | +------------- Minimalstellenzahl Exponent
;                         +--------------- anh‰ngende Nullen in Mantisse lˆschen
Format          equ     Format_Tab      ; gew‰hltes fftoa-Format

;------------------------------------------------------------------------------
; Vorgaben

                include stddef90.inc    ; Registeradressen
                include macros.inc      ; f¸r Unterroutinen benˆtigte Makros
                include mon.inc         ; Einsprungadressen TDBTMP90-Monitor

                section MainProg

;------------------------------------------------------------------------------
; Makros zur Schreiberleichterung

pushop          macro   adr,{NoExpand}  ; einen Operanden auf den Stack legen
                ld      hl,(adr+2)
                push    hl
                ld      hl,(adr)
                push    hl
                endm

storeop         macro   {NoExpand}      ; Ergebnis in Array ablegen
                ld      (iy),de
                ld      (iy+2),bc
                add     iy,4
                endm

OneOp           macro   Msg,Operation,Op1,Op2,{Expand}  ; Aufruf, Ausgabe und
                call    PSTR                              ; Zeitmessung
                db      Msg,0
                call    StartTimer
                if      "OP1"<>""
                 pushop Op1
                endif
                if      "OP2"<>""
                 pushop Op2
                endif
                call    Operation
                storeop
                call    StopTimer
                if      (("OPERATION"<>"FNOP") && ("OPERATION"<>"FFTOI"))
                 call    PSTR
                 db      ", Ergebnis ",0
                 push    bc
                 push    de
                 ld      hl,Format
                 push    hl
                 ld      hl,CharBuffer
                 push    hl
                 call    fftoa
                 call    TXTAUS
                endif
                call    PSTR
                db      CR,LF,0
                endm

;------------------------------------------------------------------------------
; Hauptroutine

                proc    Main

                ld      sp,Stack        ; Stack reservieren
                ld      iy,Erg          ; Zeiger auf Ergebnisfeld
                call    InitTimer       ; Zeitmessung vorinitialisieren

                OneOp   "Ladeoverhead         : ",fnop,Eins,Eins
                OneOp   "Addition 2+Pi        : ",fadd,Zwei,Pi
                OneOp   "Addition 100000+2    : ",fadd,Thou,Zwei
                OneOp   "Addition 0+1         : ",fadd,Null,Eins
                OneOp   "Subtraktion Pi-2     : ",fsub,Pi,Zwei
                OneOp   "Subtraktion 100000-1 : ",fsub,Thou,Eins
                OneOp   "Multiplikation 2*Pi  : ",fmul,Zwei,Pi
                OneOp   "Division 1/Pi        : ",fdiv,Eins,Pi
                OneOp   "Wurzel aus 2         : ",fsqrt,Zwei,
                OneOp   "Wurzel aus 10000     : ",fsqrt,Thou,
                OneOp   "Wurzel aus -1        : ",fsqrt,MinEins,
                OneOp   "Wandlung 1-->Float   : ",fitof,IntEins,
                OneOp   "Wandlung 1E5-->Float : ",fitof,IntThou,
                OneOp   "Wandlung 1-->Int     : ",fftoi,Eins,
                OneOp   "Wandlung 1E5-->Int   : ",fftoi,Thou,
                ld      a,10
                OneOp   "Pi*2^10              : ",fmul2,Pi,
                ld      a,-10
                OneOp   "Pi*2^(-10)           : ",fmul2,Pi,

                call    PSTR
                db      "Eingabe: ",0
                ld      hl,InpBuffer
                call    TXTAUS
                ld      hl,InpBuffer
                push    hl
                call    fatof
                storeop
                call    PSTR
                db      ", Ergebnis: ",0
                push    bc
                push    de
                ld      hl,Format
                push    hl
                ld      hl,CharBuffer
                push    hl
                call    fftoa
                call    TXTAUS
                call    PSTR
                db      13,10,0

                jp      MRET

                endp

                proc    fnop            ; Dummyroutine fÅr Overheadmessung

                link    ix,0
                unlk    ix

                retd    8

                endp

CharBuffer:     db      30 dup (?)      ; Puffer fÅr fftoa
InpBuffer:      db      "-123.456E-7",0 ; Puffer fÅr fatof

                align   4
Eins:           dd      1.0             ; benîtigte Konstanten
MinEins:        dd      -1.0
Zwei:           dd      2.0
Pi:             dd      40490fdbh	; um Vergleichsfehler durch Rundung zu
                                        ; vermeiden
Zehn:           dd      10.0
Null:           dd      0.0
Thou:           dd      100000.0
IntEins:        dd      1
IntThou:        dd      100000
Erg:            dd      40 dup (?)      ; Ergebnisfeld

                align   2               ; Platz fÅr Stack
                db      300 dup (?)
Stack:
                endsection

;------------------------------------------------------------------------------
; benîtigte Module

                include cpu_time.inc     ; Zeitmessung
                include float.inc        ; Gleitkommabibliothek

;------------------------------------------------------------------------------

                end     Main

