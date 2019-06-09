		ifndef	__stm8lrst01inc	; avoid multiple inclusion
__stm8lrst01inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File RST01.INC                                               *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8Lx01 Reset Controller         *
;*                                                                          *
;****************************************************************************

__defrst01	macro	Base
RST_CR		label	Base+$00	; Reset control register
RSTPIN_KEY	bfield	RST_CR,0,8	;  Reset pin configuration key
RST_SR		label	Base+$01	; Reset status register
SWIMF		bit	RST_SR,3	;  SWIM reset flag
ILLOPF		bit	RST_SR,2	;  Illegal opcode reset flag
IWDGF		bit	RST_SR,1	;  Independent Watchdog reset flag
PORF		bit	RST_SR,0	;  Power-on Reset (POR) flag
		endm

		restore
		endif			; __stm8lrst01inc
