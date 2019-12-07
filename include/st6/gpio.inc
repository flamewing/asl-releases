		ifndef	__st6gpioinc	; avoid multiple inclusion
__st6gpioinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File GPIO.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST62xx GPIO                       *
;*                                                                          *
;****************************************************************************

__defgpio	macro	NAME,Base
__NS		eval	"\{NAME}"
DR{__NS}	sfr	Base+00h	; Data Register
DDR{__NS}	sfr	Base+04h	; Data Direction Register
OR{__NS}	sfr	Base+0ch	; Option Register
		endm

		restore
		endif			; __st6gpioinc
