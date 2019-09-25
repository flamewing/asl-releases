		ifndef	__reg72251inc	; avoid multiple inclusion
__reg72251inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG72251.INC                                            *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST72251                           *
;*                                                                          *
;*   Source: ST72251 Data Sheet, Rev. 1.7, September 1999                   *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

RAMSTART	label	$0080		; start address internal RAM
RAMEND		label	$017f		; end     "        "      "

;----------------------------------------------------------------------------
; Interrupt Vectors

I2C_vect	label	$ffe4		; I2C Interrupt Vector
TIMB_vect	label	$ffee		; TIMER B Interrupt Vector
TIMA_vect	label	$fff2		; TIMER A Interrupt Vector
SPI_vect	label	$fff4		; SPI Interrupt Vector
EI1_vect	label	$fff8		; External Interrupt Vector EI1
EI0_vect	label	$fffa		; External Interrupt Vector EI0
TRAP_vect	label	$fffc		; TRAP (software) Interrupt Vector
RESET_vect	label	$fffe		; RESET Vector

;----------------------------------------------------------------------------
; GPIO

		include "gpio.inc"
		__defgpio "PC",$0000
		__defgpio "PB",$0004
		__defgpio "PA",$0008

;----------------------------------------------------------------------------
; Miscellaneous

MISCR		label	$0020		; Miscellaneous Register
PEI3		bit	MISCR,7		; EI1 Interrupt Polarity
PEI2		bit	MISCR,6
MCO		bit	MISCR,5		; Main Clock Out
PEI1		bit	MISCR,4		; EI0 Interrupt Polarity
PEI0		bit	MISCR,3
SMS		bit	MISCR,0		; Slow Mode Select

;----------------------------------------------------------------------------
; SPI

		include	"spi.inc"

;----------------------------------------------------------------------------
; Watchdog

WDGCR		label	$0024		; Watchdog Control register
WDGA		bit	WDGCR,7		; Activation bit

;----------------------------------------------------------------------------
; I2C

		include	"i2c.inc"
		__defi2c $0028

;----------------------------------------------------------------------------
; Timer A/B

		include	"timer.inc"
		__deftimer "TA",$0030
		__deftimer "TB",$0040

;----------------------------------------------------------------------------
; Analog/Digital Converter

		include	"adc8.inc"

		restore
		endif			; __reg72251inc
