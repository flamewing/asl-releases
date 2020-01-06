		ifndef	__reg72311inc	; avoid multiple inclusion
__reg72311inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG72311.INC                                            *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST72311                           *
;*                                                                          *
;*   Source: ST72311 Data Sheet, Rev. 1.8, May 2001                         *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

RAMSTART	label	$0080		; start address internal RAM
		switch	substr(MOMCPUNAME,8,1)
		case	"2"
RAMEND		label	$01ff		; end     "        "      "
		case	"4"
RAMEND		label	$027f		; end     "        "      "
		endcase
		
EESTART		label	$0c00		; start address internal EEPROM
EEEND		label	$0cff		; end     "        "      "

;----------------------------------------------------------------------------
; Interrupt Vectors

EEPROM_vect	label	$ffe4		; EEPROM Interrupt Vector
SCI_vect	label	$ffe6		; SCI Interrupt Vector
TIMB_vect	label	$ffe8		; TIMER B Interrupt Vector
TIMA_vect	label	$ffea		; TIMER A Interrupt Vector
SPI_vect	label	$ffec		; SPI Interrupt Vector
EI3_vect	label	$fff0		; External Interrupt Vector EI3
EI2_vect	label	$fff2		; External Interrupt Vector EI2
EI1_vect	label	$fff4		; External Interrupt Vector EI1
EI0_vect	label	$fff6		; External Interrupt Vector EI0
TRAP_vect	label	$fffc		; TRAP (software) Interrupt Vector
RESET_vect	label	$fffe		; RESET Vector

;----------------------------------------------------------------------------
; GPIO

		include	"gpio.inc"
		__defgpio "PA",$0000
		__defgpio "PC",$0004
		__defgpio "PB",$0008
		__defgpio "PE",$000c
		__defgpio "PD",$0010
		__defgpio "PF",$0014

;----------------------------------------------------------------------------
; Miscellaneous

MISCR		label	$0020		; Miscellaneous Register
PEI3		bit	MISCR,7		; EI3 Interrupt Polarity
PEI2		bit	MISCR,6		; EI2 Interrupt Polarity
MCO		bit	MISCR,5		; Main Clock Out
PEI1		bit	MISCR,4		; EI1 Interrupt Polarity
PEI0		bit	MISCR,3		; EI0 Interrupt Polarity
PSM		bfield	MISCR,1,2	; Prescaler for Slow Mode
SMS		bit	MISCR,0		; Slow Mode Select

;----------------------------------------------------------------------------
; SPI

		include	"spi.inc"

;----------------------------------------------------------------------------
; Watchdog

WDGCR		label	$002a		; Watchdog Control Register
WDGA		bit	WDGCR,7		; Activation bit
WDGSR		label	$002b		; Watchdog Status Register
WDOGF		bit	WDGSR,0		; Watchdog flag

;----------------------------------------------------------------------------
; EEPROM

EEPCR		label	$002c		; EEPROM Control Register
E2ITE		bit	EEPCR,2		;  Interrupt enable
E2LAT		bit	EEPCR,1		;  Read/Write mode
E2PGM		bit	EEPCR,0		;  Programming Control

;----------------------------------------------------------------------------
; Timer A/B

		include	"timer.inc"		
		__deftimer "TA",$0030
		__deftimer "TB",$0040

;----------------------------------------------------------------------------
; Serial Communications Interface

		include	"sci.inc"
		__defsci $0050,5

;----------------------------------------------------------------------------
; Analog/Digital Converter

		include	"adc8.inc"

		restore
		endif			; __reg72311inc
