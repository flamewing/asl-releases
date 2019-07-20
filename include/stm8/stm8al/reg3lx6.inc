		ifndef	__stm8l3lx6inc	; avoid multiple inclusion
__stm8l3lx6inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG3LX6.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8AL3L4x/6x                     *
;*   source: DS7106 Rev 9                                                   *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; same like STM8AL313x/4x/6x...

		include	"reg31x6.inc"

;----------------------------------------------------------------------------
; ...plus LCD Controller

LCD_vect	label	$8048		; LCD interrupt

		include "stm8/stm8l/lcd.inc"
		__deflcd $5400,28,14,0

                restore                 ; allow again

                endif			; __stm8l3lx6inc
