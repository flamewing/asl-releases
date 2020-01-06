		ifndef	__stm8stim5inc	; avoid multiple inclusion
__stm8stim5inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File TIM5.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S Timer 5                     *
;*                                                                          *
;****************************************************************************

                ; TIM5 is identical to TIM2, with the addition
                ; of CR2 and SMCR that fill the gap existing on some
                ; implementations of TIM2.

__deftim5	macro	Base
TIM5_CR1	label	Base+$00	; TIM5 control register 1
TIM5_ARPE	bit	TIM5_CR1,7	;  Auto-reload preload enable
TIM5_OPM	bit	TIM5_CR1,3	;  One-pulse mode
TIM5_URS	bit	TIM5_CR1,2	;  Update request source
TIM5_UDIS	bit	TIM5_CR1,1	;  Update disable
TIM5_CEN	bit	TIM5_CR1,0	;  Counter enable
TIM5_CR2	label	Base+$01	; TIM5 control register 2
TIM5_MMS	bfield	TIM5_CR2,4,3	;  Master mode selection
TIM5_SMCR	label	Base+$02	; TIM5 Slave mode control register
TIM5_MSM	bit	TIM5_SMCR,7	;  Master/slave mode
TIM5_TS		bfield	TIM5_SMCR,4,3	;  Trigger selection
TIM5_SMS	bfield	TIM5_SMCR,0,3	;  Clock/trigger/slave mode selection
TIM5_IER	label	Base+$03	; TIM5 interrupt enable register
TIM5_CC3IE	bit	TIM5_IER,3	;  Capture/compare 3 interrupt enable
TIM5_CC2IE	bit	TIM5_IER,2	;  Capture/compare 2 interrupt enable
TIM5_CC1IE	bit	TIM5_IER,1	;  Capture/compare 1 interrupt enable
TIM5_UIE	bit	TIM5_IER,0	;  Update interrupt enable
TIM5_SR1	label	Base+$04	; TIM5 status register 1
TIM5_CC3IF	bit	TIM5_SR1,3	;  Capture/compare 3 interrupt flag
TIM5_CC2IF	bit	TIM5_SR1,2	;  Capture/compare 2 interrupt flag
TIM5_CC1IF	bit	TIM5_SR1,1	;  Capture/compare 1 interrupt flag
TIM5_UIF	bit	TIM5_SR1,0	;  Update interrupt flag
TIM5_SR2	label	Base+$05	; TIM5 status register 2
TIM5_CC3OF	bit	TIM5_SR2,3	;  Capture/compare 3 overcapture flag
TIM5_CC2OF	bit	TIM5_SR2,2	;  Capture/compare 2 overcapture flag
TIM5_CC1OF	bit	TIM5_SR2,1	;  Capture/compare 1 overcapture flag
TIM5_EGR	label	Base+$06	; TIM5 event generation register
TIM5_CC3G	bit	TIM5_EGR,3	;  Capture/compare 3 generation
TIM5_CC2G	bit	TIM5_EGR,2	;  Capture/compare 2 generation
TIM5_CC1G	bit	TIM5_EGR,1	;  Capture/compare 1 generation
TIM5_UG		bit	TIM5_EGR,0	;  Update generation
TIM5_CCMR1	label	Base+$07	; TIM5 capture/compare mode register 1
TIM5_OC1M	bfield	TIM5_CCMR1,4,3	;  Output compare 1 mode
TIM5_OC1PE	bit	TIM5_CCMR1,3	;  Output compare 1 preload enable
TIM5_CC1S	bfield	TIM5_CCMR1,0,2	;  Capture/compare 1 selection
TIM5_IC1F	bfield	TIM5_CCMR1,4,4	;  Input capture 1 filter
TIM5_IC1PSC	bfield	TIM5_CCMR1,2,2	;  Input capture 1 prescaler
TIM5_CCMR2	label	Base+$08	; TIM5 capture/compare mode register 2
TIM5_OC2M	bfield	TIM5_CCMR2,4,3	;  Output compare 2 mode
TIM5_OC2PE	bit	TIM5_CCMR2,3	;  Output compare 2 preload enable
TIM5_CC2S	bfield	TIM5_CCMR2,0,2	;  Capture/compare 2 selection
TIM5_IC2F	bfield	TIM5_CCMR2,4,4	;  Input capture 2 filter
TIM5_IC2PSC	bfield	TIM5_CCMR2,2,2	;  Input capture 2 prescaler
TIM5_CCMR3	label	Base+$09	; TIM5 capture/compare mode register 3
TIM5_OC3M	bfield	TIM5_CCMR3,4,3	;  Output compare 3 mode
TIM5_OC3PE	bit	TIM5_CCMR3,3	;  Output compare 3 preload enable
TIM5_CC3S	bfield	TIM5_CCMR3,0,2	;  Capture/compare 3 selection
TIM5_IC3F	bfield	TIM5_CCMR3,4,4	;  Input capture 3 filter
TIM5_IC3PSC	bfield	TIM5_CCMR3,2,2	;  Input capture 3 prescaler
TIM5_CCER1	label	Base+$0a	; TIM5 capture/compare enable register 1
TIM5_CC2P	bit	TIM5_CCER1,5	;  Capture/compare 2 output polarity
TIM5_CC2E	bit	TIM5_CCER1,4	;  Capture/compare 2 output enable
TIM5_CC1P	bit	TIM5_CCER1,1	;  Capture/compare 1 output polarity
TIM5_CC1E	bit	TIM5_CCER1,0	;  Capture/Compare 1 output Enable
TIM5_CCER2	label	Base+$0b	; TIM5 capture/compare enable register 2
TIM5_CC3P	bit	TIM5_CCER2,1	;  Capture/compare 3 output polarity
TIM5_CC3E	bit	TIM5_CCER2,0	;  Capture/compare 3 output enable
TIM5_CNTRH	label	Base+$0c	; TIM5 counter high
TIM5_CNTRL	label	Base+$0d	; TIM5 counter low
TIM5_PSCR	label	Base+$0e	; TIM5 prescaler register
TIM5_ARRH	label	Base+$0f	; TIM5 auto-reload register high
TIM5_ARRL	label	Base+$10	; TIM5 auto-reload register low
TIM5_CCR1H	label	Base+$11	; TIM5 capture/compare register 1 high
TIM5_CCR1L	label	Base+$12	; TIM5 capture/compare register 1 low
TIM5_CCR2H	label	Base+$13	; TIM5 capture/compare reg. 2 high
TIM5_CCR2L	label	Base+$14	; TIM5 capture/compare register 2 low
TIM5_CCR3H	label	Base+$15	; TIM5 capture/compare register 3 high
TIM5_CCR3L	label	Base+$16	; TIM5 capture/compare register 3 low
		endm

		restore
		endif			; __stm8stim5inc
