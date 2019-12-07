		ifndef	__iorinc	; avoid multiple inclusion
__iorinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File IOR.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST62xx Interrupt Option Register  *
;*                                                                          *
;****************************************************************************

IOR		sfr	0c8h		; Interrupt Option Register (NOT bit-accessible!)
LES		bit	6,IOR		;  Level/Edge Selection bit
ESB		bit	5,IOR		;  Edge Selection bit
GEN		bit	4,IOR		;  Global Enable Interrupt

		restore
		endif			; __iorinc
