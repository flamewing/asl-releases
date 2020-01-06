		ifndef	__st7dminc	; avoid multiple inclusion
__st7dminc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File DM.INC                                                  *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST7 Debug Module                  *
;*                                                                          *
;****************************************************************************

__defdm		macro	Base
DMCR		label	Base+$00	; DM Control Register
RST		bit	DMCR,4		; Reset
DMSR		label	Base+$01	; DM Status Register
MTR		bit	DMSR,6
DMBK1H		label	Base+$02	; DM Breakpoint Register 1 High
DMBK1L		label	Base+$03	; DM Breakpoint Register 1 Low
DMBK2H		label	Base+$04	; DM Breakpoint Register 2 High
DMBK2L		label	Base+$05	; DM Breakpoint Register 2 Low
		endm

		restore
		endif			; __st7dminc
