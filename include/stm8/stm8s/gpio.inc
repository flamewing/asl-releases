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
{__NS}ODR	label	$5000		; data output latch register
{__NS}IDR	label	$5001		; input pin value register
{__NS}DDR	label	$5002		; data direction register
{__NS}CR1	label	$5003		; control register 1
{__NS}CR2	label	$5004		; control register 2
		endm

		restore
		endif			; __stm8sgpioinc
