		ifndef	__stm8ltim4inc	; avoid multiple inclusion
__stm8ltim4inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File TIM4.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L Timer 4                     *
;*                                                                          *
;****************************************************************************

__deftim4	macro	Base,HasDMA
TIM4_CR1	label	Base+$00	; TIM4 control register 1
TIM4_ARPE	bit	TIM4_CR1,7	;  Auto-reload preload enable
TIM4_OPM	bit	TIM4_CR1,3	;  One-pulse mode
TIM4_URS	bit	TIM4_CR1,2	;  Update request source
TIM4_UDIS	bit	TIM4_CR1,1	;  Update disable
TIM4_CEN	bit	TIM4_CR1,0	;  Counter enable
TIM4_CR2	label	Base+$01	; TIM4 control register 2
TIM4_MMS	bfield	TIM4_CR2,4,3	;  Master mode selection
TIM4_SMCR	label	Base+$02	; TIM4 Slave mode control register
TIM4_MSM	bit	TIM4_SMCR,7	;  Master/slave mode
TIM4_TS		bfield	TIM4_SMCR,4,3	;  Trigger selection
TIM4_SMS	bfield	TIM4_SMCR,0,3	;  Clock/trigger/slave mode selection
		if	HasDMA
TIM4_DER	 label	Base+$03	; TIM4 DMA1 request enable register
TIM4_UDE	 bit	TIM4_DER,0	;  Update DMA request enable
		endif
TIM4_IER	label	Base+$03+HasDMA	; TIM4 interrupt enable register
TIM4_TIE	bit	TIM4_IER,6	;  Trigger interrupt enable
TIM4_UIE	bit	TIM4_IER,0	;  Update interrupt enable
TIM4_SR1	label	Base+$04+HasDMA	; TIM4 status register 1
TIM4_TIF	bit	TIM4_SR1,6	;  Trigger interrupt flag
TIM4_UIF	bit	TIM4_SR1,0	;  Update interrupt flag
TIM4_EGR	label	Base+$05+HasDMA	; TIM4 event generation register
TIM4_TG		bit	TIM4_EGR,6	;  Trigger generation
TIM4_UG		bit	TIM4_EGR,0	;  Update generation
TIM4_CNTR	label	Base+$06+HasDMA	; TIM4 counter
TIM4_PSCR	label	Base+$07+HasDMA	; TIM4 prescaler register
TIM4_ARR	label	Base+$08+HasDMA	; TIM4 auto-reload register
		endm

		restore
		endif			; __stm8ltim4inc
