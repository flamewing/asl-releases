		ifndef	__stm8lclkinc	; avoid multiple inclusion
__stm8lclkinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File CLK.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L CLK Controller              *
;*                                                                          *
;****************************************************************************

__defclk	macro	Base
CLK_CKDIVR	label	Base+$00	; Clock master divider register
CKM		bfield	CLK_CKDIVR,0,3	;  System clock prescaler
CLK_CRTCR	label	Base+$01	; Clock RTC register
RTCDIV		bfield	CLK_CRTCR,5,3	;  Clock RTC prescaler
RTCSEL		bfield	CLK_CRTCR,1,4	;  Configurable RTC clock source selection
RTCSWBSY	bit	CLK_CRTCR,0	;  The system is busy during a RTC clock change
CLK_ICKR	label	Base+$02	; Internal clock control register
BEEPAHALT	bit	CLK_ICKR,6	;  BEEP clock Halt/Active-halt mode
FHWU		bit	CLK_ICKR,5	;  Fast wakeup from Halt/Active-halt modes
SAHALT		bit	CLK_ICKR,4	;  Slow HALT/Active-halt mode
LSIRDY		bit	CLK_ICKR,3	;  Low speed internal oscillator ready
LSION		bit	CLK_ICKR,2	;  Low speed internal RC oscillator enable
HSIRDY		bit	CLK_ICKR,1	;  High speed internal oscillator ready
HSION		bit	CLK_ICKR,0	;  High speed internal RC oscillator enable
CLK_PCKENR1	label	Base+$03	; Peripheral clock gating register 1
PCKEN		bfield	CLK_PCKENR1,0,8	;  Peripheral clock enable
CLK_PCKENR2	label	Base+$04	; Peripheral clock gating register 2
PCKEN2		bfield	CLK_PCKENR2,0,6	;  Peripheral clock enable
PCKEN27		bit	CLK_PCKENR2,7	;  Peripheral clock enable
CLK_CCOR	label	Base+$05	; Configurable clock control register
CCODIV		bfield	CLK_CCOR,5,3	;  Configurable clock output prescaler
CCOSEL		bfield	CLK_CCOR,1,4	;  Configurable clock output selection
CCOSWBSY	bit	CLK_CCOR,0	;  Configurable clock output switch busy
CLK_ECKCR	label	Base+$06	; External clock register
LSEBYP		bit	CLK_ECKR,5	;  Low speed external clock bypass
HSEBYP		bit	CLK_ECKR,4	;  High-speed external clock bypass
LSERDY		bit	CLK_ECKR,3	;  Low speed external crystal oscillator ready
LSEON		bit	CLK_ECKR,2	;  Low speed external crystal oscillator enable
HSERDY		bit	CLK_ECKR,1	;  High speed external crystal oscillator ready
HSEON		bit	CLK_ECKR,0	;  High speed external crystal oscillator enable
CLK_SCSR	labrl	Base+$07	; System clock status register
CKM		bfield	CLK_SCSR,0,8	;  System clock status bits
CLK_SWR		label	Base+$08	; Clock master switch register
SWI		bfield	CLK_SWR,0,8	;  Clock master selection bits
CLK_SWCR	label	Base+$09	; Clock switch control register
SWIF		bit	CLK_SWCR,3	;  Clock switch interrupt flag
SWIEN		bit	CLK_SWCR,2	;  Clock switch interrupt enable
SWEN		bit	CLK_SWCR,1	;  Switch start/stop
SWBSY		bit	CLK_SWCR,0	;  Switch busy
CLK_CSSR	label	Base+$0a	; Clock security system register
CSSDGON		bit	CLK_CSSR,4	;  CSS deglitcher system
CSSD		bit	CLK_CSSR,3	;  Clock security system detection
CSSDIE		bit	CLK_CSSR,2	;  Clock security system detection interrupt enable
AUX		bit	CLK_CSSR,1	;  Auxiliary oscillator connected to master clock
CSSEN		bit	CLK_CSSR,0	;  Clock security system enable
CLK_CBEEPR	label	Base+$0b	; Clock BEEP register
CLKBEEPSEL	bfield	CLK_CBEEPR,1,2	;  Configurable BEEP clock source selection
BEEPSWBSY	bit	CLK_CBEEPR,0	;  System busy during BEEP clock change
CLK_HSICALR	label	Base+$0c	; HSI calibration register
HSICAL		bfield	CLK_HSICALR,0,8	;  HSI calibration
CLK_HSITRIMR	label	Base+$0d	; HSI clock calibration trimming register
HSITRIM		bfield	CLK_HSITRIMR,0,8;  HSI trimming value
CLK_HSIUNLCKR	label	Base+$0e	; HSI unlock register
HSIUNLCK	bfield	CLK_HSIUNLCKR,0,8; HSI unlock mechanism
CLK_REGCSR	label	Base+$0f	; Main regulator control status register
EEREADY		bit	CLK_REGCSR,7	;  Flash program memory and Data EEPROM ready
EEBUSY		bit	CLK_REGCSR,6	;  Flash program memory and Data EEPROM busy
LSEPD		bit	CLK_REGCSR,5	;  LSE power-down
HSEPD		bit	CLK_REGCSR,4	;  HSE power-down
LSIPD		bit	CLK_REGCSR,3	;  LSI power-down
HSIPD		bit	CLK_REGCSR,2	;  HSI power-down
REGOFF		bit	CLK_REGCSR,1	;  Main regulator OFF
REGREADY	bit	CLK_REGCSR,0	;  Main regulator ready
CLK_PCKENR3	label	Base+$10	; Peripheral clock gating register 3
PCKEN3		bfield	CLK_PCKENR3,0,6	;  Peripheral clock enable
		endm

		restore
		endif			; __stm8lclkinc
