		ifndef	__stm8sclkinc	; avoid multiple inclusion
__stm8sclkinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File CLK.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S CLK Controller              *
;*                                                                          *
;****************************************************************************

__defclk	macro	Base
CLK_ICKR	label	Base+$00	; Internal clock control register
REGAH		bit	CLK_ICKR,5	;  Regulator power off in Active-halt mode
LSIRDY		bit	CLK_ICKR,4	;  Low speed internal oscillator ready
LSIEN		bit	CLK_ICKR,3	;  Low speed internal RC oscillator enable
FHW		bit	CLK_ICKR,2	;  Fast wakeup from Halt/Active-halt modes
HSIRDY		bit	CLK_ICKR,1	;  High speed internal oscillator ready
HSIEN		bit	CLK_ICKR,0	;  High speed internal RC oscillator enable
CLK_ECKR	label	Base+$01	; External clock control register
HSERDY		bit	CLK_ECKR,1	;  High speed external crystal oscillator ready
HSEEN		bit	CLK_ECKR,0	;  High speed external crystal oscillator enable
CLK_CMSR	label	Base+$03	; Clock master status register
CKM		bfield	CLK_CMSR,0,8	;  Clock master status bits
CLK_SWR		label	Base+$04	; Clock master switch register
SWI		bfield	CLK_SWR,0,8	;  Clock master selection bits
CLK_SWCR	label	Base+$05	; Clock switch control register
SWIF		bit	CLK_SWCR,3	;  Clock switch interrupt flag
SWIEN		bit	CLK_SWCR,2	;  Clock switch interrupt enable
SWEN		bit	CLK_SWCR,1	;  Switch start/stop
SWBSY		bit	CLK_SWCR,0	;  Switch busy
CLK_CKDIVR	label	Base+$06	; Clock divider register
HSIDIV		bfield	CLK_CKDIVR,3,2	;  High speed internal clock prescaler
CPUDIV		bfield	CLK_CKDIVR,0,3	;  CPU clock prescaler
CLK_PCKENR1	label	Base+$07	; Peripheral clock gating register 1
PCKEN		bfield	CLK_PCKENR1,0,8	;  Peripheral clock enable
CLK_CSSR	label	Base+$08	; Clock security system register
CSSD		bit	CLK_CSSR,3	;  Clock security system detection
CSSDIE		bit	CLK_CSSR,2	;  Clock security system detection interrupt enable
AUX		bit	CLK_CSSR,1	;  Auxiliary oscillator connected to master clock
CSSEN		bit	CLK_CSSR,0	;  Clock security system enable
CLK_CCOR	label	Base+$09	; Configurable clock control register
CCOBSY		bit	CLK_CCOR,6	;  Configurable clock output busy
CCORDY		bit	CLK_CCOR,5	;  Configurable clock output ready
CCOSEL		bfield	CLK_CCOR,1,4	;  Configurable clock output selection
CCOEN		bit	CLK_CCOR,0	;  Configurable clock output enable
CLK_PCKENR2	label	Base+$0a	; Peripheral clock gating register 2
PCKEN2		bfield	CLK_PCKENR2,0,8	;  Peripheral clock enable
CLK_HSITRIMR	label	Base+$0c	; HSI clock calibration trimming register
HSITRIM		bfield	CLK_HSITRIMR,0,4;  HSI trimming value
CLK_SWIMCCR	label	Base+$0d	; SWIM clock control register
CLK_SWIMCLK	bit	CLK_SWIMCCR,0	;  SWIM clock divider
		endm

		restore
		endif			; __stm8sclkinc
