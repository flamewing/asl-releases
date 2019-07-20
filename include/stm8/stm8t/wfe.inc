		ifndef	__stm8twfeinc	; avoid multiple inclusion
__stm8twfeinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File WFE.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8T Wait for Events             *
;*                                                                          *
;*   Source: RM0312 (DocID022352 Rev 3)                                     *
;*                                                                          *
;****************************************************************************

__defwfe	macro	Base,Has4
WFE_CR1		label	Base+$00	; WFE control register 1
EXTI_EV3	bit	WFE_CR1,7	;  External interrupt event 3
EXTI_EV2	bit	WFE_CR1,6	;  External interrupt event 2
EXTI_EV1	bit	WFE_CR1,5	;  External interrupt event 1
EXTI_EV0	bit	WFE_CR1,4	;  External interrupt event 0
PXS_EV1		bit	WFE_CR1,3	;  ProxSense interrupt
TIM2_EV1	bit	WFE_CR1,1	;  TIM2 interrupt event 1
TIM2_EV0	bit	WFE_CR1,0	;  TIM2 interrupt event 0
WFE_CR2		label	Base+$01	; WFE control register 2
EXTI_EVD	bit	WFE_CR2,5	;  External interrupt event on Port D
EXTI_EVB	bit	WFE_CR2,4	;  External interrupt event on Port B
EXTI_EV7	bit	WFE_CR2,3	;  External interrupt event 7
EXTI_EV6	bit	WFE_CR2,2	;  External interrupt event 6
EXTI_EV5	bit	WFE_CR2,1	;  External interrupt event 5
EXTI_EV4	bit	WFE_CR2,0	;  External interrupt event 4
		endm
		
		restore
		endif			; __stm8twfeinc
