		ifndef	__stm8sawuinc	; avoid multiple inclusion
__stm8sawuinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File AWU.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S AWU                         *
;*                                                                          *
;****************************************************************************

__defawu	macro	Base
AWU_CSR		label	Base+$00	;  AWU control/status register
AWU_AWUF	bit	AWU_CSR,5	;  Auto-wakeup flag
AWU_AWUEN	bit	AWU_CSR,4	;  Auto-wakeup enable
AWU_MSR		bit	AWU_CSR,0	;  Measurement enable

AWU_APR		label	Base+$01	;  AWU asynchronous prescaler buffer register

AWU_TBR		label	Base+$02	;  AWU timebase selection register
AWU_AWUTB	bfield	AWU_TBR,0,4	;  Auto-wakeup timebase selection
		endm

		restore
		endif			; __stm8sawuinc
