		ifndef	__stm8stim3inc	; avoid multiple inclusion
__stm8stim3inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File TIM3.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S Timer 3                     *
;*                                                                          *
;****************************************************************************

__deftim3	macro	Base,DIER
TIM3_CR1	label	Base+$00	; TIM3 control register 1
TIM3_ARPE	bit	TIM3_CR1,7	;  Auto-reload preload enable
TIM3_OPM	bit	TIM3_CR1,3	;  One-pulse mode
TIM3_URS	bit	TIM3_CR1,2	;  Update request source
TIM3_UDIS	bit	TIM3_CR1,1	;  Update disable
TIM3_CEN	bit	TIM3_CR1,0	;  Counter enable
TIM3_IER	label	Base+DIER+$01	; TIM3 interrupt enable register
TIM3_CC3IE	bit	TIM3_IER,3	;  Capture/compare 3 interrupt enable
TIM3_CC2IE	bit	TIM3_IER,2	;  Capture/compare 2 interrupt enable
TIM3_CC1IE	bit	TIM3_IER,1	;  Capture/compare 1 interrupt enable
TIM3_UIE	bit	TIM3_IER,0	;  Update interrupt enable
TIM3_SR1	label	Base+DIER+$02	; TIM3 status register 1
TIM3_CC3IF	bit	TIM3_SR1,3	;  Capture/compare 3 interrupt flag
TIM3_CC2IF	bit	TIM3_SR1,2	;  Capture/compare 2 interrupt flag
TIM3_CC1IF	bit	TIM3_SR1,1	;  Capture/compare 1 interrupt flag
TIM3_UIF	bit	TIM3_SR1,0	;  Update interrupt flag
TIM3_SR2	label	Base+DIER+$03	; TIM3 status register 2
TIM3_CC3OF	bit	TIM3_SR2,3	;  Capture/compare 3 overcapture flag
TIM3_CC2OF	bit	TIM3_SR2,2	;  Capture/compare 2 overcapture flag
TIM3_CC1OF	bit	TIM3_SR2,1	;  Capture/compare 1 overcapture flag
TIM3_EGR	label	Base+DIER+$04	; TIM3 event generation register
TIM3_CC3G	bit	TIM3_EGR,3	;  Capture/compare 3 generation
TIM3_CC2G	bit	TIM3_EGR,2	;  Capture/compare 2 generation
TIM3_CC1G	bit	TIM3_EGR,1	;  Capture/compare 1 generation
TIM3_UG		bit	TIM3_EGR,0	;  Update generation
TIM3_CCMR1	label	Base+DIER+$05	; TIM3 capture/compare mode register 1
TIM3_OC1M	bfield	TIM3_CCMR1,4,3	;  Output compare 1 mode
TIM3_OC1PE	bit	TIM3_CCMR1,3	;  Output compare 1 preload enable
TIM3_CC1S	bfield	TIM3_CCMR1,0,2	;  Capture/compare 1 selection
TIM3_IC1F	bfield	TIM3_CCMR1,4,4	;  Input capture 1 filter
TIM3_IC1PSC	bfield	TIM3_CCMR1,2,2	;  Input capture 1 prescaler
TIM3_CCMR2	label	Base+DIER+$06	; TIM3 capture/compare mode register 2
TIM3_OC2M	bfield	TIM3_CCMR2,4,3	;  Output compare 2 mode
TIM3_OC2PE	bit	TIM3_CCMR2,3	;  Output compare 2 preload enable
TIM3_CC2S	bfield	TIM3_CCMR2,0,2	;  Capture/compare 2 selection
TIM3_IC2F	bfield	TIM3_CCMR2,4,4	;  Input capture 2 filter
TIM3_IC2PSC	bfield	TIM3_CCMR2,2,2	;  Input capture 2 prescaler
TIM3_CCER1	label	Base+DIER+$07	; TIM3 capture/compare enable register 1
TIM3_CC2P	bit	TIM3_CCER1,5	;  Capture/compare 2 output polarity
TIM3_CC2E	bit	TIM3_CCER1,4	;  Capture/compare 2 output enable
TIM3_CC1P	bit	TIM3_CCER1,1	;  Capture/compare 1 output polarity
TIM3_CC1E	bit	TIM3_CCER1,0	;  Capture/Compare 1 output Enable
TIM3_CNTRH	label	Base+DIER+$08	; TIM3 counter high
TIM3_CNTRL	label	Base+DIER+$09	; TIM3 counter low
TIM3_PSCR	label	Base+DIER+$0a	; TIM3 prescaler register
TIM3_ARRH	label	Base+DIER+$0b	; TIM3 auto-reload register high
TIM3_ARRL	label	Base+DIER+$0c	; TIM3 auto-reload register low
TIM3_CCR1H	label	Base+DIER+$0d	; TIM3 capture/compare register 1 high
TIM3_CCR1L	label	Base+DIER+$0e	; TIM3 capture/compare register 1 low
TIM3_CCR2H	label	Base+DIER+$0f	; TIM3 capture/compare reg. 2 high
TIM3_CCR2L	label	Base+DIER+$10	; TIM3 capture/compare register 2 low
		endm

		restore
		endif			; __stm8stim3inc
