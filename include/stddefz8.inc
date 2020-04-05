                ifndef  stddefz8inc     ; avoid multiple inclusion
stddefz8inc     equ     1

		save
		listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File Z8.INC                                                  *
;*   								            *
;*   Contains Register Symbols for the Z8                                   *
;* 									    *
;****************************************************************************

                if      (MOMCPU<>34305)
                 fatal  "wrong target selected: only Z8601 supported"
		endif

                if      MOMPASS=1
                 message "Z8 Register Definitions"
                 message "(C) 1993 Ruediger Mersiovsky"
		endif

;----------------------------------------------------------------------------

P0              EQU	00H		; Port 0
P1		EQU	01H		; Port 1
P2		EQU	02H		; Port 2
P3		EQU	03H		; Port 3
SIO		EQU	0F0H		; Serial Input/Output Register
FLAGS		EQU	0FCH		; Flag Register
SPH		EQU	0FEH		; Stack Pointer, MSB
SPL		EQU	0FFH		; Stack Pointer, LSB
TMR		EQU	0F1H		; Counter/Timer Operation Mode
T0		EQU	0F4H		; Counter/Timer Register Channel 0
T1		EQU	0F2H		; Counter/Timer Register Channel 1
PRE0		EQU	0F5H		; T0 Prescaler Register
PRE1		EQU	0F3H		; T1 Prescaler Register
P01M		EQU	0F8H		; Port 0, Port 1 Operation Mode
P2M		EQU	0F6H		; Port 2 Operation Mode
P3M		EQU	0F7H		; Port 3 Operation Mode
IMR		EQU	0FBH		; Interrupt Mask Register
IPR		EQU	0F9H		; Interrupt Priority Register
IRQ		EQU	0FAH		; Interrupt Status Register
RP		EQU	0FDH		; Register Pointer

;---------------------------------------------------------------------------

                restore                 ; re-allow listing

                endif			; stddefz8inc
