		ifndef	__reg6255inc	; avoid multiple inclusion
__reg6255inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG6255.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST6255/6265                       *
;*                                                                          *
;*   Source: ST6255C ST6265C ST6265B Data Sheet, Rev. 3, March 2009         *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

		
RAMSTART	sfr	000h		; Start Address Internal RAM
		; area 00h..3fh maps to one RAM bank
		; area 40h..7fh is ROM read window
RAMEND		sfr	0bfh		; End Address Internal RAM

		switch	MOMCPUNAME
		case	"ST6255"
		case	"ST6265"
EESTART		 sfr	0000h		; Start Address EEPROM (tw banks shared with RAM)
EEEND		 sfr	003fh		; End     "        "
		elsecase
		 fatal	"Huh?"
		endcase

		segment	code

ROMSTART	label	0080h		; Start Address Internal ROM
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
DRBR1		bit	1,DRBR		;  Map EEPROM Page 1
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
TOUT		bit	5,TSCR		;  Timer Output Control
DOUT		bit	4,TSCR		;  Data Output

;----------------------------------------------------------------------------
; AR Timer

		include	"artimer.inc"
		__defartimer 0d0h

;----------------------------------------------------------------------------
; SPI

		include	"spi2.inc"

		restore
		endif			; __reg6255inc
