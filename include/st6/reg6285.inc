		ifndef	__reg6285inc	; avoid multiple inclusion
__reg6285inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG6285.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST6285                            *
;*                                                                          *
;*           ST62T85B/E85B Data Sheet, Rev. 2.5, August 1999                *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

		
RAMSTART	sfr	000h		; Start Address Internal RAM
		; area 00h..3fh maps to two banks
		; area 40h..7fh is ROM read window
RAMEND		sfr	0bfh		; End Address Internal RAM

EESTART		sfr	0000h		; Start Address EEPROM (two banks shared with RAM)
EEEND		sfr	003fh		; End     "        "

		segment	code

ROMSTART	label	0080h		; Start Address Internal ROM
ROMEND		label	1fffh		; End     "        "     ROM

;----------------------------------------------------------------------------
; Interrupt Vectors

ADC_vect	label	0ff0h		; A/D End Of Conversion, shared with...
UART_vect	label	0ff0h		; UART Interrupt
TIMER1_vect	label	0ff2h		; Timer 1 Underflow, shared with...
ARTIMER_vect	label	0ff2h		; AR Timer Overrflow/Compare, shared with...
PORTA_vect	label	0ff4h		; Ext. Interrupt Port A, shared with...
PORTB_vect	label	0ff4h		; Ext. Interrupt Port B, shared with...
PORTC_vect	label	0ff4h		; Ext. Interrupt Port C
SPI_vect	label	0ff6h		; SPI Interrupt
NMI_vect	label	0ffch		; Non Maskable Interrupt
RESET_vect	label	0ffeh		; RESET

;----------------------------------------------------------------------------
; GPIO (irregular layout)
; NOTE: register overview lists ORB @ 0ceh, but later description says 0cdh,
; which makes more sense:

		include	"gpio.inc"
		__defgpio "A",0c0h
DRB		sfr	0c1h		; Port B Data Register
DDRB		sfr	0c5h		; Port B Data Direction Register
ORB		sfr	0ceh		; Port B Option Register
DRC		sfr	0c3h		; Port C Data Register
DDRC		sfr	0c6h		; Port C Data Direction Register
ORC		sfr	0cfh		; Port C Option Register

;----------------------------------------------------------------------------
; CPU

		include	"ior.inc"
		
DRBR		sfr	0cbh		; Data RAM Bank Register
DRBR6		bit	6,DRBR		;  Map LCD RAM Page 2
DRBR5		bit	5,DRBR		;  Map LCD RAM Page 1
DRBR4		bit	4,DRBR		;  Map RAM Page 2
DRBR3		bit	3,DRBR		;  Map RAM Page 1
DRBR1		bit	1,DRBR		;  Map EEPROM Page 1
DRBR0		bit	0,DRBR		;  Map EEPROM Page 0

EECTL		sfr	0dfh		; EEPROM Control Register
E2OFF		bit	6,EECTL		;  Stand-by Enable Bit
E2PAR1		bit	3,EECTL		;  Parallel Start Bit
E2PAR2		bit	2,EECTL		;  Parallel Mode En
E2BUSY		bit	1,EECTL		;  EEPROM Busy Bit
E2ENA		bit	0,EECTL		;  EEPROM Enable Bit

;----------------------------------------------------------------------------
; Watchdog

		include	"wdg.inc"
DWDR		sfr	WDGR		; alternate name in older data sheets

;----------------------------------------------------------------------------
; Analog/Digital Converter

		include	"adc.inc"

;----------------------------------------------------------------------------
; Timer

		include	"timer.inc"
		__deftimer 0d2h,""
TOUT1		bit	5,TSCR		;  Timer Output Control
DOUT1		bit	4,TSCR		;  Data Output

		include	"artimbas.inc"
		__defartimbas 0e0h

;----------------------------------------------------------------------------
; UART

		include	"uart.inc"

;----------------------------------------------------------------------------
; SPI

SDSR		sfr	0ddh		; SPI Data/Shift Register
SIDR		sfr	0c2h		; SPI Interrupt Disable Register

;----------------------------------------------------------------------------
; LCD Controller

LCDCR		sfr	0dch		; LCD Mode Control Register
MUX16		bit	7,LCDCR		; Multiplexing Ratio Select
MUX11		bit	6,LCDCR
HF		bfield	LCDCR,3,3	;  Oscillator Select
LF		bfield	LCDCR,0,2	;  Base frame Frequency Select

		restore
		endif			; __reg6285inc
