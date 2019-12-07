		ifndef	__reg6235inc	; avoid multiple inclusion
__reg6235inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG6235.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST6235                            *
;*                                                                          *
;*           ST62T35B/E35B Data Sheet, Rev. 2.3, March 1998                 *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

RAMSTART	sfr	0000h		; Start Address Internal RAM
		; area 00h..3fh maps to two banks
		; area 40h..7fh is ROM read window
RAMEND		sfr	00bfh		; End Address Internal RAM

EESTART		sfr	0000h		; Start Address EEPROM (2 banks shared with RAM)
EEEND		sfr	003fh		; End     "        "

ROMSTART	label	0080h		; Start Address Internal ROM
ROMEND		label	1fffh		; End     "        "     ROM

;----------------------------------------------------------------------------
; Interrupt Vectors

ADC_vect	label	0ff0h		; A/D End Of Conversion, shared with...
TIMER_vect	label	0ff0h		; Timer Underflow, shared with...
UART_vect	label	0ff0h		; UART Tx/Rx Interrupt
ARTIMER_vect	label	0ff2h		; AR Timer Overflow/Capture
PORTD_vect	label	0ff4h		; Ext. Interrupt Port D, shared with...
PORTB_vect	label	0ff4h		; Ext. Interrupt Port B
SPI_vect	label	0ff6h		; SPI Interrupt, shared with...
PORTA_vect	label	0ff6h		; Ext. Interrupt Port A, shared with...
PORTE_vect	label	0ff6h		; Ext. Interrupt Port E
PORTC_vect	label	0ffch		; Ext. Interrupt Port C, shared with...
NMI_vect	label	0ffch		; Non Maskable Interrupt
RESET_vect	label	0ffeh		; RESET

;----------------------------------------------------------------------------
; GPIO

		include	"gpio.inc"
		__defgpio "A",0c0h
		__defgpio "B",0c1h
		__defgpio "C",0c2h
		__defgpio "D",0c3h
DRE		sfr	0fch		; Port E Data Register
DDRE		sfr	0fdh		; Port E Data Direction Register
ORE		sfr	0feh		; Port E Option Register

;----------------------------------------------------------------------------
; CPU

		include	"ior.inc"
		
DRBR		sfr	0cbh		; Data RAM Bank Register
DRBR4		bit	4,DRBR		;  Map RAM Page 2
DRBR3		bit	3,DRBR		;  Map RAM Page 1
DRBR1		bit	1,DRBR		;  Map EEPROM Page 1
DRBR0		bit	0,DRBR		;  Map EEPROM Page 0

IPR		sfr	0dah		; Interrupt Polarity Register
PortE		bit	4,IPR		;  Port E Interrupt Polarity
PortD		bit	3,IPR		;  Port D Interrupt Polarity
PortC		bit	2,IPR		;  Port C Interrupt Polarity
PortA		bit	1,IPR		;  Port A Interrupt Polarity
PortB		bit	0,IPR		;  Port B Interrupt Polarity

EECTL		sfr	0dfh		; EEPROM Control Register
E2OFF		bit	6,EECTL		;  Stand-by Enable Bit
E2PAR1		bit	3,EECTL		;  Parallel Start Bit
E2PAR2		bit	2,EECTL		;  Parallel Mode En
E2BUSY		bit	1,EECTL		;  EEPROM Busy Bit
E2ENA		bit	0,EECTL		;  EEPROM Enable Bit

;----------------------------------------------------------------------------
; Clock System

OSCR		sfr	0dbh
OSCOFF		bit	0,OSCR		; Main Oscillator Turn-Off

;----------------------------------------------------------------------------
; Watchdog

		include	"wdg.inc"
DWDR		sfr	WDGR		; alternate name in older data sheets

;----------------------------------------------------------------------------
; Analog/Digital Converter

		include	"adc.inc"
CLSEL		bit	2,ADCR		; Clock Selection

;----------------------------------------------------------------------------
; Timer 1

		include	"timer.inc"
		__deftimer 0d2h,""
TOUT		bit	5,TSCR		;  Timer Output Control
DOUT		bit	4,TSCR		;  Data Output

;----------------------------------------------------------------------------
; Auto Reload Timer

		include "artim16.inc"

;----------------------------------------------------------------------------
; UART

		include	"uart.inc"
DAT9		bit	0,UARTCR	;  Alias for Parity/Data Bit 8

;----------------------------------------------------------------------------
; SPI

		include	"spi.inc"

		restore
		endif			; __reg6235inc
