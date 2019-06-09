		ifndef	__stm8lclk01inc	; avoid multiple inclusion
__stm8lclk01inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File CLK01.INC                                               *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8Lx01 CLK Controller           *
;*                                                                          *
;****************************************************************************

__defclk01	macro	Base
CLK_CKDIVR	label	Base+$00	; Clock master divider register
HSIDIV		bfield	CLK_CKDIVR,0,2	;  High speed internal clock prescaler
CLK_PCKENR	label	Base+$03	; Peripheral clock gating register
PCKEN		bfield	CLK_PCKENR1,0,8	;  Peripheral clock enable
CLK_CCOR	label	Base+$05	; Configurable clock control register
CCOSEL		bfield	CLK_CCOR,1,2	;  Configurable clock output selection
CCOEN		bit	CLK_CCOR,0	;  Configurable clock output enable
		endm

		restore
		endif			; __stm8lclk01inc
