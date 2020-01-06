		ifndef	__stm8stim4inc	; avoid multiple inclusion
__stm8stim4inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File TIM4.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S Timer 4                     *
;*                                                                          *
;****************************************************************************

__deftim4	macro	Base,DIER
TIM4_CR1	label	Base+$00	; TIM4 control register 1
TIM4_ARPE	bit	TIM4_CR1,7	;  Auto-reload preload enable
TIM4_OPM	bit	TIM4_CR1,3	;  One-pulse mode
TIM4_URS	bit	TIM4_CR1,2	;  Update request source
TIM4_UDIS	bit	TIM4_CR1,1	;  Update disable
TIM4_CEN	bit	TIM4_CR1,0	;  Counter enable
TIM4_IER	label	Base+DIER+$01	; TIM4 interrupt enable register
TIM4_TIE	bit	TIM4_IER,6	;  Trigger interrupt enable
TIM4_UIE	bit	TIM4_IER,0	;  Update interrupt enable
TIM4_SR		label	Base+DIER+$02	; TIM4 status register
TIM4_TIF	bit	TIM4_SR,6	;  Trigger interrupt flag
TIM4_UIF	bit	TIM4_SR,0	;  Update interrupt flag
TIM4_EGR	label	Base+DIER+$03	; TIM4 event generation register
TIM4_TG		bit	TIM4_EGR,6	;  Trigger generation
TIM4_UG		bit	TIM4_EGR,0	;  Update generation
TIM4_CNTR	label	Base+DIER+$04	; TIM4 counter
TIM4_PSCR	label	Base+DIER+$05	; TIM4 prescaler register
TIM4_ARR	label	Base+DIER+$06	; TIM4 auto-reload register
		endm

		restore
		endif			; __stm8stim4inc
