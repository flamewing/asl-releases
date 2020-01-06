		ifndef	__stm8stim1inc	; avoid multiple inclusion
__stm8stim1inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File TIM1.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S Timer 1                     *
;*                                                                          *
;****************************************************************************

__deftim1	macro	Base
TIM1_CR1	label	Base+$00	; TIM1 control register 1
TIM1_ARPE	bit	TIM1_CR1,7	;  Auto-reload preload enable
TIM1_CMS	bfield	TIM1_CR1,5,2	;  Center-aligned mode selection
TIM1_DIR	bit	TIM1_CR1,4	;  Direction
TIM1_OPM	bit	TIM1_CR1,3	;  One-pulse mode
TIM1_URS	bit	TIM1_CR1,2	;  Update request source
TIM1_UDIS	bit	TIM1_CR1,1	;  Update disable
TIM1_CEN	bit	TIM1_CR1,0	;  Counter enable
TIM1_CR2	label	Base+$01	; TIM1 control register 2
TIM1_MMS	bfield	TIM1_CR2,4,3	;  Master mode selection
TIM1_COMS	bit	TIM1_CR2,2	;  Capture/compare control update selection
TIM1_CCPC	bit	TIM1_CR2,0	;  Capture/compare preloaded control
TIM1_SMCR	label	Base+$02	; TIM1 slave mode control register
TIM1_MSM	bit	TIM1_SMCR,7	;  Master/slave mode
TIM1_TS		bfield	TIM1_SMCR,4,3	;  Trigger selection
TIM1_SMS	bfield	TIM1_SMCR,0,3	;  Clock/trigger/slave mode selection
TIM1_ETR	label	Base+$03	; TIM1 external trigger register
TIM1_ETP	bit	TIM1_ETR,7	;  External trigger polarity
TIM1_ECE	bit	TIM1_ETR,6	;  External clock enable
TIM1_ETPS	bfield	TIM1_ETR,4,2	;  External trigger prescaler
TIM1_ETF	bfield	TIM1_ETR,0,4	;  External trigger filter
TIM1_IER	label	Base+$04	; TIM1 Interrupt enable register
TIM1_BIE	bit	TIM1_IER,7	;  Break interrupt enable
TIM1_TIE	bit	TIM1_IER,6	;  Trigger interrupt enable
TIM1_COMIE	bit	TIM1_IER,5	;  Commutation interrupt enable
TIM1_CC4IE	bit	TIM1_IER,4	;  Capture/compare 4 interrupt enable
TIM1_CC3IE	bit	TIM1_IER,3	;  Capture/compare 3 interrupt enable
TIM1_CC2IE	bit	TIM1_IER,2	;  Capture/compare 2 interrupt enable
TIM1_CC1IE	bit	TIM1_IER,1	;  Capture/compare 1 interrupt enable
TIM1_UIE	bit	TIM1_IER,0	;  Update interrupt enable
TIM1_SR1	label	Base+$05	; TIM1 status register 1
TIM1_BIF	bit	TIM1_SR1,7	;  Break interrupt flag
TIM1_TIF	bit	TIM1_SR1,6	;  Trigger interrupt flag
TIM1_COMIF	bit	TIM1_SR1,5	;  Commutation interrupt flag
TIM1_CC4IF	bit	TIM1_SR1,4	;  Capture/compare 4 interrupt flag
TIM1_CC3IF	bit	TIM1_SR1,3	;  Capture/compare 3 interrupt flag
TIM1_CC2IF	bit	TIM1_SR1,2	;  Capture/compare 2 interrupt flag
TIM1_CC1IF	bit	TIM1_SR1,1	;  Capture/compare 1 interrupt flag
TIM1_UIF	bit	TIM1_SR1,0	;  Update interrupt flag
TIM1_SR2	label	Base+$06	; TIM1 status register 2
TIM1_CC4OF	bit	TIM1_SR2,4	;  Capture/compare 4 overcapture flag
TIM1_CC3OF	bit	TIM1_SR2,3	;  Capture/compare 3 overcapture flag
TIM1_CC2OF	bit	TIM1_SR2,2	;  Capture/compare 2 overcapture flag
TIM1_CC1OF	bit	TIM1_SR2,1	;  Capture/compare 1 overcapture flag
TIM1_EGR	label	Base+$07	; TIM1 event generation register
TIM1_BG		bit	TIM1_EGR,7	;  Break generation
TIM1_TG		bit	TIM1_EGR,6	;  Trigger generation
TIM1_COMG	bit	TIM1_EGR,5	;  Capture/compare control update generation
TIM1_CC4G	bit	TIM1_EGR,4	;  Capture/compare 4 generation
TIM1_CC3G	bit	TIM1_EGR,3	;  Capture/compare 3 generation
TIM1_CC2G	bit	TIM1_EGR,2	;  Capture/compare 2 generation
TIM1_CC1G	bit	TIM1_EGR,1	;  Capture/compare 1 generation
TIM1_UG		bit	TIM1_EGR,0	;  Update generation
TIM1_CCMR1	label	Base+$08	; TIM1 capture/compare mode register 1
TIM1_OC1CE	bit	TIM1_CCMR1,7	;  Output compare 1 clear enable
TIM1_OC1M	bfield	TIM1_CCMR1,4,3	;  Output compare 1 mode
TIM1_OC1PE	bit	TIM1_CCMR1,3	;  Output compare 1 preload enable
TIM1_OC1FE	bit	TIM1_CCMR1,2	;  Output compare 1 fast enable
TIM1_CC1S	bfield	TIM1_CCMR1,0,2	;  Capture/compare 1 selection
TIM1_IC1F	bfield	TIM1_CCMR1,4,4	;  Input capture 1 filter
TIM1_IC1PSC	bfield	TIM1_CCMR1,2,2	;  Input capture 1 prescaler
TIM1_CCMR2	label	Base+$09	; TIM1 capture/compare mode register 2
TIM1_OC2CE	bit	TIM1_CCMR2,7	;  Output compare 2 clear enable
TIM1_OC2M	bfield	TIM1_CCMR2,4,3	;  Output compare 2 mode
TIM1_OC2PE	bit	TIM1_CCMR2,3	;  Output compare 2 preload enable
TIM1_OC2FE	bit	TIM1_CCMR2,2	;  Output compare 2 fast enable
TIM1_CC2S	bfield	TIM1_CCMR2,0,2	;  Capture/compare 2 selection
TIM1_IC2F	bfield	TIM1_CCMR2,4,4	;  Input capture 2 filter
TIM1_IC2PSC	bfield	TIM1_CCMR2,2,2	;  Input capture 2 prescaler
TIM1_CCMR3	label	Base+$0a	; TIM1 capture/compare mode register 3
TIM1_OC3CE	bit	TIM1_CCMR3,7	;  Output compare 3 clear enable
TIM1_OC3M	bfield	TIM1_CCMR3,4,3	;  Output compare 3 mode
TIM1_OC3PE	bit	TIM1_CCMR3,3	;  Output compare 3 preload enable
TIM1_OC3FE	bit	TIM1_CCMR3,2	;  Output compare 3 fast enable
TIM1_CC3S	bfield	TIM1_CCMR3,0,2	;  Capture/compare 3 selection
TIM1_IC3F	bfield	TIM1_CCMR3,4,4	;  Input capture 3 filter
TIM1_IC3PSC	bfield	TIM1_CCMR3,2,2	;  Input capture 3 prescaler
TIM1_CCMR4	label	Base+$0b	; TIM1 capture/compare mode register 4
TIM1_OC4CE	bit	TIM1_CCMR4,7	;  Output compare 4 clear enable
TIM1_OC4M	bfield	TIM1_CCMR4,4,3	;  Output compare 4 mode
TIM1_OC4PE	bit	TIM1_CCMR4,3	;  Output compare 4 preload enable
TIM1_OC4FE	bit	TIM1_CCMR4,2	;  Output compare 4 fast enable
TIM1_CC4S	bfield	TIM1_CCMR4,0,2	;  Capture/compare 4 selection
TIM1_IC4F	bfield	TIM1_CCMR4,4,4	;  Input capture 4 filter
TIM1_IC4PSC	bfield	TIM1_CCMR4,2,2	;  Input capture 4 prescaler
TIM1_CCER1	label	Base+$0c	; TIM1 capture/compare enable register 1
TIM1_CC2NP	bit	TIM1_CCER1,7	;  Capture/compare 2 complementary output polarity
TIM1_CC2NE	bit	TIM1_CCER1,6	;  Capture/compare 2 complementary output enable
TIM1_CC2P	bit	TIM1_CCER1,5	;  Capture/compare 2 output polarity
TIM1_CC2E	bit	TIM1_CCER1,4	;  Capture/compare 2 output enable
TIM1_CC1NP	bit	TIM1_CCER1,3	;  Capture/compare 1 complementary output polarity
TIM1_CC1NE	bit	TIM1_CCER1,2	;  Capture/compare 1 complementary output enable
TIM1_CC1P	bit	TIM1_CCER1,1	;  Capture/compare 1 output polarity
TIM1_CC1E	bit	TIM1_CCER1,0	;  Capture/compare 1 output enable
TIM1_CCER2	label	Base+$0d	; TIM1 capture/compare enable register 2
TIM1_CC4P	bit	TIM1_CCER2,5	;  Capture/compare 4 output polarity
TIM1_CC4E	bit	TIM1_CCER2,4	;  Capture/compare 4 output enable
TIM1_CC3NP	bit	TIM1_CCER2,3	;  Capture/compare 3 complementary output polarity
TIM1_CC3NE	bit	TIM1_CCER2,2	;  Capture/compare 3 complementary output enable
TIM1_CC3P	bit	TIM1_CCER2,1	;  Capture/compare 3 output polarity
TIM1_CC3E	bit	TIM1_CCER2,0	;  Capture/compare 3 output enable
TIM1_CNTRH	label	Base+$0e	; TIM1 counter high
TIM1_CNTRL	label	Base+$0f	; TIM1 counter low
TIM1_PSCRH	label	Base+$10	; TIM1 prescaler register high
TIM1_PSCRL	label	Base+$11	; TIM1 prescaler register low
TIM1_ARRH	label	Base+$12	; TIM1 auto-reload register high
TIM1_ARRL	label	Base+$13	; TIM1 auto-reload register low
TIM1_RCR	label	Base+$14	; TIM1 repetition counter register
TIM1_CCR1H	label	Base+$15	; TIM1 capture/compare register 1 high
TIM1_CCR1L	label	Base+$16	; TIM1 capture/compare register 1 low
TIM1_CCR2H	label	Base+$17	; TIM1 capture/compare register 2 high
TIM1_CCR2L	label	Base+$18	; TIM1 capture/compare register 2 low
TIM1_CCR3H	label	Base+$19	; TIM1 capture/compare register 3 high
TIM1_CCR3L	label	Base+$1a	; TIM1 capture/compare register 3 low
TIM1_CCR4H	label	Base+$1b	; TIM1 capture/compare register 4 high
TIM1_CCR4L	label	Base+$1c	; TIM1 capture/compare register 4 low
TIM1_BKR	label	Base+$1d	; TIM1 break register
TIM1_MOE	bit	TIM1_BKR,7	;  Main output enable
TIM1_AOE	bit	TIM1_BKR,6	;  Automatic output enable
TIM1_BKP	bit	TIM1_BKR,5	;  Break polarity
TIM1_BKE	bit	TIM1_BKR,4	;  Break enable
TIM1_OSSR	bit	TIM1_BKR,3	;  Off state selection for Run mode
TIM1_OSSI	bit	TIM1_BKR,2	;  Off state selection for idle mode
TIM1_LOCK	bfield	TIM1_BKR,0,2	;  Lock configuration
TIM1_DTR	label	Base+$1e	; TIM1 dead-time register
TIM1_OISR	label	Base+$1f	; TIM1 output idle state register
TIM1_OIS4	bit	TIM1_OISR,6	;  Output idle state 4 (OC4 output)
TIM1_OIS3N	bit	TIM1_OISR,5	;  Output idle state 3 (OC3N output)
TIM1_OIS3	bit	TIM1_OISR,4	;  Output idle state 3 (OC3 output)
TIM1_OIS2N	bit	TIM1_OISR,3	;  Output idle state 2 (OC2N output)
TIM1_OIS2	bit	TIM1_OISR,2	;  Output idle state 2 (OC2 output)
TIM1_OIS1N	bit	TIM1_OISR,1	;  Output idle state 1 (OC1N output)
TIM1_OIS1	bit	TIM1_OISR,0	;  Output idle state 1 (OC1 output)
		endm

		restore
		endif			; __stm8stim1inc
