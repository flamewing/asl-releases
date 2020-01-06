		ifndef	__timer1inc	; avoid multiple inclusion
__timer1inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File TIMER1.INC                                              *
;*                                                                          *
;*   contains common SFR and Bit Definitions for ST62xx Timer1              *
;*                                                                          *
;****************************************************************************

__deftimer	macro	BASE,NUM
__NS		eval	"\{NUM}"
PSCR{__NS}	sfr	BASE+0		; Prescaler Register
TCR{__NS}	sfr	BASE+1		; Downcounter Register
TSCR{__NS}	sfr	BASE+2		; Status Control Register
TMZ{__NS}	bit	7,TSCR{__NS}	;  Timer Zero Bit
ETI{__NS}	bit	6,TSCR{__NS}	;  Enable Timer Interrupt
PSI{__NS}	bit	3,TSCR{__NS}	;  Prescaler Initialize Bit
PS{__NS}	bfield	TSCR{__NS},0,3	;  Prescaler Mux. Select
		endm

		restore
		endif			; __timer1inc
