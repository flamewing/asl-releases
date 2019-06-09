		ifndef	__stm8li2cinc	; avoid multiple inclusion
__stm8li2cinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File I2C.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L I2C                         *
;*                                                                          *
;****************************************************************************

		include	"stm8/stm8l/i2c01.inc"
__defi2c	macro	NAME,Base,HasDual
		__defi2c01 NAME,Base,HasDual
__NS		set	"\{NAME}_"
{__NS}PECR	label	Base+$e		; packet error checking register
		endm

		restore
		endif			; __stm8li2cinc
