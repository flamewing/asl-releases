		ifndef	__regfoxinc	; avoid multiple inclusion
__regfoxinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGFOX.INC                                              *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST7FOXx[12]                       *
;*                                                                          *
;*   Source: ST7FOXF1/K1/K2 Data Sheet, Rev. 4, February 2008               *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

RAMSTART	label	$0080		; Start Address Internal RAM
RAMEND		label	$01ff		; End     "        "      "

RCCRH_USER	label	$1000		; RC Oscillator Calibration Values (user defined)
RCCRL_USER	label	$1001

RCCRH_DEF	label	$dee0		; RC Oscillator Calibration Values (predefined)
RCCRL_DEF	label	$dee1

;----------------------------------------------------------------------------
; GPIO

		include	"gpio.inc"
		__defgpio "PA",$0000
		__defgpio "PB",$0003
		__defgpio "PC",$0006

;----------------------------------------------------------------------------
; Lite Timer

LTCSR2		label	$000c		; Lite Timer Control/Status Register 2
TB2IE		bit	LTCSR2,1	;  Timebase 2 Interrupt Enable Bit
TB2F		bit	LTCSR2,0	;  Timebase 2 Interrupt Flag
LTARR		label	$000d		; Lite Timer Auto-reload Register
LTCNTR		label	$000e		; Lite Timer Counter Register
LTCSR1		label	$000f		; Lite Timer Control/Status Register 1
TBICIE		bit	LTCSR1,4	;  Interrupt Enable Bit
TBICF		bit	LTCSR1,3	;  Input Capture Flag
TB		bit	LTCSR1,2	;  Timebase Period Selection Bit
TB1IE		bit	LTCSR1,1	;  Timebase Interrupt Enable Bit
TB1F		bit	LTCSR1,0	;  Timebase Interrupt Flag
LTICR		label	$0010		; Lite Timer Input Capture Register

;----------------------------------------------------------------------------
; Auto-Reload Timer

ATCSR		label	$0011		; Timer Control/Status Register
ICF		bit	ATCSR,6		;  Input Capture flag
ICIE		bit	ATCSR,5		;  IC Interrupt Enable bit
CK		bfield	ATCSR,3,2	;  Counter Clock Selection
OVF1		bit	ATCSR,2		;  Overflow flag
OVFIE1		bit	ATCSR,1		;  Overflow Interrupt Enable bit
CMPIE		bit	ATCSR,0		;  Compare Interrupt Enable bit
CNTR1H		label	$0012		; Counter Register 1 High
CNTR1L		label	$0013		; Counter Register 1 Low
ATR1H		label	$0014		; Auto-Reload Register 1 High
ATR1L		label	$0015		; Auto-Reload Register 1 Low
PWMCR		label	$0016		; PWM Output Control Register
OE3		bit	PWMCR,6		;  PWM3 Output Enable
OE2		bit	PWMCR,4		;  PWM2 Output Enable
OE1		bit	PWMCR,2		;  PWM1 Output Enable
OE0		bit	PWMCR,0		;  PWM0 Output Enable
PWM0CSR		label	$0017		; PWM 0 Control/Status Register
OP0		bit	PWM0CSR,1	;  PWM0 Output Polarity Bit
CMPF0		bit	PWM0CSR,0	;  PWM0 Compare Flag
PWM1CSR		label	$0018		; PWM 1 Control/Status Register
OP1		bit	PWM1CSR,1	;  PWM1 Output Polarity Bit
CMPF1		bit	PWM1CSR,0	;  PWM1 Compare Flag
PWM2CSR		label	$0019		; PWM 2 Control/Status Register
OP2		bit	PWM2CSR,1	;  PWM2 Output Polarity Bit
CMPF2		bit	PWM2CSR,0	;  PWM2 Compare Flag
PWM3CSR		label	$001a		; PWM 3 Control/Status Register
OP_EN		bit	PWM3CSR,3	;  One Pulse Mode Enable Bit
OPEDGE		bit	PWM3CSR,2	;  One Pulse Edge Selection Bit
OP3		bit	PWM3CSR,1	;  PWM3 Output Polarity Bit
CMPF3		bit	PWM3CSR,0	;  PWM3 Compare Flag
DCR0H		label	$001b		; PWM 0 Duty Cycle Register High
DCR0L		label	$001c		; PWM 0 Duty Cycle Register Low
DCR1H		label	$001d		; PWM 1 Duty Cycle Register High
DCR1L		label	$001e		; PWM 1 Duty Cycle Register Low
DCR2H		label	$001f		; PWM 2 Duty Cycle Register High
DCR2L		label	$0020		; PWM 2 Duty Cycle Register Low
DCR3H		label	$0021		; PWM 3 Duty Cycle Register High
DCR3L		label	$0022		; PWM 3 Duty Cycle Register Low
ATICRH		label	$0023		; Input Capture Register High
ATICRL		label	$0024		; Input Capture Register Low
ATCSR2		label	$0025		; Timer Control/Status Register 2
FORCE2		bit	ATCSR2,7	;  Force Counter 2 Overflow Bit
FORCE1		bit	ATCSR2,6	;  Force Counter 1 Overflow Bit
ICS		bit	ATCSR2,5	;  Input Capture Shorted Bit
OVFIE2		bit	ATCSR2,4	;  Overflow Interrupt 2 Enable Bit
OVF2		bit	ATCSR2,3	;  Overflow Flag
ENCNTR2		bit	ATCSR2,2	;  Enable Counter 2 For PWM2/3
TRAN2		bit	ATCSR2,1	;  Transfer Enable 2 Bit
TRAN1		bit	ATCSR2,0	;  Transfer Enable 1 Bit
BREAKCR1	label	$0026		; Break Control Register
PWM		bfield	BREAKCR1,0,4	;  Break Pattern bits
ATR2H		label	$0027		; Auto-Reload Register 2 High
ATR2L		label	$0028		; Auto-Reload Register 2 Low
DTGR		label	$0029		; Dead Time Generation Register
BREAKEN		label	$002a		; Break Enable Register

;----------------------------------------------------------------------------
; ITC

ISPR0		label	$002d		; Interrupt Software Priority Register 0
I0_0		bit	ISPR0,0
I1_0		bit	ISPR0,1
I0_1		bit	ISPR0,2
I1_1		bit	ISPR0,3
I0_2		bit	ISPR0,4
I1_2		bit	ISPR0,5
I0_3		bit	ISPR0,6
I1_3		bit	ISPR0,7
ISPR1		label	$002e		; Interrupt Software Priority Register 1
I0_4		bit	ISPR1,0
I1_4		bit	ISPR1,1
I0_5		bit	ISPR1,2
I1_5		bit	ISPR1,3
I0_6		bit	ISPR1,4
I1_6		bit	ISPR1,5
I0_7		bit	ISPR1,6
I1_7		bit	ISPR1,7
ISPR2		label	$002f		; Interrupt Software Priority Register 2
I0_8		bit	ISPR2,0
I1_8		bit	ISPR2,1
I0_9		bit	ISPR2,2
I1_9		bit	ISPR2,3
I0_10		bit	ISPR2,4
I1_10		bit	ISPR2,5
I0_11		bit	ISPR2,6
I1_11		bit	ISPR2,7
ISPR3		label	$0030		; Interrupt Software Priority Register 3
I0_12		bit	ISPR3,0
I1_12		bit	ISPR3,1
EICR		label	$0031		; External Interrupt Control Register
IS2		bfield	EICR,4,2	;  ei2 Sensitivity
IS1		bfield	EICR,2,2	;  ei1 Sensitivity
IS0		bfield	EICR,0,2	;  ei0 Sensitivity

;----------------------------------------------------------------------------
; Watchdog

WDGCR		label	$0033		; Watchdog Control Register
WDGA		bit	WDGCR,7		; Activation bit

;----------------------------------------------------------------------------
; Flash

FCSR		label	$0034		; Flash Control/Status Register

;----------------------------------------------------------------------------
; RC Calibration

RCC_CSR		label	$0035		; RC Calibration Control/Status Register
RCCLAT		bit	RCC_CSR,1	;  Latch Access Transfer Bit
RCCPGM		bit	RCC_CSR,0	;  Programming Control and Status Bit

;----------------------------------------------------------------------------
; Analog/Digital Converter

		include	"adc10.inc"
		__defadc10 $0036

;----------------------------------------------------------------------------
; MCC

MCCSR		label	$003a		; Main Clock Control / Status Register
MCO		bit	MCCSR,1		;  Main Clock Out Enable Bit
SMS		bit	MCCSR,0		;  Slow Mode Selection Bit

;----------------------------------------------------------------------------
; Clock and Reset

RCCRH		label	$003b		; RC Oscillator Control Register High
RCCRL		label	$003c		; RC Oscillator Control Register Low
PSCR		label	$003d		; Prescaler Register

;----------------------------------------------------------------------------
; Auto Wakeup Unit

AWUCSR		label	$0048		; AWU Control/Status Register
AWUF		bit	AWUCSR,2	;  Auto Wakeup Flag
AWUM		bit	AWUCSR,1	;  Auto Wakeup Measurement Bit
AWUEN		bit	AWUCSR,0	;  Auto Wakeup From Halt Enabled Bit
AWUPR		label	$0049		; AWU Preload Register

;----------------------------------------------------------------------------
; Debug Module

		include	"dm.inc"
		__defdm	$004a
DMCR2		label	$0050		; DM Control Register 2

;----------------------------------------------------------------------------
; Clock Controller

CKCNTCSR	label	$0051		; Clock Controller Status Register
AWU_FLAG	bit	CKCNTCSR,3	;  AWU Selection Bit
RC_FLAG		bit	CKCNTCSR,2	;  RC Selection Bit
RC_AWU		bit	CKCNTCSR,0	;  RC/AWU Selection Bit

;----------------------------------------------------------------------------
; I2C

		include	"i2c.inc"
		__defi2c $0064

		restore
		endif			; __regfoxinc
