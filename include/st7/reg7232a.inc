		ifndef	__reg7232inc	; avoid multiple inclusion
__reg7232inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG7232.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST7232                            *
;*                                                                          *
;*   Source: ST7232 Data Sheet, Rev. 2, Decemberr 2005                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

RAMSTART	label	$0080		; start address internal RAM
RAMEND		label	$047f		; end     "        "      "

;----------------------------------------------------------------------------
; Interrupt Vectors

SCI_vect	label	$ffe6		; SCI Interrupt Vector
TIMB_vect	label	$ffe8		; TIMER B Interrupt Vector
TIMA_vect	label	$ffea		; TIMER A Interrupt Vector
SPI_vect	label	$ffec		; SPI Interrupt Vector
EI3_vect	label	$fff0		; External Interrupt Vector EI3
EI2_vect	label	$fff2		; External Interrupt Vector EI2
EI1_vect	label	$fff4		; External Interrupt Vector EI1
EI0_vect	label	$fff6		; External Interrupt Vector EI0
MCC_RTC_vect	label	$fff8		; MCC/RTC
TRAP_vect	label	$fffc		; TRAP (software) Interrupt Vector
RESET_vect	label	$fffe		; RESET Vector

;----------------------------------------------------------------------------
; GPIO

		include "gpio.inc"
		__defgpio "PA",$0000
		__defgpio "PB",$0003
		__defgpio "PC",$0006
		__defgpio "PD",$0009
		__defgpio "PE",$000c
		__defgpio "PF",$000f

;----------------------------------------------------------------------------
; SPI

		include	"spi2.inc"
		__defspi $0021

;----------------------------------------------------------------------------
; ITC

ISPR0		label	$0024		; Interrupt Software Priority Register 0
I0_1		bit	ISPR0,2		;  MCC+SI
I1_1		bit	ISPR0,3
I0_2		bit	ISPR0,4		;  EI0
I1_2		bit	ISPR0,5
I0_3		bit	ISPR0,6		;  EI1
I1_3		bit	ISPR0,7
ISPR1		label	$0025		; Interrupt Software Priority Register 1
I0_4		bit	ISPR1,0		;  EI2
I1_4		bit	ISPR1,1
I0_5		bit	ISPR1,2		;  EI3
I1_5		bit	ISPR1,3
I0_6		bit	ISPR1,4
I1_6		bit	ISPR1,5
I0_7		bit	ISPR1,6		;  SPI
I1_7		bit	ISPR1,7
ISPR2		label	$0026		; Interrupt Software Priority Register 2
I0_8		bit	ISPR2,0		;  Timer A
I1_8		bit	ISPR2,1
I0_9		bit	ISPR2,2		;  Timer B
I1_9		bit	ISPR2,3
I0_10		bit	ISPR2,4		;  SCI
I1_10		bit	ISPR2,5
I0_11		bit	ISPR2,6		;  AVD
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

WDGCR		label	$002a		; Watchdog Control register
WDGA		bit	WDGCR,7		; Activation bit

;----------------------------------------------------------------------------
; System Integrity

SICSR		label	$002b		; System Integrity Control/Status Register
WDGRF		bit	SICSR,0		; Watchdog Reset Flag

;----------------------------------------------------------------------------
; Timer A/B

		include	"timer.inc"
		__deftimer "TA",$0030
		__deftimer "TB",$0040

;----------------------------------------------------------------------------
; Serial Communications Interface

		include	"sci2.inc"
		__defsci2 $0050,5

;----------------------------------------------------------------------------
; Analog/Digital Converter

		include	"adc10.inc"
		__defadc10 $0070

		restore
		endif			; __reg7232inc
