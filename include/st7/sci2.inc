		ifndef	__st7sci2inc	; avoid multiple inclusion
__st7sci2inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File SCI2.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST7 SCI with Parity Function      *
;*                                                                          *
;****************************************************************************

		include	"sci.inc"

__defsci2	macro	Base,ERPROffs
		__defsci Base,ERPROffs
SCI_PE		bit	SCISR,0		;  Parity error
SCID		bit	SCICR1,5	;  Disabled for Low Power Consumption
PCE		bit	SCICR1,2	;  Parity Control Enable
PS		bit	SCICR1,1	;  Parity Selection
PIE		bit	SCICR1,0	;  Parity Interrupt Enable
		endm

		restore
		endif			; __st7sci2inc
