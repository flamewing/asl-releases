		ifndef	__reglit4xinc	; avoid multiple inclusion
__reglit4xinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGLIT4X.INC                                            *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST7LITE4x                         *
;*                                                                          *
;*   Source: ST7LITE49K2 Data Sheet, Rev. 4, February 2009                  *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

RAMSTART	label	$0080		; Start Address Internal RAM
RAMEND		label	$01ff		; End     "        "      "

EESTART		label	$1000		; Start Address Internal EEPROM
EEEND		label	$10ff		; End     "        "      "

RCCRH0		label	$dee0		; RC Oscillator Calibration Values
RCCRL0		label	$dee1
RCCRH1		label	$dee2
RCCRL1		label	$dee3

;----------------------------------------------------------------------------
; Interrupt Vectors

LT_RTC1_vect	label	$ffe0		; Lite Timer RTC1 Interrupt, shared with...
LT_RTC2_vect	label	$ffe0		; LITE TIMER RTC2 interrupt, shared with...
LT_IC_vect	label	$ffe0		; Lite Timer Input Capture Interrupt
TIM16_vect	label	$ffe2		; 16-bit Timer Peripheral Interrupt
SPI_vect	label	$ffe4		; SPI Interrupt
I2C_vect	label	$ffe6		; I2C Interrupt
AT_OF2_vect	label	$ffe8		; AT Timer Overflow 2 Interrupt
AT_OF1_vect	label	$ffea		; AT Timer Overflow 1 Interrupt
AT_OC_vect	label	$ffec		; AT Timer Output Compare Interrupt, shared with...
AT_IC_vect	label	$ffec		; AT Timer Input Capture Interrupt
EI2_vect	label	$ffee		; External Interrupt Vector EI2
EI1_vect	label	$fff0		; External Interrupt Vector EI1
EI0_vect	label	$fff2		; External Interrupt Vector EI0
COMPB_vect	label	$fff4		; Comparator B Interrupt
COMPA_vect	label	$fff6		; Comparator A Interrupt
SI_vect		label	$fff8		; AVD Interrupt
AWU_vect	label	$fffa		; Auto Wakeup Interrupt Vector
TRAP_vect	label	$fffc		; TRAP (software) Interrupt Vector
RESET_vect	label	$fffe		; RESET Vector

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
TBICIE		bit	LTCSR1,7	;  Interrupt Enable Bit
TBICF		bit	LTCSR1,6	;  Input Capture Flag
TB		bit	LTCSR1,5	;  Timebase Period Selection Bit
TB1IE		bit	LTCSR1,4	;  Timebase 1 Interrupt Enable Bit
TB1F		bit	LTCSR1,3	;  Timebase 1 Interrupt Flag
LTICR		label	$0010		; Lite Timer Input Capture Register

;----------------------------------------------------------------------------
; Auto-Reload Timer

ATCSR		label	$0011		; Timer Control/Status Register
ICF		bit	ATCSR,6		;  Input Capture flag
ICIE		bit	ATCSR,5		;  IC Interrupt Enable bit
AT_CK		bfield	ATCSR,3,2	;  Counter Clock Selection
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
ENCNTR2		bit	ATCSR2,2	;  Enable Counter 2 for PWM2/3
TRAN2		bit	ATCSR2,1	;  Transfer Enable 2 bit
TRAN1		bit	ATCSR2,0	;  Transfer Enable 1 bit
BREAKCR		label	$0026		; Break Control Register
BR1SEL		bit	BREAKCR,7	;  Break 1 Input Selection Bit
BR1EDGE		bit	BREAKCR,6	;  Break 1 input Edge Selection Bit
BA1		bit	BREAKCR,5	;  Break 1 Active Bit
BP1EN		bit	BREAKCR,4	;  Break 1 Pin Enable Bit
PWM3		bit	BREAKCR,3	;  Break Pattern Bits
PWM2		bit	BREAKCR,2
PWM1		bit	BREAKCR,1
PWM0		bit	BREAKCR,0
ATR2H		label	$0027		; Auto-Reload Register 2 High
ATR2L		label	$0028		; Auto-Reload Register 2 Low
DTGR		label	$0029		; Dead Time Generator Register
BREAKEN		label	$002a		; Break Enable Register
BREN2		bit	BREAKEN,1	;  Break Enable for Counter 2 Bit
BREN1		bit	BREAKEN,0	;  Break Enable for Counter 1 Bit
BREAKCR2	label	$002c		; Break Control Register 2
BR2SEL		bit	BREAKCR2,7	;  Break 2 Input Selection Bit
BR2EDGE		bit	BREAKCR2,6	;  Break 2 Input Edge Selection Bit
BA2		bit	BREAKCR2,5	;  Break 2 Active Bit
BP2EN		bit	BREAKCR2,4	;  Break 2 Pin Enable Bit
SWBR2		bit	BREAKCR2,1	;  Switch Break for Counter 2 Bit
SWBR1		bit	BREAKCR2,0	;  Switch Break for Counter 1 Bit

;----------------------------------------------------------------------------
; ITC

ISPR0		label	$002d		; Interrupt Software Priority Register 0
I0_0		bit	ISPR0,0		; Vector $fffa
I1_0		bit	ISPR0,1
I0_1		bit	ISPR0,2		; Vector $fff8
I1_1		bit	ISPR0,3
I0_2		bit	ISPR0,4		; Vector $fff6
I1_2		bit	ISPR0,5
I0_3		bit	ISPR0,6		; Vector $fff4
I1_3		bit	ISPR0,7
ISPR1		label	$002e		; Interrupt Software Priority Register 1
I0_4		bit	ISPR1,0		; Vector $fff2
I1_4		bit	ISPR1,1
I0_5		bit	ISPR1,2		; Vector $fff0
I1_5		bit	ISPR1,3
I0_6		bit	ISPR1,4		; Vector $ffee
I1_6		bit	ISPR1,5
I0_7		bit	ISPR1,6		; Vector $ffec
I1_7		bit	ISPR1,7
ISPR2		label	$002f		; Interrupt Software Priority Register 2
I0_8		bit	ISPR2,0		; Vector $ffea
I1_8		bit	ISPR2,1
I0_9		bit	ISPR2,2		; Vector $ffe8
I1_9		bit	ISPR2,3
I0_10		bit	ISPR2,4		; Vector $ffe6
I1_10		bit	ISPR2,5
I0_11		bit	ISPR2,6		; Vector $ffe4
I1_11		bit	ISPR2,7
ISPR3		label	$0030		; Interrupt Software Priority Register 3
I0_12		bit	ISPR3,0		; Vector $ffe2
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
OPT		bit	FCSR,2
LAT		bit	FCSR,1
PGM		bit	FCSR,0

;----------------------------------------------------------------------------
; EEPROM

EECSR		label	$0035		; Data EEPROM Control/Status Register
E2LAT		bit	EECSR,1		;  Latch Access Transfer
E2PGM		bit	EECSR,0		;  Programming control and status

;----------------------------------------------------------------------------
; Analog/Digital Converter

		include	"adc10.inc"
		__defadc10 $0036
AMPCAL		bit	ADCDRL,4	;  Amplifier Calibration Bit
SLOW		bit	ADCDRL,3	;  Slow Mode
AMPSEL		bit	ADCDRL,2	;  Amplifier Selection Bit

;----------------------------------------------------------------------------
; MCC

MCCSR		label	$003a		; Main Clock Control / Status Register
MCO		bit	MCCSR,1		;  Main Clock Out Enable Bit
SMS		bit	MCCSR,0		;  Slow Mode Selection Bit

;----------------------------------------------------------------------------
; RC Calibration

RCCR		label	$003b		; RC Calibration Control/Status Register (bit 2..9)
SICSR		label	$003c		; System Integrity Control/Status Register
CR		bfield	SICSR,5,2	;  RC Calibration (bit 0..1)
WDGRF		bit	SICSR,4		;  Watchdog Reset Flag
LVDRF		bit	SICSR,2		;  LVD Reset Flag
AVDF		bit	SICSR,1		;  Voltage Detector Flag
AVDIE		bit	SICSR,0		;  Voltage Detector Interrupt Enable
AVDTHCR		label	$003d		; AVD Threshold Selection Register / RC Prescaler
RC_CK		bfield	AVDTHCR,5,3	;  Internal RC Prescaler Selection
AVD		bfield	AVDTHCR,0,2	;  AVD Threshold Selection

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
; Comparator Voltage Reference

VREFCR		label	$0052		; Internal Voltage Reference Control Register
VCEXT		bit	VREFCR,7	;  External Voltage Reference for Comparators
VCBGR		bit	VREFCR,6	;  Bandgap Voltage for Comparators
VR		bfield	VREFCR,2,4	;  Programmable Internal Voltage Reference Range Selection
VCEXTB		bit	VREFCR,1	;  External Voltage Reference for Comparator B

;----------------------------------------------------------------------------
; Comparator

CMPACR		label	$0053		; Comparator A and Internal Reference Control Register
CAHYST		bit	CMPACR,7	;  Comparator A Hysteresis Enable
CAINV		bit	CMPACR,5	;  Comparator A Output Inversion Select
CMPAIF		bit	CMPACR,4	;  Comparator A Interrupt Flag
CMPAIE		bit	CMPACR,3	;  Comparator A Interrupt Enable
CMPA		bit	CMPACR,2	;  Comparator A Output
CAOUT		bit	CMPACR,1	;  Comparator A Output Enable on Port
CMPAON		bit	CMPACR,0	;  Comparator A ON/OFF
CMPBCR		label	$0054		; Comparator B and Internal Reference Control Register
CBHYST		bit	CMPBCR,7	;  Comparator B Hysteresis Enable
CBINV		bit	CMPBCR,5	;  Comparator B Output Inversion Select
CMPBIF		bit	CMPBCR,4	;  Comparator B Interrupt Flag
CMPBIE		bit	CMPBCR,3	;  Comparator B Interrupt Enable
CMPB		bit	CMPBCR,2	;  Comparator B Output
CBOUT		bit	CMPBCR,1	;  Comparator B Output Enable on Port
CMPBON		bit	CMPBCR,0	;  Comparator B ON/OFF

;----------------------------------------------------------------------------
; 16-bit Timer

		include	"timer.inc"
		__deftimer "TA",$0054
TACSR		label	TASR
TIMD		bit	TACSR,2		;  Timer Disable

;----------------------------------------------------------------------------
; I2C

		include	"i2c.inc"
		__defi2c $0064

;----------------------------------------------------------------------------
; SPI

		include	"spi2.inc"
		__defspi $0070

		restore
		endif			; __reglit4xinc
