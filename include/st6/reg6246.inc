		ifndef	__reg6246inc	; avoid multiple inclusion
__reg6246inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG6246.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST6246                            *
;*                                                                          *
;*           ST62T46B/E46B Data Sheet, Rev. 2.5, September 1998             *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

		
RAMSTART	sfr	000h		; Start Address Internal RAM
		; area 00h..3fh maps to one bank
		; area 40h..7fh is ROM read window
RAMEND		sfr	0bfh		; End Address Internal RAM

		segment	data
		org	0e0h
LCDRAMSTART	block	24		; Start Address LCD RAM
LCDRAMEND	sfr	PC-1		; End     "        "

DATARAMSTART	block	7		; Start Address Data RAM (unused LCD RAM?)
DATARAMEND	sfr	PC-1		; End     "        "

EESTART		sfr	0000h		; Start Address EEPROM (2 banks shared with RAM)
EEEND		sfr	003fh		; End     "        "

		segment	code

ROMSTART	label	0080h		; Start Address Internal ROM
ROMEND		label	0fffh		; End     "        "     ROM

;----------------------------------------------------------------------------
; Interrupt Vectors

ADC_vect	label	0ff0h		; A/D End Of Conversion
TIMER1_vect	label	0ff2h		; Timer 1 Underflow, shared with...
TIMER2_vect	label	0ff2h		; Timer 2 Underflow, shared with...
OSC_vect	label	0ff2h		; 32kHz OSC Interrupt
PORTA_vect	label	0ff4h		; Ext. Interrupt Port A, shared with...
PORTB_vect	label	0ff4h		; Ext. Interrupt Port B, shared with...
PORTC_vect	label	0ff4h		; Ext. Interrupt Port C
SPI_vect	label	0ff6h		; SPI Interrupt
PSS_vect	label	0ffch		; PSS Interrupt, shared with...
NMI_vect	label	0ffch		; Non Maskable Interrupt
RESET_vect	label	0ffeh		; RESET

;----------------------------------------------------------------------------
; GPIO (irregular layout for Port C)
; NOTE: register overview lists ORB @ 0ceh, but later description says 0cdh,
; which makes more sense:

		include	"gpio.inc"
		__defgpio "A",0c0h
		__defgpio "B",0c1h
DRC		sfr	0c3h		; Port C Data Register
DDRC		sfr	0c6h		; Port C Data Direction Register
ORC		sfr	0cfh		; Port C Option Register

;----------------------------------------------------------------------------
; CPU

		include	"ior.inc"
		
DRBR		sfr	0cbh		; Data RAM Bank Register
DRBR4		bit	4,DRBR		;  Map RAM Page 2
DRBR1		bit	1,DRBR		;  Map EEPROM Page 1
DRBR0		bit	0,DRBR		;  Map EEPROM Page 0

EECTL		sfr	0dfh		; EEPROM Control Register
E2OFF		bit	6,EECTL		;  Stand-by Enable Bit
E2PAR1		bit	3,EECTL		;  Parallel Start Bit
E2PAR2		bit	2,EECTL		;  Parallel Mode En
E2BUSY		bit	1,EECTL		;  EEPROM Busy Bit
E2ENA		bit	0,EECTL		;  EEPROM Enable Bit

;----------------------------------------------------------------------------
; Power Supply Supervisor Device

PSSCR		sfr	0dah		; PSS Status Control Register
PIF		bit	7,PSSCR		;  Interrupt Flag Bit
PEI		bit	6,PSSCR		;  Interrupt Mask Bit
PDV		bfield	PSSCR,4,2	;  Division Rate Selection
PDR		bfield	PSSCR,1,3	;  Division Rate Selection Bit
D0		bit	0,PSSCR		;  The PSS comparator Output

;----------------------------------------------------------------------------
; Clock System

OCR32		sfr	0dbh		; 32kHz Oscillator Register
EOSCI		bit	7,OCR32		;  Enable Oscillator Interrupt
OSCEOC		bit	6,OCR32		;  Oscillator Interrupt Flag
S_S		bit	5,OCR32		;  Oscillator Start/Stop Bit

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
		__deftimer 0d2h,"1"
		__deftimer 0d5h,"2"

;----------------------------------------------------------------------------
; SPI

SDSR		sfr	0ddh		; SPI Data/Shift Register
SIDR		sfr	0c2h		; SPI Interrupt Disable Register

;----------------------------------------------------------------------------
; LCD Controller

		include "lcd.inc"

		restore
		endif			; __reg6246inc
