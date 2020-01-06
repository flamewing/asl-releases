		ifndef	__reg6215inc	; avoid multiple inclusion
__reg6215inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG6215.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST6215/25                         *
;*                                                                          *
;*   Source: ST6215/25C Data Sheet, Rev. 2.8, August 1999                   *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

RAMSTART	sfr	0080h		; Start Address Internal RAM
RAMEND		sfr	00bfh		; End Address Internal RAM

		switch  MOMCPUNAME
		case	"ST6215"
ROMSTART	label	0880h		; End     "        "     ROM
		case	"ST6225"
ROMSTART	label	0080h		; End     "        "     ROM
		elsecase
		fatal	"Huh?"
		endcase

;----------------------------------------------------------------------------
; Interrupt Vectors

ADC_vect	label	0ff0h		; End Of Conversion
TIMER_vect	label	0ff2h		; Timer Underflow
PORTC_vect	label	0ff4h		; Ext. Interrupt Port C, shared with...
PORTB_vect	label	0ff4h		; Ext. Interrupt Port B
PORTA_vect	label	0ff6h		; Ext. Interrupt Port A
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

;----------------------------------------------------------------------------
; Watchdog

		include	"wdg.inc"
DWDR		sfr	WDGR		; alternate name in older data sheets

;----------------------------------------------------------------------------
; Analog/Digital Converter

                include	"adc.inc"
OSCOFF		bit	2,ADCR		;  Main Oscillator Off

;----------------------------------------------------------------------------
; Timer 1

		include	"timer.inc"
		__deftimer 0d2h,""
TOUT		bit	5,TSCR		;  Timer Output Control
DOUT		bit	4,TSCR		;  Data Output

		restore
		endif			; __reg6215inc
