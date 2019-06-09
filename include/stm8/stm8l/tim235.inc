		ifndef	__stm8ltim2inc	; avoid multiple inclusion
__stm8ltim2inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File TIM235.INC                                              *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L Timer 2/3/5                 *
;*                                                                          *
;****************************************************************************

__deftim235	macro	NAME,Base,HasDMA
__NS		set	"\{NAME}_"
{__NS}CR1	label	Base+$00	; TIMx control register 1
{__NS}ARPE	bit	{__NS}CR1,7	;  Auto-reload preload enable
{__NS}CMS	bfield	{__NS}CR1,5,2	;  Center-aligned mode selection
{__NS}DIR	bit	{__NS}CR1,4	;  Direction
{__NS}OPM	bit	{__NS}CR1,3	;  One-pulse mode
{__NS}URS	bit	{__NS}CR1,2	;  Update request source
{__NS}UDIS	bit	{__NS}CR1,1	;  Update disable
{__NS}CEN	bit	{__NS}CR1,0	;  Counter enable
{__NS}CR2	label	Base+$01	; TIMx control register 2
{__NS}MMS	bfield	{__NS}CR2,4,3	;  Master mode selection
		if	HasDMA
{__NS}TI1S	 bit	{__NS}CR2,7	;  TI1 (digital filter input) selection
{__NS}CCDS	 bit	{__NS}CR2,3	;  Capture/compare DMA selection
		endif
{__NS}SMCR	label	Base+$02	; TIMx Slave mode control register
{__NS}MSM	bit	{__NS}SMCR,7	;  Master/slave mode
{__NS}TS	bfield	{__NS}SMCR,4,3	;  Trigger selection
{__NS}SMS	bfield	{__NS}SMCR,0,3	;  Clock/trigger/slave mode selection
{__NS}ETR	label	Base+$03	; TIMx external trigger register
{__NS}ETP	bit	{__NS}ETR,7	;  External trigger polarity
{__NS}ECE	bit	{__NS}ETR,6	;  External clock enable
{__NS}ETPS	bfield	{__NS}ETR,4,2	;  External trigger prescaler
{__NS}ETF	bfield	{__NS}ETR,0,4	;  External trigger filter
		if	HasDMA
{__NS}DER	 label	Base+$04	; TIMx DMA1 request enable register
{__NS}CC2DE	 bit	{__NS}DER,2	;  Capture/compare 2 DMA request enable
{__NS}CC1DE	 bit	{__NS}DER,1	;  Capture/compare 1 DMA request enable
{__NS}UDE	 bit	{__NS}DER,0	;  Update DMA request enable
		endif
{__NS}IER	label	Base+$04+HasDMA	; TIMx interrupt enable register
{__NS}BIE	bit	{__NS}IER,7	;  Break interrupt enable
{__NS}TIE	bit	{__NS}IER,6	;  Trigger interrupt enable
{__NS}CC2IE	bit	{__NS}IER,2	;  Capture/compare 2 interrupt enable
{__NS}CC1IE	bit	{__NS}IER,1	;  Capture/compare 1 interrupt enable
{__NS}UIE	bit	{__NS}IER,0	;  Update interrupt enable
{__NS}SR1	label	Base+$05+HasDMA	; TIMx status register 1
{__NS}BIF	bit	{__NS}SR1,7	;  Break interrupt flag
{__NS}TIF	bit	{__NS}SR1,6	;  Trigger interrupt flag
{__NS}CC2IF	bit	{__NS}SR1,2	;  Capture/compare 2 interrupt flag
{__NS}CC1IF	bit	{__NS}SR1,1	;  Capture/compare 1 interrupt flag
{__NS}UIF	bit	{__NS}SR1,0	;  Update interrupt flag
{__NS}SR2	label	Base+$06+HasDMA	; TIMx status register 2
{__NS}CC2OF	bit	{__NS}SR2,2	;  Capture/compare 2 overcapture flag
{__NS}CC1OF	bit	{__NS}SR2,1	;  Capture/compare 1 overcapture flag
{__NS}EGR	label	Base+$07+HasDMA	; TIMx event generation register
{__NS}BG	bit	{__NS}EGR,7	;  Break generation
{__NS}TG	bit	{__NS}EGR,6	;  Trigger generation
{__NS}CC2G	bit	{__NS}EGR,2	;  Capture/compare 2 generation
{__NS}CC1G	bit	{__NS}EGR,1	;  Capture/compare 1 generation
{__NS}UG	bit	{__NS}EGR,0	;  Update generation
{__NS}CCMR1	label	Base+$08+HasDMA	; TIMx capture/compare mode register 1
{__NS}OC1M	bfield	{__NS}CCMR1,4,3	;  Output compare 1 mode
{__NS}OC1PE	bit	{__NS}CCMR1,3	;  Output compare 1 preload enable
{__NS}OC1FE	bit	{__NS}CCMR1,2	;  Output compare 1 fast enable
{__NS}CC1S	bfield	{__NS}CCMR1,0,2	;  Capture/compare 1 selection
{__NS}IC1F	bfield	{__NS}CCMR1,4,4	;  Input capture 1 filter
{__NS}IC1PSC	bfield	{__NS}CCMR1,2,2	;  Input capture 1 prescaler
{__NS}CCMR2	label	Base+$09+HasDMA	; TIMx capture/compare mode register 2
{__NS}OC2M	bfield	{__NS}CCMR2,4,3	;  Output compare 2 mode
{__NS}OC2PE	bit	{__NS}CCMR2,3	;  Output compare 2 preload enable
{__NS}OC2FE	bit	{__NS}CCMR2,2	;  Output compare 2 fast enable
{__NS}CC2S	bfield	{__NS}CCMR2,0,2	;  Capture/compare 2 selection
{__NS}IC2F	bfield	{__NS}CCMR2,4,4	;  Input capture 2 filter
{__NS}IC2PSC	bfield	{__NS}CCMR2,2,2	;  Input capture 2 prescaler
{__NS}CCER1	label	Base+$0a+HasDMA	; TIMx capture/compare enable register 1
{__NS}CC2P	bit	{__NS}CCER1,5	;  Capture/compare 2 output polarity
{__NS}CC2E	bit	{__NS}CCER1,4	;  Capture/compare 2 output enable
{__NS}CC1P	bit	{__NS}CCER1,1	;  Capture/compare 1 output polarity
{__NS}CC1E	bit	{__NS}CCER1,0	;  Capture/Compare 1 output Enable
{__NS}CNTRH	label	Base+$0b+HasDMA	; TIMx counter high
{__NS}CNTRL	label	Base+$0c+HasDMA	; TIMx counter low
{__NS}PSCR	label	Base+$0d+HasDMA	; TIMx prescaler register
{__NS}ARRH	label	Base+$0e+HasDMA	; TIMx auto-reload register high
{__NS}ARRL	label	Base+$0f+HasDMA	; TIMx auto-reload register low
{__NS}CCR1H	label	Base+$10+HasDMA	; TIMx capture/compare register 1 high
{__NS}CCR1L	label	Base+$11+HasDMA	; TIMx capture/compare register 1 low
{__NS}CCR2H	label	Base+$12+HasDMA	; TIMx capture/compare reg. 2 high
{__NS}CCR2L	label	Base+$13+HasDMA	; TIMx capture/compare register 2 low
{__NS}BKR	label	Base+$14+HasDMA	; TIMx break register
{__NS}MOE	bit	{__NS}BKR,7	;  Main output enable
{__NS}AOE	bit	{__NS}BKR,6	;  Automatic output enable
{__NS}BKP	bit	{__NS}BKR,5	;  Break polarity
{__NS}BKE	bit	{__NS}BKR,4	;  Break enable
{__NS}OSSI	bit	{__NS}BKR,2	;  Off state selection for idle mode
{__NS}LOCK	bfield	{__NS}BKR,0,2	;  Lock configuration
{__NS}OISR	label	Base+$15+HasDMA	; TIMx output idle state register
{__NS}OIS2	bit	{__NS}OISR,2	;  Output idle state 2
{__NS}OIS1	bit	{__NS}OISR,0	;  Output idle state 1
		endm

		restore
		endif			; __stm8ltim2inc
