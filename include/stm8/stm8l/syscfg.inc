		ifndef	__stm8lsyscfginc	; avoid multiple inclusion
__stm8lsyscfginc equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File SYSCFG.INC                                              *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L System Configuration        *
;*                                                                          *
;****************************************************************************

__defsyscfg	macro	Base,Has3
		if	Has3
SYSCFG_RMPCR3	label	Base+$00		; Remapping control register 3
TIM2_CH2_REMAP	bit	SYSCFG_RMPCR3,7		;  TIM2 channel 2 remapping (20-pin package low-density devices only)
TIM2_CH1_REMAP	bit	SYSCFG_RMPCR3,6		;  TIM2 channel 1 remapping (20-pin package low-density devices only)
CCO_REMAP	bit	SYSCFG_RMPCR3,5		;  Configurable clock output remapping
TIM3_CH2_REMAP	bit	SYSCFG_RMPCR3,4		;  TIM3 channel 2 remapping
TIM3_CH1_REMAP	bit	SYSCFG_RMPCR3,3		;  TIM3 channel 1 remapping
USART3CK_REMAP	bit	SYSCFG_RMPCR3,2		;  USART3_CK remapping
USART3TR_REMAP	bit	SYSCFG_RMPCR3,1		;  USART3_TX and USART3_RX remapping
SPI1_REMAP2	bit	SYSCFG_RMPCR3,0		;  SPI1 remapping (80-pin packages only)
		endif				; Has3
SYSCFG_RMPCR1	label	Base+$01		; Remapping control register 1
SPI1_REMAP1	bit	SYSCFG_RMPCR1,7		;  SPI1 remapping
USART1CK_REMAP	bit	SYSCFG_RMPCR1,6		;  USART1_CK remapping
USART1TR_REMAP	bfield	SYSCFG_RMPCR1,4,2	;  USART1_TX and USART1_RX remapping
TIM4DMA_REMAP	bfield	SYSCFG_RMPCR1,2,2	;  TIM4 DMA channel remapping
ADC1DMA_REMAP	bfield	SYSCFG_RMPCR1,0,2	;  ADC1 DMA channel remapping
SYSCFG_RMPCR2	label	Base+$02		; Remapping control register 2
TIM23BKIN_REMAP	bit	SYSCFG_RMPCR2,7		;  TIM2 break input and TIM3 break input remapping
TIM3TRIG_REMAP2	bit	SYSCFG_RMPCR2,6		;  TIM3 trigger remapping
SPI2_REMAP	bit	SYSCFG_RMPCR2,5		;  SPI2 remapping
TIM3TRIGLSE_REMAP bit	SYSCFG_RMPCR2,4		;  TIM3 trigger controlled by LSE
TIM2TRIGLSE_REMAP bit	SYSCFG_RMPCR2,3		;  TIM2 trigger controlled by LSE
TIM3TRIG_REMAP1	bit	SYSCFG_RMPCR2,2		;  TIM3 trigger remapping
TIM2TRIG_REMAP	bit	SYSCFG_RMPCR2,1		;  TIM2 trigger remapping
ADC1TRIG_REMAP	bit	SYSCFG_RMPCR2,0		;  ADC1 trigger remapping
		endm

		restore
		endif			; __stm8lsyscfginc
