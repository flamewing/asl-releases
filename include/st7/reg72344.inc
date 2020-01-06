		ifndef	__reg72344inc	; avoid multiple inclusion
__reg72344inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG72344.INC                                            *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST72344                           *
;*                                                                          *
;*   Source: ST72344/45 Data Sheet, Rev. 6, July 2012                       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

RAMSTART	label	$0080		; start address internal RAM
		switch	substr(MOMCPUNAME,8,1)
		case	"2"
RAMEND		label	$027f		; end     "        "      "
		case	"4"
RAMEND		label	$047f		; end     "        "      "
		endcase
		
EESTART		label	$c00		; start address internal EEPROM
EEEND		label	$cff		; end     "        "      "
		
;----------------------------------------------------------------------------
; Interrupt Vectors

I2C_vect	label	$ffe0		; I2C Peripheral interrupts
AVD_vect	label	$ffe2		; Auxiliary Voltage detector interrupt
SCI_vect	label	$ffe4		; SCI Interrupt Vector
TIMB_vect	label	$ffe6		; TIMER B Interrupt Vector
TIMA_vect	label	$ffe8		; TIMER A Interrupt Vector
SPI_vect	label	$ffea		; SPI Interrupt Vector
EI3_vect	label	$fff0		; External Interrupt Vector B7..4
EI2_vect	label	$fff2		; External Interrupt Vector B3..0
EI1_vect	label	$fff4		; External Interrupt Vector F2..0
EI0_vect	label	$fff6		; External Interrupt Vector A3..0
MCC_RTC_vect	label	$fff8		; Main clock controller time base interrupt
AWU_vect	label	$fffa		; Auto wakeup interrupt
TRAP_vect	label	$fffc		; TRAP (software) Interrupt Vector
RESET_vect	label	$fffe		; RESET Vector

;----------------------------------------------------------------------------
; GPIO

		include	"gpio.inc"
		__defgpio "PA",$0000
		__defgpio "PB",$0003
		__defgpio "PC",$0006
		__defgpio "PD",$0009
		__defgpio "PE",$000c
		__defgpio "PF",$000f

;----------------------------------------------------------------------------
; RC Oscillator

RCCRH		label	$0017		; RC Oscillator Control Register High
RCCRL		label	$0018		; RC Oscillator Control Register Low

;----------------------------------------------------------------------------
; EEPROM

EECSR		label	$0020		; EEPROM Control/Status Register
E2LAT		bit	EECSR,1		;  Latch Access Transfer
E2PGM		bit	EECSR,0		;  Programming control and status

;----------------------------------------------------------------------------
; SPI

		include	"spi2.inc"
		__defspi $0021

;----------------------------------------------------------------------------
; ITC

ISPR0		label	$0024		; Interrupt Software Priority Register 0
I0_0		bit	ISPR0,0
I1_0		bit	ISPR0,1
I0_1		bit	ISPR0,2
I1_1		bit	ISPR0,3
I0_2		bit	ISPR0,4
I1_2		bit	ISPR0,5
I0_3		bit	ISPR0,6
I1_3		bit	ISPR0,7
ISPR1		label	$0025		; Interrupt Software Priority Register 1
I0_4		bit	ISPR1,0
I1_4		bit	ISPR1,1
I0_5		bit	ISPR1,2
I1_5		bit	ISPR1,3
I0_6		bit	ISPR1,4
I1_6		bit	ISPR1,5
I0_7		bit	ISPR1,6
I1_7		bit	ISPR1,7
ISPR2		label	$0026		; Interrupt Software Priority Register 2
I0_8		bit	ISPR2,0
I1_8		bit	ISPR2,1
I0_9		bit	ISPR2,2
I1_9		bit	ISPR2,3
I0_10		bit	ISPR2,4
I1_10		bit	ISPR2,5
I0_11		bit	ISPR2,6
I1_11		bit	ISPR2,7
ISPR3		label	$0027		; Interrupt Software Priority Register 3
I0_12		bit	ISPR3,0
I1_12		bit	ISPR3,1
I0_13		bit	ISPR3,2
I1_13		bit	ISPR3,3
EICR		label	$0028		; External Interrupt Control Register
IS1		bfield	EICR,6,2	;  ei2 and ei3 sensitivity
IPB		bit	EICR,5		;  Interrupt polarity for port B
IS2		bfield	EICR,3,2	;  ei0 and ei1 sensitivity
IPA		bit	EICR,2		;  Interrupt polarity for port A

;----------------------------------------------------------------------------
; Flash

FCSR		label	$0029		; Flash Control/Status Register

;----------------------------------------------------------------------------
; Watchdog

WDGCR		label	$002a		; Watchdog Control Register
WDGA		bit	WDGCR,7		; Activation bit

;----------------------------------------------------------------------------
; System Integrity

SICSR		label	$002b		; System Integrity Control/Status Register
AVDIE		bit	SICSR,6		;  Voltage Detector interrupt enable
AVDF		bit	SICSR,5		;  Voltage Detector flag
LVDRF		bit	SICSR,4		;  LVD reset flag
LOCKED		bit	SICSR,3		;  PLL Locked Flag
WDGRF		bit	SICSR,0		;  Watchdog reset flag

;----------------------------------------------------------------------------
; MCC

MCCSR		label	$002c		; Main Clock Control / Status Register
MCO		bit	MCCSR,7		;  Main clock out selection
CP		bfield	MCCSR,5,2	;  CPU clock prescaler
SMS		bit	MCCSR,4		;  Slow mode select
TB		bfield	MCCSR,2,2	;  Time base control
OIE		bit	MCCSR,1		;  Oscillator interrupt enable
OIF		bit	MCCSR,0		;  Oscillator interrupt flag
MCCBCR		label	$002d		; Main Clock Controller: Beep Control Register
BC		bfield	MCCBCR,0,2	;  Beep control

;----------------------------------------------------------------------------
; AWU

AWUCSR		label	$002e		; AWU Control/Status Register
AWUF		bit	AWUCSR,2	;  Auto-wakeup flag
AWUM		bit	AWUCSR,1	;  Auto-wakeup measurement
AWUEN		bit	AWUCSR,0	;  Auto-wakeup from Halt Enabled
AWUPR		label	$002f		; AWU Prescaler Register

;----------------------------------------------------------------------------
; Windowed Watchdog

WDGWR		label	$0030		; Window Watchdog Control Register

;----------------------------------------------------------------------------
; Timer A/B

		include	"timer.inc"		
		__deftimer "TA",$0030
		__deftimer "TB",$0040

;----------------------------------------------------------------------------
; Serial Communications Interface

; SCIERPR is on $0056 instead of $0055 on all other types - error in data sheet?

		include	"sci2.inc"
		__defsci2 $0050,6

;----------------------------------------------------------------------------
; I2C

		include	"i2c.inc"
		__defi2c $0058

;----------------------------------------------------------------------------
; Analog/Digital Converter

		include	"adc10.inc"
		__defadc10 $0070

;----------------------------------------------------------------------------
; PWM ART

		include	"pwm_art.inc"
		__defpwmart $0073

		restore
		endif			; __reg72344inc
