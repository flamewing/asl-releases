		ifndef	__stm8tsyscfginc	; avoid multiple inclusion
__stm8tsyscfginc equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File SYSCFG.INC                                              *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8T System Configuration        *
;*                                                                          *
;*   Source: RM0312 (DocID022352 Rev 3)                                     *
;*                                                                          *
;****************************************************************************

__defsyscfg	macro	Base,Has3
SYSCFG_RMPCR1	label	Base+$01		; Remapping control register 1
SYSCFG_TIM3_CH2	bit	SYSCFG_RMPCR1,3		;  Timer3 channel2 mapping
SYSCFG_TIM3_CH1	bit	SYSCFG_RMPCR1,2		;  Timer3 channel1 mapping
		endm

		restore
		endif			; __stm8tsyscfginc
