		ifndef	__stm8l152x8inc	; avoid multiple inclusion
__stm8l152x8inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG152X8.INC                                            *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L152X{6|8}                    *
;*   source: DS6948 Rev 11                                                  *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; just add LCD resources to STM8L1511{6|8}

		include "reg151x8.inc"

LCD_vect	label	$8048		; LCD interrupt

;----------------------------------------------------------------------------
; LCD Controller

		include "lcd.inc"
		__deflcd $5400,44,22,1

                restore                 ; allow again

                endif			; __stm8l152x8inc
