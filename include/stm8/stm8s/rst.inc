		ifndef	__stm8srstinc	; avoid multiple inclusion
__stm8srstinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File RST.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S Reset Controller            *
;*                                                                          *
;****************************************************************************

__defrst	macro	Base
RST_SR		label	Base+$00	; Reset status register
EMCF		bit	RST_SR,4	;  EMC reset flag
SWIMF		bit	RST_SR,3	;  SWIM reset flag
ILLOPF		bit	RST_SR,2	;  Illegal opcode reset flag
IWDGF		bit	RST_SR,1	;  Independent Watchdog reset flag
WWDGF		bit	RST_SR,0	;  Window Watchdog reset flag
		endm

		restore
		endif			; __stm8srstinc
