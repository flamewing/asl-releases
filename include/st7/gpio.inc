		ifndef	__st7gpioinc	; avoid multiple inclusion
__st7gpioinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File GPIO.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST72xxx GPIO                      *
;*                                                                          *
;****************************************************************************

__defgpio	macro	NAME,Base
__NS		set	"\{NAME}"
{__NS}DR	label	Base+$00	; Data Register
{__NS}DDR	label	Base+$02	; Data Direction Register
{__NS}OR	label	Base+$03	; Option Register
		endm

		restore
		endif			; __st7gpioinc
