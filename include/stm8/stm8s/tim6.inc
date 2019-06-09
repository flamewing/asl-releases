		ifndef	__stm8stim6inc	; avoid multiple inclusion
__stm8stim6inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File TIM6.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S Timer 6                     *
;*                                                                          *
;****************************************************************************

                ; TIM6 is identical to TIM4, with the addition
                ; of CR2 and SMCR that fill the gap existing on some
                ; implementations of TIM4.

__deftim6	macro	Base
TIM6_CR1	label	Base+$00	; TIM6 control register 1
TIM6_ARPE	bit	TIM6_CR1,7	;  Auto-reload preload enable
TIM6_OPM	bit	TIM6_CR1,3	;  One-pulse mode
TIM6_URS	bit	TIM6_CR1,2	;  Update request source
TIM6_UDIS	bit	TIM6_CR1,1	;  Update disable
TIM6_CEN	bit	TIM6_CR1,0	;  Counter enable
TIM6_CR2	label	Base+$01	; TIM6 control register 2
TIM6_MMS	bfield	TIM6_CR2,4,3	;  Master mode selection
TIM6_SMCR	label	Base+$02	; TIM6 slave mode control register
TIM6_MSM	bit	TIM6_SMCR,7	;  Master/slave mode
TIM6_TS		bfield	TIM6_SMCR,4,3	;  Trigger selection
TIM6_SMS	bfield	TIM6_SMCR,0,3	;  Clock/trigger/slave mode selection
TIM6_IER	label	Base+$03	; TIM6 interrupt enable register
TIM6_TIE	bit	TIM6_IER,6	;  Trigger interrupt enable
TIM6_UIE	bit	TIM6_IER,0	;  Update interrupt enable
TIM6_SR		label	Base+$04	; TIM6 status register
TIM6_TIF	bit	TIM6_SR,6	;  Trigger interrupt flag
TIM6_UIF	bit	TIM6_SR,0	;  Update interrupt flag
TIM6_EGR	label	Base+$05	; TIM6 event generation register
TIM6_TG		bit	TIM6_EGR,6	;  Trigger generation
TIM6_UG		bit	TIM6_EGR,0	;  Update generation
TIM6_CNTR	label	Base+$06	; TIM6 counter
TIM6_PSCR	label	Base+$07	; TIM6 prescaler register
TIM6_ARR	label	Base+$08	; TIM6 auto-reload register
		endm

		restore
		endif			; __stm8stim6inc
