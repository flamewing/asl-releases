		ifndef	__stm8lwfeinc	; avoid multiple inclusion
__stm8lwfeinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File WFE.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L Wait for Events             *
;*                                                                          *
;****************************************************************************

__defwfe	macro	Base,Has4
WFE_CR1		label	Base+$00	; WFE control register 1
EXTI_EV3	bit	WFE_CR1,7	;  External interrupt event 3
EXTI_EV2	bit	WFE_CR1,6	;  External interrupt event 2
EXTI_EV1	bit	WFE_CR1,5	;  External interrupt event 1
EXTI_EV0	bit	WFE_CR1,4	;  External interrupt event 0
TIM1_EV1	bit	WFE_CR1,3	;  TIM1 interrupt event 1
TIM1_EV0	bit	WFE_CR1,2	;  TIM1 interrupt event 0
TIM2_EV1	bit	WFE_CR1,1	;  TIM2 interrupt event 1
TIM2_EV0	bit	WFE_CR1,0	;  TIM2 interrupt event 0
WFE_CR2		label	Base+$01	; WFE control register 2
ADC1_COMP_EV	bit	WFE_CR2,7	;  ADC1 and comparators used to wake up the CPU from WFE mode
EXTI_EVE_F	bit	WFE_CR2,6	;  External interrupt event on Port E or Port F
EXTI_EVD_H	bit	WFE_CR2,5	;  External interrupt event on Port D or Port H
EXTI_EVB_G	bit	WFE_CR2,4	;  External interrupt event on Port B or Port G
EXTI_EV7	bit	WFE_CR2,3	;  External interrupt event 7
EXTI_EV6	bit	WFE_CR2,2	;  External interrupt event 6
EXTI_EV5	bit	WFE_CR2,1	;  External interrupt event 5
EXTI_EV4	bit	WFE_CR2,0	;  External interrupt event 4
WFE_CR3		label	Base+$02	; WFE control register 3
DMA1CH23_EV	bit	WFE_CR3,7	;  DMA1 channel 2 and 3 interrupt event
DMA1CH01_EV	bit	WFE_CR3,6	;  DMA1 channel 0 and 1 interrupt event
USART1_EV	bit	WFE_CR3,5	;  USART1 Rx and Tx interrupt event
I2C1_EV		bit	WFE_CR3,4	;  I2C1 Rx and Tx interrupt event
SPI1_EV		bit	WFE_CR3,3	;  SPI1Rx and Tx interrupt event
TIM4_EV		bit	WFE_CR3,2	;  TIM4 interrupt event
TIM3_EV1	bit	WFE_CR3,1	;  TIM3 interrupt event 1
TIM3_EV0	bit	WFE_CR3,0	;  TIM3 interrupt event 0
		if	Has4
WFE_CR4		label	Base+$03	; WFE control register 4
AES_EV		bit	WFE_CR4,6	;  AES interrupt event
TIM5_EV1	bit	WFE_CR4,5	;  TIM5 interrupt event 1
TIM5_EV0	bit	WFE_CR4,4	;  TIM5 interrupt event 0
USART3_EV	bit	WFE_CR4,3	;  USART3 Rx and Tx interrupt event
USART2_EV	bit	WFE_CR4,2	;  USART2 Rx and Tx interrupt event
SPI2_EV		bit	WFE_CR4,1	;  SPI2 Rx and Tx interrupt event
RTC_CSSLSE_EV	bit	WFE_CR4,0	;  "RTC" or "CSS on LSE" interrupt event
		endif			; Has4
		endm
		
		restore
		endif			; __stm8lwfeinc
