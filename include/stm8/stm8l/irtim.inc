		ifndef	__stm8lirtiminc	; avoid multiple inclusion
__stm8lirtiminc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File IRTIM.INC                                               *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L Infrared Timer              *
;*                                                                          *
;****************************************************************************

__defirtim	macro	Base
IR_CR		label	Base+$00	; Infrared control register
IR_HS_EN	bit	IR_CR,1		;  High Sink LED driver capability enable
IR_EN		bit	IR_CR,0		;  Infrared output enable
		endm

		restore
		endif			; __stm8lirtiminc
