		ifndef	__stm8l152x4inc	; avoid multiple inclusion
__stm8l152x4inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG152X4.INC                                            *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L152X{4|6}                    *
;*   source: DocID15962 Rev 15                                              *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; just add LCD resources to STM8L151X{4|6}

		include "reg151x4.inc"

LCD_vect	label	$8048		; LCD interrupt

;----------------------------------------------------------------------------
; LCD Controller

		include "lcd.inc"
		__deflcd $5400,28,14,0

                restore                 ; allow again

                endif			; __stm8l152x4inc
