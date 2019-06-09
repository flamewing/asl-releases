		ifndef	__stm8lrstinc	; avoid multiple inclusion
__stm8lrstinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File RST.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L Reset Controller            *
;*                                                                          *
;****************************************************************************

		include	"stm8/stm8l/rst01.inc"
__defrst	macro	Base
		__defrst01 Base
BORF		bit	RST_SR,5	;  Brownout reset flag
WWDGF		bit	RST_SR,4	;  Window Watchdog reset flag
		endm

		restore
		endif			; __stm8lrstinc
