		ifndef	__reg6218inc	; avoid multiple inclusion
__reg6218inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG6218.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST6218                            *
;*                                                                          *
;*   Source: ST62T18C/E18C Data Sheet, Rev. 2.6, July 2001                  *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

RAMSTART	sfr	0000h		; Start Address Internal RAM
		; area 00h..3fh maps to two banks
		; area 40h..7fh is ROM read window
RAMEND		sfr	00bfh		; End Address Internal RAM

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
PORTA_vect	label	0ff6h		; Ext. Interrupt Port A
NMI_vect	label	0ffch		; Non Maskable Interrupt
RESET_vect	label	0ffeh		; RESET

;----------------------------------------------------------------------------
; GPIO

		include	"gpio.inc"
		__defgpio "A",0c0h
		__defgpio "B",0c1h
		__defgpio "D",0c3h

;----------------------------------------------------------------------------
; CPU

		include	"ior.inc"
		
DRBR		sfr	0cbh		; Data RAM Bank Register
DRBR4		bit	4,DRBR		;  Map RAM Page 2
DRBR3		bit	3,DRBR		;  Map RAM Page 1

IPR		sfr	0dah		; Interrupt Polarity Register
PortD		bit	3,IPR		;  Port D Interrupt Polarity
PortA		bit	1,IPR		;  Port A Interrupt Polarity
PortB		bit	0,IPR		;  Port B Interrupt Polarity

;----------------------------------------------------------------------------
; Watchdog

		include	"wdg.inc"
DWDR		sfr	WDGR		; alternate name in older data sheets

;----------------------------------------------------------------------------
; Analog/Digital Converter

		include	"adc.inc"
OSCOFF		bit	2,ADCR		;  Main Oscillator Off
CLSEL		bit	3,ADCR		; Clock Selection

;----------------------------------------------------------------------------
; Timer 1

		include	"timer.inc"
		__deftimer 0d2h,""
TOUT		bit	5,TSCR		;  Timer Output Control
DOUT		bit	4,TSCR		;  Data Output

;----------------------------------------------------------------------------
; Auto Reload Timer

		include "artimer.inc"
		__defartimer 0e0h

;----------------------------------------------------------------------------
; UART

		include	"uart.inc"

		restore
		endif			; __reg6218inc
