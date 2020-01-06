		ifndef	__reglit0xinc	; avoid multiple inclusion
__reglit0xinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGLIT0X.INC                                            *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST7LITE0x                         *
;*                                                                          *
;*   Source: ST7LITE0xY0 Data Sheet, Rev. 6, November 2007                  *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

RAMSTART	label	$0080		; Start Address Internal RAM
RAMEND		label	$00ff		; End     "        "      "

		if	SUBSTR(MOMCPUNAME,7,1)="S"
ROMSTART	 equ	 $fa00
		elseif
ROMSTART	 equ	 $fc00
		endif

		switch	SUBSTR(MOMCPUNAME,STRLEN(MOMCPUNAME)-3,1)
		case	"2"
__hasadc	 equ	0
		case	"5"
__hasadc	 equ	1
		case	"9"
__hasadc	 equ	1
EESTART		 label	$1000
EEEND		 label	$107f
		elsecase
		 fatal	"Huh?"
		endcase

RCCR0		label	$1000		; RC Oscillator Calibration Values
RCCR1		label	$1001

;----------------------------------------------------------------------------
; Interrupt Vectors

SPI_vect	label	$ffe2		; SPI Interrupt
LT_RTC1vect	label	$ffe4		; Lite Timer RTC1 Interrupt
LT_IC_vect	label	$ffe6		; Lite Timer Input Capture Interrupt
AT_OF_vect	label	$ffe8		; AT Timer Overflow 1 Interrupt
AT_OC_vect	label	$ffea		; AT Timer Output Compare Interrupt
SI_vect		label	$ffec		; AVD Interrupt
EI3_vect	label	$fff2		; External Interrupt Vector EI3
EI2_vect	label	$fff4		; External Interrupt Vector EI2
EI1_vect	label	$fff6		; External Interrupt Vector EI1
EI0_vect	label	$fff8		; External Interrupt Vector EI0
TRAP_vect	label	$fffc		; TRAP (software) Interrupt Vector
RESET_vect	label	$fffe		; RESET Vector

;----------------------------------------------------------------------------
; GPIO

		include	"gpio.inc"
		__defgpio "PA",$0000
		__defgpio "PB",$0003

;----------------------------------------------------------------------------
; Lite Timer

LTCSR		label	$000b		; Lite Timer Control/Status Register 2
ICIE		bit	LTCSR,7		;  Input Capture Interrupt Enable
ICF		bit	LTCSR,6		;  Input Capture Flag
TB		bit	LTCSR,5		;  Timebase Period Selection
TBIE		bit	LTCSR,4		;  Timebase Interrupt Enable Bit
TBF		bit	LTCSR,3		;  Timebase Interrupt Flag
WDGR		bit	LTCSR,2		;  Force Reset/ Reset Status Flag
WDGE		bit	LTCSR,1		;  Watchdog Enable
WDGD		bit	LTCSR,0		;  Watchdog Reset Delay
LTICR		label	$000c		; Lite Timer Input Capture Register

;----------------------------------------------------------------------------
; Auto-Reload Timer

ATCSR		label	$000d		; Timer Control/Status Register
CK		bfield	ATCSR,3,2	;  Counter Clock Selection
OVF		bit	ATCSR,2		;  Overflow Flag
OVFIE		bit	ATCSR,1		;  Overflow Interrupt Enable Bit
CMPIE		bit	ATCSR,0		;  Compare Interrupt Enable Bit
CNTRH		label	$000e		; Counter Register High
CNTRL		label	$000f		; Counter Register Low
ATRH		label	$0010		; Auto-Reload Register 1 High
ATRL		label	$0011		; Auto-Reload Register 1 Low
PWMCR		label	$0012		; PWM Output Control Register
OE0		bit	PWMCR,0		;  PWM0 Output Enable
PWM0CSR		label	$0013		; PWM 0 Control/Status Register
OP0		bit	PWM0CSR,1	;  PWM0 Output Polarity Bit
CMPF0		bit	PWM0CSR,0	;  PWM0 Compare Flag
DCR0H		label	$0017		; PWM 0 Duty Cycle Register High
DCR0L		label	$0018		; PWM 0 Duty Cycle Register Low

;----------------------------------------------------------------------------
; Flash

FCSR		label	$002f		; Flash Control/Status Register
OPT		bit	FCSR,2
LAT		bit	FCSR,1
PGM		bit	FCSR,0

;----------------------------------------------------------------------------
; EEPROM

		ifdef	EESTART
EECSR		label	$0030		; Data EEPROM Control/Status Register
E2LAT		bit	EECSR,1		;  Latch Access Transfer
E2PGM		bit	EECSR,0		;  Programming Control and Status
		endif

;----------------------------------------------------------------------------
; SPI

		include	"spi2.inc"
		__defspi $0031

;----------------------------------------------------------------------------
; Analog/Digital Converter

		if	__hasadc
ADCCSR		label	$0034		; A/D Control Status Register
EOC		bit	ADCCSR,7	;  Conversion Complete
SPEED		bit	ADCCSR,6	;  ADC Clock Selection
ADON		bit	ADCCSR,5	;  A/D Converter and Amplifier On
CH		bfield	ADCCSR,0,3	;  Channel Selection
ADCDR		label	$0035		; A/D Data Register
ADCAMP		label	$0036		; A/D Amplifier Control Register
SLOW		bit	ADCAMP,3	;  Slow Mode
AMPSEL		bit	ADCAMP,2	;  Amplifier Selection Bit
		endif

;----------------------------------------------------------------------------
; ITC

EICR		label	$0037		; External Interrupt Control Register
IS3		bfield	EICR,6,2	;  ei3 Sensitivity
IS2		bfield	EICR,4,2	;  ei2 Sensitivity
IS1		bfield	EICR,2,2	;  ei1 Sensitivity
IS0		bfield	EICR,0,2	;  ei0 Sensitivity

;----------------------------------------------------------------------------
; CLOCKS

MCCSR		label	$0038		; Main Clock Control / Status Register
MCO		bit	MCCSR,1		;  Main Clock Out Enable Bit
SMS		bit	MCCSR,0		;  Slow Mode Selection Bit

;----------------------------------------------------------------------------
; RC Calibration

RCCR		label	$0039		; RC Calibration Control/Status Register
CR		bfield	RCCR,0,8	;  RC Oscillator Frequency Adjustment Bits

;----------------------------------------------------------------------------
; SI

SICSR		label	$003a		; System Integrity Control/Status Register
LOCKED		bit	SICSR,3		;  PLL Locked Flag
LVDRF		bit	SICSR,2		;  LVD Reset Flag
AVDF		bit	SICSR,1		;  Voltage Detector Flag
AVDIE		bit	SICSR,0		;  Voltage Detector Interrupt Enable

		restore
		endif			; __reglit0xinc
