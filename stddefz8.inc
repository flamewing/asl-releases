		save
		listing off   ; kein Listing Åber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.39 - Datei Z8.INC                                                 *
;*   								            *
;*   Sinn : enthÑlt RegisterkÅrzel fÅr den Z8-Prozessor                     *
;* 									    *
;*   letzte énderungen : 16.12.1993 RÅdiger Mersiovsky                      *
;*                       25.12.1993 Copyrightvermerk                        *
;*                       15.03.1997 Z8604/08/30                             *
;*                                                                          *
;****************************************************************************

                ifndef  stddefz8inc     ; verhindert Mehrfacheinbindung

stddefz8inc     equ     1

                if      (MOMCPUNAME<>"Z8601")&&(MOMCPUNAME<>"Z8604")&&(MOMCPUNAME<>"Z8608")&&(MOMCPUNAME<>"Z8630")&&(MOMCPUNAME<>"Z8631")
                 fatal  "Falscher Prozessortyp eingestellt: nur Z8601, Z8604, Z8608, Z8630 oder Z8631 erlaubt!"
		endif

                if      MOMPASS=1
                 message "Z8-KÅrzel-Definitionen"
                 message "(C) 1993 RÅdiger Mersiovsky"
		endif

;----------------------------------------------------------------------------
; CPU-Kern

SPL             EQU     0FFH            ; Stackpointer, Lowteil
                if      MOMCPU=8601h
SPH              EQU     0FEH           ; Stackpointer, Highteil
                endif

FLAGS           EQU     0FCH            ; Flagregister

RP              EQU     0FDH            ; Registerpointer

GPR             EQU     0feh            ; frei benutzbar

;----------------------------------------------------------------------------
; Ports

P0              EQU	00H		; Port 0
P01M            EQU     0F8H            ; Tor 0, Tor 1 Betriebsart
                if      MOMCPU=8601h
P1               EQU     01H            ; Port 1
                endif
P2		EQU	02H		; Port 2
P2M             EQU     0F6H            ; Tor 2 Betriebsart
P3              EQU     03H             ; Port 3
P3M             EQU     0F7H            ; Tor 3 Betriebsart

;---------------------------------------------------------------------------
; Timer

TMR             EQU     0F1H            ; ZÑhler/Zeitgeberbetriebsart
T0              EQU     0F4H            ; ZÑhler/Zeitgeberregister Kanal 0
T1              EQU     0F2H            ; ZÑhler/Zeitgeberregister Kanal 1
PRE0            EQU     0F5H            ; T0-Vorteilerregister
PRE1            EQU     0F3H            ; T1-Vorteilerregister

;---------------------------------------------------------------------------
; Interrupts

IMR             EQU     0FBH            ; Interrupt-Maskierungsregister
IPR             EQU     0F9H            ; Interrupt-PrioritÑtsregister
IRQ             EQU     0FAH            ; Interrupt-Anmelderegister

;---------------------------------------------------------------------------
; serielles

                if      MOMCPU=8601h
SIO              EQU     0F0H           ; serielles Ein-Ausgaberegister
                endif

;---------------------------------------------------------------------------
; Register des Z8630 in anderen BÑnken

                if      MOMCPU>=8630h
PCON             equ     0              ; (Bank 15)
SMR              equ     11             ; (Bank 15)
WDTMR            equ     15             ; (Bank 15)
                endif

;---------------------------------------------------------------------------
; Speicheradressen

                switch  MOMCPU
                case    8604h
IRAM             equ    4
IRAMEND          equ    7fh
IROM             equ    0
IROMEND          equ    3ffh
                case    8608h
IRAM             equ    4
IRAMEND          equ    7fh
IROM             equ    0
IROMEND          equ    7ffh
                case    8630h
IRAM             equ    4
IRAMEND          equ    0efh
IROM             equ    0
IROMEND          equ    0fffh
                case    8631h
IRAM             equ    4
IRAMEND          equ    07fh
IROM             equ    0
IROMEND          equ    07ffh
                endcase

;---------------------------------------------------------------------------

                endif

                restore                 ; wieder erlauben

