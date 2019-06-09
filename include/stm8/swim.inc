		ifndef	__stm8sswiminc	; avoid multiple inclusion
__stm8sswiminc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File SWIM.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S SWIM                        *
;*                                                                          *
;****************************************************************************

__defswim	macro	Base
SWIM_CSR	label	Base+$00	; SWIM control status register
SWIMCLK		bit	SWIM_CSR,0	;  SWIM clock divider
		endm

		restore
		endif			; __stm8sswiminc
