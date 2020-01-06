		ifndef	__reg6253inc	; avoid multiple inclusion
__reg6253inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG6253.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST6253/6260/6263                  *
;*                                                                          *
;*   Source: ST62T53C/T60C/T63C, ST62E60C Data Sheet, Rev. 2.8, July 2001   *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

		
RAMSTART	sfr	000h		; Start Address Internal RAM
		; area 00h..3fh maps to one RAM bank
		; area 40h..7fh is ROM read window
RAMEND		sfr	0bfh		; End Address Internal RAM

		switch	MOMCPUNAME
		case	"ST6253"
		case	"ST6260"
EESTART		 sfr	0000h		; Start Address EEPROM (two banks shared with RAM)
EEEND		 sfr	003fh		; End     "        "
		case	"ST6263"
EESTART		 sfr	0000h		; Start Address EEPROM (one bank shared with RAM)
EEEND		 sfr	003fh		; End     "        "
		elsecase
		 fatal	"Huh?"
		endcase

		segment	code

ROMSTART	label	0880h		; Start Address Internal ROM
ROMEND		label	0fffh		; End     "        "     ROM

;----------------------------------------------------------------------------
; Interrupt Vectors

ADC_vect	label	0ff0h		; A/D End Of Conversion, shared with...
TIMER_vect	label	0ff0h		; Timer Underflow
ARTIMER_vect	label	0ff2h		; AR Timer Overflow/Compare
PORTC_vect	label	0ff4h		; Ext. Interrupt Port C, shared with...
SPI_vect	label	0ff4h		; SPI Interrupt
PORTA_vect	label	0ff6h		; Ext. Interrupt Port A, shared with...
PORTB_vect	label	0ff6h		; Ext. Interrupt Port B
NMI_vect	label	0ffch		; Non Maskable Interrupt
RESET_vect	label	0ffeh		; RESET

;----------------------------------------------------------------------------
; GPIO

		include	"gpio.inc"
		__defgpio "A",0c0h
		__defgpio "B",0c1h
		__defgpio "C",0c2h

;----------------------------------------------------------------------------
; CPU

		include	"ior.inc"
		
DRBR		sfr	0e8h		; Data RAM Bank Register
DRBR4		bit	4,DRBR		;  Map RAM Page 2
		ifdef	EESTART
		if	MOMCPUNAME="ST6260"
DRBR1		bit	4,DRBR		;  Map EEPROM Page 1
		endif
DRBR0		bit	0,DRBR		;  Map EEPROM Page 0
		endif

		ifdef	EESTART
EECTL		sfr	0eah		; EEPROM Control Register
E2OFF		bit	6,EECTL		;  Stand-by Enable Bit
E2PAR1		bit	3,EECTL		;  Parallel Start Bit
E2PAR2		bit	2,EECTL		;  Parallel Mode En
E2BUSY		bit	1,EECTL		;  EEPROM Busy Bit
E2ENA		bit	0,EECTL		;  EEPROM Enable Bit
		endif

OSCCTL		sfr	0dch		; Oscillator Control
RS		bfield	OSCCTL,0,2	;  Division Ratio

;----------------------------------------------------------------------------
; Watchdog

		include	"wdg.inc"
DWDR		sfr	WDGR		; alternate name in older data sheets

;----------------------------------------------------------------------------
; Analog/Digital Converter

		include	"adc.inc"
OSCOFF		bit	2,ADCR		; Disable Main Oscillator

;----------------------------------------------------------------------------
; Timer

		include	"timer.inc"
		__deftimer 0d2h,""

;----------------------------------------------------------------------------
; AR Timer

		include	"artimer.inc"
		__defartimer 0d0h

;----------------------------------------------------------------------------
; SPI

		include	"spi2.inc"

		restore
		endif			; __reg6253inc
