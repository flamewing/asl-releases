		ifndef	__stm8tclkinc	; avoid multiple inclusion
__stm8tclkinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File CLK.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8T CLK Controller              *
;*                                                                          *
;*   Source: RM0312 (DocID022352 Rev 3)                                     *
;*                                                                          *
;****************************************************************************

__defclk	macro	Base
CLK_CKDIVR	label	Base+$00	; Clock master divider register
HSIDIV		bfield	CLK_CKDIVR,0,2	;  High speed internal clock prescaler
CLK_PCKENR1	label	Base+$03	; Peripheral clock gating register 1
PCKENR17	bit	CLK_PCKENR1,7	;  ProxSense
PCKENR16	bit	CLK_PCKENR1,6	;  AWU
PCKENR15	bit	CLK_PCKENR1,5	;  USART
PCKENR14	bit	CLK_PCKENR1,4	;  SPI
PCKENR13	bit	CLK_PCKENR1,3	;  I2C
PCKENR12	bit	CLK_PCKENR1,2	;  TIM4
PCKENR11	bit	CLK_PCKENR1,1	;  TIM3
PCKENR10	bit	CLK_PCKENR1,0	;  TIM2
CLK_PCKENR2	label	Base+$04	; Peripheral clock gating register 2
PCKENR20	bit	CLK_PCKENR2,0	;  WWDG
CLK_CCOR	label	Base+$05	; Configurable clock control register
CCPSLP		bfield	CLK_CCOR,4,2	;  Configurable clock output buffer slope
CCOSEL		bfield	CLK_CCOR,1,3	;  Configurable clock output selection
CCOEN		bit	CLK_CCOR,0	;  Configurable clock output enable
		endm

		restore
		endif			; __stm8lclkinc
