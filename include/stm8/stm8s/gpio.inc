		ifndef	__stm8sgpioinc	; avoid multiple inclusion
__stm8sgpioinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File GPIO.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S GPIO                        *
;*                                                                          *
;****************************************************************************

__defgpio	macro	NAME,Base
__NS		set	"\{NAME}_"
{__NS}ODR	label	Base+$00	; Data Output Latch Register
{__NS}IDR	label	Base+$01	; Input Pin Value Register
{__NS}DDR	label	Base+$02	; Data Direction Register
{__NS}CR1	label	Base+$03	; Control Register 1
{__NS}CR2	label	Base+$04	; Control Register 2
		endm

		restore
		endif			; __stm8sgpioinc
