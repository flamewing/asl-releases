		ifndef	__lcdinc	; avoid multiple inclusion
__lcdinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File LCD.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST62xx LCD Controller             *
;*                                                                          *
;****************************************************************************

LCDCR		sfr	0dch		; LCD Mode Control Register
DS		bfield	LCDCR,6,2	;  Multiplexing Ratio Select
HF		bfield	LCDCR,3,3	;  Oscillator Select
LF		bfield	LCDCR,0,3	;  Base Frame Frequency

		restore
		endif			; __lcdinc
