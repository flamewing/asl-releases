		ifndef	__stm8trstinc	; avoid multiple inclusion
__stm8trstinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File RST.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8T Reset Controller            *
;*                                                                          *
;*   Source: RM0312 (DocID022352 Rev 3)                                     *
;*                                                                          *
;****************************************************************************

__defrst	macro	Base
RST_CR		label	Base+$00	; Reset control register
RSTPIN_KEY	bfield	RST_CR,0,8	;  Reset pin configuration key
RST_SR		label	Base+$01	; Reset status register
WWDGF		bit	RST_SR,4	;  Window Watchdog reset flag
SWIMF		bit	RST_SR,3	;  SWIM reset flag
ILLOPF		bit	RST_SR,2	;  Illegal opcode reset flag
IWDGF		bit	RST_SR,1	;  Independent Watchdog reset flag
PORF		bit	RST_SR,0	;  Power-on Reset (POR) flag
		endm

		restore
		endif			; __stm8lrst01inc
