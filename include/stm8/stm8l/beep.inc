		ifndef	__stm8lbeepinc	; avoid multiple inclusion
__stm8lbeepinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File BEEP.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L Beeper                      *
;*                                                                          *
;****************************************************************************

__defbeep	macro	Base
BEEP_CSR1	label	Base+$00	; Beeper control/status register 1
BEEP_MSR	bit	BEEP_CSR1,0	;  Measurement Enable
BEEP_CSR2	label	Base+$03	; Beeper control/status register 2
BEEPSEL		bfield	BEEP_CSR2,6,2	;  Beep selection
BEEPEN		bit	BEEP_CSR2,5	;  Beep enable
BEEPDIV		bfield	BEEP_CSR2,0,5	;  Beep prescaler divider
		endm

		restore
		endif			; __stm8lbeepinc
