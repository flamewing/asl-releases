		ifndef	__stm8sbeepinc	; avoid multiple inclusion
__stm8sbeepinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File BEEP.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S (and STM8Lx01) Beeper       *
;*                                                                          *
;****************************************************************************

__defbeep	macro	Base
BEEP_CSR	label	Base+$00	; Beeper control/status register
BEEPSEL		bfield	BEEP_CSR,6,2	;  Beep selection
BEEPEN		bit	BEEP_CSR,5	;  Beep enable
BEEPDIV		bfield	BEEP_CSR,0,5	;  Beep prescaler divider
		endm

		restore
		endif			; __stm8sbeepinc
