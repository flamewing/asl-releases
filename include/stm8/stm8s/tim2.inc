		ifndef	__stm8stim2inc	; avoid multiple inclusion
__stm8stim2inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File TIM2.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S Timer 2                     *
;*                                                                          *
;****************************************************************************

__deftim2	macro	Base,DIER
TIM2_CR1	label	Base+$00	; TIM2 control register 1
TIM2_ARPE	bit	TIM2_CR1,7	;  Auto-reload preload enable
TIM2_OPM	bit	TIM2_CR1,3	;  One-pulse mode
TIM2_URS	bit	TIM2_CR1,2	;  Update request source
TIM2_UDIS	bit	TIM2_CR1,1	;  Update disable
TIM2_CEN	bit	TIM2_CR1,0	;  Counter enable
TIM2_IER	label	Base+DIER+$01	; TIM2 interrupt enable register
TIM2_CC3IE	bit	TIM2_IER,3	;  Capture/compare 3 interrupt enable
TIM2_CC2IE	bit	TIM2_IER,2	;  Capture/compare 2 interrupt enable
TIM2_CC1IE	bit	TIM2_IER,1	;  Capture/compare 1 interrupt enable
TIM2_UIE	bit	TIM2_IER,0	;  Update interrupt enable
TIM2_SR1	label	Base+DIER+$02	; TIM2 status register 1
TIM2_CC3IF	bit	TIM2_SR1,3	;  Capture/compare 3 interrupt flag
TIM2_CC2IF	bit	TIM2_SR1,2	;  Capture/compare 2 interrupt flag
TIM2_CC1IF	bit	TIM2_SR1,1	;  Capture/compare 1 interrupt flag
TIM2_UIF	bit	TIM2_SR1,0	;  Update interrupt flag
TIM2_SR2	label	Base+DIER+$03	; TIM2 status register 2
TIM2_CC3OF	bit	TIM2_SR2,3	;  Capture/compare 3 overcapture flag
TIM2_CC2OF	bit	TIM2_SR2,2	;  Capture/compare 2 overcapture flag
TIM2_CC1OF	bit	TIM2_SR2,1	;  Capture/compare 1 overcapture flag
TIM2_EGR	label	Base+DIER+$04	; TIM2 event generation register
TIM2_CC3G	bit	TIM2_EGR,3	;  Capture/compare 3 generation
TIM2_CC2G	bit	TIM2_EGR,2	;  Capture/compare 2 generation
TIM2_CC1G	bit	TIM2_EGR,1	;  Capture/compare 1 generation
TIM2_UG		bit	TIM2_EGR,0	;  Update generation
TIM2_CCMR1	label	Base+DIER+$05	; TIM2 capture/compare mode register 1
TIM2_OC1M	bfield	TIM2_CCMR1,4,3	;  Output compare 1 mode
TIM2_OC1PE	bit	TIM2_CCMR1,3	;  Output compare 1 preload enable
TIM2_CC1S	bfield	TIM2_CCMR1,0,2	;  Capture/compare 1 selection
TIM2_IC1F	bfield	TIM2_CCMR1,4,4	;  Input capture 1 filter
TIM2_IC1PSC	bfield	TIM2_CCMR1,2,2	;  Input capture 1 prescaler
TIM2_CCMR2	label	Base+DIER+$06	; TIM2 capture/compare mode register 2
TIM2_OC2M	bfield	TIM2_CCMR2,4,3	;  Output compare 2 mode
TIM2_OC2PE	bit	TIM2_CCMR2,3	;  Output compare 2 preload enable
TIM2_CC2S	bfield	TIM2_CCMR2,0,2	;  Capture/compare 2 selection
TIM2_IC2F	bfield	TIM2_CCMR2,4,4	;  Input capture 2 filter
TIM2_IC2PSC	bfield	TIM2_CCMR2,2,2	;  Input capture 2 prescaler
TIM2_CCMR3	label	Base+DIER+$07	; TIM2 capture/compare mode register 3
TIM2_OC3M	bfield	TIM2_CCMR3,4,3	;  Output compare 3 mode
TIM2_OC3PE	bit	TIM2_CCMR3,3	;  Output compare 3 preload enable
TIM2_CC3S	bfield	TIM2_CCMR3,0,2	;  Capture/compare 3 selection
TIM2_IC3F	bfield	TIM2_CCMR3,4,4	;  Input capture 3 filter
TIM2_IC3PSC	bfield	TIM2_CCMR3,2,2	;  Input capture 3 prescaler
TIM2_CCER1	label	Base+DIER+$08	; TIM2 capture/compare enable register 1
TIM2_CC2P	bit	TIM2_CCER1,5	;  Capture/compare 2 output polarity
TIM2_CC2E	bit	TIM2_CCER1,4	;  Capture/compare 2 output enable
TIM2_CC1P	bit	TIM2_CCER1,1	;  Capture/compare 1 output polarity
TIM2_CC1E	bit	TIM2_CCER1,0	;  Capture/Compare 1 output Enable
TIM2_CCER2	label	Base+DIER+$09	; TIM2 capture/compare enable register 2
TIM2_CC3P	bit	TIM2_CCER2,1	;  Capture/compare 3 output polarity
TIM2_CC3E	bit	TIM2_CCER2,0	;  Capture/compare 3 output enable
TIM2_CNTRH	label	Base+DIER+$0a	; TIM2 counter high
TIM2_CNTRL	label	Base+DIER+$0b	; TIM2 counter low
TIM2_PSCR	label	Base+DIER+$0c	; TIM2 prescaler register
TIM2_ARRH	label	Base+DIER+$0d	; TIM2 auto-reload register high
TIM2_ARRL	label	Base+DIER+$0e	; TIM2 auto-reload register low
TIM2_CCR1H	label	Base+DIER+$0f	; TIM2 capture/compare register 1 high
TIM2_CCR1L	label	Base+DIER+$10	; TIM2 capture/compare register 1 low
TIM2_CCR2H	label	Base+DIER+$11	; TIM2 capture/compare reg. 2 high
TIM2_CCR2L	label	Base+DIER+$12	; TIM2 capture/compare register 2 low
TIM2_CCR3H	label	Base+DIER+$13	; TIM2 capture/compare register 3 high
TIM2_CCR3L	label	Base+DIER+$14	; TIM2 capture/compare register 3 low
		endm

		restore
		endif			; __stm8stim2inc
