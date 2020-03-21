		ifndef	__regz86xxinc
__regz86xxinc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGZ86XX.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for Z8601/8603                     *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; CPU Core

SPL		sfr	0ffh		; Stack Pointer
SPH		sfr	0feh
RP		sfr	0fdh		; Register Pointer
FLAGS		sfr	0fch		; CPU Flags

;----------------------------------------------------------------------------
; Memory Limits

;----------------------------------------------------------------------------
; GPIO

P0              sfr	00h		; Port 0 Data
P1		sfr	01h		; Port 1 Data
P2		sfr	02h		; Port 2 Data
P3		sfr	03h		; Port 3 Data
P01M		sfr	0f8h		; Port 0, Port 1 Betriebsart
P0_03_MODE	__z8bfield P01M,0,2	;  Port 0.0..3 Mode
STACKSEL	__z8bit	P01M,2		;  Stack external/internal?
P1_07_MODE	__z8bfield P01M,3,2	;  Port 1.0..7 Mode
XMEM_TIMING	__z8bit	P01M,5		;  Externel memory timing
P0_47_MODE	__z8bfield P01M,6,2	;  Port 0.4..7 Mode
P2M		sfr	0f6h		; Port 2 Mode
P3M		sfr	0f7h		; Port 3 Mode
P2_PUP_EN	__z8bit	P3M,0		;  Enable Pullups on P2
RDY0_DAV0_EN	__z8bit	P3M,2		;  Enable P32/P35 as RDY0/DAV0
RDY1_DAV1_EN	__z8bfield P3M,3,2	;  Enable P33/P34 as RDY1/DAV1
RDY2_DAV2_EN	__z8bit	P3M,5		;  Enable P31/P36 as RDY2/DAV2
SIO_EN		__z8bit P3M,6		;  Enable P30/P37 as SIN/SOUT 
PAR_EN		__z8bit	P3M,7		;  Parity Enable

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum	 RESET_vect=0	; Reset Entry
		nextenum IRQ0_vect	; External Interrupt Request 0
		nextenum IRQ1_vect	; External Interrupt Request 1
		nextenum IRQ2_vect	; External Interrupt Request 2
		nextenum IRQ3_vect	; External Interrupt Request 3, shared with...
SIO_RX_vect	label	IRQ3_vect	; SIO character received
		nextenum SIO_TX_vect	; SIO character transmitted, shared with...
T0_vect		label	SIO_TX_vect	; Timer 0 end of count
		if	__hastimer1
		 nextenum T1_vect	; [IRQ5] Timer 1 end of count
		endif

;----------------------------------------------------------------------------
; Interrupt Vectors

IMR		sfr	0fbh		; Interrupt Mask Register
EIRQ0		__z8bit IMR,0		;  Enable IRQ0
EIRQ1		__z8bit IMR,1		;  Enable IRQ1
EIRQ2		__z8bit IMR,2		;  Enable IRQ2
EIRQ3		__z8bit IMR,3		;  Enable IRQ3
EIRQ4		__z8bit IMR,4		;  Enable IRQ4
EIRQ5		__z8bit IMR,5		;  Enable IRQ5
EINT		__z8bit	IMR,7		;  Master Interrupt Enable

IPR		sfr	0f9h		; Interrupt Priory Register

IRQ		sfr	0fah		; Interrupt Request Register
IRQ0		__z8bit IRQ,0		;  IRQ0 requested
IRQ1		__z8bit IRQ,1		;  IRQ1 requested
IRQ2		__z8bit IRQ,2		;  IRQ2 requested
IRQ3		__z8bit IRQ,3		;  IRQ3 requested
IRQ4		__z8bit IRQ,4		;  IRQ4 requested
IRQ5		__z8bit IRQ,5		;  IRQ5 requested
IEDGE		__z8bfield 6,2		;  IRQ Edge Selection

;----------------------------------------------------------------------------
; Timers

TMR		sfr	0f1h		; Timer Operation Mode
T0_LOAD		__z8bit	TMR,0		;  Timer 0 Load
T0_EN		__z8bit TMR,1		;  Enable Timer 0
TIN_MODES	__z8bfield TMR,4,2	;  Tin Input Modes
TOUT_MODES	__z8bfield TMR,6,2	;  Tout Output Modes

T0		sfr	0f4h		; Timer 0 Value
PRE0		sfr	0f5h		; Timer 0 Prescaler
T0_COUNTMODE	__z8bit	PRE0,0		;  Count Mode (once or modulo)
T0_PRESCALER	__z8bfield PRE0,2,6	;  Prescaler Value

		if	__hastimer1
T1_LOAD		__z8bit	TMR,2		;  Timer 1 Load
T1_EN		__z8bit TMR,3		;  Enable Timer 1
T1		sfr	0f2h		; Timer 1 Value
PRE1		sfr	0f3h		; Timer 1 Prescaler
T1_COUNTMODE	__z8bit	PRE1,0		;  Count Mode (once or modulo)
T1_CLKSRC	__z8bit PRE1,1		;  Clock Source
T1_PRESCALER	__z8bfield PRE1,2,6	;  Prescaler Value
		endif			; __hastimer1

;----------------------------------------------------------------------------
; UART

SIO		sfr	0f0h		; Serial Input/Output Register

;----------------------------------------------------------------------------

		restore

		endif			; __regz86xxinc
